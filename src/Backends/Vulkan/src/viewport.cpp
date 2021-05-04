#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanViewport::VulkanViewport(const VulkanRenderPipelineLayout& layout) noexcept :
    Viewport(), VulkanRuntimeObject(layout.getDevice())
{
}

VulkanViewport::~VulkanViewport() noexcept = default;