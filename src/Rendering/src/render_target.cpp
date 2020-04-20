#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderTarget::RenderTargetImpl {
private:
    RenderTargetType m_type = RenderTargetType::Color;
    int m_samples = 1;
    bool m_clearBuffer = false, m_clearStencil = false, m_volatile = false;

public:
    RenderTargetImpl() = default;

public:
    RenderTargetType getType() const noexcept
    {
        return m_type;
    }

    void setType(const RenderTargetType & type)
    {
        m_type = type;
    }

    int getSamples() const noexcept
    {
        return m_samples;
    }

    void setSamples(const int& samples)
    {
        m_samples = samples;
    }

    bool getClearBuffer() const noexcept
    {
        return m_clearBuffer;
    }

    void setClearBuffer(const bool& clear)
    {
        m_clearBuffer = clear;
    }

    bool getClearStencil() const noexcept
    {
        return m_clearStencil;
    }

    void setClearStencil(const bool& clear)
    {
        m_clearStencil = clear;
    }

    bool getVolatile() const noexcept
    {
        return m_volatile;
    }

    void setVolatile(const bool& isVolatile)
    {
        m_volatile = isVolatile;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget() :
    m_impl(makePimpl<RenderTargetImpl>())
{
}

RenderTarget::~RenderTarget() noexcept = default;

RenderTargetType RenderTarget::getType() const noexcept 
{
    return m_impl->getType();
}

void RenderTarget::setType(const RenderTargetType& type) 
{
    m_impl->setType(type);
}

int RenderTarget::getSamples() const noexcept 
{
    return m_impl->getSamples();
}

void RenderTarget::setSamples(const int& samples) 
{
    m_impl->setSamples(samples);
}

bool RenderTarget::getClearBuffer() const noexcept 
{
    return m_impl->getClearBuffer();
}

void RenderTarget::setClearBuffer(const bool& clear) 
{
    m_impl->setClearBuffer(clear);
}

bool RenderTarget::getClearStencil() const noexcept 
{
    return m_impl->getClearStencil();
}

void RenderTarget::setClearStencil(const bool& clear) 
{
    m_impl->setClearStencil(clear);
}

bool RenderTarget::getVolatile() const noexcept 
{
    return m_impl->getVolatile();
}

void RenderTarget::setVolatile(const bool& isVolatile) 
{
    m_impl->setVolatile(isVolatile);
}