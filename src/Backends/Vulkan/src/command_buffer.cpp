#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl : public Implement<VulkanCommandBuffer> {
public:
	friend class VulkanCommandBuffer;

private:
	const VulkanQueue* m_queue;
	const VulkanDevice* m_device;

public:
	VulkanCommandBufferImpl(VulkanCommandBuffer* parent, const VulkanQueue* queue) :
		base(parent), m_queue(queue)
	{
		if (queue == nullptr)
			throw std::invalid_argument("The argument `queue` must be initialized.");

		m_device = dynamic_cast<const VulkanDevice*>(queue->getDevice());

		if (m_device == nullptr)
			throw std::invalid_argument("The provided device queue has not been created using a valid Vulkan device.");
	}

	~VulkanCommandBufferImpl()
	{
		::vkFreeCommandBuffers(m_device->handle(), m_queue->getCommandPool(), 1, &m_parent->handle());
	}

public:
	VkCommandBuffer initialize()
	{
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = m_queue->getCommandPool();
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;

		if (::vkAllocateCommandBuffers(m_device->handle(), &bufferInfo, &buffer) != VK_SUCCESS)
		    throw std::runtime_error("Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue* queue) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this, queue)), IResource(nullptr)
{
	this->handle() = m_impl->initialize();
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept = default;

const ICommandQueue* VulkanCommandBuffer::getQueue() const noexcept
{
	return m_impl->m_queue;
}

void VulkanCommandBuffer::begin() const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (::vkBeginCommandBuffer(this->handle(), &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Unable to begin command recording.");
}

void VulkanCommandBuffer::end() const
{
	if (::vkEndCommandBuffer(this->handle()) != VK_SUCCESS)
		throw std::runtime_error("Unable to stop command recording.");
}

void VulkanCommandBuffer::submit(const bool& waitForQueue) const
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->handle();

	// Submit the command buffer to the transfer queue.
	if (::vkQueueSubmit(m_impl->m_queue->handle(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Unable to submit command buffer to queue.");

	// If required, wait for the queue to return into idle state.
	if (waitForQueue && (::vkQueueWaitIdle(m_impl->m_queue->handle()) != VK_SUCCESS))
		throw std::runtime_error("Unable to wait for queue to return into idle state.");
}