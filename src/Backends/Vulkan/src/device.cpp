#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanDevice::VulkanDeviceImpl {
private:
	SharedPtr<VulkanQueue> m_queue;

public:
	VulkanDeviceImpl(SharedPtr<VulkanQueue> queue) noexcept :
		m_queue(queue) { }
};

VulkanDevice::VulkanDevice(const VkDevice device, SharedPtr<VulkanQueue> queue, const Array<String>& extensions) :
	IResource(device), m_impl(makePimpl<VulkanDeviceImpl>(queue))
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");
	
	if (queue == nullptr)
		throw std::invalid_argument("The argument `queue` must be initialized.");

	queue->initDeviceQueue(this);
}

VulkanDevice::~VulkanDevice() noexcept
{
	::vkDestroyDevice(this->handle(), nullptr);
}