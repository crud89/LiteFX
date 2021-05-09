#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl : public Implement<VulkanCommandBuffer> {
public:
	friend class VulkanCommandBuffer;

private:
	VkFence m_fence;

public:
	VulkanCommandBufferImpl(VulkanCommandBuffer* parent) :
		base(parent)
	{
	}

	~VulkanCommandBufferImpl()
	{
		::vkDestroyFence(m_parent->getDevice()->handle(), m_fence, nullptr);
		::vkFreeCommandBuffers(m_parent->getDevice()->handle(), m_parent->parent().getCommandPool(), 1, &m_parent->handle());
	}

public:
	VkCommandBuffer initialize()
	{
		// Allocate the fence.
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Start in signaled state, so that the first `begin` call does not wait.

		if (::vkCreateFence(m_parent->getDevice()->handle(), &fenceInfo, nullptr, &m_fence) != VK_SUCCESS)
			throw std::runtime_error("Unable to allocate fence for command buffer synchronization.");

		// Create the command buffer.
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = m_parent->parent().getCommandPool();
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;

		if (::vkAllocateCommandBuffers(m_parent->getDevice()->handle(), &bufferInfo, &buffer) != VK_SUCCESS)
		    throw std::runtime_error("Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this)), VulkanRuntimeObject<VulkanQueue>(queue, queue.getDevice()), IResource(nullptr)
{
	if (!queue.isBound())
		throw std::invalid_argument("You must bind the queue before creating a command buffer from it.");

	this->handle() = m_impl->initialize();
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept = default;

void VulkanCommandBuffer::begin() const
{
	// Wait for the fence to be signaled.
	if(::vkWaitForFences(this->getDevice()->handle(), 1, &m_impl->m_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		throw std::runtime_error("Unable to wait for command buffer synchronization fence.");

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (::vkBeginCommandBuffer(this->handle(), &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Unable to begin command recording.");

	// Reset the fence signaled state.
	if (::vkResetFences(this->getDevice()->handle(), 1, &m_impl->m_fence) != VK_SUCCESS)
		throw std::runtime_error("Unable to reset command buffer synchronization fence.");
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
	if (::vkQueueSubmit(this->parent().handle(), 1, &submitInfo, m_impl->m_fence) != VK_SUCCESS)
		throw std::runtime_error("Unable to submit command buffer to queue.");

	// If required, wait for the queue to return into idle state.
	if (waitForQueue && (::vkQueueWaitIdle(this->parent().handle()) != VK_SUCCESS))
		throw std::runtime_error("Unable to wait for queue to return into idle state.");
}

void VulkanCommandBuffer::submit(const Array<VkSemaphore>& waitForSemaphores, const Array<VkPipelineStageFlags>& waitForStages, const Array<VkSemaphore>& signalSemaphores, const bool& waitForQueue) const
{
	if (waitForSemaphores.size() != waitForStages.size())
		throw std::invalid_argument("A pipeline stage to wait for needs to be provided for each semaphore to wait for.");

	// Submit the command buffer.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = waitForSemaphores.size();
	submitInfo.pWaitSemaphores = waitForSemaphores.data();
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();
	submitInfo.pWaitDstStageMask = waitForStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->handle();

	// Submit the command buffer to the transfer queue.
	if (::vkQueueSubmit(this->parent().handle(), 1, &submitInfo, m_impl->m_fence) != VK_SUCCESS)
		throw std::runtime_error("Unable to submit command buffer to queue.");

	// If required, wait for the queue to return into idle state.
	if (waitForQueue && (::vkQueueWaitIdle(this->parent().handle()) != VK_SUCCESS))
		throw std::runtime_error("Unable to wait for queue to return into idle state.");
}