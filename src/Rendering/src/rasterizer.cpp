#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

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
    RasterizerImpl(Rasterizer* parent, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const bool& useDepthBias, const Float& depthBiasClamp, const Float& depthBiasConstantFactor, const Float& depthBiasSlopeFactor) :
        base(parent), m_polygonMode(polygonMode), m_cullMode(cullMode), m_cullOrder(cullOrder), m_lineWidth(lineWidth), m_depthBias(useDepthBias), m_depthBiasClamp(depthBiasClamp), m_depthBiasConstantFactor(depthBiasConstantFactor), m_depthBiasSlopeFactor(depthBiasSlopeFactor)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Rasterizer::Rasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const bool& useDepthBias, const Float& depthBiasClamp, const Float& depthBiasConstantFactor, const Float& depthBiasSlopeFactor) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, polygonMode, cullMode, cullOrder, lineWidth, useDepthBias, depthBiasClamp, depthBiasConstantFactor, depthBiasSlopeFactor))
{
}

Rasterizer::Rasterizer(const Rasterizer& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, _other.polygonMode(), _other.cullMode(), _other.cullOrder(), _other.lineWidth(), _other.useDepthBias(), _other.depthBiasClamp(), _other.depthBiasConstantFactor(), _other.depthBiasSlopeFactor()))
{

}

Rasterizer::Rasterizer(Rasterizer&& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, _other.polygonMode(), _other.cullMode(), _other.cullOrder(), _other.lineWidth(), _other.useDepthBias(), _other.depthBiasClamp(), _other.depthBiasConstantFactor(), _other.depthBiasSlopeFactor()))
{
    // TODO: We could move out the properties of `_other`, but I guess moving around rasterizer states really should not be the bottleneck in most applications.
}

Rasterizer::~Rasterizer() noexcept = default;

const PolygonMode& Rasterizer::polygonMode() const noexcept
{
    return m_impl->m_polygonMode;
}

const CullMode& Rasterizer::cullMode() const noexcept
{
    return m_impl->m_cullMode;
}

const CullOrder& Rasterizer::cullOrder() const noexcept
{
    return m_impl->m_cullOrder;
}

const Float& Rasterizer::lineWidth() const noexcept
{
    return m_impl->m_lineWidth;
}

bool Rasterizer::useDepthBias() const noexcept
{
    return m_impl->m_depthBias;
}

const Float& Rasterizer::depthBiasClamp() const noexcept
{
    return m_impl->m_depthBiasClamp;
}

const Float& Rasterizer::depthBiasConstantFactor() const noexcept
{
    return m_impl->m_depthBiasConstantFactor;
}

const Float& Rasterizer::depthBiasSlopeFactor() const noexcept
{
    return m_impl->m_depthBiasSlopeFactor;
}

PolygonMode& Rasterizer::polygonMode() noexcept
{
    return m_impl->m_polygonMode;
}

CullMode& Rasterizer::cullMode() noexcept
{
    return m_impl->m_cullMode;
}

CullOrder& Rasterizer::cullOrder() noexcept
{
    return m_impl->m_cullOrder;
}

Float& Rasterizer::lineWidth() noexcept
{
    return m_impl->m_lineWidth;
}

bool& Rasterizer::useDepthBias() noexcept
{
    return m_impl->m_depthBias;
}

Float& Rasterizer::depthBiasClamp() noexcept
{
    return m_impl->m_depthBiasClamp;
}

Float& Rasterizer::depthBiasConstantFactor() noexcept
{
    return m_impl->m_depthBiasConstantFactor;
}

Float& Rasterizer::depthBiasSlopeFactor() noexcept
{
    return m_impl->m_depthBiasSlopeFactor;
}