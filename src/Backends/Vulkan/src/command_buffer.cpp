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

void VulkanCommandBuffer::generateMipMaps(IVulkanImage& image) noexcept
{
	//VkImageMemoryBarrier barrier{
	//	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	//	.pNext = nullptr,
	//	.srcAccessMask = 0,
	//	.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	//	.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	//	.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	//	.image = this->handle(),
	//	.subresourceRange = VkImageSubresourceRange {.aspectMask = m_impl->getAspectMask() }
	//};

	//auto layout = ::getImageLayout(this->state());

	//for (UInt32 layer(0); layer < this->layers(); ++layer)
	//{
	//	Int32 mipWidth = static_cast<Int32>(this->extent().width());
	//	Int32 mipHeight = static_cast<Int32>(this->extent().height());
	//	Int32 mipDepth = static_cast<Int32>(this->extent().depth());

	//	for (UInt32 level(1); level < this->levels(); ++level)
	//	{
	//		// Transition the previous level to transfer source.
	//		barrier.subresourceRange.aspectMask = m_impl->getAspectMask();
	//		barrier.subresourceRange.baseArrayLayer = layer;
	//		barrier.subresourceRange.layerCount = 1;
	//		barrier.subresourceRange.baseMipLevel = level - 1;
	//		barrier.subresourceRange.levelCount = 1;
	//		barrier.oldLayout = layout;
	//		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	//		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	//		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	//		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	//		// Blit the image of the previous level into the current level.
	//		VkImageBlit blit{
	//			.srcSubresource = VkImageSubresourceLayers {
	//				.aspectMask = m_impl->getAspectMask(),
	//				.mipLevel = level - 1,
	//				.baseArrayLayer = layer,
	//				.layerCount = 1
	//			},
	//			.dstSubresource = VkImageSubresourceLayers {
	//				.aspectMask = m_impl->getAspectMask(),
	//				.mipLevel = level,
	//				.baseArrayLayer = layer,
	//				.layerCount = 1
	//			}
	//		};

	//		blit.srcOffsets[0] = { 0, 0, 0 };
	//		blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };
	//		blit.dstOffsets[0] = { 0, 0, 0 };
	//		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 };

	//		::vkCmdBlitImage(commandBuffer.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	//		// Compute the new size.
	//		mipWidth = std::max(mipWidth / 2, 1);
	//		mipHeight = std::max(mipHeight / 2, 1);
	//		mipDepth = std::max(mipDepth / 2, 1);
	//	}
	//}

	//// Finally, transition all the levels back to the original layout.
	//if (layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	//{
	//	barrier.subresourceRange.aspectMask = m_impl->getAspectMask();
	//	barrier.subresourceRange.baseArrayLayer = 0;
	//	barrier.subresourceRange.layerCount = this->layers();
	//	barrier.subresourceRange.baseMipLevel = 0;
	//	barrier.subresourceRange.levelCount = this->levels();
	//	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	barrier.newLayout = layout;
	//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	//	::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	//}
}

void VulkanCommandBuffer::barrier(const VulkanBarrier& barrier, const bool& invert) const noexcept
{
	if (invert)
		barrier.executeInverse(*this);
	else
		barrier.execute(*this);
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	throw;
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	throw;
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	throw;
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const
{
	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);

	throw;
}