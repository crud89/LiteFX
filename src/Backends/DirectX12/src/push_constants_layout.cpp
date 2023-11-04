#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PushConstantsLayout::DirectX12PushConstantsLayoutImpl : public Implement<DirectX12PushConstantsLayout> {
public:
    friend class DirectX12PushConstantsLayoutBuilder;
    friend class DirectX12PushConstantsLayout;

private:
    Dictionary<ShaderStage, DirectX12PushConstantsRange*> m_ranges;
    Array<UniquePtr<DirectX12PushConstantsRange>> m_rangePointers;
    UInt32 m_size;

public:
    DirectX12PushConstantsLayoutImpl(DirectX12PushConstantsLayout* parent, const UInt32& size) :
        base(parent), m_size(size)
    {
        // Align the size to 4 bytes.
        m_size = size % 4 == 0 ? (size + 4 - 1) & ~(size - 1) : size;

        // Issue a warning, if the size is too large.
        if (m_size > 128)
            LITEFX_WARNING(DIRECTX12_LOG, "The push constant layout backing memory is defined with a size greater than 128 bytes. Blocks larger than 128 bytes are not forbidden, but also not guaranteed to be supported on all hardware.");
    }

private:
    void setRanges(Enumerable<UniquePtr<DirectX12PushConstantsRange>>&& ranges)
    {
        m_rangePointers = ranges | std::views::as_rvalue | std::ranges::to<std::vector>();

        std::ranges::for_each(m_rangePointers, [this](const UniquePtr<DirectX12PushConstantsRange>& range) {
            if (m_ranges.contains(static_cast<ShaderStage>(range->stage())))
                throw InvalidArgumentException("Only one push constant range can be mapped to a shader stage.");

            m_ranges[range->stage()] = range.get();
        });
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12PushConstantsLayout::DirectX12PushConstantsLayout(Enumerable<UniquePtr<DirectX12PushConstantsRange>>&& ranges, const UInt32& size) :
    m_impl(makePimpl<DirectX12PushConstantsLayoutImpl>(this, size))
{
    m_impl->setRanges(std::move(ranges));
}

DirectX12PushConstantsLayout::DirectX12PushConstantsLayout(const UInt32& size) :
    m_impl(makePimpl<DirectX12PushConstantsLayoutImpl>(this, size))
{
}

DirectX12PushConstantsLayout::~DirectX12PushConstantsLayout() noexcept = default;

const UInt32& DirectX12PushConstantsLayout::size() const noexcept
{
    return m_impl->m_size;
}

const DirectX12PushConstantsRange& DirectX12PushConstantsLayout::range(ShaderStage stage) const
{
    auto bits = std::to_underlying(stage);

    if (!(bits && !(bits & (bits - 1))))
        throw ArgumentOutOfRangeException("The stage mask must only contain one shader stage.");

    if (!m_impl->m_ranges.contains(stage))
        throw InvalidArgumentException("No push constant range has been associated with the provided shader stage.");

    return *m_impl->m_ranges[stage];
}

Enumerable<const DirectX12PushConstantsRange*> DirectX12PushConstantsLayout::ranges() const noexcept
{
    return m_impl->m_rangePointers | std::views::transform([](const UniquePtr<DirectX12PushConstantsRange>& range) { return range.get(); });
}

Enumerable<DirectX12PushConstantsRange*> DirectX12PushConstantsLayout::ranges() noexcept
{
    return m_impl->m_rangePointers | std::views::transform([](UniquePtr<DirectX12PushConstantsRange>& range) { return range.get(); });
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Push constants layout builder shared interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12PushConstantsLayoutBuilder::DirectX12PushConstantsLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 size) :
    PushConstantsLayoutBuilder(parent, UniquePtr<DirectX12PushConstantsLayout>(new DirectX12PushConstantsLayout(size)))
{
}

constexpr DirectX12PushConstantsLayoutBuilder::~DirectX12PushConstantsLayoutBuilder() noexcept = default;

void DirectX12PushConstantsLayoutBuilder::build()
{
    this->instance()->m_impl->setRanges(std::move(m_state.ranges | std::views::as_rvalue | std::ranges::to<Enumerable<UniquePtr<DirectX12PushConstantsRange>>>()));
}

constexpr UniquePtr<DirectX12PushConstantsRange> DirectX12PushConstantsLayoutBuilder::makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding)
{
    return makeUnique<DirectX12PushConstantsRange>(shaderStages, offset, size, space, binding);
}
#endif // defined(BUILD_DEFINE_BUILDERS)