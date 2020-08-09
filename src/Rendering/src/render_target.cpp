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
    RenderTargetImpl(RenderTarget* parent) : base(parent) { }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget() :
    m_impl(makePimpl<RenderTargetImpl>(this))
{
}

RenderTarget::~RenderTarget() noexcept = default;

RenderTargetType RenderTarget::getType() const noexcept 
{
    return m_impl->m_type;
}

void RenderTarget::setType(const RenderTargetType& type) 
{
    m_impl->m_type = type;
}

MultiSamplingLevel RenderTarget::getSamples() const noexcept
{
    return m_impl->m_samples;
}

void RenderTarget::setSamples(const MultiSamplingLevel& samples)
{
    m_impl->m_samples = samples;
}

bool RenderTarget::getClearBuffer() const noexcept 
{
    return m_impl->m_clearBuffer;
}

void RenderTarget::setClearBuffer(const bool& clear) 
{
    m_impl->m_clearBuffer = clear;
}

bool RenderTarget::getClearStencil() const noexcept 
{
    return m_impl->m_clearStencil;
}

void RenderTarget::setClearStencil(const bool& clear) 
{
    m_impl->m_clearStencil = clear;
}

Format RenderTarget::getFormat() const noexcept
{
    return m_impl->m_format;
}

void RenderTarget::setFormat(const Format& format)
{
    m_impl->m_format = format;
}

bool RenderTarget::getVolatile() const noexcept 
{
    return m_impl->m_volatile;
}

void RenderTarget::setVolatile(const bool& isVolatile) 
{
    m_impl->m_volatile = isVolatile;
}

const Vector4f& RenderTarget::getClearValues() const noexcept
{
    return m_impl->m_clearValues;
}

void RenderTarget::setClearValues(const Vector4f& values)
{
    m_impl->m_clearValues = values;
}