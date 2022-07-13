#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayout::VulkanPipelineLayoutImpl : public Implement<VulkanPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanComputePipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;
    const VulkanDevice& m_device;

public:
    VulkanPipelineLayoutImpl(VulkanPipelineLayout* parent, const VulkanDevice& device, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
        base(parent), m_device(device), m_shaderProgram(std::move(shaderProgram)), m_descriptorSetLayouts(std::move(descriptorLayouts)), m_pushConstantsLayout(std::move(pushConstantsLayout))
    {
    }

    VulkanPipelineLayoutImpl(VulkanPipelineLayout* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
    {
    }

public:
    VkPipelineLayout initialize()
    {
        // Query for the descriptor set layout handles.
        auto descriptorSetLayouts = m_descriptorSetLayouts |
            std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return std::as_const(*layout.get()).handle(); }) |
            ranges::to<Array<VkDescriptorSetLayout>>();

        // Query for push constant ranges.
        Array<const VulkanPushConstantsRange*> ranges = m_pushConstantsLayout == nullptr ? Array<const VulkanPushConstantsRange*>{} : m_pushConstantsLayout->ranges();
        auto rangeHandles = ranges |
            std::views::transform([](const VulkanPushConstantsRange* range) { return VkPushConstantRange{ .stageFlags = static_cast<VkShaderStageFlags>(Vk::getShaderStage(range->stage())), .offset = range->offset(), .size = range->size() }; }) |
            ranges::to<Array<VkPushConstantRange>>();

        // Create the pipeline layout.
        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0} {{ Descriptor Sets: {1}, Push Constant Ranges: {2} }}...", fmt::ptr(m_parent), descriptorSetLayouts.size(), rangeHandles.size());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<UInt32>(rangeHandles.size());
        pipelineLayoutInfo.pPushConstantRanges = rangeHandles.data();

        VkPipelineLayout layout;
        raiseIfFailed<RuntimeException>(::vkCreatePipelineLayout(m_device.handle(), &pipelineLayoutInfo, nullptr, &layout), "Unable to create pipeline layout.");
        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this, device, std::move(shaderProgram), std::move(descriptorSetLayouts), std::move(pushConstantsLayout))), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this, device)), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout() noexcept
{
    ::vkDestroyPipelineLayout(m_impl->m_device.handle(), this->handle(), nullptr);
}

const VulkanDevice& VulkanPipelineLayout::device() const noexcept
{
    return m_impl->m_device;
}

const VulkanShaderProgram& VulkanPipelineLayout::program() const noexcept
{
    return *m_impl->m_shaderProgram.get();
}

const VulkanDescriptorSetLayout& VulkanPipelineLayout::descriptorSet(const UInt32& space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No descriptor set layout uses the provided space {0}.", space);
}

Array<const VulkanDescriptorSetLayout*> VulkanPipelineLayout::descriptorSets() const noexcept
{
    return m_impl->m_descriptorSetLayouts |
        std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const VulkanDescriptorSetLayout*>>();
}

const VulkanPushConstantsLayout* VulkanPipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}