#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanQueue::VulkanQueueImpl {
private:
	VkQueue m_queue;
	QueueType m_type;
	uint32_t m_id;

public:
	VulkanQueueImpl(const QueueType& type, const uint32_t id) noexcept :
		m_type(type), m_id(id), m_queue(nullptr) { }
	~VulkanQueueImpl() noexcept = default;

public:
	uint32_t getId() const noexcept
	{
		return m_id;
	}

	QueueType getType() const noexcept
	{
		return m_type;
	}

	VkQueue createDeviceQueue(const VulkanDevice* device) noexcept
	{
		::vkGetDeviceQueue(device->handle(), m_id, 0, &m_queue);
		return m_queue;
	}
};

VulkanQueue::VulkanQueue(const QueueType& type, const uint32_t id) noexcept :
	IResource(nullptr), m_impl(makePimpl<VulkanQueueImpl>(type, id))
{
}

VulkanQueue::~VulkanQueue() noexcept = default;

void VulkanQueue::initDeviceQueue(const VulkanDevice* device)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` is not initialized.");
	
	if (this->handle() != nullptr)
		throw std::runtime_error("The queue is already initialized.");

	this->handle() = m_impl->createDeviceQueue(device);
}

uint32_t VulkanQueue::getId() const noexcept
{
	return m_impl->getId();
}

QueueType VulkanQueue::getType() const noexcept
{
	return m_impl->getType();
}