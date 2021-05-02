#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Viewport::DirectX12Viewport(const DirectX12RenderPipelineLayout& layout) noexcept :
    Viewport()
{
}

DirectX12Viewport::~DirectX12Viewport() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12ViewportBuilder& DirectX12ViewportBuilder::withRectangle(const RectF & rectangle)
{
    this->instance()->setRectangle(rectangle);
    return *this;
}

DirectX12ViewportBuilder& DirectX12ViewportBuilder::addScissor(const RectF & scissor)
{
    this->instance()->getScissors().push_back(scissor);
    return *this;
}