#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState)
{
}

VulkanRasterizer::VulkanRasterizer() noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise)
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

void VulkanRasterizer::updateLineWidth(const Float& lineWidth) noexcept
{
    this->lineWidth() = lineWidth;
}