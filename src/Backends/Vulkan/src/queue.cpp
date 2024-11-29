#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// Import required extensions.
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkQueueBeginDebugUtilsLabelEXT   vkQueueBeginDebugUtilsLabel;
extern PFN_vkQueueEndDebugUtilsLabelEXT     vkQueueEndDebugUtilsLabel;
extern PFN_vkQueueInsertDebugUtilsLabelEXT  vkQueueInsertDebugUtilsLabel;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanQueue::VulkanQueueImpl {
public:
	friend class VulkanQueue;

private:
	QueueType m_type;
	QueuePriority m_priority;
	UInt32 m_familyId, m_queueId;
	VkSemaphore m_timelineSemaphore{};
	UInt64 m_fenceValue{ 0 };
	mutable std::mutex m_mutex;
	const VulkanDevice& m_device;
	Array<Tuple<UInt64, SharedPtr<const VulkanCommandBuffer>>> m_submittedCommandBuffers;

public:
	VulkanQueueImpl(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId) :
		m_type(type), m_priority(priority), m_familyId(familyId), m_queueId(queueId), m_device(device)
	{
	}

public:
	void release()
	{
		m_submittedCommandBuffers.clear();

		if (m_timelineSemaphore != VK_NULL_HANDLE)
			::vkDestroySemaphore(m_device.handle(), m_timelineSemaphore, nullptr);

		m_timelineSemaphore = {};
	}

	VkQueue initialize()
	{
		// Create the queue instance.
		VkQueue queue{};
		::vkGetDeviceQueue(m_device.handle(), m_familyId, m_queueId, &queue);

		// Create a timeline semaphore for queue synchronization.
		VkSemaphoreTypeCreateInfo timelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = m_fenceValue
		};

		VkSemaphoreCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineCreateInfo,
			.flags = 0
		};

		raiseIfFailed(::vkCreateSemaphore(m_device.handle(), &createInfo, nullptr, &m_timelineSemaphore), "Unable to create queue synchronization semaphore.");

		return queue;
	}

	void releaseCommandBuffers(const VulkanQueue& queue, UInt64 beforeFence)
	{
		// Release all shared command buffers until this point.
		const auto [from, to] = std::ranges::remove_if(m_submittedCommandBuffers, [&queue, &beforeFence](auto& pair) {
			if (std::get<0>(pair) > beforeFence)
				return false;

			queue.releaseSharedState(*std::get<1>(pair));
			return true;
		});

		this->m_submittedCommandBuffers.erase(from, to);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanQueue::VulkanQueue(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId) :
	Resource<VkQueue>(nullptr), m_impl(device, type, priority, familyId, queueId)
{
	this->handle() = m_impl->initialize();
}


VulkanQueue::VulkanQueue(VulkanQueue&&) noexcept = default;
VulkanQueue& VulkanQueue::operator=(VulkanQueue&&) noexcept = default;

VulkanQueue::~VulkanQueue() noexcept
{
	m_impl->release();
}

const VulkanDevice& VulkanQueue::device() const noexcept
{
	return m_impl->m_device;
}

UInt32 VulkanQueue::familyId() const noexcept
{
	return m_impl->m_familyId;
}

UInt32 VulkanQueue::queueId() const noexcept
{
	return m_impl->m_queueId;
}

const VkSemaphore& VulkanQueue::timelineSemaphore() const noexcept
{
	return m_impl->m_timelineSemaphore;
}

QueueType VulkanQueue::type() const noexcept
{
	return m_impl->m_type;
}

#ifdef LITEFX_BUILD_SUPPORT_DEBUG_MARKERS
void VulkanQueue::beginDebugRegion(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	VkDebugUtilsLabelEXT labelInfo {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { static_cast<float>(color.x()) / static_cast<float>(std::numeric_limits<Byte>::max()), static_cast<float>(color.y()) / static_cast<float>(std::numeric_limits<Byte>::max()), static_cast<float>(color.z()) / static_cast<float>(std::numeric_limits<Byte>::max()), 1.0f }
	};
	
	::vkQueueBeginDebugUtilsLabel(this->handle(), &labelInfo);
}

void VulkanQueue::endDebugRegion() const noexcept
{
	::vkQueueEndDebugUtilsLabel(this->handle());
}

void VulkanQueue::setDebugMarker(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	VkDebugUtilsLabelEXT labelInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { static_cast<float>(color.x()) / static_cast<float>(std::numeric_limits<Byte>::max()), static_cast<float>(color.y()) / static_cast<float>(std::numeric_limits<Byte>::max()), static_cast<float>(color.z()) / static_cast<float>(std::numeric_limits<Byte>::max()), 1.0f }
	};

	::vkQueueInsertDebugUtilsLabel(this->handle(), &labelInfo);
}
#endif // LITEFX_BUILD_SUPPORT_DEBUG_MARKERS

QueuePriority VulkanQueue::priority() const noexcept
{
	return m_impl->m_priority;
}

SharedPtr<VulkanCommandBuffer> VulkanQueue::createCommandBuffer(bool beginRecording, bool secondary) const
{
	return VulkanCommandBuffer::create(*this, beginRecording, !secondary);
}

UInt64 VulkanQueue::submit(SharedPtr<const VulkanCommandBuffer> commandBuffer) const
{
	if (commandBuffer == nullptr) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "The command buffer must be initialized.");

	if (commandBuffer->isSecondary()) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "The command buffer must be a primary command buffer.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// Begin event.
	this->submitting(this, { { std::static_pointer_cast<const ICommandBuffer>(commandBuffer) } });

	// Remove all previously submitted command buffers, that have already finished.
	UInt64 completedValue = 0;
	::vkGetSemaphoreCounterValue(m_impl->m_device.handle(), m_impl->m_timelineSemaphore, &completedValue);
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	commandBuffer->end();

	// Submit the command buffer.
	auto fence = ++m_impl->m_fenceValue;

	VkSemaphoreSubmitInfo signalSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = fence,
		.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
	};

	VkCommandBufferSubmitInfo commandBufferInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = commandBuffer->handle()
	};

	VkSubmitInfo2 submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	// Submit the command buffer to the transfer queue.
	raiseIfFailed(::vkQueueSubmit2(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");

	// Add the command buffer to the submitted command buffers list.
	m_impl->m_submittedCommandBuffers.emplace_back(fence, commandBuffer);

	// Fire end event.
	this->submitted(this, { fence });
	return fence;
}

UInt64 VulkanQueue::submit(const Enumerable<SharedPtr<const VulkanCommandBuffer>>& commandBuffers) const
{
	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return buffer != nullptr; })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is not initialized.");

	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return !buffer->isSecondary(); })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is a secondary command buffer, which is not allowed to be submitted to a command queue.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// Begin event.
	auto buffers = commandBuffers | std::views::transform([](auto& buffer) { return std::static_pointer_cast<const ICommandBuffer>(buffer); });
	this->submitting(this, { buffers });

	// Remove all previously submitted command buffers, that have already finished.
	UInt64 completedValue = 0;
	::vkGetSemaphoreCounterValue(m_impl->m_device.handle(), m_impl->m_timelineSemaphore, &completedValue);
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	auto commandBufferInfos = [&commandBuffers]() -> std::generator<VkCommandBufferSubmitInfo> {
		for (auto buffer = commandBuffers.begin(); buffer != commandBuffers.end(); ++buffer) {
			(*buffer)->end();

			co_yield VkCommandBufferSubmitInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.commandBuffer = (*buffer)->handle()
			};
		}
	}() | std::ranges::to<Array<VkCommandBufferSubmitInfo>>();

	// Submit the command buffer.
	auto fence = ++m_impl->m_fenceValue;

	VkSemaphoreSubmitInfo signalSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = fence,
		.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
	};

	VkSubmitInfo2 submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = static_cast<UInt32>(commandBufferInfos.size()),
		.pCommandBufferInfos = commandBufferInfos.data(),
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	// Submit the command buffer to the transfer queue.
	raiseIfFailed(::vkQueueSubmit2(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");

	// Add the command buffers to the submitted command buffers list.
	std::ranges::for_each(commandBuffers, [this, &fence](auto& buffer) { m_impl->m_submittedCommandBuffers.push_back({ fence, buffer }); });

	// Fire end event.
	this->submitted(this, { fence });
	return fence;
}

void VulkanQueue::waitFor(UInt64 fence) const noexcept
{
	UInt64 completedValue{ 0 };
	//raiseIfFailed(::vkGetSemaphoreCounterValue(m_impl->m_device.handle(), m_impl->m_timelineSemaphore, &completedValue), "Unable to query current queue timeline semaphore value.");
	::vkGetSemaphoreCounterValue(m_impl->m_device.handle(), m_impl->m_timelineSemaphore, &completedValue);

	if (completedValue < fence)
	{
		VkSemaphoreWaitInfo waitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_impl->m_timelineSemaphore,
			.pValues = &fence
		};

		//raiseIfFailed(::vkWaitSemaphores(m_impl->m_device.handle(), &waitInfo, std::numeric_limits<UInt64>::max()), "Unable to wait for queue timeline semaphore.");
		::vkWaitSemaphores(m_impl->m_device.handle(), &waitInfo, std::numeric_limits<UInt64>::max());
	}

	m_impl->releaseCommandBuffers(*this, fence);
}

void VulkanQueue::waitFor(const VulkanQueue& queue, UInt64 fence) const noexcept
{
	VkSemaphoreSubmitInfo waitSemaphoreInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = queue.m_impl->m_timelineSemaphore,
		.value = fence,
		.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
	};

	VkSubmitInfo2 submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo
	};

	::vkQueueSubmit2(this->handle(), 1, &submitInfo, VK_NULL_HANDLE);
}

UInt64 VulkanQueue::currentFence() const noexcept
{
	return m_impl->m_fenceValue;
}