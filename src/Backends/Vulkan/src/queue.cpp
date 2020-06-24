#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanQueue::VulkanQueueImpl : public Implement<VulkanQueue> {
public:
	friend class VulkanQueue;

private:
	QueueType m_type;
	uint32_t m_id;

public:
	VulkanQueueImpl(VulkanQueue* parent, const QueueType& type, const uint32_t id) :
		base(parent), m_type(type), m_id(id) { }

public:
	void createDeviceQueue(const VulkanDevice* device, VkQueue& queue) const noexcept
	{
		::vkGetDeviceQueue(device->handle(), m_id, 0, &queue);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanQueue::VulkanQueue(const QueueType& type, const uint32_t id) :
	IResource(nullptr), m_impl(makePimpl<VulkanQueueImpl>(this, type, id))
{
}

VulkanQueue::~VulkanQueue() noexcept = default;

void VulkanQueue::initDeviceQueue(const VulkanDevice* device)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` is not initialized.");
	
	if (this->handle() != nullptr)
		throw std::runtime_error("The queue is already initialized.");

	LITEFX_TRACE(VULKAN_LOG, "Initializing device queue for device {0} (id: {1}, type: {2})...", fmt::ptr(device), this->getId(), this->getType());
	m_impl->createDeviceQueue(device, this->handle());
}

uint32_t VulkanQueue::getId() const noexcept
{
	return m_impl->m_id;
}

QueueType VulkanQueue::getType() const noexcept
{
	return m_impl->m_type;
}