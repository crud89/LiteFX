#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Rasterizer::DirectX12Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState)
{
}

DirectX12Rasterizer::DirectX12Rasterizer() noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise)
{
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RasterizerBuilder::DirectX12RasterizerBuilder() :
    RasterizerBuilder(SharedPtr<DirectX12Rasterizer>(new DirectX12Rasterizer()))
{
}

DirectX12RasterizerBuilder::~DirectX12RasterizerBuilder() noexcept = default;

void DirectX12RasterizerBuilder::build()
{
    this->instance()->polygonMode() = this->state().polygonMode;
    this->instance()->cullMode() = this->state().cullMode;
    this->instance()->cullOrder() = this->state().cullOrder;
    this->instance()->lineWidth() = this->state().lineWidth;
    this->instance()->depthStencilState().depthBias() = this->state().depthBias;
    this->instance()->depthStencilState().depthState() = this->state().depthState;
    this->instance()->depthStencilState().stencilState() = this->state().stencilState;
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)