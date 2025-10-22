#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, bool depthClip, const DepthStencilState& depthStencilState, bool conservativeRasterization) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthClip, depthStencilState, conservativeRasterization)
{
}

VulkanRasterizer::VulkanRasterizer() noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise)
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

void VulkanRasterizer::updateLineWidth(Float lineWidth) noexcept
{
    this->lineWidth() = lineWidth;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizerBuilder::VulkanRasterizerBuilder() :
    RasterizerBuilder(VulkanRasterizer::create())
{
}

VulkanRasterizerBuilder::~VulkanRasterizerBuilder() noexcept = default;

void VulkanRasterizerBuilder::build()
{
    this->instance()->polygonMode() = this->state().polygonMode;
    this->instance()->cullMode() = this->state().cullMode;
    this->instance()->cullOrder() = this->state().cullOrder;
    this->instance()->lineWidth() = this->state().lineWidth;
    this->instance()->depthClip() = this->state().depthClip;
    this->instance()->conservativeRasterization() = this->state().conservativeRasterization;
    this->instance()->depthStencilState().depthBias() = this->state().depthBias;
    this->instance()->depthStencilState().depthState() = this->state().depthState;
    this->instance()->depthStencilState().stencilState() = this->state().stencilState;
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)