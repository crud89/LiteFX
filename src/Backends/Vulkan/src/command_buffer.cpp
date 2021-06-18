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

public:
	VkCommandBuffer initialize()
	{
		// Allocate the fence.
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Start in signaled state, so that the first `begin` call does not wait.

		raiseIfFailed<RuntimeException>(::vkCreateFence(m_parent->getDevice()->handle(), &fenceInfo, nullptr, &m_fence), "Unable to allocate fence for command buffer synchronization.");

		// Create the command buffer.
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = m_parent->parent().commandPool();
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;
		raiseIfFailed<RuntimeException>(::vkAllocateCommandBuffers(m_parent->getDevice()->handle(), &bufferInfo, &buffer), "Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue, const bool& begin) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this)), VulkanRuntimeObject<VulkanQueue>(queue, queue.getDevice()), Resource<VkCommandBuffer>(nullptr)
{
	if (!queue.isBound())
		throw InvalidArgumentException("You must bind the queue before creating a command buffer from it.");

	this->handle() = m_impl->initialize();

	if (begin)
		this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept
{
	::vkDestroyFence(this->getDevice()->handle(), m_impl->m_fence, nullptr);
	::vkFreeCommandBuffers(this->getDevice()->handle(), this->parent().commandPool(), 1, &this->handle());
}

void VulkanCommandBuffer::wait() const
{
	// Wait for the fence to be signaled.
	raiseIfFailed<RuntimeException>(::vkWaitForFences(this->getDevice()->handle(), 1, &m_impl->m_fence, VK_TRUE, UINT64_MAX), "Unable to wait for command buffer synchronization fence.");
}

void VulkanCommandBuffer::begin() const
{
	// Wait possible previous executions to finish.
	this->wait();

	// Set the buffer into recording state.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	raiseIfFailed<RuntimeException>(::vkBeginCommandBuffer(this->handle(), &beginInfo), "Unable to begin command recording.");

	// Reset the fence signaled state.
	raiseIfFailed<RuntimeException>(::vkResetFences(this->getDevice()->handle(), 1, &m_impl->m_fence), "Unable to reset command buffer synchronization fence.");
}

void VulkanCommandBuffer::end(const bool& submit, const bool& wait) const
{
	// End recording.
	raiseIfFailed<RuntimeException>(::vkEndCommandBuffer(this->handle()), "Unable to stop command recording.");

	if (submit)
		this->submit(wait);
}

void VulkanCommandBuffer::submit(const bool& wait) const
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->handle();

	// Submit the command buffer to the transfer queue.
	raiseIfFailed<RuntimeException>(::vkQueueSubmit(this->parent().handle(), 1, &submitInfo, m_impl->m_fence), "Unable to submit command buffer to queue.");

	// If required, wait for the queue to return into idle state.
	if (wait)
		//raiseIfFailed<RuntimeException>(::vkQueueWaitIdle(this->parent().handle()), "Unable to wait for queue to return into idle state.");
		this->wait();
}

void VulkanCommandBuffer::submit(const Array<VkSemaphore>& waitForSemaphores, const Array<VkPipelineStageFlags>& waitForStages, const Array<VkSemaphore>& signalSemaphores, const bool& waitForQueue) const
{
	if (waitForSemaphores.size() != waitForStages.size())
		throw InvalidArgumentException("A pipeline stage to wait for needs to be provided for each semaphore to wait for.");

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
	raiseIfFailed<RuntimeException>(::vkQueueSubmit(this->parent().handle(), 1, &submitInfo, m_impl->m_fence), "Unable to submit command buffer to queue.");

	// If required, wait for the queue to return into idle state.
	if (waitForQueue)
		//raiseIfFailed<RuntimeException>(::vkQueueWaitIdle(this->parent().handle()), "Unable to wait for queue to return into idle state.");
		this->wait();
}