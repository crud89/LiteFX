#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayout::VulkanPipelineLayoutImpl {
public:
    friend class VulkanPipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    SharedPtr<const VulkanDevice> m_device;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<SharedPtr<const VulkanDescriptorSetLayout>> m_descriptorSetLayouts;
    Array<VkDescriptorSetAndBindingMappingEXT> m_mappings;
    Dictionary<UInt32, UInt32> m_setPushOffsets; // Maps the descriptor set space to the offset of the resource or sampler index.
    VkShaderDescriptorSetAndBindingMappingInfoEXT m_mappingInfo{ .sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT };

public:
    VulkanPipelineLayoutImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this())
    {
    }
    
    VulkanPipelineLayoutImpl(const VulkanPipelineLayoutImpl&) = delete;
    VulkanPipelineLayoutImpl(VulkanPipelineLayoutImpl&&) noexcept = default;
    VulkanPipelineLayoutImpl& operator=(const VulkanPipelineLayoutImpl&) = delete;
    VulkanPipelineLayoutImpl& operator=(VulkanPipelineLayoutImpl&&) noexcept = default;

public:
    void createMappings()
    {
        constexpr UInt32 resourceStride = VulkanDevice::RESOURCE_DESCRIPTOR_STRIDE, samplerStride = VulkanDevice::SAMPLER_DESCRIPTOR_STRIDE;
        UInt32 offset = align(m_pushConstantsLayout == nullptr ? 0u : m_pushConstantsLayout->size(), 4u);

        // Create a mapping for each descriptor set.
        for (auto& layout : m_descriptorSetLayouts)
        {
            // Store the offset to the push constant for the descriptor set space.
            auto space = layout->space();
            auto pushOffset = offset;
            m_setPushOffsets[space] = pushOffset;
            offset += 2u * sizeof(UInt32);

            // Map each descriptor.
            for (auto& descriptor : layout->descriptors())
            {
                VkDescriptorSetAndBindingMappingEXT mapping {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
                    .descriptorSet = space,
                    .firstBinding = descriptor.binding(),
                    .bindingCount = 1u,
                    .resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT
                };

                if (descriptor.descriptorType() == DescriptorType::ResourceDescriptorHeap)
                {
                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
                    mapping.sourceData.constantOffset = { .heapOffset = 0u, .heapArrayStride = resourceStride };
                }
                else if (descriptor.descriptorType() == DescriptorType::SamplerDescriptorHeap)
                {
                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
                    mapping.sourceData.constantOffset = { .heapOffset = 0u, .heapArrayStride = samplerStride };
                }
                else if (descriptor.staticSampler() != nullptr)
                {
                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
                    mapping.sourceData.constantOffset = { .pEmbeddedSampler = &descriptor.staticSampler()->createInfo() };
                }
                else
                {
                    auto range = layout->allocationLayout().range(descriptor.binding());
                    auto isSampler = range->Heap == DescriptorHeapType::Sampler;
                    auto stride = isSampler ? samplerStride : resourceStride;

                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
                    mapping.sourceData.pushIndex = {
                        .heapOffset = range->RelativeOffset * stride,
                        .pushOffset = pushOffset + (isSampler ? 4u : 0u),
                        .heapIndexStride = stride,
                        .heapArrayStride = stride 
                    };
                }

                m_mappings.push_back(mapping);
            }
        }

        if (offset > m_device->maxPushDataSize()) [[unlikely]]
            throw RuntimeException("The pipeline layout requires {0} bytes of push data, but the device only supports {1} bytes.", offset, m_device->maxPushDataSize());

        m_mappingInfo.mappingCount = static_cast<UInt32>(m_mappings.size());
        m_mappingInfo.pMappings = m_mappings.data();
    }

    void initialize([[maybe_unused]] const VulkanPipelineLayout& pipelineLayout, Array<SharedPtr<VulkanDescriptorSetLayout>> descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout)
    {
        if (std::ranges::any_of(descriptorSetLayouts, [](const auto& layout) { return layout == nullptr; })) [[unlikely]]
            throw ArgumentNotInitializedException("descriptorSetLayouts", "At least one of the provided descriptor layouts is not initialized.");

        // Sort the descriptor sets by space, to allow for duplicate detection and stable push constant offsets.
        std::ranges::sort(descriptorSetLayouts, [](const auto& a, const auto& b) { return a->space() < b->space(); });

        // Check if there are descriptor sets occupying the same space.
        if (auto duplicate = std::ranges::adjacent_find(descriptorSetLayouts, {}, [](const auto& layout) { return layout->space(); }); duplicate != descriptorSetLayouts.end()) [[unlikely]]
            throw InvalidArgumentException("descriptorSetLayouts", "Two layouts defined for the same descriptor set {}. Each descriptor set must use it's own space.", (*duplicate)->space());

        LITEFX_TRACE(VULKAN_LOG, "Creating pipeline layout {0} {{ Descriptor Sets: {1}, Push Constants: {2} bytes }}...", static_cast<const void*>(&pipelineLayout), descriptorSetLayouts.size(), pushConstantsLayout == nullptr ? 0u : pushConstantsLayout->size());

        // Store the layouts.
        m_pushConstantsLayout = std::move(pushConstantsLayout);
        m_descriptorSetLayouts.append_range(std::move(descriptorSetLayouts));

        // Map the descriptor bindings onto the global descriptor heaps.
        this->createMappings();
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
    m_impl(device)
{
    m_impl->initialize(*this, descriptorSetLayouts | std::ranges::to<std::vector>(), std::move(pushConstantsLayout));
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device) noexcept : // NOLINT(bugprone-exception-escape)
    m_impl(device)
{
}

const VkShaderDescriptorSetAndBindingMappingInfoEXT& VulkanPipelineLayout::mappingInfo() const noexcept
{
    return m_impl->m_mappingInfo;
}

Optional<UInt32> VulkanPipelineLayout::pushDataOffset(UInt32 space, DescriptorHeapType heap) const
{
    if (auto match = m_impl->m_setPushOffsets.find(space); match != m_impl->m_setPushOffsets.end())
        return match->second + (heap == DescriptorHeapType::Sampler ? 4u : 0u);

    return std::nullopt;
}

const VulkanDevice& VulkanPipelineLayout::device() const noexcept
{
    return *m_impl->m_device;
}

const VulkanDescriptorSetLayout& VulkanPipelineLayout::descriptorSet(UInt32 space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const auto& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw InvalidArgumentException("space", "No descriptor set layout uses the provided space {0}.", space);
}

const Array<SharedPtr<const VulkanDescriptorSetLayout>>& VulkanPipelineLayout::descriptorSets() const
{
    return m_impl->m_descriptorSetLayouts;
}

const VulkanPushConstantsLayout* VulkanPipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}

bool VulkanPipelineLayout::dynamicResourceHeapAccess() const
{
    auto descriptorLayouts = m_impl->m_descriptorSetLayouts 
        | std::views::transform([](const auto& layout) { return layout->descriptors(); }) 
        | std::views::join;

    return std::ranges::any_of(descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ResourceDescriptorHeap; });
}

bool VulkanPipelineLayout::dynamicSamplerHeapAccess() const
{
    auto descriptorLayouts = m_impl->m_descriptorSetLayouts
        | std::views::transform([](const auto& layout) { return layout->descriptors(); })
        | std::views::join;

    return std::ranges::any_of(descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::SamplerDescriptorHeap; });
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayoutBuilder::VulkanPipelineLayoutBuilder(const VulkanDevice& parent) :
    PipelineLayoutBuilder(VulkanPipelineLayout::create(parent))
{
}

VulkanPipelineLayoutBuilder::~VulkanPipelineLayoutBuilder() noexcept = default;

void VulkanPipelineLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->initialize(*instance, std::move(this->state().descriptorSetLayouts), std::move(this->state().pushConstantsLayout));
}

VulkanDescriptorSetLayoutBuilder VulkanPipelineLayoutBuilder::descriptorSet(UInt32 space, ShaderStage stages)
{
    return VulkanDescriptorSetLayoutBuilder(*this, space, stages);
}

VulkanPushConstantsLayoutBuilder VulkanPipelineLayoutBuilder::pushConstants(UInt32 size)
{
    return VulkanPushConstantsLayoutBuilder(*this, size);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)