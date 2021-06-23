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
    DepthStencilState m_depthStencilState;

public:
    RasterizerImpl(Rasterizer* parent, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) :
        base(parent), m_polygonMode(polygonMode), m_cullMode(cullMode), m_cullOrder(cullOrder), m_lineWidth(lineWidth), m_depthStencilState(depthStencilState)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Rasterizer::Rasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, polygonMode, cullMode, cullOrder, lineWidth, depthStencilState))
{
}

Rasterizer::Rasterizer(const Rasterizer& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, _other.polygonMode(), _other.cullMode(), _other.cullOrder(), _other.lineWidth(), _other.depthStencilState()))
{

}

Rasterizer::Rasterizer(Rasterizer&& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, _other.polygonMode(), _other.cullMode(), _other.cullOrder(), _other.lineWidth(), _other.depthStencilState()))
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

const DepthStencilState& Rasterizer::depthStencilState() const noexcept
{
    return m_impl->m_depthStencilState;
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

DepthStencilState& Rasterizer::depthStencilState() noexcept
{
    return m_impl->m_depthStencilState;
}