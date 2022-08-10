#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include <spirv_reflect.h>
#include <fstream>
#include <numeric>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgram::VulkanShaderProgramImpl : public Implement<VulkanShaderProgram> {
public:
    friend class VulkanShaderProgramBuilder;
    friend class VulkanShaderProgram;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;
    const VulkanDevice& m_device;

private:
    struct DescriptorInfo {
    public:
        UInt32 location;
        UInt32 elementSize;
        UInt32 elements;
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
        UInt32 space;
        ShaderStage stage;
        Array<DescriptorInfo> descriptors;
    };

    struct PushConstantRangeInfo {
    public:
        ShaderStage stage;
        UInt32 offset;
        UInt32 size;
    };

public:
    VulkanShaderProgramImpl(VulkanShaderProgram* parent, const VulkanDevice& device, Array<UniquePtr<VulkanShaderModule>>&& modules) :
        base(parent), m_device(device), m_modules(std::move(modules))
    {
    }

    VulkanShaderProgramImpl(VulkanShaderProgram* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
    {
    }

public:
    SharedPtr<VulkanPipelineLayout> reflectPipelineLayout()
    {
        // First, filter the descriptor sets and push constant ranges.
        Dictionary<UInt32, DescriptorSetInfo> descriptorSetLayouts;
        Array<PushConstantRangeInfo> pushConstantRanges;

        // Extract reflection data from all shader modules.
        std::ranges::for_each(m_modules, [this, &descriptorSetLayouts, &pushConstantRanges](UniquePtr<VulkanShaderModule>& shaderModule) {
            // Read the file and initialize a reflection module.
            auto bytecode = shaderModule->bytecode();
            spv_reflect::ShaderModule reflection(bytecode.size(), bytecode.c_str());
            auto result = reflection.GetResult();

            if (result != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to reflect shader module (Error {0}).", reflection.GetResult());

            // Get the number of descriptor sets and push constants.
            UInt32 descriptorSetCount, pushConstantCount;

            if ((result = reflection.EnumerateDescriptorSets(&descriptorSetCount, nullptr)) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to get descriptor set count (Error {0}).", result);

            if ((result = reflection.EnumeratePushConstants(&pushConstantCount, nullptr)) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to get push constants count (Error {0}).", result);

            // Acquire the descriptor sets and push constants.
            Array<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
            Array<SpvReflectBlockVariable*> pushConstants(pushConstantCount);

            if ((result = reflection.EnumerateDescriptorSets(&descriptorSetCount, descriptorSets.data())) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to enumerate descriptor sets (Error {0}).", result);

            if ((result = reflection.EnumeratePushConstants(&pushConstantCount, pushConstants.data())) != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
                throw RuntimeException("Unable to enumerate push constants (Error {0}).", result);

            // Parse the descriptor sets.
            std::ranges::for_each(descriptorSets, [this, &shaderModule, &reflection, &descriptorSetLayouts](const SpvReflectDescriptorSet* descriptorSet) {
                // Get all descriptor layouts.
                Array<DescriptorInfo> descriptors(descriptorSet->binding_count);

                std::ranges::generate(descriptors, [&descriptorSet, i = 0]() mutable {
                    auto descriptor = descriptorSet->bindings[i++];

                    // Filter the descriptor type.
                    DescriptorType type;

                    switch (descriptor->descriptor_type)
                    {
                    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:    throw RuntimeException("The shader exposes a combined image samplers, which is currently not supported.");
                    case SPV_REFLECT_TYPE_FLAG_EXTERNAL_ACCELERATION_STRUCTURE: throw RuntimeException("The shader exposes an acceleration structure, which is currently not supported.");
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:    throw RuntimeException("The shader exposes a dynamic buffer, which is currently not supported.");
                    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:                   type = DescriptorType::Sampler; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:             type = DescriptorType::Texture; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:             type = DescriptorType::WritableTexture; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:      type = DescriptorType::Buffer; break;  // NOTE: Maps back to STORAGE_TEXEL_BUFFER, which might cause issues.
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:      type = DescriptorType::WritableBuffer; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:            type = DescriptorType::Uniform; break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:            type = DescriptorType::Storage; break; // NOTE: There does not seem to be a representation for `WritableStorage`.
                    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:          type = DescriptorType::InputAttachment; break;
                    }

                    // Count the array elements.
                    UInt32 descriptors = 1;

                    for (int i(0); i < descriptor->array.dims_count; ++i)
                        descriptors *= descriptor->array.dims[i];

                    // Create the descriptor layout.
                    return DescriptorInfo{ .location = descriptor->binding, .elementSize = descriptor->block.padded_size, .elements = descriptors, .type = type };
                });

                if (!descriptorSetLayouts.contains(descriptorSet->set))
                    descriptorSetLayouts.insert(std::make_pair(descriptorSet->set, DescriptorSetInfo{ .space = descriptorSet->set, .stage = shaderModule->type(), .descriptors = descriptors}));
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
                            LITEFX_WARNING(VULKAN_LOG, "Mismatching descriptors detected: the descriptor at location {0} ({3} elements with size of {4} bytes) of the descriptor set {1} in shader stage {2} conflicts with a descriptor from at least one other shader stage and will be dropped (conflicts with descriptor of type {9} in stage/s {6} with {7} elements of {8} bytes).",
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

            std::ranges::for_each(pushConstants, [this, &shaderModule, &reflection, &pushConstantRanges](const SpvReflectBlockVariable* pushConstant) {
                pushConstantRanges.push_back(PushConstantRangeInfo{ .stage = shaderModule->type(), .offset = pushConstant->absolute_offset, .size = pushConstant->padded_size });
            });
        });

        // Create the descriptor set layouts.
        Array<UniquePtr<VulkanDescriptorSetLayout>> descriptorSets(descriptorSetLayouts.size());
        std::ranges::generate(descriptorSets, [this, &descriptorSetLayouts, i = 0]() mutable {
            // Get the descriptor set layout.
            auto it = descriptorSetLayouts.begin();
            std::advance(it, i++);
            auto& descriptorSet = it->second;

            // Create the descriptor layouts.
            Array<UniquePtr<VulkanDescriptorLayout>> descriptors(descriptorSet.descriptors.size());
            std::ranges::generate(descriptors, [&descriptorSet, j = 0]() mutable {
                auto& descriptor = descriptorSet.descriptors[j++];
                return makeUnique<VulkanDescriptorLayout>(descriptor.type, descriptor.location, descriptor.elementSize, descriptor.elements);
            });

            return makeUnique<VulkanDescriptorSetLayout>(m_device, std::move(descriptors), descriptorSet.space, descriptorSet.stage);
        });

        // Create the push constants layout.
        Array<UniquePtr<VulkanPushConstantsRange>> pushConstants(pushConstantRanges.size());
        std::ranges::generate(pushConstants, [&pushConstantRanges, i = 0]() mutable {
            auto& range = pushConstantRanges[i++];
            return makeUnique<VulkanPushConstantsRange>(range.stage, range.offset, range.size, 0, 0);   // No space or binding for Vulkan push constants.
        });

        auto overallSize = std::accumulate(pushConstantRanges.begin(), pushConstantRanges.end(), 0, [](UInt32 currentSize, const auto& range) { return currentSize + range.size; });
        auto pushConstantsLayout = makeUnique<VulkanPushConstantsLayout>(std::move(pushConstants), overallSize);

        // Return the pipeline layout.
        return makeShared<VulkanPipelineLayout>(m_device, std::move(descriptorSets), std::move(pushConstantsLayout));
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanDevice& device, Array<UniquePtr<VulkanShaderModule>>&& modules) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this, device, std::move(modules)))
{
}

VulkanShaderProgram::VulkanShaderProgram(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this, device))
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const VulkanShaderModule*> VulkanShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<VulkanShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const VulkanShaderModule*>>();
}

SharedPtr<VulkanPipelineLayout> VulkanShaderProgram::reflectPipelineLayout() const
{
    return m_impl->reflectPipelineLayout();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Graphics shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgramBuilder::VulkanShaderProgramBuilderImpl : public Implement<VulkanShaderProgramBuilder> {
public:
    friend class VulkanShaderProgramBuilder;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;
    const VulkanDevice& m_device;

public:
    VulkanShaderProgramBuilderImpl(VulkanShaderProgramBuilder* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Graphics shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder::VulkanShaderProgramBuilder(const VulkanDevice& device) :
    m_impl(makePimpl<VulkanShaderProgramBuilderImpl>(this, device)), ShaderProgramBuilder(SharedPtr<VulkanShaderProgram>(new VulkanShaderProgram(device)))
{
}

VulkanShaderProgramBuilder::~VulkanShaderProgramBuilder() noexcept = default;

void VulkanShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(m_impl->m_device, type, fileName, entryPoint));
    return *this;
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, fileName, entryPoint);
}
#endif // defined(BUILD_DEFINE_BUILDERS)