#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Rasterizer::RasterizerImpl {
public:
    friend class Rasterizer;

private:
    PolygonMode m_polygonMode{ PolygonMode::Solid };
    CullMode m_cullMode{ CullMode::BackFaces };
    CullOrder m_cullOrder{ CullOrder::CounterClockWise };
    Float m_lineWidth{ 1.f };
    bool m_depthClip{ true };
    DepthStencilState m_depthStencilState{};

public:
    RasterizerImpl(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, bool depthClip, DepthStencilState depthStencilState) :
        m_polygonMode(polygonMode), m_cullMode(cullMode), m_cullOrder(cullOrder), m_lineWidth(lineWidth), m_depthClip(depthClip), m_depthStencilState(std::move(depthStencilState))
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Rasterizer::Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, bool depthClip, const DepthStencilState& depthStencilState) noexcept :
    m_impl(polygonMode, cullMode, cullOrder, lineWidth, depthClip, depthStencilState)
{
}

Rasterizer::Rasterizer(const Rasterizer& _other) = default;
Rasterizer::Rasterizer(Rasterizer&& _other) noexcept = default;
Rasterizer& Rasterizer::operator=(const Rasterizer& _other) = default;
Rasterizer& Rasterizer::operator=(Rasterizer&& _other) noexcept = default;
Rasterizer::~Rasterizer() noexcept = default;

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

bool Rasterizer::depthClip() const noexcept
{
    return m_impl->m_depthClip;
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

bool& Rasterizer::depthClip() noexcept
{
    return m_impl->m_depthClip;
}

DepthStencilState& Rasterizer::depthStencilState() noexcept
{
    return m_impl->m_depthStencilState;
}