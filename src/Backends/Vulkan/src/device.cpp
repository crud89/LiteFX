#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanDevice::VulkanDevice(const VkDevice device) :
	GraphicsDevice(), IResource(device)
{
}

VulkanDevice::~VulkanDevice()
{
	::vkDestroyDevice(this->handle(), nullptr);
}