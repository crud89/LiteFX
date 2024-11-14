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
    RasterizerImpl(Rasterizer* parent, PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, const DepthStencilState& depthStencilState) :
        base(parent), m_polygonMode(polygonMode), m_cullMode(cullMode), m_cullOrder(cullOrder), m_lineWidth(lineWidth), m_depthStencilState(depthStencilState)
    {
    }

    RasterizerImpl(Rasterizer* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Rasterizer::Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, const DepthStencilState& depthStencilState) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, polygonMode, cullMode, cullOrder, lineWidth, depthStencilState))
{
}

Rasterizer::Rasterizer(const Rasterizer& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this, _other.polygonMode(), _other.cullMode(), _other.cullOrder(), _other.lineWidth(), _other.depthStencilState()))
{
}

Rasterizer::Rasterizer(Rasterizer&& _other) noexcept :
    m_impl(makePimpl<RasterizerImpl>(this))
{
    m_impl->m_polygonMode = std::move(_other.polygonMode());
    m_impl->m_cullMode = std::move(_other.cullMode());
    m_impl->m_cullOrder = std::move(_other.cullOrder());
    m_impl->m_lineWidth = std::move(_other.lineWidth());
    m_impl->m_depthStencilState = std::move(_other.depthStencilState());
}

Rasterizer::~Rasterizer() noexcept = default;

Rasterizer& Rasterizer::operator=(const Rasterizer& _other) noexcept
{
    m_impl->m_polygonMode = _other.polygonMode();
    m_impl->m_cullMode = _other.cullMode();
    m_impl->m_cullOrder = _other.cullOrder();
    m_impl->m_lineWidth = _other.lineWidth();
    m_impl->m_depthStencilState = _other.depthStencilState();

    return *this;
}

Rasterizer& Rasterizer::operator=(Rasterizer&& _other) noexcept
{
    m_impl = std::move(_other.m_impl);
    m_impl->m_parent = this;

    return *this;
}

PolygonMode Rasterizer::polygonMode() const noexcept
{
    return m_impl->m_polygonMode;
}

CullMode Rasterizer::cullMode() const noexcept
{
    return m_impl->m_cullMode;
}

CullOrder Rasterizer::cullOrder() const noexcept
{
    return m_impl->m_cullOrder;
}

Float Rasterizer::lineWidth() const noexcept
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