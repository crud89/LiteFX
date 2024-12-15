#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include <spirv_reflect.h>
#include <fstream>
#include <numeric>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Formatter for SpvReflectResult.
// ------------------------------------------------------------------------------------------------

template <>
struct LITEFX_VULKAN_API std::formatter<SpvReflectResult> : std::formatter<std::string_view> {
    auto format(SpvReflectResult t, std::format_context& ctx) {
        string_view name;

        switch (t)
        {
        case SPV_REFLECT_RESULT_SUCCESS: name = "SPV_REFLECT_RESULT_SUCCESS"; break;
        case SPV_REFLECT_RESULT_NOT_READY: name = "SPV_REFLECT_RESULT_NOT_READY"; break;
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED: name = "SPV_REFLECT_RESULT_ERROR_PARSE_FAILED"; break;
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED: name = "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED"; break;
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED: name = "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED"; break;
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER: name = "SPV_REFLECT_RESULT_ERROR_NULL_POINTER"; break;
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR: name = "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR"; break;
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH: name = "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH"; break;
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND: name = "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT"; break;
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE: name = "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE"; break;
        default: name = "Status: unknown"; break;
        }

        return formatter<string_view>::format(name, ctx);
    }
};

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgram::VulkanShaderProgramImpl {
public:
    friend class VulkanShaderProgramBuilder;
    friend class VulkanShaderProgram;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;
    SharedPtr<const VulkanDevice> m_device;

private:
    struct DescriptorInfo {
    public:
        UInt32 location;
        UInt32 elementSize;
        UInt32 elements;
        UInt32 inputAttachmentIndex;
        DescriptorType type;

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
        UInt32 space{};
        ShaderStage stage{};
        Array<DescriptorInfo> descriptors;
    };

    struct PushConstantRangeInfo {
    public:
        ShaderStage stage;
        UInt32 offset;
        UInt32 size;
    };

public:
    VulkanShaderProgramImpl(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules) :
        m_device(device.shared_from_this())
    {
        m_modules = std::move(modules) | std::views::as_rvalue | std::ranges::to<std::vector>();
    }

    VulkanShaderProgramImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this())
    {
    }

public:
    void validate()
    {
        // First check if there are any modules at all, or any that are uninitialized.
        if (m_modules.empty()) [[unlikely]]
            return; // Not exactly a reason to throw, but rather an empty group cannot be meaningful used anyway.

        if (std::ranges::contains(m_modules, std::unique_ptr<VulkanShaderModule>(nullptr))) [[unlikely]]
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

    SharedPtr<VulkanPipelineLayout> reflectPipelineLayout()
    {
        // First, filter the descriptor sets and push constant ranges.
        Dictionary<UInt32, DescriptorSetInfo> descriptorSetLayouts;
        Array<PushConstantRangeInfo> pushConstantRanges;

        // Extract reflection data from all shader modules.
        std::ranges::for_each(m_modules, [&](UniquePtr<VulkanShaderModule>& shaderModule) {
            // Read the file and initialize a reflection module.
            auto bytecode = shaderModule->bytecode();
            spv_reflect::ShaderModule reflection(bytecode.size(), bytecode.c_str());
            auto result = reflection.GetResult();

            if (result != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to reflect shader module (Error {0:x}).", static_cast<UInt32>(reflection.GetResult()));

            // Get the number of descriptor sets and push constants.
            UInt32 descriptorSetCount{}, pushConstantCount{};

            // NOLINTBEGIN(bugprone-assignment-in-if-condition)
            if ((result = reflection.EnumerateDescriptorSets(&descriptorSetCount, nullptr)) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to get descriptor set count (Error {0:x}).", static_cast<UInt32>(result));

            if ((result = reflection.EnumeratePushConstantBlocks(&pushConstantCount, nullptr)) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to get push constants count (Error {0:x}).", static_cast<UInt32>(result));

            // Acquire the descriptor sets and push constants.
            Array<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
            Array<SpvReflectBlockVariable*> pushConstants(pushConstantCount);

            if ((result = reflection.EnumerateDescriptorSets(&descriptorSetCount, descriptorSets.data())) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to enumerate descriptor sets (Error {0:x}).", static_cast<UInt32>(result));

            if ((result = reflection.EnumeratePushConstantBlocks(&pushConstantCount, pushConstants.data())) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to enumerate push constants (Error {0:x}).", static_cast<UInt32>(result));
            // NOLINTEND(bugprone-assignment-in-if-condition)
            
            // Parse the descriptor sets.
            std::ranges::for_each(descriptorSets, [&shaderModule, &descriptorSetLayouts](const SpvReflectDescriptorSet* descriptorSet) {
                // Get all descriptor layouts.
                Array<DescriptorInfo> descriptors(descriptorSet->binding_count);

                std::ranges::generate(descriptors, [&descriptorSet, i = 0]() mutable {
                    auto descriptor = descriptorSet->bindings[i++]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

                    // Filter the descriptor type.
                    DescriptorType type{};
                    UInt32 inputAttachmentIndex = 0;

                    switch (descriptor->descriptor_type)
                    {
                    default: throw RuntimeException("Unsupported descriptor type detected.");
                    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     throw RuntimeException("The shader exposes a combined image samplers, which is currently not supported.");
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     throw RuntimeException("The shader exposes a dynamic buffer, which is currently not supported.");
                    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           type = DescriptorType::InputAttachment; inputAttachmentIndex = descriptor->input_attachment_index; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:                    type = DescriptorType::Sampler; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              type = DescriptorType::Texture; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:              type = DescriptorType::RWTexture; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             type = DescriptorType::ConstantBuffer; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       type = DescriptorType::Buffer; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       type = DescriptorType::RWBuffer; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: type = DescriptorType::AccelerationStructure; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    {
                        // NOTE: Storage buffers need special care here. For more information see: 
                        //       https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#constant-texture-structured-byte-buffers.
                        //       Structured buffers and byte address buffers all translate into storage buffers, which in Vulkan terms only differ in how they are bound. We still try to approximate 
                        //       which buffer type was used for compilation, but at least for how Vulkan is concerned it does not matter anyway.
                        // TODO: There's also  `TextureBuffer`/`tbuffer`, that lands here. But how does it relate to texel buffers?
                        
                        // All buffers should have at least one member that stores the type info about the contained type. Descriptor arrays are of type `SpvOpTypeRuntimeArray`. To differentiate
                        // between `ByteAddressBuffer` and `StructuredBuffer`, we check the type flags of the first member. If it identifies an array of DWORDs, we treat the descriptor as 
                        // `ByteAddressBuffer`, though it could be a flavor of `StructuredBuffer<int>`. This is conceptually identical, so it ultimately makes no difference.
                        if ((descriptor->type_description->members[0].type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) == SPV_REFLECT_TYPE_FLAG_STRUCT) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                            type = (descriptor->resource_type & SPV_REFLECT_RESOURCE_FLAG_SRV) == SPV_REFLECT_RESOURCE_FLAG_SRV ? DescriptorType::StructuredBuffer : DescriptorType::RWStructuredBuffer;
                        else
                            type = (descriptor->resource_type & SPV_REFLECT_RESOURCE_FLAG_SRV) == SPV_REFLECT_RESOURCE_FLAG_SRV ? DescriptorType::ByteAddressBuffer : DescriptorType::RWByteAddressBuffer;

                        break;
                    }
                    }

                    // Count the array elements.
                    // NOTE: Actually there is a difference between declaring a descriptor an array (e.g. `StructuredBuffer<T> buffers[10]`) and declaring an array of descriptors 
                    //       (e.g. `StructuredBuffer<T> buffers[]`). The first variant only takes up a single descriptor, to which a buffer array can be bound. The second variant describes an 
                    //       variable-sized array of descriptors (aka runtime array). In the engine we treat both identically. A runtime array is defined as a descriptor with 0xFFFFFFFF elements.
                    //       Theoretically, we could bind a buffer array to an descriptor within a descriptor array, which is currently an unsupported use case. In the future, we might want to have
                    //       a separate descriptor flag for descriptor arrays and array descriptors and also provide methods to bind them both.
                    UInt32 descriptors = 1;

                    if (descriptor->type_description->op == SpvOp::SpvOpTypeRuntimeArray)
                        descriptors = std::numeric_limits<UInt32>::max();   // Unbounded.
                    else
                        for (UInt32 d(0); d < descriptor->array.dims_count; ++d)
                            descriptors *= descriptor->array.dims[d];

                    // Create the descriptor layout.
                    return DescriptorInfo{ .location = descriptor->binding, .elementSize = descriptor->block.padded_size, .elements = descriptors, .inputAttachmentIndex = inputAttachmentIndex, .type = type };
                });

                if (!descriptorSetLayouts.contains(descriptorSet->set))
                    descriptorSetLayouts.insert(std::make_pair(descriptorSet->set, DescriptorSetInfo{ .space = descriptorSet->set, .stage = shaderModule->type(), .descriptors = descriptors }));
                else
                {
                    // If the set already exists in another stage, merge it.
                    auto& layout = descriptorSetLayouts[descriptorSet->set];

                    for (auto& descriptor : descriptors)
                    {
                        // Add the descriptor, if no other descriptor has been bound to the location. Otherwise, check if the descriptors are equal and drop it, if they aren't.
                        if (auto match = std::ranges::find_if(layout.descriptors, [&descriptor](const DescriptorInfo& element) { return element.location == descriptor.location; }); match == layout.descriptors.end())
                            layout.descriptors.push_back(descriptor);
                        else if (!descriptor.equals(*match))
                            LITEFX_WARNING(VULKAN_LOG, "Mismatching descriptors detected: the descriptor at location {0} ({3} elements with size of {4} bytes) of the descriptor set {1} in shader stage {2} conflicts with a descriptor from at least one other shader stage and will be dropped (conflicts with descriptor of type {8} in stage/s {5} with {6} elements of {7} bytes).",
                                descriptor.location, descriptorSet->set, shaderModule->type(), descriptor.elements, descriptor.elementSize, layout.stage, match->elements, match->elementSize, match->type);
                    }

                    // Store the stage.
                    layout.stage = shaderModule->type() | layout.stage;
                }
            });

            // Parse push constants.
            // NOTE: Block variables are not exposing the shader stage, they are used from. If there are two shader modules created from the same source, but with different 
            //       entry points, each using their own push constants, it would be valid, but we are not able to tell which push constant range belongs to which stage.
            if (pushConstantCount > 1)
                LITEFX_WARNING(VULKAN_LOG, "More than one push constant range detected for shader stage {0}. If you have multiple entry points, you may be able to split them up into different shader files.", shaderModule->type());

            std::ranges::for_each(pushConstants, [&shaderModule, &pushConstantRanges](const SpvReflectBlockVariable* pushConstant) {
                pushConstantRanges.push_back(PushConstantRangeInfo{ .stage = shaderModule->type(), .offset = pushConstant->absolute_offset, .size = pushConstant->padded_size });
            });
        });

        // Create the descriptor set layouts.
        auto descriptorSets = [](SharedPtr<const VulkanDevice> device, Dictionary<UInt32, DescriptorSetInfo> descriptorSetLayouts) -> std::generator<SharedPtr<VulkanDescriptorSetLayout>> {
            for (auto it = descriptorSetLayouts.begin(); it != descriptorSetLayouts.end(); ++it)
            {
                auto space = it->second.space;
                auto stage = it->second.stage;

                // Create the descriptor layouts.
                auto descriptorLayouts = [](DescriptorSetInfo descriptorSet) -> std::generator<VulkanDescriptorLayout> {
                    for (auto descriptor = descriptorSet.descriptors.begin(); descriptor != descriptorSet.descriptors.end(); ++descriptor)
                        co_yield descriptor->type == DescriptorType::InputAttachment ?
                            VulkanDescriptorLayout { descriptor->type, descriptor->location, descriptor->inputAttachmentIndex} :
                            VulkanDescriptorLayout { descriptor->type, descriptor->location, descriptor->elementSize, descriptor->elements };
                }(std::move(it->second));

                co_yield VulkanDescriptorSetLayout::create(*device, descriptorLayouts, space, stage);
            }
        }(m_device, std::move(descriptorSetLayouts)) | std::views::as_rvalue | std::ranges::to<Enumerable<SharedPtr<VulkanDescriptorSetLayout>>>();

        // Create the push constants layout.
        auto overallSize = std::accumulate(pushConstantRanges.begin(), pushConstantRanges.end(), 0, [](UInt32 currentSize, const auto& range) { return currentSize + range.size; });
        auto pushConstants = [](Array<PushConstantRangeInfo> pushConstantRanges) -> std::generator<UniquePtr<VulkanPushConstantsRange>> {
            for (auto it = pushConstantRanges.begin(); it != pushConstantRanges.end(); ++it)
                co_yield makeUnique<VulkanPushConstantsRange>(it->stage, it->offset, it->size, 0, 0);   // No space or binding for Vulkan push constants.
        }(std::move(pushConstantRanges)) | std::views::as_rvalue | std::ranges::to<Enumerable<UniquePtr<VulkanPushConstantsRange>>>();

        auto pushConstantsLayout = makeUnique<VulkanPushConstantsLayout>(std::move(pushConstants), overallSize);

        // Return the pipeline layout.
        return VulkanPipelineLayout::create(*m_device, std::move(descriptorSets), std::move(pushConstantsLayout));
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules) :
    m_impl(device, std::move(modules))
{
    m_impl->validate();
}

VulkanShaderProgram::VulkanShaderProgram(const VulkanDevice& device) :
    m_impl(device)
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Enumerable<const VulkanShaderModule*> VulkanShaderProgram::modules() const
{
    return m_impl->m_modules | std::views::transform([](const UniquePtr<VulkanShaderModule>& shader) { return shader.get(); });
}

SharedPtr<VulkanPipelineLayout> VulkanShaderProgram::reflectPipelineLayout() const
{
    return m_impl->reflectPipelineLayout();
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder::VulkanShaderProgramBuilder(const VulkanDevice& device) :
    ShaderProgramBuilder(VulkanShaderProgram::create(device))
{
}

VulkanShaderProgramBuilder::~VulkanShaderProgramBuilder() noexcept = default;

void VulkanShaderProgramBuilder::build()
{
    this->instance()->m_impl->m_modules = std::move(this->state().modules);
    this->instance()->m_impl->validate();
}

UniquePtr<VulkanShaderModule> VulkanShaderProgramBuilder::makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor)
{
    return makeUnique<VulkanShaderModule>(*this->instance()->m_impl->m_device, type, fileName, entryPoint, shaderLocalDescriptor);
}

UniquePtr<VulkanShaderModule> VulkanShaderProgramBuilder::makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor)
{
    return makeUnique<VulkanShaderModule>(*this->instance()->m_impl->m_device, type, stream, name, entryPoint, shaderLocalDescriptor);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)