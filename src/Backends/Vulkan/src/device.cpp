#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanDevice::VulkanDevice(const VkDevice device) :
	GraphicsDevice(), IResource(device)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");
}

VulkanDevice::~VulkanDevice()
{
	::vkDestroyDevice(this->handle(), nullptr);
}