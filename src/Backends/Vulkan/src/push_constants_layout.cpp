#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsLayout::VulkanPushConstantsLayoutImpl : public Implement<VulkanPushConstantsLayout> {
public:
    friend class VulkanRenderPipelinePushConstantsLayoutBuilder;
    friend class VulkanComputePipelinePushConstantsLayoutBuilder;
    friend class VulkanPushConstantsLayout;

private:
    Dictionary<ShaderStage, VulkanPushConstantsRange*> m_ranges;
    Array<UniquePtr<VulkanPushConstantsRange>> m_rangePointers;
    UInt32 m_size;
    const VulkanPipelineLayout& m_pipelineLayout;

public:
    VulkanPushConstantsLayoutImpl(VulkanPushConstantsLayout* parent, const VulkanPipelineLayout& pipelineLayout, const UInt32& size) :
        base(parent), m_size(size), m_pipelineLayout(pipelineLayout)
    {
        // Align the size to 4 bytes.
        m_size = size % 4 == 0 ? (size + 4 - 1) & ~(size - 1) : size;

        // Issue a warning, if the size is too large.
        if (m_size > 128)
            LITEFX_WARNING(VULKAN_LOG, "The push constant layout backing memory is defined with a size greater than 128 bytes. Blocks larger than 128 bytes are not forbidden, but also not guaranteed to be supported on all hardware.");
    }

private:
    void setRanges(Array<UniquePtr<VulkanPushConstantsRange>>&& ranges)
    {
        m_rangePointers = std::move(ranges);

        std::ranges::for_each(m_rangePointers, [this](const UniquePtr<VulkanPushConstantsRange>& range) {
            if (m_ranges.contains(static_cast<ShaderStage>(range->stage())))
                throw InvalidArgumentException("Only one push constant range can be mapped to a shader stage.");

            m_ranges[range->stage()] = range.get();
        });
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsLayout::VulkanPushConstantsLayout(const VulkanPipelineLayout& pipelineLayout, Array<UniquePtr<VulkanPushConstantsRange>>&& ranges, const UInt32& size) :
    m_impl(makePimpl<VulkanPushConstantsLayoutImpl>(this, pipelineLayout, size))
{
    m_impl->setRanges(std::move(ranges));
}

VulkanPushConstantsLayout::VulkanPushConstantsLayout(const VulkanPipelineLayout& pipelineLayout, const UInt32& size) :
    m_impl(makePimpl<VulkanPushConstantsLayoutImpl>(this, pipelineLayout, size))
{
}

VulkanPushConstantsLayout::~VulkanPushConstantsLayout() noexcept = default;

const VulkanPipelineLayout& VulkanPushConstantsLayout::pipelineLayout() const noexcept
{
    return m_impl->m_pipelineLayout;
}

const UInt32& VulkanPushConstantsLayout::size() const noexcept
{
    return m_impl->m_size;
}

const VulkanPushConstantsRange& VulkanPushConstantsLayout::range(const ShaderStage& stage) const
{
    if (!(static_cast<UInt32>(stage) && !(static_cast<UInt32>(stage) & (static_cast<UInt32>(stage) - 1))))
        throw ArgumentOutOfRangeException("The stage mask must only contain one shader stage.");

    if (!m_impl->m_ranges.contains(stage))
        throw InvalidArgumentException("No push constant range has been associated with the provided shader stage.");

    return *m_impl->m_ranges[stage];
}

Array<const VulkanPushConstantsRange*> VulkanPushConstantsLayout::ranges() const noexcept
{
    return m_impl->m_rangePointers |
        std::views::transform([](const UniquePtr<VulkanPushConstantsRange>& range) { return range.get(); }) |
        ranges::to<Array<const VulkanPushConstantsRange*>>();
}