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
    const VulkanDevice& m_device;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanPipelineLayoutImpl(const VulkanDevice& device, Enumerable<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
        m_device(device), m_pushConstantsLayout(std::move(pushConstantsLayout))
    {
        m_descriptorSetLayouts = descriptorLayouts | std::views::as_rvalue | std::ranges::to<std::vector>();
    }

    VulkanPipelineLayoutImpl(const VulkanDevice& device) :
        m_device(device)
    {
    }

public:
    VkPipelineLayout initialize([[maybe_unused]] const VulkanPipelineLayout& pipelineLayout)
    {
        // Since Vulkan does not know spaces, descriptor sets are mapped to their indices based on the order they are defined. Hence we need to sort the descriptor set layouts accordingly.
        std::ranges::sort(m_descriptorSetLayouts, [](const UniquePtr<VulkanDescriptorSetLayout>& a, const UniquePtr<VulkanDescriptorSetLayout>& b) { return a->space() < b->space(); });

        // Find unused and duplicate descriptor sets. Initialize with all the set indices up until the first set index is reached.
        Array<UInt32> emptySets;

        if (!m_descriptorSetLayouts.empty())
            emptySets.append_range(std::views::iota(0u, m_descriptorSetLayouts.front()->space()));

        for (Tuple<UInt32, UInt32> spaces : m_descriptorSetLayouts | std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout->space(); }) | std::views::adjacent_transform<2>([](UInt32 a, UInt32 b) { return std::make_tuple(a, b); }))
        {
            auto [a, b] = spaces;

            if (a == b) [[unlikely]]
                throw InvalidArgumentException("descriptorSetLayouts", "Two layouts defined for the same descriptor set {}. Each descriptor set must use it's own space.", a);

            // Fill space until we reached the last descriptor set.
            emptySets.append_range(std::views::iota(a + 1, b));
        }

        // Add empty sets.
        if (!emptySets.empty())
        {
            for (auto s : emptySets)
                m_descriptorSetLayouts.push_back(UniquePtr<VulkanDescriptorSetLayout>{ new VulkanDescriptorSetLayout(m_device, { }, s, ShaderStage::Any) }); // No descriptor can ever be allocated from an empty descriptor set.

            // Re-order them.
            std::ranges::sort(m_descriptorSetLayouts, [](const UniquePtr<VulkanDescriptorSetLayout>& a, const UniquePtr<VulkanDescriptorSetLayout>& b) { return a->space() < b->space(); });
        }

        // Store the pipeline layout on the push constants.
        if (m_pushConstantsLayout != nullptr)
            m_pushConstantsLayout->pipelineLayout(pipelineLayout);

        // Query for the descriptor set layout handles.
        auto descriptorSetLayouts = m_descriptorSetLayouts |
            std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return std::as_const(*layout.get()).handle(); }) |
            std::ranges::to<Array<VkDescriptorSetLayout>>();

        // Query for push constant ranges.
        auto ranges = m_pushConstantsLayout == nullptr ? Enumerable<const VulkanPushConstantsRange*>{} : m_pushConstantsLayout->ranges();
        auto rangeHandles = ranges |
            std::views::transform([](const VulkanPushConstantsRange* range) { return VkPushConstantRange{ .stageFlags = static_cast<VkShaderStageFlags>(Vk::getShaderStage(range->stage())), .offset = range->offset(), .size = range->size() }; }) |
            std::ranges::to<Array<VkPushConstantRange>>();

        // Create the pipeline layout.
        LITEFX_TRACE(VULKAN_LOG, "Creating pipeline layout {0} {{ Descriptor Sets: {1}, Push Constant Ranges: {2} }}...", static_cast<const void*>(&pipelineLayout), descriptorSetLayouts.size(), rangeHandles.size());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<UInt32>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<UInt32>(rangeHandles.size());
        pipelineLayoutInfo.pPushConstantRanges = rangeHandles.data();

        VkPipelineLayout layout;
        raiseIfFailed(::vkCreatePipelineLayout(m_device.handle(), &pipelineLayoutInfo, nullptr, &layout), "Unable to create pipeline layout.");
        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device, Enumerable<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
    Resource<VkPipelineLayout>(VK_NULL_HANDLE), m_impl(device, std::move(descriptorSetLayouts), std::move(pushConstantsLayout))
{
    this->handle() = m_impl->initialize(*this);
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device) noexcept :
    Resource<VkPipelineLayout>(VK_NULL_HANDLE), m_impl(device)
{
}

VulkanPipelineLayout::VulkanPipelineLayout(VulkanPipelineLayout&&) noexcept = default;
VulkanPipelineLayout& VulkanPipelineLayout::operator=(VulkanPipelineLayout&&) noexcept = default;

VulkanPipelineLayout::~VulkanPipelineLayout() noexcept
{
    ::vkDestroyPipelineLayout(m_impl->m_device.handle(), this->handle(), nullptr);
}

const VulkanDevice& VulkanPipelineLayout::device() const noexcept
{
    return m_impl->m_device;
}

const VulkanDescriptorSetLayout& VulkanPipelineLayout::descriptorSet(UInt32 space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw InvalidArgumentException("space", "No descriptor set layout uses the provided space {0}.", space);
}

Enumerable<const VulkanDescriptorSetLayout*> VulkanPipelineLayout::descriptorSets() const noexcept
{
    return m_impl->m_descriptorSetLayouts | std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout.get(); });
}

const VulkanPushConstantsLayout* VulkanPipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Pipeline layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayoutBuilder::VulkanPipelineLayoutBuilderImpl {
public:
    friend class VulkanPipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    const VulkanDevice& m_device;

public:
    VulkanPipelineLayoutBuilderImpl(const VulkanDevice& device) :
        m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayoutBuilder::VulkanPipelineLayoutBuilder(const VulkanDevice& parent) :
    PipelineLayoutBuilder(SharedPtr<VulkanPipelineLayout>(new VulkanPipelineLayout(parent))), m_impl(parent)
{
}

VulkanPipelineLayoutBuilder::~VulkanPipelineLayoutBuilder() noexcept = default;

void VulkanPipelineLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorSetLayouts = std::move(m_state.descriptorSetLayouts);
    instance->m_impl->m_pushConstantsLayout = std::move(m_state.pushConstantsLayout);
    instance->handle() = instance->m_impl->initialize(*instance);
}

VulkanDescriptorSetLayoutBuilder VulkanPipelineLayoutBuilder::descriptorSet(UInt32 space, ShaderStage stages)
{
    return VulkanDescriptorSetLayoutBuilder(*this, space, stages);
}

VulkanPushConstantsLayoutBuilder VulkanPipelineLayoutBuilder::pushConstants(UInt32 size)
{
    return VulkanPushConstantsLayoutBuilder(*this, size);
}

const VulkanDevice& VulkanPipelineLayoutBuilder::device() const noexcept
{
    return m_impl->m_device;
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)