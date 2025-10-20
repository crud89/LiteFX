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
    DepthStencilState m_depthStencilState{};
    bool m_conservativeRasterization{ false };

public:
    RasterizerImpl(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, DepthStencilState depthStencilState, bool conservativeRasterization) :
        m_polygonMode(polygonMode), m_cullMode(cullMode), m_cullOrder(cullOrder), m_lineWidth(lineWidth), m_depthStencilState(std::move(depthStencilState)), m_conservativeRasterization(conservativeRasterization)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Rasterizer::Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, const DepthStencilState& depthStencilState, bool conservativeRasterization) noexcept :
    m_impl(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState, conservativeRasterization)
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

bool Rasterizer::conservativeRasterization() const noexcept
{
    return m_impl->m_conservativeRasterization;
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

bool& Rasterizer::conservativeRasterization() noexcept
{
    return m_impl->m_conservativeRasterization;
}