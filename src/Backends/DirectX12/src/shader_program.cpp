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
    return static_cast<UInt32>((Byte)ch0) | static_cast<UInt32>((Byte)ch1) << 8 | static_cast<UInt32>((Byte)ch2) << 16 | static_cast<UInt32>((Byte)ch3) << 24;
}

static bool SUPPRESS_MISSING_ROOT_SIGNATURE_WARNING = false;

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

class DirectX12ShaderProgram::DirectX12ShaderProgramImpl : public Implement<DirectX12ShaderProgram> {
public:
    friend class DirectX12ShaderProgramBuilder;
    friend class DirectX12ShaderProgram;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;
    const DirectX12Device& m_device;

private:
    struct DescriptorInfo {
    public:
        UInt32 location;
        UInt32 elementSize;
        UInt32 elements;
        DescriptorType type;
        Optional<D3D12_STATIC_SAMPLER_DESC> staticSamplerState;

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
        UInt32 space;
        ShaderStage stage;
        Array<DescriptorInfo> descriptors;
    };

    struct PushConstantRangeInfo {
    public:
        ShaderStage stage;
        UInt32 offset;
        UInt32 size;
        UInt32 location;
        UInt32 space;
    };

public:
    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent, const DirectX12Device& device, Array<UniquePtr<DirectX12ShaderModule>>&& modules) :
        base(parent), m_modules(std::move(modules)), m_device(device)
    {
    }

    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }

public:
    void reflectRootSignature(ComPtr<ID3D12RootSignatureDeserializer> deserializer, Dictionary<UInt32, DescriptorSetInfo>& descriptorSetLayouts, Array<PushConstantRangeInfo>& pushConstantRanges)
    {
        // Collect the shader stages.
        ShaderStage stages{ };
        std::ranges::for_each(m_modules, [&stages](const auto& shaderModule) { stages = stages | shaderModule->type(); });

        // Get the root signature description.
        auto description = deserializer->GetRootSignatureDesc();

        // Iterate the static samplers.
        for (int i(0); i < description->NumStaticSamplers; ++i)
        {
            auto staticSampler = description->pStaticSamplers[i];

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

        for (int i(0); i < description->NumParameters; ++i)
        {
            auto rootParameter = description->pParameters[i];

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
                    case D3D12_SHADER_VISIBILITY_ALL:
                    case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
                    case D3D12_SHADER_VISIBILITY_MESH:
                    default: throw InvalidArgumentException("The push constants for a shader are defined for invalid or unsupported shader stages. Note that a push constant must only be defined for a single shader stage.");
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
                throw InvalidArgumentException("The shader modules root signature defines a descriptor table for parameter {0}, which is currently not supported. Convert each parameter of the table into a separate root parameter.", i);
            default: 
                throw InvalidArgumentException("The shader modules root signature exposes an unknown root parameter type {1} for parameter {0}.", i, rootParameter.ParameterType);
            }
        }
    }

    SharedPtr<DirectX12PipelineLayout> reflectPipelineLayout()
    {
        // First, filter the descriptor sets and push constant ranges.
        Dictionary<UInt32, DescriptorSetInfo> descriptorSetLayouts;
        Array<PushConstantRangeInfo> pushConstantRanges;

        // Extract reflection data from all shader modules.
        std::ranges::for_each(m_modules, [this, &descriptorSetLayouts](UniquePtr<DirectX12ShaderModule>& shaderModule) {
            // Load the shader reflection.
            ComPtr<IDxcContainerReflection> reflection;
            raiseIfFailed<RuntimeException>(::DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&reflection)), "Unable to access DirectX shader reflection.");
            raiseIfFailed<RuntimeException>(reflection->Load(std::as_const(*shaderModule).handle().Get()), "Unable to load reflection from shader module.");

            // Verify reflection and get the actual shader reflection interface.
            UINT32 shaderIdx;
            ComPtr<ID3D12ShaderReflection> shaderReflection;
            raiseIfFailed<RuntimeException>(reflection->FindFirstPartKind(FOUR_CC('D', 'X', 'I', 'L'), &shaderIdx), "The shader module does not contain a valid DXIL shader.");
            raiseIfFailed<RuntimeException>(reflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&shaderReflection)), "Unable to query shader reflection from DXIL module.");

            // Get the shader description from the reflection.
            D3D12_SHADER_DESC shaderInfo;
            raiseIfFailed<RuntimeException>(shaderReflection->GetDesc(&shaderInfo), "Unable to acquire meta-data from shader module.");

            // Iterate the bound resources to extract the descriptor sets.
            for (int i(0); i < shaderInfo.BoundResources; ++i)
            {
                // Get the bound resource description.
                D3D12_SHADER_INPUT_BIND_DESC inputDesc;
                shaderReflection->GetResourceBindingDesc(i, &inputDesc);

                // First, create a description of the descriptor.
                DescriptorType type;
                UInt32 elementSize = 0;

                switch (inputDesc.Type)
                {
                case D3D_SIT_CBUFFER: 
                {
                    D3D12_SHADER_BUFFER_DESC bufferDesc;
                    auto constantBuffer = shaderReflection->GetConstantBufferByName(inputDesc.Name);
                    raiseIfFailed<RuntimeException>(constantBuffer->GetDesc(&bufferDesc), "Unable to query constant buffer \"{0}\" from shader module {1}.", inputDesc.Name, shaderModule->type());
                    
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
                case D3D_SIT_SAMPLER:     type = DescriptorType::Sampler; break;
                case D3D_SIT_RTACCELERATIONSTRUCTURE:
                case D3D_SIT_UAV_FEEDBACKTEXTURE: throw RuntimeException("The shader exposes an unsupported resource of type {1} at binding point {0}.", i, inputDesc.Type);
                default: throw RuntimeException("The shader exposes an unknown resource type in binding {0}.", i);
                }

                auto descriptor = DescriptorInfo {
                    .location = inputDesc.BindPoint,
                    .elementSize = elementSize,
                    .elements = inputDesc.BindCount,
                    .type = type
                };

                // Unbounded arrays have a bind count of -1.
                if (inputDesc.BindCount == 0)
                    descriptor.elements = -1;

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
        auto descriptorSets = [this, &descriptorSetLayouts]() -> std::generator<UniquePtr<DirectX12DescriptorSetLayout>> {
            for (auto it = descriptorSetLayouts.begin(); it != descriptorSetLayouts.end(); ++it)
            {
                auto& descriptorSet = it->second;

                // Create the descriptor layouts.
                auto descriptors = [this, &descriptorSet]() -> std::generator<UniquePtr<DirectX12DescriptorLayout>> {
                    for (auto descriptor = descriptorSet.descriptors.begin(); descriptor != descriptorSet.descriptors.end(); ++descriptor)
                        co_yield descriptor->staticSamplerState.has_value() ?
                            makeUnique<DirectX12DescriptorLayout>(makeUnique<DirectX12Sampler>(m_device,
                                D3D12_DECODE_MAG_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? FilterMode::Nearest : FilterMode::Linear,
                                D3D12_DECODE_MIN_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? FilterMode::Nearest : FilterMode::Linear,
                                DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressU), DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressV), DECODE_BORDER_MODE(descriptor->staticSamplerState->AddressW),
                                D3D12_DECODE_MIP_FILTER(descriptor->staticSamplerState->Filter) == D3D12_FILTER_TYPE_POINT ? MipMapMode::Nearest : MipMapMode::Linear,
                                descriptor->staticSamplerState->MipLODBias, descriptor->staticSamplerState->MinLOD, descriptor->staticSamplerState->MaxLOD, static_cast<Float>(descriptor->staticSamplerState->MaxAnisotropy)), descriptor->location) :
                            makeUnique<DirectX12DescriptorLayout>(descriptor->type, descriptor->location, descriptor->elementSize, descriptor->elements);
                }() | std::views::as_rvalue;

                co_yield makeUnique<DirectX12DescriptorSetLayout>(m_device, std::move(descriptors), descriptorSet.space, descriptorSet.stage);
            }
        }() | std::views::as_rvalue;

        // Create the push constants layout.
        auto pushConstants = [&pushConstantRanges]() -> std::generator<UniquePtr<DirectX12PushConstantsRange>> {
            for (auto range = pushConstantRanges.begin(); range != pushConstantRanges.end(); ++range)
                co_yield makeUnique<DirectX12PushConstantsRange>(range->stage, range->offset, range->size, range->space, range->location);
        }() | std::views::as_rvalue;

        auto overallSize = std::accumulate(pushConstantRanges.begin(), pushConstantRanges.end(), 0, [](UInt32 currentSize, const auto& range) { return currentSize + range.size; });
        auto pushConstantsLayout = makeUnique<DirectX12PushConstantsLayout>(std::move(pushConstants), overallSize);

        // Return the pipeline layout.
        return makeShared<DirectX12PipelineLayout>(m_device, std::move(descriptorSets), std::move(pushConstantsLayout));
    }
};

void DirectX12ShaderProgram::suppressMissingRootSignatureWarning(bool disableWarning) noexcept
{
    SUPPRESS_MISSING_ROOT_SIGNATURE_WARNING = disableWarning;
}

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12Device& device, Array<UniquePtr<DirectX12ShaderModule>>&& modules) noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this, device, std::move(modules)))
{
}

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12Device& device) noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this, device))
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

Array<const DirectX12ShaderModule*> DirectX12ShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<DirectX12ShaderModule>& shader) { return shader.get(); }) |
        std::ranges::to<Array<const DirectX12ShaderModule*>>();
}

SharedPtr<DirectX12PipelineLayout> DirectX12ShaderProgram::reflectPipelineLayout() const
{
    return m_impl->reflectPipelineLayout();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilderImpl : public Implement<DirectX12ShaderProgramBuilder> {
public:
    friend class DirectX12ShaderProgramBuilder;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;
    const DirectX12Device& m_device;

public:
    DirectX12ShaderProgramBuilderImpl(DirectX12ShaderProgramBuilder* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilder(const DirectX12Device& device) :
    m_impl(makePimpl<DirectX12ShaderProgramBuilderImpl>(this, device)), ShaderProgramBuilder(UniquePtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram(device)))
{
}

DirectX12ShaderProgramBuilder::~DirectX12ShaderProgramBuilder() noexcept = default;

void DirectX12ShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<DirectX12ShaderModule>(m_impl->m_device, type, fileName, entryPoint));
    return *this;
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withShaderModule(const ShaderStage& type, std::istream& stream, const String& name, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<DirectX12ShaderModule>(m_impl->m_device, type, stream, name, entryPoint));
    return *this;
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withVertexShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, stream, name, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationControlShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, stream, name, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationEvaluationShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, stream, name, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withGeometryShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, stream, name, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withFragmentShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, stream, name, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withComputeShaderModule(std::istream& stream, const String& name, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, stream, name, entryPoint);
}
#endif // defined(BUILD_DEFINE_BUILDERS)