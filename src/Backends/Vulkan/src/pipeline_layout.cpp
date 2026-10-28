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
    VkShaderDescriptorSetAndBindingMappingInfoEXT m_mappingInfo{ .sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT };
    bool m_directResourceHeapAccess{}, m_directSamplerHeapAccess{};
    
    // TODO: Make those indices optional, which saves space in the push data signature if a descriptor set only binds to a single heap.
    Dictionary<UInt32, UInt32> m_setPushOffsets; // Maps the descriptor set space to the offset of the resource or sampler index. 

public:
    VulkanPipelineLayoutImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this())
    {
    }
    
    VulkanPipelineLayoutImpl(const VulkanPipelineLayoutImpl&) = delete;
    VulkanPipelineLayoutImpl(VulkanPipelineLayoutImpl&&) noexcept = default;
    VulkanPipelineLayoutImpl& operator=(const VulkanPipelineLayoutImpl&) = delete;
    VulkanPipelineLayoutImpl& operator=(VulkanPipelineLayoutImpl&&) noexcept = default;
    ~VulkanPipelineLayoutImpl() noexcept = default;

public:
    void createMappings()
    {
        UInt32 resourceStride = m_device->descriptorStride(DescriptorHeapType::Resource), samplerStride = m_device->descriptorStride(DescriptorHeapType::Sampler), pushDataEndOffset{ 0u };

        // Each descriptor set binding a heap receives two heap indices in push data: one for the resource heap and one for the sampler heap.
        constexpr UInt32 SET_PUSH_DATA_SIZE = 2u * sizeof(UInt32); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        constexpr UInt32 SAMPLER_INDEX_OFFSET = sizeof(UInt32);

        // First, compute the offset of the end of the push constants range. Push constants are always at the beginning of push data.
        if (m_pushConstantsLayout != nullptr)
        {
            pushDataEndOffset = m_pushConstantsLayout->size();

            for (auto& range : m_pushConstantsLayout->ranges())
                pushDataEndOffset = std::max(pushDataEndOffset, range->offset() + range->size());
        }

        pushDataEndOffset = alignPoT(pushDataEndOffset, 4u);

        // Create a mapping for each descriptor set.
        for (auto& layout : m_descriptorSetLayouts)
        {
            auto space = layout->space();
            auto& allocationLayout = layout->allocationLayout();

            // Descriptor sets that only contain shader-local descriptors or static samplers are never bound and don't need indices in the push data mapping.
            UInt32 pushOffset{ 0u };

            if (allocationLayout.binds(DescriptorHeapType::Resource) || allocationLayout.binds(DescriptorHeapType::Sampler))
            {
                pushOffset = pushDataEndOffset;
                m_setPushOffsets[space] = pushOffset;
                pushDataEndOffset += SET_PUSH_DATA_SIZE;
            }

            // Map each descriptor.
            for (auto& descriptor : layout->descriptors())
            {
                VkDescriptorSetAndBindingMappingEXT mapping{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
                    .descriptorSet = space,
                    .firstBinding = descriptor.binding(),
                    .bindingCount = 1u,
                    .resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT
                };

                if (descriptor.local())
                {
                    // Shader-local data is stored inline in the shader record, matching local root constants in D3D12.
                    if (descriptor.descriptorType() != DescriptorType::ConstantBuffer) [[unlikely]]
                        throw RuntimeException("The shader-local descriptor at binding {0} (space {1}) must be a constant buffer.", descriptor.binding(), space);

                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT;
                    mapping.sourceData.shaderRecordDataOffset = 0u;
                }
                else if (descriptor.staticSampler() != nullptr)
                {
                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
                    mapping.sourceData.constantOffset = { .pEmbeddedSampler = &descriptor.staticSampler()->createInfo() };
                }
                else
                {
                    auto range = allocationLayout.range(descriptor.binding());

                    if (!range.has_value()) [[unlikely]]
                        throw RuntimeException("The descriptor at binding {0} (space {1}) has no heap range in its descriptor set layout.", descriptor.binding(), space);

                    auto isSampler = range->Heap == DescriptorHeapType::Sampler;
                    auto stride = isSampler ? samplerStride : resourceStride;

                    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
                    mapping.sourceData.pushIndex = {
                        .heapOffset = range->RelativeOffset * stride,
                        .pushOffset = pushOffset + (isSampler ? SAMPLER_INDEX_OFFSET : 0u),
                        .heapIndexStride = stride,
                        .heapArrayStride = stride
                    };
                }

                m_mappings.push_back(mapping);
            }
        }

        // TODO: Remove those once DXC's native descriptor heap lowering is used.
        if (m_directResourceHeapAccess)
        {
            m_mappings.push_back(VkDescriptorSetAndBindingMappingEXT {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
                .descriptorSet = VulkanPipelineLayout::DESCRIPTOR_HEAP_SPACE,
                .firstBinding = VulkanPipelineLayout::RESOURCE_HEAP_BINDING,
                .bindingCount = 1u,
                .resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT,
                .source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
                .sourceData = { .constantOffset = { .heapOffset = 0u, .heapArrayStride = resourceStride } }
            });
        }
        
        if (m_directSamplerHeapAccess)
        {
            m_mappings.push_back(VkDescriptorSetAndBindingMappingEXT {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
                .descriptorSet = VulkanPipelineLayout::DESCRIPTOR_HEAP_SPACE,
                .firstBinding = VulkanPipelineLayout::SAMPLER_HEAP_BINDING,
                .bindingCount = 1u,
                .resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT,
                .source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
                .sourceData = { .constantOffset = { .heapOffset = 0u, .heapArrayStride = samplerStride } }
            });
        }

        if (pushDataEndOffset > m_device->maxPushDataSize()) [[unlikely]]
            throw RuntimeException("The pipeline layout requires {0} bytes of push data, but the device only supports {1} bytes.", pushDataEndOffset, m_device->maxPushDataSize());

        m_mappingInfo.mappingCount = static_cast<UInt32>(m_mappings.size());
        m_mappingInfo.pMappings = m_mappings.data();
    }

    void initialize([[maybe_unused]] const VulkanPipelineLayout& pipelineLayout, Array<SharedPtr<VulkanDescriptorSetLayout>> descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout, bool directResourceHeapAccess, bool directSamplerHeapAccess)
    {
        if (std::ranges::any_of(descriptorSetLayouts, [](const auto& layout) { return layout == nullptr; })) [[unlikely]]
            throw ArgumentNotInitializedException("descriptorSetLayouts", "At least one of the provided descriptor layouts is not initialized.");

        // Sort the descriptor sets by space, to allow for duplicate detection and stable push constant offsets.
        std::ranges::sort(descriptorSetLayouts, [](const auto& a, const auto& b) { return a->space() < b->space(); });

        // Check if there are descriptor sets occupying the same space.
        if (auto duplicate = std::ranges::adjacent_find(descriptorSetLayouts, {}, [](const auto& layout) { return layout->space(); }); duplicate != descriptorSetLayouts.end()) [[unlikely]]
            throw InvalidArgumentException("descriptorSetLayouts", "Two layouts defined for the same descriptor set {}. Each descriptor set must use it's own space.", (*duplicate)->space());

        // TODO: Remove those once DXC's native descriptor heap lowering is used.
        if (std::ranges::any_of(descriptorSetLayouts, [](const auto& layout) { return layout->space() == VulkanPipelineLayout::DESCRIPTOR_HEAP_SPACE; })) [[unlikely]]
            throw InvalidArgumentException("descriptorSetLayouts", "The descriptor set {} is reserved for direct heap access.", VulkanPipelineLayout::DESCRIPTOR_HEAP_SPACE);

        LITEFX_TRACE(VULKAN_LOG, "Creating pipeline layout {0} {{ Descriptor Sets: {1}, Push Constants: {2} bytes }}...", static_cast<const void*>(&pipelineLayout), descriptorSetLayouts.size(), pushConstantsLayout == nullptr ? 0u : pushConstantsLayout->size());

        // Store the layouts.
        m_pushConstantsLayout = std::move(pushConstantsLayout);
        m_descriptorSetLayouts.append_range(std::move(descriptorSetLayouts));
        m_directResourceHeapAccess = directResourceHeapAccess;
        m_directSamplerHeapAccess = directSamplerHeapAccess;

        // Map the descriptor bindings onto the global descriptor heaps.
        this->createMappings();
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout, bool directResourceHeapAccess, bool directSamplerHeapAccess) :
    m_impl(device)
{
    m_impl->initialize(*this, descriptorSetLayouts | std::ranges::to<std::vector>(), std::move(pushConstantsLayout), directResourceHeapAccess, directSamplerHeapAccess);
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
    return m_impl->m_directResourceHeapAccess;
}

bool VulkanPipelineLayout::dynamicSamplerHeapAccess() const
{
    return m_impl->m_directSamplerHeapAccess;
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
    instance->m_impl->initialize(*instance, std::move(this->state().descriptorSetLayouts), std::move(this->state().pushConstantsLayout), this->state().directResourceHeapAccess, this->state().directSamplerHeapAccess);
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