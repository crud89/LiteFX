#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanViewport::VulkanViewport(const VulkanRenderPipelineLayout& layout) noexcept :
    Viewport(), RuntimeObject(layout.getDevice())
{
}

VulkanViewport::~VulkanViewport() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanViewportBuilder& VulkanViewportBuilder::withRectangle(const RectF& rectangle)
{
    this->instance()->setRectangle(rectangle);
    return *this;
}

VulkanViewportBuilder& VulkanViewportBuilder::addScissor(const RectF& scissor)
{
    this->instance()->getScissors().push_back(scissor);
    return *this;
}