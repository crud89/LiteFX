#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanDevice::VulkanDeviceImpl {
private:
	VkDevice m_device;
	SharedPtr<VulkanQueue> m_queue;

public:
	VulkanDeviceImpl(const VkDevice device, SharedPtr<VulkanQueue> queue) noexcept :
		m_device(device), m_queue(queue)  { }
	~VulkanDeviceImpl() noexcept { ::vkDestroyDevice(m_device, nullptr); }
};

VulkanDevice::VulkanDevice(const VkDevice device, SharedPtr<VulkanQueue> queue) :
	IResource(device), m_impl(makePimpl<VulkanDeviceImpl>(device, queue))
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");
	
	if (queue == nullptr)
		throw std::invalid_argument("The argument `queue` must be initialized.");

	queue->initDeviceQueue(this);
}

VulkanDevice::~VulkanDevice() noexcept = default;