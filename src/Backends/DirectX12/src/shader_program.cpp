#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>
#include <d3d12shader.h>
#include <numeric>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

consteval UInt32 FOUR_CC(char ch0, char ch1, char ch2, char ch3) 
{
    return static_cast<UInt32>((Byte)ch0) | static_cast<UInt32>((Byte)ch1) << 8 | static_cast<UInt32>((Byte)ch2) << 16 | static_cast<UInt32>((Byte)ch3) << 24; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

static bool SUPPRESS_MISSING_ROOT_SIGNATURE_WARNING = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

constexpr BorderMode DECODE_BORDER_MODE(D3D12_TEXTURE_ADDRESS_MODE addressMode) noexcept
{
    switch (addressMode)
    {
    default:
    case D3D12_TEXTURE_ADDRESS_MODE_WRAP:           return BorderMode::Repeat;
    case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:          return BorderMode::ClampToEdge;
    case D3D12_TEXTURE_ADDRESS_MODE_BORDER:         return BorderMode::ClampToBorder;
    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:         return BorderMode::RepeatMirrored;
    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:    return BorderMode::ClampToEdgeMirrored;
    }
}

class DirectX12ShaderProgram::DirectX12ShaderProgramImpl {
public:
    friend class DirectX12ShaderProgramBuilder;
    friend class DirectX12ShaderProgram;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;
    WeakPtr<const DirectX12Device> m_device;

private:
    struct DescriptorInfo {
    public:
        UInt32 location;
        UInt32 elementSize;
        UInt32 elements;
        DescriptorType type;
        Optional<D3D12_STATIC_SAMPLER_DESC> staticSamplerState;
        bool local = false;

        bool equals(const DescriptorInfo& rhs)
        {
            return
                this->location == rhs.location &&
                this->elements == rhs.elements &&
                this->elementSize == rhs.elementSize &&
                this->type == rhs.type;
        }
    };

    struct DescriptorSetInfo {
    public:
        UInt32 space{ 0 };
        ShaderStage stage{ ShaderStage::Other };
        Array<DescriptorInfo> descriptors{};
    };

    struct PushConstantRangeInfo {
    public:
        ShaderStage stage{ ShaderStage::Other };
        UInt32 offset{ 0 };
        UInt32 size{ 0 };
        UInt32 location{ 0 };
        UInt32 space{ 0 };
    };

public:
    DirectX12ShaderProgramImpl(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules) :
        m_device(device.weak_from_this())
    {
        m_modules = std::move(modules) | std::views::as_rvalue | std::ranges::to<std::vector>();
    }

    DirectX12ShaderProgramImpl(const DirectX12Device& device) :
        m_device(device.weak_from_this())
    {
    }

public:
    void validate()
    {
        // First check if there are any modules at all, or any that are uninitialized.
        if (m_modules.empty()) [[unlikely]]
            return; // Not exactly a reason to throw, but rather an empty group cannot be meaningful used anyway.

        if (std::ranges::contains(m_modules, nullptr)) [[unlikely]]
            throw InvalidArgumentException("modules", "At least one of the shader modules is not initialized.");

        // Check if there are combinations, that are not supported.
        Dictionary<ShaderStage, UInt32> shaders = {
            { ShaderStage::Compute, 0 },
            { ShaderStage::Vertex, 0 },
            { ShaderStage::Geometry, 0 },
            { ShaderStage::TessellationControl, 0 },
            { ShaderStage::TessellationEvaluation, 0 },
            { ShaderStage::Fragment, 0 },
            { ShaderStage::Task, 0 },
            { ShaderStage::Mesh, 0 },
            { ShaderStage::RayGeneration, 0 },
            { ShaderStage::Miss, 0 },
            { ShaderStage::Callable, 0 },
            { ShaderStage::AnyHit, 0 },
            { ShaderStage::ClosestHit, 0 },
            { ShaderStage::Intersection, 0 }
        };

        std::ranges::for_each(m_modules, [&shaders](auto& module) { shaders[module->type()]++; });

        bool containsComputeGroup    = shaders[ShaderStage::Compute] > 0;
        bool containsGraphicsGroup   = shaders[ShaderStage::Vertex] > 0 || shaders[ShaderStage::Geometry] > 0 || shaders[ShaderStage::TessellationControl] > 0 || shaders[ShaderStage::TessellationEvaluation] > 0;
        bool containsFragmentGroup   = shaders[ShaderStage::Fragment] > 0;
        bool containsMeshGroup       = shaders[ShaderStage::Task] > 0 || shaders[ShaderStage::Mesh] > 0;
        bool containsRaytracingGroup = shaders[ShaderStage::RayGeneration] > 0 || shaders[ShaderStage::Miss] > 0 || shaders[ShaderStage::Callable] > 0 || shaders[ShaderStage::AnyHit] > 0 || shaders[ShaderStage::ClosestHit] > 0 || shaders[ShaderStage::Intersection] > 0;

        // Compute groups must be compute only.
        if (containsComputeGroup)
        {
            if (containsGraphicsGroup || containsMeshGroup || containsFragmentGroup || containsRaytracingGroup) [[unlikely]]
                throw InvalidArgumentException("modules", "The provided shader modules mix compute shaders with non-compute shaders.");
            if (shaders[ShaderStage::Compute] > 1) [[unlikely]]
                throw InvalidArgumentException("modules", "If a shader program contains a compute shader, it must contain only one shader module.");

            return;
        }

        // No compute shaders from this point - are we on a ray-tracing group?
        if (containsRaytracingGroup)
        {
            if (containsGraphicsGroup || containsMeshGroup || containsFragmentGroup) [[unlikely]]
                throw InvalidArgumentException("modules", "If a shader program contains ray-tracing shaders, it must only contain ray-tracing shaders.");
            if (shaders[ShaderStage::RayGeneration] != 1) [[unlikely]]
                throw InvalidArgumentException("modules", "If ray-tracing shaders are present, there must also be exactly one ray generation shader.");
                
            return;
        }

        // No ray-tracing from this point... next are mesh shaders.
        if (containsMeshGroup)
        {
            if (containsGraphicsGroup) [[unlikely]]
                throw InvalidArgumentException("modules", "Mesh shaders must not be combined with graphics shaders.");
            if (shaders[ShaderStage::Fragment] != 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a mesh shader program, there must be exactly one fragment/pixel shader.");
            if (shaders[ShaderStage::Mesh] != 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a mesh shader program, there must be exactly one mesh shader.");
            if (shaders[ShaderStage::Task] > 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a mesh shader program, there must be at most one mesh shader.");

            return;
        }

        // Now on to the standard graphics shaders.
        if (containsGraphicsGroup)
        {
            if (shaders[ShaderStage::Fragment] != 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a graphics shader program, there must be exactly one fragment/pixel shader.");
            if (shaders[ShaderStage::Vertex] != 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a graphics shader program, there must be exactly one vertex shader.");
            if (shaders[ShaderStage::TessellationControl] > 1 || shaders[ShaderStage::TessellationEvaluation] > 1 || shaders[ShaderStage::Geometry] > 1) [[unlikely]]
                throw InvalidArgumentException("modules", "In a graphics shader program, there must be at most one geometry, tessellation control/domain or tessellation evaluation/hull shader.");

            return;
        }

        // Finally, let's check if there's a lonely fragment shader.
        if (containsFragmentGroup) [[unlikely]]
            throw InvalidArgumentException("modules", "A shader program that contains only a fragment/pixel shader is not valid.");
    }

    void reflectRootSignature(const ComPtr<ID3D12RootSignatureDeserializer>& deserializer, Dictionary<UInt32, DescriptorSetInfo>& descriptorSetLayouts, Array<PushConstantRangeInfo>& pushConstantRanges)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        // Collect the shader stages.
        ShaderStage stages{ };
        std::ranges::for_each(m_modules, [&stages](const auto& shaderModule) { stages = stages | shaderModule->type(); });

        // Get the root signature description.
        auto description = deserializer->GetRootSignatureDesc();

        // Iterate the static samplers.
        for (UInt32 i(0); i < description->NumStaticSamplers; ++i)
        {
            auto staticSampler = description->pStaticSamplers[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            // If the descriptor space 
            if (!descriptorSetLayouts.contains(staticSampler.RegisterSpace)) [[unlikely]]
            {
                LITEFX_DEBUG(DIRECTX12_LOG, "The root signature of the shader defines a descriptor set at space {0}, which the reflection did not find.", staticSampler.RegisterSpace);
                descriptorSetLayouts.insert(std::make_pair(staticSampler.RegisterSpace, DescriptorSetInfo{ .space = staticSampler.RegisterSpace, .stage = stages }));
            }

            // Find the sampler for the register and overwrite it. If it does not exist, create it.
            if (auto match = std::ranges::find_if(descriptorSetLayouts[staticSampler.RegisterSpace].descriptors, [&staticSampler](const auto& descriptor) { return descriptor.type == DescriptorType::Sampler && descriptor.location == staticSampler.ShaderRegister; }); match == descriptorSetLayouts[staticSampler.RegisterSpace].descriptors.end())
                descriptorSetLayouts[staticSampler.RegisterSpace].descriptors.push_back(DescriptorInfo{ .location = staticSampler.ShaderRegister, .elementSize = 0, .elements = 1, .type = DescriptorType::Sampler, .staticSamplerState = staticSampler });
            else
                match->staticSamplerState = staticSampler;
        }

        // Iterate the root parameters.
        UInt32 pushConstantOffset = 0;

        for (UInt32 i(0); i < description->NumParameters; ++i)
        {
            auto rootParameter = description->pParameters[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            // Check if there's a descriptor set for the space.
            if (!descriptorSetLayouts.contains(rootParameter.Descriptor.RegisterSpace)) [[unlikely]]
            {
                // NOTE: This is only ever valid for static samplers, since other descriptors cannot be defined this way (they would be missing array and element sizes). The descriptor set must stay empty.
                LITEFX_DEBUG(DIRECTX12_LOG, "The root signature of the shader defines a descriptor set at space {0}, which the reflection did not find.", rootParameter.Descriptor.RegisterSpace);
                descriptorSetLayouts.insert(std::make_pair(rootParameter.Descriptor.RegisterSpace, DescriptorSetInfo{ .space = rootParameter.Descriptor.RegisterSpace, .stage = stages }));
            }

            switch (rootParameter.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: 
                // Find the descriptor.
                if (auto match = std::ranges::find_if(descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors, [&rootParameter](const auto& descriptor) { return descriptor.location == rootParameter.Descriptor.ShaderRegister; }); match == descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors.end()) [[unlikely]]
                    LITEFX_WARNING(DIRECTX12_LOG, "The root signature defines a descriptor at {0} (space {1}), which the shader reflection did not find. The descriptor will be ignored.", rootParameter.Descriptor.ShaderRegister, rootParameter.Descriptor.RegisterSpace);
                else
                {
                    // Convert the descriptor to a push constant range.
                    ShaderStage stage{ };

                    switch (rootParameter.ShaderVisibility)
                    {
                    case D3D12_SHADER_VISIBILITY_VERTEX:        stage = ShaderStage::Vertex; break;
                    case D3D12_SHADER_VISIBILITY_HULL:          stage = ShaderStage::TessellationControl; break;
                    case D3D12_SHADER_VISIBILITY_DOMAIN:        stage = ShaderStage::TessellationEvaluation; break;
                    case D3D12_SHADER_VISIBILITY_GEOMETRY:      stage = ShaderStage::Geometry; break;
                    case D3D12_SHADER_VISIBILITY_PIXEL:         stage = ShaderStage::Fragment; break;
                    case D3D12_SHADER_VISIBILITY_AMPLIFICATION: stage = ShaderStage::Task; break;
                    case D3D12_SHADER_VISIBILITY_MESH:          stage = ShaderStage::Mesh; break;
                    case D3D12_SHADER_VISIBILITY_ALL:           stage = ShaderStage::Any; break; // TODO: Might not work as intended.
                    default: throw InvalidArgumentException("pushConstantRanges", "The push constants for a shader are defined for invalid or unsupported shader stages. Note that a push constant must only be defined for a single shader stage.");
                    }

                    pushConstantRanges.push_back(PushConstantRangeInfo{ .stage = stage, .offset = pushConstantOffset, .size = rootParameter.Constants.Num32BitValues * 4, .location = rootParameter.Descriptor.ShaderRegister, .space = rootParameter.Descriptor.RegisterSpace });
                    pushConstantOffset += rootParameter.Constants.Num32BitValues * 4;
                    
                    // Remove the descriptor from the descriptor set.
                    descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors.erase(match);

                    // Remove the descriptor set, if it is empty.
                    if (descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors.empty())
                        descriptorSetLayouts.erase(rootParameter.Descriptor.RegisterSpace);
                }

                break;
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                // Check if the descriptor is defined.
                if (auto match = std::ranges::find_if(descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors, [&rootParameter](const auto& descriptor) { return descriptor.location == rootParameter.Descriptor.ShaderRegister; }); match == descriptorSetLayouts[rootParameter.Descriptor.RegisterSpace].descriptors.end()) [[unlikely]]
                    LITEFX_WARNING(DIRECTX12_LOG, "The root signature defines a descriptor at {0} (space {1}), which the shader reflection did not find. The descriptor will be ignored.", rootParameter.Descriptor.ShaderRegister, rootParameter.Descriptor.RegisterSpace);

                break;
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                throw InvalidArgumentException("modules", "The shader modules root signature defines a descriptor table for parameter {0}, which is currently not supported. Convert each parameter of the table into a separate root parameter.", i);
            default: 
                throw InvalidArgumentException("modules", "The shader modules root signature exposes an unknown root parameter type {1} for parameter {0}.", i, rootParameter.ParameterType);
            }
        }

        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    }

    template <typename TReflection>
    DescriptorInfo getReflectionDescriptorDesc(D3D12_SHADER_INPUT_BIND_DESC inputDesc, TReflection* shaderReflection)
    {
        // First, create a description of the descriptor.
        DescriptorType type{ };
        UInt32 elementSize = 0;

        switch (inputDesc.Type)
        {
        case D3D_SIT_CBUFFER:
        {
            D3D12_SHADER_BUFFER_DESC bufferDesc;
            auto constantBuffer = shaderReflection->GetConstantBufferByName(inputDesc.Name);
            raiseIfFailed(constantBuffer->GetDesc(&bufferDesc), "Unable to query constant buffer \"{0}\" at binding point {1} (space {2}).", inputDesc.Name, inputDesc.BindPoint, inputDesc.Space);

            elementSize = bufferDesc.Size;
            type = DescriptorType::ConstantBuffer;
            break;
        }
        case D3D_SIT_BYTEADDRESS:
        {
            elementSize = 4;    // Byte address buffers align to DWORDs.
            type = DescriptorType::ByteAddressBuffer;
            break;
        }
        case D3D_SIT_UAV_RWBYTEADDRESS:
        {
            elementSize = 4;    // Byte address buffers align to DWORDs.
            type = DescriptorType::RWByteAddressBuffer;
            break;
        }
        case D3D_SIT_TBUFFER:   // Exotic mixture between constant buffer and structured buffer. We'll map it to StructuredBuffer for now.
        case D3D_SIT_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        {
            elementSize = inputDesc.NumSamples;
            type = DescriptorType::StructuredBuffer;
            break;
        }
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        {
            elementSize = inputDesc.NumSamples;
            type = DescriptorType::RWStructuredBuffer;
            break;
        }
        case D3D_SIT_TEXTURE:
        {
            type = inputDesc.Dimension == D3D_SRV_DIMENSION_BUFFER ? DescriptorType::Buffer : DescriptorType::Texture;
            break;
        }
        case D3D_SIT_UAV_RWTYPED:
        {
            type = inputDesc.Dimension == D3D_SRV_DIMENSION_BUFFER ? DescriptorType::RWBuffer : DescriptorType::RWTexture;
            break;
        }
        case D3D_SIT_SAMPLER:                 type = DescriptorType::Sampler; break;
        case D3D_SIT_RTACCELERATIONSTRUCTURE: type = DescriptorType::AccelerationStructure; break;
        case D3D_SIT_UAV_FEEDBACKTEXTURE: throw RuntimeException("The shader exposes an unsupported resource of type {1} at binding point {0} (space {2}).", inputDesc.BindPoint, inputDesc.Type, inputDesc.Space);
        default: throw RuntimeException("The shader exposes an unknown resource type in binding {0} (space {1}).", inputDesc.BindPoint, inputDesc.Space);
        }

        DescriptorInfo descriptor = {
            .location = inputDesc.BindPoint,
            .elementSize = elementSize,
            .elements = inputDesc.BindCount,
            .type = type
        };

        // Unbounded arrays have a bind count of -1.
        if (inputDesc.BindCount == 0)
            descriptor.elements = std::numeric_limits<UInt32>::max();

        return descriptor;
    }

    SharedPtr<DirectX12PipelineLayout> reflectPipelineLayout()
    {
        // Check if the device is still valid.
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            throw RuntimeException("Cannot create pipeline layout on a released device instance.");

        // First, filter the descriptor sets and push constant ranges.
        Dictionary<UInt32, DescriptorSetInfo> descriptorSetLayouts;
        Array<PushConstantRangeInfo> pushConstantRanges;

        // Extract reflection data from all shader modules.
        std::ranges::for_each(m_modules, [this, &descriptorSetLayouts](UniquePtr<DirectX12ShaderModule>& shaderModule) {
            // Load the shader reflection.
            ComPtr<IDxcContainerReflection> reflection;
            raiseIfFailed(::DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&reflection)), "Unable to access DirectX shader reflection.");
            raiseIfFailed(reflection->Load(std::as_const(*shaderModule).handle().Get()), "Unable to load reflection from shader module.");

            // Callback to register a new descriptor set or merge a descriptor into an existing one.
            auto registerDescriptor = [&descriptorSetLayouts](DescriptorInfo& descriptor, D3D12_SHADER_INPUT_BIND_DESC inputDesc, const IShaderModule* shaderModule) {
                // Mark the descriptor as part of the local root signature, if the shader module has a local descriptor.
                descriptor.local = !shaderModule->shaderLocalDescriptor().has_value() ? false :
                    shaderModule->shaderLocalDescriptor().value().Register == inputDesc.BindPoint && shaderModule->shaderLocalDescriptor().value().Space == inputDesc.Space;

                // Check if a descriptor set has already been defined for the space.
                if (!descriptorSetLayouts.contains(inputDesc.Space))
                    descriptorSetLayouts.insert(std::make_pair(inputDesc.Space, DescriptorSetInfo{ .space = inputDesc.Space, .stage = shaderModule->type(), .descriptors = { descriptor } }));
                else
                {
                    auto& descriptorSetLayout = descriptorSetLayouts[inputDesc.Space];
                    descriptorSetLayout.stage = descriptorSetLayouts[inputDesc.Space].stage | shaderModule->type();

                    // If another descriptor is bound to the same register, check if they are compatible. Otherwise, simply add the descriptor.
                    if (auto match = std::ranges::find_if(descriptorSetLayouts[inputDesc.Space].descriptors, [&](const auto& descriptor) { return descriptor.location == inputDesc.BindPoint; }); match == descriptorSetLayouts[inputDesc.Space].descriptors.end())
                        descriptorSetLayout.descriptors.push_back(descriptor);
                    else if (!match->equals(descriptor)) [[unlikely]]
                        LITEFX_WARNING(DIRECTX12_LOG, "Two incompatible descriptors are bound to the same location ({0} in space {1}) at different shader stages.", descriptor.location, inputDesc.Space);
                }
            };

            // Libraries need a different reflection path from standard modules.
            if (LITEFX_FLAG_IS_SET(ShaderStage::RayTracingPipeline, shaderModule->type()))
            {
                // Verify reflection and get the actual shader reflection interface.
                UINT32 shaderIdx{ 0 };
                ComPtr<ID3D12LibraryReflection> shaderReflection;
                raiseIfFailed(reflection->FindFirstPartKind(FOUR_CC('D', 'X', 'I', 'L'), &shaderIdx), "The shader module does not contain a valid DXIL shader.");
                raiseIfFailed(reflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&shaderReflection)), "Unable to query shader reflection from DXIL module.");

                // Get the shader description from the reflection.
                D3D12_LIBRARY_DESC shaderInfo;
                raiseIfFailed(shaderReflection->GetDesc(&shaderInfo), "Unable to acquire meta-data from shader module.");
                
                // Parse each function in the module.
                for (UInt32 f(0); f < shaderInfo.FunctionCount; ++f)
                {
                    D3D12_FUNCTION_DESC functionDesc;
                    auto functionReflection = shaderReflection->GetFunctionByIndex(static_cast<INT>(f));
                    functionReflection->GetDesc(&functionDesc);

                    for (UInt32 i(0); i < functionDesc.BoundResources; ++i)
                    {
                        // Get the bound resource description.
                        D3D12_SHADER_INPUT_BIND_DESC inputDesc;
                        functionReflection->GetResourceBindingDesc(i, &inputDesc);
                        auto descriptor = this->getReflectionDescriptorDesc(inputDesc, functionReflection);

                        // Register the descriptor.
                        registerDescriptor(descriptor, inputDesc, shaderModule.get());
                    }
                }
            }
            else
            {
                // Verify reflection and get the actual shader reflection interface.
                UINT32 shaderIdx{ 0 };
                ComPtr<ID3D12ShaderReflection> shaderReflection;
                raiseIfFailed(reflection->FindFirstPartKind(FOUR_CC('D', 'X', 'I', 'L'), &shaderIdx), "The shader module does not contain a valid DXIL shader.");
                raiseIfFailed(reflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&shaderReflection)), "Unable to query shader reflection from DXIL module.");

                // Get the shader description from the reflection.
                D3D12_SHADER_DESC shaderInfo;
                raiseIfFailed(shaderReflection->GetDesc(&shaderInfo), "Unable to acquire meta-data from shader module.");

                // Iterate the bound resources to extract the descriptor sets.
                for (UInt32 i(0); i < shaderInfo.BoundResources; ++i)
                {
                    // Get the bound resource description.
                    D3D12_SHADER_INPUT_BIND_DESC inputDesc;
                    shaderReflection->GetResourceBindingDesc(i, &inputDesc);
                    auto descriptor = this->getReflectionDescriptorDesc(inputDesc, shaderReflection.Get());

                    // Register the descriptor.
                    registerDescriptor(descriptor, inputDesc, shaderModule.get());
                }
            }
        });

        // Attempt to find a shader module that exports a root signature. If none is found, issue a warning.
        // NOTE: Root signature is only ever expected to be provided in one shader module. If multiple are provided, it is not defined, which one will be picked.
        bool hasRootSignature = false;

        for (const auto& shaderModule : m_modules)
        {
            ComPtr<ID3D12RootSignatureDeserializer> deserializer;

            if (SUCCEEDED(::D3D12CreateRootSignatureDeserializer(std::as_const(*shaderModule).handle()->GetBufferPointer(), std::as_const(*shaderModule).handle()->GetBufferSize(), IID_PPV_ARGS(&deserializer))))
            {
                // Reflect the root signature in order to define static samplers and push constants.
                LITEFX_TRACE(DIRECTX12_LOG, "Found root signature in shader module {0}.", shaderModule->type());
                this->reflectRootSignature(deserializer, descriptorSetLayouts, pushConstantRanges);
                hasRootSignature = true;
                break;
            }
        }

        // Otherwise, fall back to traditional reflection to acquire the root signature.
        if (!hasRootSignature && !SUPPRESS_MISSING_ROOT_SIGNATURE_WARNING)
            LITEFX_WARNING(DIRECTX12_LOG, "None of the provided shader modules exports a root signature. Descriptor sets will be acquired using reflection. Some features (such as root/push constants) are not supported.");

        // Create the descriptor set layouts.
        // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
        auto descriptorSets = [](SharedPtr<const DirectX12Device> device, const Dictionary<UInt32, DescriptorSetInfo>& descriptorSetLayouts) -> std::generator<UniquePtr<DirectX12DescriptorSetLayout>> {
            for (auto it = descriptorSetLayouts.begin(); it != descriptorSetLayouts.end(); ++it)
            {
                auto& descriptorSet = it->second;

                // Create the descriptor layouts.
                auto descriptors = [](const DescriptorSetInfo& descriptorSet) -> std::generator<UniquePtr<DirectX12DescriptorLayout>> {
                    for (auto descriptor = descriptorSet.descriptors.begin(); descriptor != descriptorSet.descriptors.end(); ++descriptor)
                        co_yield descriptor->staticSamplerState.has_value() ?
                            makeUnique<DirectX12DescriptorLayout>(DirectX12Sampler::allocate(
                                D3D12_DECODE_MAG_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? FilterMode::Nearest : FilterMode::Linear,
                                D3D12_DECODE_MIN_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? FilterMode::Nearest : FilterMode::Linear,
                                DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressU), DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressV), DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressW),
                                D3D12_DECODE_MIP_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? MipMapMode::Nearest : MipMapMode::Linear,
                                descriptor->staticSamplerState->MipLODBias, descriptor->staticSamplerState->MinLOD, descriptor->staticSamplerState->MaxLOD, static_cast<Float>(descriptor->staticSamplerState->MaxAnisotropy)), descriptor->location) :
                            makeUnique<DirectX12DescriptorLayout>(descriptor->type, descriptor->location, descriptor->elementSize, descriptor->elements, descriptor->local);
                }(descriptorSet) | std::views::as_rvalue;

                co_yield makeUnique<DirectX12DescriptorSetLayout>(*device.get(), std::move(descriptors), descriptorSet.space, descriptorSet.stage);
            }
        }(device, descriptorSetLayouts) | std::views::as_rvalue | std::ranges::to<Enumerable<UniquePtr<DirectX12DescriptorSetLayout>>>();

        // Create the push constants layout.
        auto pushConstants = [](const Array<PushConstantRangeInfo>& pushConstantRanges) -> std::generator<UniquePtr<DirectX12PushConstantsRange>> {
            for (auto range = pushConstantRanges.begin(); range != pushConstantRanges.end(); ++range)
                co_yield makeUnique<DirectX12PushConstantsRange>(range->stage, range->offset, range->size, range->space, range->location);
        }(pushConstantRanges) | std::views::as_rvalue | std::ranges::to<Enumerable<UniquePtr<DirectX12PushConstantsRange>>>();
        // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)

        auto overallSize = std::accumulate(pushConstantRanges.begin(), pushConstantRanges.end(), 0, [](UInt32 currentSize, const auto& range) { return currentSize + range.size; });
        auto pushConstantsLayout = makeUnique<DirectX12PushConstantsLayout>(std::move(pushConstants), overallSize);

        // Return the pipeline layout.
        return makeShared<DirectX12PipelineLayout>(*device.get(), std::move(descriptorSets), std::move(pushConstantsLayout));
    }
};

void DirectX12ShaderProgram::suppressMissingRootSignatureWarning(bool disableWarning) noexcept
{
    SUPPRESS_MISSING_ROOT_SIGNATURE_WARNING = disableWarning;
}

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules) :
    m_impl(device, std::move(modules))
{
    m_impl->validate();
}

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12Device& device) noexcept :
    m_impl(device)
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

SharedPtr<DirectX12ShaderProgram> DirectX12ShaderProgram::create(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules)
{
    return SharedPtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram(device, std::move(modules)));
}

Enumerable<const DirectX12ShaderModule*> DirectX12ShaderProgram::modules() const
{
    return m_impl->m_modules | std::views::transform([](const UniquePtr<DirectX12ShaderModule>& shader) { return shader.get(); });
}

SharedPtr<DirectX12PipelineLayout> DirectX12ShaderProgram::reflectPipelineLayout() const
{
    return m_impl->reflectPipelineLayout();
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilderImpl {
public:
    friend class DirectX12ShaderProgramBuilder;

private:
    SharedPtr<const DirectX12Device> m_device;

public:
    DirectX12ShaderProgramBuilderImpl(const DirectX12Device& device) :
        m_device(device.shared_from_this())
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilder(const DirectX12Device& device) :
    ShaderProgramBuilder(SharedPtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram(device))), m_impl(device)
{
}

DirectX12ShaderProgramBuilder::~DirectX12ShaderProgramBuilder() noexcept = default;

void DirectX12ShaderProgramBuilder::build()
{
    this->instance()->m_impl->m_modules = std::move(this->state().modules);
    this->instance()->m_impl->validate();
}

UniquePtr<DirectX12ShaderModule> DirectX12ShaderProgramBuilder::makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor)
{
    return makeUnique<DirectX12ShaderModule>(*m_impl->m_device.get(), type, fileName, entryPoint, shaderLocalDescriptor);
}

UniquePtr<DirectX12ShaderModule> DirectX12ShaderProgramBuilder::makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor)
{
    return makeUnique<DirectX12ShaderModule>(*m_impl->m_device.get(), type, stream, name, entryPoint, shaderLocalDescriptor);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)