#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Rasterizer::DirectX12Rasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState)
{
}

DirectX12Rasterizer::DirectX12Rasterizer() noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise)
{
}

DirectX12Rasterizer::~DirectX12Rasterizer() noexcept = default;

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12RasterizerBuilder::DirectX12RasterizerBuilder() noexcept :
    RasterizerBuilder(SharedPtr<DirectX12Rasterizer>(new DirectX12Rasterizer()))
{
}

constexpr DirectX12RasterizerBuilder::~DirectX12RasterizerBuilder() noexcept = default;

constexpr void DirectX12RasterizerBuilder::build()
{
    this->instance()->polygonMode() = m_state.polygonMode;
    this->instance()->cullMode() = m_state.cullMode;
    this->instance()->cullOrder() = m_state.cullOrder;
    this->instance()->lineWidth() = m_state.lineWidth;
    this->instance()->depthStencilState().depthBias() = m_state.depthBias;
    this->instance()->depthStencilState().depthState() = m_state.depthState;
    this->instance()->depthStencilState().stencilState() = m_state.stencilState;
}
#endif // defined(BUILD_DEFINE_BUILDERS)