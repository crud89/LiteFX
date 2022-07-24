#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PushConstantsLayout::DirectX12PushConstantsLayoutImpl : public Implement<DirectX12PushConstantsLayout> {
public:
    friend class DirectX12RenderPipelinePushConstantsLayoutBuilder;
    friend class DirectX12ComputePipelinePushConstantsLayoutBuilder;
    friend class DirectX12PushConstantsLayout;

private:
    Dictionary<ShaderStage, DirectX12PushConstantsRange*> m_ranges;
    Array<UniquePtr<DirectX12PushConstantsRange>> m_rangePointers;
    UInt32 m_size;
    const DirectX12PipelineLayout& m_pipelineLayout;

public:
    DirectX12PushConstantsLayoutImpl(DirectX12PushConstantsLayout* parent, const DirectX12PipelineLayout& pipelineLayout, const UInt32& size) :
        base(parent), m_pipelineLayout(pipelineLayout), m_size(size)
    {
        // Align the size to 4 bytes.
        m_size = size % 4 == 0 ? (size + 4 - 1) & ~(size - 1) : size;

        // Issue a warning, if the size is too large.
        if (m_size > 128)
            LITEFX_WARNING(DIRECTX12_LOG, "The push constant layout backing memory is defined with a size greater than 128 bytes. Blocks larger than 128 bytes are not forbidden, but also not guaranteed to be supported on all hardware.");
    }

private:
    void setRanges(Array<UniquePtr<DirectX12PushConstantsRange>>&& ranges)
    {
        m_rangePointers = std::move(ranges);

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

DirectX12PushConstantsLayout::DirectX12PushConstantsLayout(const DirectX12PipelineLayout& parent, Array<UniquePtr<DirectX12PushConstantsRange>>&& ranges, const UInt32& size) :
    m_impl(makePimpl<DirectX12PushConstantsLayoutImpl>(this, parent, size))
{
    m_impl->setRanges(std::move(ranges));
}

DirectX12PushConstantsLayout::DirectX12PushConstantsLayout(const DirectX12PipelineLayout& parent, const UInt32& size) :
    m_impl(makePimpl<DirectX12PushConstantsLayoutImpl>(this, parent, size))
{
}

DirectX12PushConstantsLayout::~DirectX12PushConstantsLayout() noexcept = default;

const UInt32& DirectX12PushConstantsLayout::size() const noexcept
{
    return m_impl->m_size;
}

const DirectX12PushConstantsRange& DirectX12PushConstantsLayout::range(const ShaderStage& stage) const
{
    auto bits = static_cast<UInt32>(stage);

    if (!(bits && !(bits & (bits - 1))))
        throw ArgumentOutOfRangeException("The stage mask must only contain one shader stage.");

    if (!m_impl->m_ranges.contains(stage))
        throw InvalidArgumentException("No push constant range has been associated with the provided shader stage.");

    return *m_impl->m_ranges[stage];
}

Array<const DirectX12PushConstantsRange*> DirectX12PushConstantsLayout::ranges() const noexcept
{
    return m_impl->m_rangePointers |
        std::views::transform([](const UniquePtr<DirectX12PushConstantsRange>& range) { return range.get(); }) |
        ranges::to<Array<const DirectX12PushConstantsRange*>>();
}

Array<DirectX12PushConstantsRange*> DirectX12PushConstantsLayout::ranges() noexcept
{
    return m_impl->m_rangePointers |
        std::views::transform([](UniquePtr<DirectX12PushConstantsRange>& range) { return range.get(); }) |
        ranges::to<Array<DirectX12PushConstantsRange*>>();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipelinePushConstantsLayoutBuilder::DirectX12RenderPipelinePushConstantsLayoutBuilderImpl : public Implement<DirectX12RenderPipelinePushConstantsLayoutBuilder> {
public:
    friend class DirectX12RenderPipelinePushConstantsLayoutBuilder;

private:
    Array<UniquePtr<DirectX12PushConstantsRange>> m_ranges;
    UInt32 m_size;

public:
    DirectX12RenderPipelinePushConstantsLayoutBuilderImpl(DirectX12RenderPipelinePushConstantsLayoutBuilder* parent, const UInt32& size) :
        base(parent), m_size(size)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelinePushConstantsLayoutBuilder::DirectX12RenderPipelinePushConstantsLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& size) :
    m_impl(makePimpl<DirectX12RenderPipelinePushConstantsLayoutBuilderImpl>(this, size)), PushConstantsLayoutBuilder(parent, UniquePtr<DirectX12PushConstantsLayout>(new DirectX12PushConstantsLayout(*std::as_const(parent).instance(), size)))
{
}

DirectX12RenderPipelinePushConstantsLayoutBuilder::~DirectX12RenderPipelinePushConstantsLayoutBuilder() noexcept = default;

DirectX12RenderPipelineLayoutBuilder& DirectX12RenderPipelinePushConstantsLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->setRanges(std::move(m_impl->m_ranges));

    return PushConstantsLayoutBuilder::go();
}

DirectX12RenderPipelinePushConstantsLayoutBuilder& DirectX12RenderPipelinePushConstantsLayoutBuilder::addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding)
{
    m_impl->m_ranges.push_back(makeUnique<DirectX12PushConstantsRange>(shaderStages, offset, size, space, binding));
    return *this;
}
#endif // defined(BUILD_DEFINE_BUILDERS)