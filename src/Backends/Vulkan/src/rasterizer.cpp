#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState)
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

constexpr VulkanRasterizerBuilder::VulkanRasterizerBuilder() noexcept :
    RasterizerBuilder(SharedPtr<VulkanRasterizer>(new VulkanRasterizer()))
{
}

constexpr VulkanRasterizerBuilder::~VulkanRasterizerBuilder() noexcept = default;

void VulkanRasterizerBuilder::build()
{
    this->instance()->polygonMode() = m_state.polygonMode;
    this->instance()->cullMode() = m_state.cullMode;
    this->instance()->cullOrder() = m_state.cullOrder;
    this->instance()->lineWidth() = m_state.lineWidth;
    this->instance()->depthStencilState().depthBias() = m_state.depthBias;
    this->instance()->depthStencilState().depthState() = m_state.depthState;
    this->instance()->depthStencilState().stencilState() = m_state.stencilState;
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)