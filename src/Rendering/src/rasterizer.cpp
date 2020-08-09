#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class Rasterizer::RasterizerImpl : public Implement<Rasterizer> {
public:
    friend class Rasterizer;

private:
    PolygonMode m_polygonMode = PolygonMode::Solid;
    CullMode m_cullMode = CullMode::BackFaces;
    CullOrder m_cullOrder = CullOrder::CounterClockWise;
    Float m_lineWidth = 1.f;
    Float m_depthBiasClamp = 0.f, m_depthBiasConstantFactor = 0.f, m_depthBiasSlopeFactor = 0.f;
    bool m_depthBias = false;

public:
    RasterizerImpl(Rasterizer* parent) : base(parent) { }
};

Rasterizer::Rasterizer() :
    m_impl(makePimpl<RasterizerImpl>(this))
{
}

Rasterizer::~Rasterizer() noexcept = default;

PolygonMode Rasterizer::getPolygonMode() const noexcept 
{
    return m_impl->m_polygonMode;
}

void Rasterizer::setPolygonMode(const PolygonMode& mode) noexcept
{
    m_impl->m_polygonMode = mode;
}

CullMode Rasterizer::getCullMode() const noexcept 
{
    return m_impl->m_cullMode;
}

void Rasterizer::setCullMode(const CullMode& mode) noexcept 
{
    m_impl->m_cullMode = mode;
}

CullOrder Rasterizer::getCullOrder() const noexcept 
{
    return m_impl->m_cullOrder;
}

void Rasterizer::setCullOrder(const CullOrder& order) noexcept 
{
    m_impl->m_cullOrder = order;
}

Float Rasterizer::getLineWidth() const noexcept 
{
    return m_impl->m_lineWidth;
}

void Rasterizer::setLineWidth(const Float& width) noexcept 
{
    m_impl->m_lineWidth = width;
}

bool Rasterizer::getDepthBiasEnabled() const noexcept 
{
    return m_impl->m_depthBias;
}

void Rasterizer::setDepthBiasEnabled(const bool& enable) noexcept
{
    m_impl->m_depthBias = enable;
}

float Rasterizer::getDepthBiasClamp() const noexcept 
{
    return m_impl->m_depthBiasClamp;
}

void Rasterizer::setDepthBiasClamp(const Float& clamp) noexcept
{
    m_impl->m_depthBiasClamp = clamp;
}

float Rasterizer::getDepthBiasConstantFactor() const noexcept 
{
    return m_impl->m_depthBiasConstantFactor;
}

void Rasterizer::setDepthBiasConstantFactor(const Float& factor) noexcept
{
    m_impl->m_depthBiasConstantFactor = factor;
}

float Rasterizer::getDepthBiasSlopeFactor() const noexcept
{
    return m_impl->m_depthBiasSlopeFactor;
}

void Rasterizer::setDepthBiasSlopeFactor(const Float& factor) noexcept
{
    m_impl->m_depthBiasSlopeFactor = factor;
}