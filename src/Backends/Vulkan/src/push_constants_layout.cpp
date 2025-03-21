#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsLayout::VulkanPushConstantsLayoutImpl {
public:
    friend class VulkanPushConstantsLayoutBuilder;
    friend class VulkanPushConstantsLayout;

private:
    Dictionary<ShaderStage, VulkanPushConstantsRange*> m_ranges;
    Array<UniquePtr<VulkanPushConstantsRange>> m_rangePointers;
    UInt32 m_size;

public:
    VulkanPushConstantsLayoutImpl(UInt32 size) :
        m_size(size % 4 == 0 ? (size + 4 - 1) & ~(size - 1) : size) // Align the size to 4 bytes.
    {
        constexpr UInt32 MAX_GUARANTEED_RANGE_SIZE = 128;

        // Issue a warning, if the size is too large.
        if (m_size > MAX_GUARANTEED_RANGE_SIZE)
            LITEFX_WARNING(VULKAN_LOG, "The push constant layout backing memory is defined with a size greater than 128 bytes. Blocks larger than 128 bytes are not forbidden, but also not guaranteed to be supported on all hardware.");
    }

private:
    void setRanges(Enumerable<UniquePtr<VulkanPushConstantsRange>>&& ranges)
    {
        m_rangePointers = std::move(ranges) | std::views::as_rvalue | std::ranges::to<std::vector>();

        std::ranges::for_each(m_rangePointers, [this](const UniquePtr<VulkanPushConstantsRange>& range) {
            if (m_ranges.contains(static_cast<ShaderStage>(range->stage())))
                throw InvalidArgumentException("ranges", "Only one push constant range can be mapped to a shader stage.");

            m_ranges[range->stage()] = range.get();
        });
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsLayout::VulkanPushConstantsLayout(Enumerable<UniquePtr<VulkanPushConstantsRange>>&& ranges, UInt32 size) :
    m_impl(size)
{
    m_impl->setRanges(std::move(ranges));
}

VulkanPushConstantsLayout::VulkanPushConstantsLayout(UInt32 size) :
    m_impl(size)
{
}

VulkanPushConstantsLayout::VulkanPushConstantsLayout(VulkanPushConstantsLayout&&) noexcept = default;
VulkanPushConstantsLayout& VulkanPushConstantsLayout::operator=(VulkanPushConstantsLayout&&) noexcept = default;
VulkanPushConstantsLayout::~VulkanPushConstantsLayout() noexcept = default;

UInt32 VulkanPushConstantsLayout::size() const noexcept
{
    return m_impl->m_size;
}

const VulkanPushConstantsRange& VulkanPushConstantsLayout::range(ShaderStage stage) const
{
    if (!(std::to_underlying(stage) && !(std::to_underlying(stage) & (std::to_underlying(stage) - 1))))
        throw InvalidArgumentException("stage", "The stage mask must only contain one shader stage.");

    if (!m_impl->m_ranges.contains(stage))
        throw InvalidArgumentException("stage", "No push constant range has been associated with the provided shader stage.");

    return *m_impl->m_ranges[stage];
}

const Array<UniquePtr<VulkanPushConstantsRange>>& VulkanPushConstantsLayout::ranges() const
{
    return m_impl->m_rangePointers;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Push constants layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsLayoutBuilder::VulkanPushConstantsLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 size) :
    PushConstantsLayoutBuilder(parent, UniquePtr<VulkanPushConstantsLayout>(new VulkanPushConstantsLayout(size)))
{
}

VulkanPushConstantsLayoutBuilder::~VulkanPushConstantsLayoutBuilder() noexcept = default;

void VulkanPushConstantsLayoutBuilder::build()
{
    this->instance()->m_impl->setRanges(this->state().ranges | std::views::as_rvalue | std::ranges::to<Enumerable<UniquePtr<VulkanPushConstantsRange>>>());
}

UniquePtr<VulkanPushConstantsRange> VulkanPushConstantsLayoutBuilder::makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding)
{
    return makeUnique<VulkanPushConstantsRange>(shaderStages, offset, size, space, binding);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)