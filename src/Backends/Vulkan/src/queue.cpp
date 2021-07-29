#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

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
	VkSemaphore m_timelineSemaphore;
	UInt64 m_fenceValue{ 0 };
	mutable std::mutex m_mutex;
	bool m_bound;

public:
	VulkanQueueImpl(VulkanQueue* parent, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId) :
		base(parent), m_type(type), m_priority(priority), m_familyId(familyId), m_queueId(queueId), m_bound(false)
	{
	}

	~VulkanQueueImpl()
	{
		this->release();
	}

public:
	void release()
	{
		::vkDestroySemaphore(m_parent->getDevice()->handle(), m_timelineSemaphore, nullptr);

		if (m_bound)
			::vkDestroyCommandPool(m_parent->getDevice()->handle(), m_commandPool, nullptr);

		m_bound = false;
		m_commandPool = {};
	}

	void bind()
	{
		if (m_bound)
			return;

		// Store the queue handle, if not done in previous binds.
		if (m_parent->handle() == nullptr)
			::vkGetDeviceQueue(m_parent->getDevice()->handle(), m_familyId, m_queueId, &m_parent->handle());

		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_familyId;

		// Transfer pools can be transient.
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (m_type == QueueType::Transfer)
			poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		raiseIfFailed<RuntimeException>(::vkCreateCommandPool(m_parent->getDevice()->handle(), &poolInfo, nullptr, &m_commandPool), "Unable to create command pool.");

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

		raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_parent->getDevice()->handle(), &createInfo, NULL, &m_timelineSemaphore), "Unable to create queue synchronization semaphore.");

		m_bound = true;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanQueue::VulkanQueue(const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId) :
	Resource<VkQueue>(nullptr), VulkanRuntimeObject<VulkanDevice>(device, &device), m_impl(makePimpl<VulkanQueueImpl>(this, type, priority, familyId, queueId))
{
}

VulkanQueue::~VulkanQueue() noexcept = default;

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

UniquePtr<VulkanCommandBuffer> VulkanQueue::createCommandBuffer(const bool& beginRecording) const
{
	return makeUnique<VulkanCommandBuffer>(*this, beginRecording);
}

UInt64 VulkanQueue::submit(const VulkanCommandBuffer& commandBuffer) const
{
	return this->submit(commandBuffer, {}, {}, {});
}

UInt64 VulkanQueue::submit(const VulkanCommandBuffer& commandBuffer, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End the command buffer.
	commandBuffer.end();

	// Create an array of all signal semaphores.
	Array<VkSemaphore> semaphoresToSignal(signalSemaphores.size());
	std::ranges::generate(semaphoresToSignal, [&signalSemaphores, i = 0]() mutable { return signalSemaphores[i++]; });
	semaphoresToSignal.insert(semaphoresToSignal.begin(), m_impl->m_timelineSemaphore);

	// Submit the command buffer.
	Array<UInt64> waitValues(waitForSemaphores.size(), 0);
	Array<UInt64> signalValues(semaphoresToSignal.size(), 0);
	signalValues[0] = ++m_impl->m_fenceValue;

	VkTimelineSemaphoreSubmitInfo timelineInfo {
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = static_cast<UInt32>(waitValues.size()),
		.pWaitSemaphoreValues = waitValues.data(),
		.signalSemaphoreValueCount = static_cast<UInt32>(signalValues.size()),
		.pSignalSemaphoreValues = signalValues.data()
	};

	VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = static_cast<UInt32>(waitForSemaphores.size()),
		.pWaitSemaphores = waitForSemaphores.data(),
		.pWaitDstStageMask = waitForStages.data(),
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer.handle(),
		.signalSemaphoreCount = static_cast<UInt32>(semaphoresToSignal.size()),
		.pSignalSemaphores = semaphoresToSignal.data()
	};

	// Submit the command buffer to the transfer queue.
	raiseIfFailed<RuntimeException>(::vkQueueSubmit(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");

	return signalValues[0];
}

UInt64 VulkanQueue::submit(Span<const VulkanCommandBuffer> commandBuffers) const
{
	return this->submit(commandBuffers, {}, {}, {});
}

UInt64 VulkanQueue::submit(Span<const VulkanCommandBuffer> commandBuffers, Span<VkSemaphore> waitForSemaphores, Span<VkPipelineStageFlags> waitForStages, Span<VkSemaphore> signalSemaphores) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End the command buffer.
	Array<VkCommandBuffer> handles(commandBuffers.size());
	std::ranges::generate(handles, [&commandBuffers, i = 0]() mutable {
		const auto& commandBuffer = commandBuffers[i++];
		commandBuffer.end();
		return commandBuffer.handle();
	});

	// Create an array of all signal semaphores.
	Array<VkSemaphore> semaphoresToSignal(signalSemaphores.size());
	std::ranges::generate(semaphoresToSignal, [&signalSemaphores, i = 0]() mutable { return signalSemaphores[i++]; });
	semaphoresToSignal.insert(semaphoresToSignal.begin(), m_impl->m_timelineSemaphore);

	// Submit the command buffer.
	Array<UInt64> waitValues(waitForSemaphores.size(), 0);
	Array<UInt64> signalValues(semaphoresToSignal.size(), 0);
	signalValues[0] = ++m_impl->m_fenceValue;

	VkTimelineSemaphoreSubmitInfo timelineInfo {
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = static_cast<UInt32>(waitValues.size()),
		.pWaitSemaphoreValues = waitValues.data(),
		.signalSemaphoreValueCount = static_cast<UInt32>(signalValues.size()),
		.pSignalSemaphoreValues = signalValues.data()
	};

	VkSubmitInfo submitInfo {
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

	return signalValues[0];
}

void VulkanQueue::waitFor(const UInt64& fence) const noexcept
{
	UInt64 completedValue{ 0 };
	//raiseIfFailed<RuntimeException>(::vkGetSemaphoreCounterValue(this->getDevice()->handle(), m_impl->m_timelineSemaphore, &completedValue), "Unable to query current queue timeline semaphore value.");
	::vkGetSemaphoreCounterValue(this->getDevice()->handle(), m_impl->m_timelineSemaphore, &completedValue);

	if (completedValue < fence)
	{
		VkSemaphoreWaitInfo waitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.pNext = NULL,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &m_impl->m_timelineSemaphore,
			.pValues = &fence
		};

		//raiseIfFailed<RuntimeException>(::vkWaitSemaphores(this->getDevice()->handle(), &waitInfo, std::numeric_limits<UInt64>::max()), "Unable to wait for queue timeline semaphore.");
		::vkWaitSemaphores(this->getDevice()->handle(), &waitInfo, std::numeric_limits<UInt64>::max());
	}
}

UInt64 VulkanQueue::currentFence() const noexcept
{
	return m_impl->m_fenceValue;
}