#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanDevice::VulkanDevice(const VkDevice device) :
	GraphicsDevice(device)
{
	if (device == nullptr)
		throw std::invalid_argument("The parameter `device` must be initialized.");
}

VulkanDevice::~VulkanDevice()
{
	::vkDestroyDevice(IResource::getHandle<VkDevice>(), nullptr);
}