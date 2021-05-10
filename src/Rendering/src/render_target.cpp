#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderTarget::RenderTargetImpl : public Implement<RenderTarget> {
public:
    friend class RenderTarget;

private:
    RenderTargetType m_type = RenderTargetType::Color;
    Format m_format = Format::B8G8R8A8_SRGB;
    MultiSamplingLevel m_samples = MultiSamplingLevel::x1;
    bool m_clearBuffer = false, m_clearStencil = false, m_volatile = false;
    Vector4f m_clearValues;

public:
    RenderTargetImpl(RenderTarget* parent, const RenderTargetType& type, const Format& format, const bool& clearBuffer, const Vector4f& clearValues, const bool& clearStencil, const MultiSamplingLevel& samples, const bool& isVolatile) :
        base(parent), m_type(type), m_format(format), m_clearBuffer(clearBuffer), m_clearValues(clearValues), m_clearStencil(clearStencil), m_samples(samples), m_volatile(isVolatile)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget(const RenderTargetType& type, const Format& format, const bool& clearBuffer, const Vector4f& clearValues, const bool& clearStencil, const MultiSamplingLevel& samples, const bool& isVolatile) :
    m_impl(makePimpl<RenderTargetImpl>(this, type, format, clearBuffer, clearValues, clearStencil, samples, isVolatile))
{
}

RenderTarget::~RenderTarget() noexcept = default;

const RenderTargetType& RenderTarget::type() const noexcept
{
    return m_impl->m_type;
}

const MultiSamplingLevel& RenderTarget::samples() const noexcept
{
    return m_impl->m_samples;
}

const Format& RenderTarget::format() const noexcept
{
    return m_impl->m_format;
}

const bool& RenderTarget::clearBuffer() const noexcept
{
    return m_impl->m_clearBuffer;
}

const bool& RenderTarget::clearStencil() const noexcept
{
    return m_impl->m_clearStencil;
}

const Vector4f& RenderTarget::clearValues() const noexcept
{
    return m_impl->m_clearValues;
}

const bool& RenderTarget::isVolatile() const noexcept
{
    return m_impl->m_volatile;
}