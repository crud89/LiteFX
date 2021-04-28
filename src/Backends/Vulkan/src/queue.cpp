#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanQueue::VulkanQueueImpl : public Implement<VulkanQueue> {
public:
	friend class VulkanQueue;

private:
	const VulkanDevice* m_device;
	VkCommandPool m_commandPool{};
	QueueType m_type;
	uint32_t m_id;
	bool m_bound;

public:
	VulkanQueueImpl(VulkanQueue* parent, const VulkanDevice* device, const QueueType& type, const uint32_t id) :
		base(parent), m_type(type), m_id(id), m_bound(false), m_device(device) { }

	~VulkanQueueImpl()
	{
		this->release();
	}

private:
	void release()
	{
		if (m_bound)
			::vkDestroyCommandPool(m_device->handle(), m_commandPool, nullptr);

		m_bound = false;
		m_commandPool = {};
	}

public:
	void bind()
	{
		if (m_bound)
			return;

		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_id;

		// Transfer pools can be transient.
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (m_type == QueueType::Transfer)
			poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		if (::vkCreateCommandPool(m_device->handle(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
			throw std::runtime_error("Unable to create command pool.");

		::vkGetDeviceQueue(m_device->handle(), m_id, 0, &m_parent->handle());

		m_bound = true;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanQueue::VulkanQueue(const IGraphicsDevice* device, const QueueType& type, const uint32_t id) :
	IResource(nullptr), m_impl(makePimpl<VulkanQueueImpl>(this, dynamic_cast<const VulkanDevice*>(device), type, id))
{
	if (m_impl->m_device == nullptr)
		throw std::invalid_argument("The device must be initialized.");
}

VulkanQueue::~VulkanQueue() noexcept
{
	this->release();
}

void VulkanQueue::bind()
{
	m_impl->bind();
}

void VulkanQueue::release()
{
	m_impl->release();
}

bool VulkanQueue::isBound() const noexcept
{
	return m_impl->m_bound;
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