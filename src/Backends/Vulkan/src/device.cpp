#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanDevice::VulkanDevice(VkPhysicalDevice device) :
	RenderDevice(device)
{
}