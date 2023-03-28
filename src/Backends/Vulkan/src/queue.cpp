#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// Import required extensions.
extern PFN_vkQueueBeginDebugUtilsLabelEXT   vkQueueBeginDebugUtilsLabel;
extern PFN_vkQueueEndDebugUtilsLabelEXT     vkQueueEndDebugUtilsLabel;
extern PFN_vkQueueInsertDebugUtilsLabelEXT  vkQueueInsertDebugUtilsLabel;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanQueue::VulkanQueueImpl : public Implement<VulkanQueue> {
public:
	friend class VulkanQueue;

private:
	VkCommandPool m_commandPool{};
	QueueType m_type;
	QueuePriority m_priority;
	UInt32 m_familyId, m_queueId;
	VkSemaphore m_timelineSemaphore{};
	UInt64 m_fenceValue{ 0 };
	mutable std::mutex m_mutex;
	bool m_bound;
	const VulkanDevice& m_device;
	Array<Tuple<UInt64, SharedPtr<const VulkanCommandBuffer>>> m_submittedCommandBuffers;

public:
	VulkanQueueImpl(VulkanQueue* parent, const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId) :
		base(parent), m_type(type), m_priority(priority), m_familyId(familyId), m_queueId(queueId), m_bound(false), m_device(device)
	{
	}

	~VulkanQueueImpl()
	{
		this->release();
	}

public:
	void release()
	{
		m_submittedCommandBuffers.clear();

		if (m_timelineSemaphore != nullptr)
			::vkDestroySemaphore(m_device.handle(), m_timelineSemaphore, nullptr);

		if (m_bound)
			::vkDestroyCommandPool(m_device.handle(), m_commandPool, nullptr);

		m_bound = false;
		m_commandPool = {};
		m_timelineSemaphore = {};
	}

	void bind()
	{
		if (m_bound)
			return;

		// Store the queue handle, if not done in previous binds.
		if (m_parent->handle() == nullptr)
			::vkGetDeviceQueue(m_device.handle(), m_familyId, m_queueId, &m_parent->handle());

		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_familyId;

		// Transfer pools can be transient.
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (m_type == QueueType::Transfer)
			poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		raiseIfFailed<RuntimeException>(::vkCreateCommandPool(m_device.handle(), &poolInfo, nullptr, &m_commandPool), "Unable to create command pool.");

		// Create a timeline semaphore for queue synchronization.
		VkSemaphoreTypeCreateInfo timelineCreateInfo;
		timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineCreateInfo.pNext = NULL;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = m_fenceValue;

		VkSemaphoreCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = &timelineCreateInfo;
		createInfo.flags = 0;

		raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_device.handle(), &createInfo, NULL, &m_timelineSemaphore), "Unable to create queue synchronization semaphore.");

		m_bound = true;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanQueue::VulkanQueue(const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId) :
	Resource<VkQueue>(nullptr), m_impl(makePimpl<VulkanQueueImpl>(this, device, type, priority, familyId, queueId))
{
}

VulkanQueue::~VulkanQueue() noexcept = default;

const VulkanDevice& VulkanQueue::device() const noexcept
{
	return m_impl->m_device;
}

const VkCommandPool& VulkanQueue::commandPool() const noexcept
{
	return m_impl->m_commandPool;
}

const UInt32& VulkanQueue::familyId() const noexcept
{
	return m_impl->m_familyId;
}

const UInt32& VulkanQueue::queueId() const noexcept
{
	return m_impl->m_queueId;
}

const VkSemaphore& VulkanQueue::timelineSemaphore() const noexcept
{
	return m_impl->m_timelineSemaphore;
}

bool VulkanQueue::isBound() const noexcept
{
	return m_impl->m_bound;
}

const QueueType& VulkanQueue::type() const noexcept
{
	return m_impl->m_type;
}

#ifndef NDEBUG
void VulkanQueue::BeginDebugRegion(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	VkDebugUtilsLabelEXT labelInfo {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { color.x() / 255.0f, color.y() / 255.0f,color.z() / 255.0f, 1.0f }
	};
	
	::vkQueueBeginDebugUtilsLabel(this->handle(), &labelInfo);
}

void VulkanQueue::EndDebugRegion() const noexcept
{
	::vkQueueEndDebugUtilsLabel(this->handle());
}

void VulkanQueue::SetDebugMarker(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	VkDebugUtilsLabelEXT labelInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { color.x() / 255.0f, color.y() / 255.0f,color.z() / 255.0f, 1.0f }
	};

	::vkQueueInsertDebugUtilsLabel(this->handle(), &labelInfo);
}
#endif

const QueuePriority& VulkanQueue::priority() const noexcept
{
	return m_impl->m_priority;
}

void VulkanQueue::bind()
{
	m_impl->bind();
}

void VulkanQueue::release()
{
	m_impl->release();
}

SharedPtr<VulkanCommandBuffer> VulkanQueue::createCommandBuffer(const bool& beginRecording) const
{
	return this->createCommandBuffer(false, beginRecording);
}

SharedPtr<VulkanCommandBuffer> VulkanQueue::createCommandBuffer(const bool& secondary, const bool& beginRecording) const
{
	return makeShared<VulkanCommandBuffer>(*this, beginRecording, !secondary);
}

UInt64 VulkanQueue::submit(SharedPtr<const VulkanCommandBuffer> commandBuffer) const
{
	return this->submit(commandBuffer, {}, {}, {});
}

UInt64 VulkanQueue::submit(SharedPtr<const VulkanCommandBuffer> commandBuffer, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores) const
{
	if (commandBuffer == nullptr)
		throw InvalidArgumentException("The command buffer must be initialized.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End the command buffer.
	commandBuffer->end();

	// Create an array of all signal semaphores.
	Array<VkSemaphore> semaphoresToSignal(signalSemaphores.size());
	std::ranges::generate(semaphoresToSignal, [&signalSemaphores, i = 0]() mutable { return signalSemaphores[i++]; });
	semaphoresToSignal.insert(semaphoresToSignal.begin(), m_impl->m_timelineSemaphore);

	// Submit the command buffer.
	auto fence = ++m_impl->m_fenceValue;
	Array<UInt64> waitValues(waitForSemaphores.size(), 0);
	Array<UInt64> signalValues(semaphoresToSignal.size(), 0);
	signalValues[0] = fence;

	VkTimelineSemaphoreSubmitInfo timelineInfo{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = static_cast<UInt32>(waitValues.size()),
		.pWaitSemaphoreValues = waitValues.data(),
		.signalSemaphoreValueCount = static_cast<UInt32>(signalValues.size()),
		.pSignalSemaphoreValues = signalValues.data()
	};

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = static_cast<UInt32>(waitForSemaphores.size()),
		.pWaitSemaphores = waitForSemaphores.data(),
		.pWaitDstStageMask = waitForStages.data(),
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer->handle(),
		.signalSemaphoreCount = static_cast<UInt32>(semaphoresToSignal.size()),
		.pSignalSemaphores = semaphoresToSignal.data()
	};

	// Submit the command buffer to the transfer queue.
	raiseIfFailed<RuntimeException>(::vkQueueSubmit(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");

	// Add the command buffer to the submitted command buffers list and return the fence.
	m_impl->m_submittedCommandBuffers.push_back({ fence, commandBuffer });
	return fence;
}

UInt64 VulkanQueue::submit(const Array<SharedPtr<const VulkanCommandBuffer>>& commandBuffers) const
{
	return this->submit(commandBuffers, {}, {}, {});
}

UInt64 VulkanQueue::submit(const Array<SharedPtr<const VulkanCommandBuffer>>& commandBuffers, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores) const
{
	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return buffer != nullptr; }))
		throw InvalidArgumentException("At least one command buffer is not initialized.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End the command buffer.
	Array<VkCommandBuffer> handles(commandBuffers.size());
	std::ranges::generate(handles, [&commandBuffers, i = 0]() mutable {
		const auto& commandBuffer = commandBuffers[i++];
		commandBuffer->end();
		return commandBuffer->handle();
	});

	// Create an array of all signal semaphores.
	Array<VkSemaphore> semaphoresToSignal(signalSemaphores.size());
	std::ranges::generate(semaphoresToSignal, [&signalSemaphores, i = 0]() mutable { return signalSemaphores[i++]; });
	semaphoresToSignal.insert(semaphoresToSignal.begin(), m_impl->m_timelineSemaphore);

	// Submit the command buffer.
	auto fence = ++m_impl->m_fenceValue;
	Array<UInt64> waitValues(waitForSemaphores.size(), 0);
	Array<UInt64> signalValues(semaphoresToSignal.size(), 0);
	signalValues[0] = fence;

	VkTimelineSemaphoreSubmitInfo timelineInfo{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = static_cast<UInt32>(waitValues.size()),
		.pWaitSemaphoreValues = waitValues.data(),
		.signalSemaphoreValueCount = static_cast<UInt32>(signalValues.size()),
		.pSignalSemaphoreValues = signalValues.data()
	};

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = static_cast<UInt32>(waitForSemaphores.size()),
		.pWaitSemaphores = waitForSemaphores.data(),
		.pWaitDstStageMask = waitForStages.data(),
		.commandBufferCount = static_cast<UInt32>(handles.size()),
		.pCommandBuffers = handles.data(),
		.signalSemaphoreCount = static_cast<UInt32>(semaphoresToSignal.size()),
		.pSignalSemaphores = semaphoresToSignal.data()
	};

	// Submit the command buffer to the transfer queue.
	raiseIfFailed<RuntimeException>(::vkQueueSubmit(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");

	// Add the command buffers to the submitted command buffers list and return the fence.
	std::ranges::for_each(commandBuffers, [this, &fence](auto& buffer) { m_impl->m_submittedCommandBuffers.push_back({ fence, buffer }); });
	return fence;
}

void VulkanQueue::waitFor(const UInt64& fence) const noexcept
{
	UInt64 completedValue{ 0 };
	//raiseIfFailed<RuntimeException>(::vkGetSemaphoreCounterValue(this->getDevice()->handle(), m_impl->m_timelineSemaphore, &completedValue), "Unable to query current queue timeline semaphore value.");
	::vkGetSemaphoreCounterValue(m_impl->m_device.handle(), m_impl->m_timelineSemaphore, &completedValue);

	if (completedValue < fence)
	{
		VkSemaphoreWaitInfo waitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_impl->m_timelineSemaphore,
			.pValues = &fence
		};

		//raiseIfFailed<RuntimeException>(::vkWaitSemaphores(this->getDevice()->handle(), &waitInfo, std::numeric_limits<UInt64>::max()), "Unable to wait for queue timeline semaphore.");
		::vkWaitSemaphores(m_impl->m_device.handle(), &waitInfo, std::numeric_limits<UInt64>::max());
	}

	// Release all shared command buffers until this point.
	const auto [from, to] = std::ranges::remove_if(m_impl->m_submittedCommandBuffers, [this, &completedValue](auto& pair) {
		if (std::get<0>(pair) > completedValue)
			return false;

		this->releaseSharedState(*std::get<1>(pair));
		return true;
	});
	m_impl->m_submittedCommandBuffers.erase(from, to);
}

UInt64 VulkanQueue::currentFence() const noexcept
{
	return m_impl->m_fenceValue;
}