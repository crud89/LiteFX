#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class Rasterizer::RasterizerImpl {
private:
    PolygonMode m_polygonMode = PolygonMode::Solid;
    CullMode m_cullMode = CullMode::BackFaces;
    CullOrder m_cullOrder = CullOrder::CounterClockWise;
    Float m_lineWidth = 1.f;
    Float m_depthBiasClamp = 0.f, m_depthBiasConstantFactor = 0.f, m_depthBiasSlopeFactor = 0.f;
    bool m_depthBias = false;

public:
    RasterizerImpl() noexcept = default;
    
public:
    PolygonMode getPolygonMode() const noexcept
    {
        return m_polygonMode;
    }

    void setPolygonMode(const PolygonMode& mode) noexcept
    {
        m_polygonMode = mode;
    }

    CullMode getCullMode() const noexcept
    {
        return m_cullMode;
    }

    void setCullMode(const CullMode& mode) noexcept
    {
        m_cullMode = mode;
    }

    CullOrder getCullOrder() const noexcept
    {
        return m_cullOrder;
    }

    void setCullOrder(const CullOrder& order) noexcept
    {
        m_cullOrder = order;
    }

    Float getLineWidth() const noexcept
    {
        return m_lineWidth;
    }

    void setLineWidth(const Float& width) noexcept
    {
        m_lineWidth = width;
    }

    bool getDepthBiasEnabled() const noexcept
    {
        return m_depthBias;
    }

    void setDepthBiasEnabled(const bool& enable) noexcept
    {
        m_depthBias = enable;
    }

    float getDepthBiasClamp() const noexcept
    {
        return m_depthBiasClamp;
    }

    void setDepthBiasClamp(const Float& clamp) noexcept
    {
        m_depthBiasClamp = clamp;
    }

    float getDepthBiasConstantFactor() const noexcept
    {
        return m_depthBiasConstantFactor;
    }

    void setDepthBiasConstantFactor(const Float& factor) noexcept
    {
        m_depthBiasConstantFactor = factor;
    }

    float getDepthBiasSlopeFactor() const noexcept
    {
        return m_depthBiasSlopeFactor;
    }

    void setDepthBiasSlopeFactor(const Float& factor) noexcept
    {
        m_depthBiasSlopeFactor = factor;
    }
};


Rasterizer::Rasterizer() noexcept :
    m_impl(makePimpl<RasterizerImpl>())
{
}

Rasterizer::~Rasterizer() noexcept = default;

PolygonMode Rasterizer::getPolygonMode() const noexcept 
{
    return m_impl->getPolygonMode();
}

void Rasterizer::setPolygonMode(const PolygonMode& mode) noexcept
{
    m_impl->setPolygonMode(mode);
}

CullMode Rasterizer::getCullMode() const noexcept 
{
    return m_impl->getCullMode();
}

void Rasterizer::setCullMode(const CullMode& mode) noexcept 
{
    m_impl->setCullMode(mode);
}

CullOrder Rasterizer::getCullOrder() const noexcept 
{
    return m_impl->getCullOrder();
}

void Rasterizer::setCullOrder(const CullOrder& order) noexcept 
{
    m_impl->setCullOrder(order);
}

Float Rasterizer::getLineWidth() const noexcept 
{
    return m_impl->getLineWidth();
}

void Rasterizer::setLineWidth(const Float& width) noexcept 
{
    m_impl->setLineWidth(width);
}

bool Rasterizer::getDepthBiasEnabled() const noexcept 
{
    return m_impl->getDepthBiasEnabled();
}

void Rasterizer::setDepthBiasEnabled(const bool& enable) noexcept
{
    m_impl->setDepthBiasEnabled(enable);
}

float Rasterizer::getDepthBiasClamp() const noexcept 
{
    return m_impl->getDepthBiasClamp();
}

void Rasterizer::setDepthBiasClamp(const Float& clamp) noexcept
{
    m_impl->setDepthBiasClamp(clamp);
}

float Rasterizer::getDepthBiasConstantFactor() const noexcept 
{
    return m_impl->getDepthBiasConstantFactor();
}

void Rasterizer::setDepthBiasConstantFactor(const Float& factor) noexcept
{
    m_impl->setDepthBiasConstantFactor(factor);
}

float Rasterizer::getDepthBiasSlopeFactor() const noexcept
{
    return m_impl->getDepthBiasSlopeFactor();
}

void Rasterizer::setDepthBiasSlopeFactor(const Float& factor) noexcept
{
    m_impl->setDepthBiasSlopeFactor(factor);
}