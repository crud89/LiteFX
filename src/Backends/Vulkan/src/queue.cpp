#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanQueue::VulkanQueueImpl : public Implement<VulkanQueue> {
public:
	friend class VulkanQueue;

private:
	const VulkanDevice* m_device{ nullptr };
	VkCommandPool m_commandPool{};
	QueueType m_type;
	uint32_t m_id;

public:
	VulkanQueueImpl(VulkanQueue* parent, const QueueType& type, const uint32_t id) :
		base(parent), m_type(type), m_id(id) { }

	~VulkanQueueImpl()
	{
		this->release();
	}

private:
	void release()
	{
		if (m_device != nullptr)
			::vkDestroyCommandPool(m_device->handle(), m_commandPool, nullptr);

		m_device = nullptr;
		m_commandPool = {};
	}

public:
	void bindDevice(const VulkanDevice* device)
	{
		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_id;

		// Transfer pools can be transient.
		poolInfo.flags = 0;

		if (m_type == QueueType::Transfer)
			poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		if (::vkCreateCommandPool(device->handle(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
			throw std::runtime_error("Unable to create command pool.");

		::vkGetDeviceQueue(device->handle(), m_id, 0, &m_parent->handle());

		m_device = device;
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

void VulkanQueue::bindDevice(const IGraphicsDevice* d)
{
	auto device = dynamic_cast<const VulkanDevice*>(d);

	if (device == nullptr)
		throw std::invalid_argument("The argument `device` is not initialized.");
	
	if (this->handle() != nullptr)
		throw std::runtime_error("The queue is already bound to a device.");

	LITEFX_TRACE(VULKAN_LOG, "Initializing device queue for device {0} {{ Id: {1}, type: {2} }}...", fmt::ptr(device), this->getId(), this->getType());
	m_impl->bindDevice(device);
}

void VulkanQueue::release()
{
	m_impl->release();
}

uint32_t VulkanQueue::getId() const noexcept
{
	return m_impl->m_id;
}

VkCommandPool VulkanQueue::getCommandPool() const noexcept
{
	return m_impl->m_commandPool;
}

QueueType VulkanQueue::getType() const noexcept
{
	return m_impl->m_type;
}

const IGraphicsDevice* VulkanQueue::getDevice() const noexcept
{
	return m_impl->m_device;
}

UniquePtr<ICommandBuffer> VulkanQueue::createCommandBuffer() const
{
	return makeUnique<VulkanCommandBuffer>(this);
}