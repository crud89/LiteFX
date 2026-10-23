#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// Import required extensions.
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkQueueBeginDebugUtilsLabelEXT   vkQueueBeginDebugUtilsLabel;
extern PFN_vkQueueEndDebugUtilsLabelEXT     vkQueueEndDebugUtilsLabel;
extern PFN_vkQueueInsertDebugUtilsLabelEXT  vkQueueInsertDebugUtilsLabel;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Command allocator pool.
// ------------------------------------------------------------------------------------------------

class VulkanCommandAllocatorPool final {
public:
	struct Allocator {
		VkCommandPool Pool{};
		Array<VkCommandBuffer> FreePrimaries{}, FreeSecondaries{};
		Array<VkCommandBuffer> UsedPrimaries{}, UsedSecondaries{};
		UInt32 Unsubmitted{ 0 };
		UInt64 Fence{ 0 };
		bool Leased{ false }, Sealed{ false };
		ICommandBuffer::TrackedResources TrackedState{};

		bool isAvailable() const noexcept { return !Leased && !Sealed; }
		bool isIdle() const noexcept { return !Leased && Unsubmitted == 0; }
	};

private:
	static constexpr UInt32 MAX_IDLE_ALLOCATORS = 8;

	mutable std::mutex m_mutex;
	WeakPtr<const VulkanDevice> m_device;
	UInt32 m_queueFamily;
	Array<UniquePtr<Allocator>> m_allocators{};

public:
	VulkanCommandAllocatorPool(const VulkanDevice& device, UInt32 queueFamily) :
		m_device(device.weak_from_this()), m_queueFamily(queueFamily)
	{
	}

	VulkanCommandAllocatorPool(const VulkanCommandAllocatorPool&) = delete;
	VulkanCommandAllocatorPool(VulkanCommandAllocatorPool&&) noexcept = delete;
	VulkanCommandAllocatorPool& operator=(const VulkanCommandAllocatorPool&) = delete;
	VulkanCommandAllocatorPool& operator=(VulkanCommandAllocatorPool&&) noexcept = delete;

	~VulkanCommandAllocatorPool() noexcept
	{
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
		{
			if (!m_allocators.empty())
				LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release command allocators after the parent device has been released.");

			return;
		}

		// Destroying a command pool also frees all command buffers allocated from it.
		for (auto& allocator : m_allocators)
			::vkDestroyCommandPool(device->handle(), allocator->Pool, nullptr);
	}

private:
	// Collects all resources associated with the provided fence, so that they can be released, and resets the pools that are completed or discarded.
	void reset(UInt64 completedFence, Array<ICommandBuffer::TrackedResources>& resources)
	{
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			return;

		// Go through each allocator that is currently not leased and can thus be cleaned up.
		for (auto& allocator : m_allocators | std::views::filter([](const auto& allocator) { return allocator->isIdle(); }))
		{
			// Sealed allocators must wait for their fence. Unsealed ones only need a reset, if they contain discarded recordings.
			bool hasCompleted = allocator->Sealed && allocator->Fence <= completedFence;
			bool hasDiscarded = !allocator->Sealed && (!allocator->UsedPrimaries.empty() || !allocator->UsedSecondaries.empty());

			if (!hasCompleted && !hasDiscarded)
				continue;

			// Reset the pool.
			raiseIfFailed(::vkResetCommandPool(device->handle(), allocator->Pool, 0), "Unable to reset command pool.");

			// After a pool reset, all command buffers allocated from it are in initial state and can be reused.
			allocator->FreePrimaries.append_range(allocator->UsedPrimaries);
			allocator->FreeSecondaries.append_range(allocator->UsedSecondaries);
			allocator->UsedPrimaries.clear();
			allocator->UsedSecondaries.clear();

			// Store the resources tracked by the allocator in the caller's array.
			resources.push_back(std::exchange(allocator->TrackedState, {}));
			allocator->Sealed = false;
			allocator->Fence = 0;
		}

		// Trim idle allocators beyond the limit.
		UInt32 idle{ 0 };

		std::erase_if(m_allocators, [&](const auto& allocator) {
			if (!allocator->isIdle() || allocator->Sealed || ++idle <= MAX_IDLE_ALLOCATORS)
				return false;

			::vkDestroyCommandPool(device->handle(), allocator->Pool, nullptr);
			return true;
		});
	}

	// Creates a new allocator instance. 
	Allocator* getAllocator()
	{
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot create command allocator for a released device instance.");

		VkCommandPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = m_queueFamily
		};

		auto allocator = makeUnique<Allocator>();
		raiseIfFailed(::vkCreateCommandPool(device->handle(), &poolInfo, nullptr, &allocator->Pool), "Unable to create command pool.");

		return m_allocators.emplace_back(std::move(allocator)).get();
	}

public:
	Allocator& acquire(UInt64 completedFence)
	{
		// Store the tracked resources that can be released until the lock is released. Declaration order matters here; garbage is released after the lock is unlocked.
		Array<ICommandBuffer::TrackedResources> garbage{};
		std::lock_guard lock(m_mutex);

		// Reset all allocators that have completed.
		this->reset(completedFence, garbage);

		// Find an available allocator. Prefer allocators which already store recordings, to reduce the total number of allocators if possible.
		auto match = std::ranges::find_if(m_allocators, [](const auto& allocator) { return allocator->isAvailable() && allocator->Unsubmitted > 0; });

		if (match == m_allocators.end())
			match = std::ranges::find_if(m_allocators, [](const auto& allocator) { return allocator->isAvailable(); });

		// Get the allocator or create a new one, if none was found.
		Allocator* allocator = match != m_allocators.end() ? match->get() : this->getAllocator();
		allocator->Leased = true;
		++allocator->Unsubmitted;

		return *allocator;
	}

	// Returns a fresh command buffer from the allocator.
	VkCommandBuffer allocate(Allocator& allocator, bool secondary)
	{
		std::lock_guard lock(m_mutex);
		assert(allocator.Leased);

		// Check if there are unused command buffers in the allocator and re-use them.
		auto& freeBuffers = secondary ? allocator.FreeSecondaries : allocator.FreePrimaries;
		auto& usedBuffers = secondary ? allocator.UsedSecondaries : allocator.UsedPrimaries;

		VkCommandBuffer buffer{};

		if (!freeBuffers.empty())
		{
			buffer = freeBuffers.back();
			freeBuffers.pop_back();
		}
		else
		{
			// Otherwise create a new command buffer.
			auto device = m_device.lock();

			if (device == nullptr) [[unlikely]]
				throw RuntimeException("Cannot allocate command buffer from a released device instance.");

			VkCommandBufferAllocateInfo bufferInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = allocator.Pool,
				.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};

			raiseIfFailed(::vkAllocateCommandBuffers(device->handle(), &bufferInfo, &buffer), "Unable to allocate command buffer.");
		}

		usedBuffers.push_back(buffer);
		return buffer;
	}

	// Ends the recording. Other command buffers may record into the allocator afterwards.
	void release(Allocator& allocator) noexcept
	{
		std::lock_guard lock(m_mutex);
		allocator.Leased = false;
	}

	// Seals the allocator and retains copies of the tracked resources until `fence` has passed.
	void submitted(Allocator& allocator, UInt64 fence, bool firstSubmission, const ICommandBuffer::TrackedResources& resources)
	{
		std::lock_guard lock(m_mutex);

		allocator.Sealed = true;
		allocator.Fence = std::max(allocator.Fence, fence);

		if (firstSubmission)
		{
			assert(allocator.Unsubmitted > 0);
			--allocator.Unsubmitted;
		}

		auto& state = allocator.TrackedState;

		state.Resources.append_range(resources.Resources);
		state.DescriptorSets.append_range(resources.DescriptorSets);
		state.DescriptorRanges.append_range(resources.DescriptorRanges);
		state.SecondaryCommandBuffers.append_range(resources.SecondaryCommandBuffers);
	}

	// Drops a recording that has never been submitted. The command buffer keeps its own references, so no state is needed.
	void discarded(Allocator& allocator) noexcept
	{
		std::lock_guard lock(m_mutex);

		assert(allocator.Unsubmitted > 0);
		allocator.Leased = false;
		--allocator.Unsubmitted;
	}

	// Resets all allocators whose recordings have completed or have been discarded.
	void recycle(UInt64 completedFence)
	{
		// Store the tracked resources that can be released until the lock is released. Declaration order matters here; garbage is released after the lock is unlocked.
		Array<ICommandBuffer::TrackedResources> garbage{};
		std::lock_guard lock(m_mutex);

		this->reset(completedFence, garbage);
	}
};

// ------------------------------------------------------------------------------------------------
// Command allocator handle.
// ------------------------------------------------------------------------------------------------

class VulkanCommandAllocatorHandle final : public ICommandAllocatorHandle {
private:
	SharedPtr<VulkanCommandAllocatorPool> m_pool; 
	VulkanCommandAllocatorPool::Allocator* m_allocator;
	VkCommandBuffer m_commandBuffer;
	bool m_recording{ true }, m_submitted{ false };

public:
	VulkanCommandAllocatorHandle(SharedPtr<VulkanCommandAllocatorPool> pool, VulkanCommandAllocatorPool::Allocator& allocator, VkCommandBuffer commandBuffer) noexcept :
		m_pool(std::move(pool)), m_allocator(&allocator), m_commandBuffer(commandBuffer)
	{
	}

	VulkanCommandAllocatorHandle(const VulkanCommandAllocatorHandle&) = delete;
	VulkanCommandAllocatorHandle(VulkanCommandAllocatorHandle&&) noexcept = delete;
	VulkanCommandAllocatorHandle& operator=(const VulkanCommandAllocatorHandle&) = delete;
	VulkanCommandAllocatorHandle& operator=(VulkanCommandAllocatorHandle&&) noexcept = delete;

	~VulkanCommandAllocatorHandle() noexcept override
	{
		if (!m_submitted)
			m_pool->discarded(*m_allocator);
	}

public:
	VkCommandBuffer commandBuffer() const noexcept 
	{
		return m_commandBuffer; 
	}

	void release() noexcept override
	{
		if (m_recording)
		{
			m_pool->release(*m_allocator);
			m_recording = false;
		}
	}

	void submitted(UInt64 fence, const ICommandBuffer::TrackedResources& resources)
	{
		assert(!m_recording && "Command buffers must be ended before they are submitted.");
		assert(!m_submitted && "A recording can only be submitted once.");

		m_pool->submitted(*m_allocator, fence, true, resources);
		m_submitted = true;
		m_allocator = nullptr;
	}
};

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
	std::atomic<UInt64> m_fenceValue{ 0 };
	WeakPtr<const VulkanDevice> m_device;
	Array<Tuple<UInt64, SharedPtr<const VulkanCommandBuffer>>> m_submittedCommandBuffers;
	Array<VkSemaphoreSubmitInfo> m_crossQueueWaitInfos{};

public:
	VulkanQueueImpl(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId) :
		m_type(type), m_priority(priority), m_familyId(familyId), m_queueId(queueId), m_device(device.weak_from_this())
	{
	}

public:
	void release()
	{
		m_submittedCommandBuffers.clear();

		if (m_timelineSemaphore != VK_NULL_HANDLE)
		{
			auto device = m_device.lock();

			if (device != nullptr) [[likely]]
				::vkDestroySemaphore(device->handle(), m_timelineSemaphore, nullptr);
			else
				LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release command queue after the parent device instance.");
		}

		m_timelineSemaphore = {};
	}

	VkQueue initialize(const VulkanDevice& device)
	{
		// Create the queue instance.
		VkQueue queue{};
		::vkGetDeviceQueue(device.handle(), m_familyId, m_queueId, &queue);

		// Create a timeline semaphore for queue synchronization.
		VkSemaphoreTypeCreateInfo timelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = m_fenceValue.load(std::memory_order_acquire)
		};

		VkSemaphoreCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineCreateInfo,
			.flags = 0
		};

		raiseIfFailed(::vkCreateSemaphore(device.handle(), &createInfo, nullptr, &m_timelineSemaphore), "Unable to create queue synchronization semaphore.");

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
	this->handle() = m_impl->initialize(device);
}

VulkanQueue::~VulkanQueue() noexcept // NOLINT(bugprone-exception-escape)
{
	m_impl->release();
}

SharedPtr<const VulkanDevice> VulkanQueue::device() const noexcept
{
	return m_impl->m_device.lock();
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
void VulkanQueue::beginDebugRegion(const String& label, const Vectors::ByteVector3& color) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot begin debug region in a queue on a released device instance.");

	std::lock_guard<std::mutex> lock{ device->queueMutex(m_impl->m_familyId, m_impl->m_queueId) };

	VkDebugUtilsLabelEXT labelInfo {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { static_cast<float>(color.x()) / static_cast<float>(std::numeric_limits<UInt8>::max()), static_cast<float>(color.y()) / static_cast<float>(std::numeric_limits<UInt8>::max()), static_cast<float>(color.z()) / static_cast<float>(std::numeric_limits<UInt8>::max()), 1.0f }
	};
	
	if (::vkQueueBeginDebugUtilsLabel)
		::vkQueueBeginDebugUtilsLabel(this->handle(), &labelInfo);
}

void VulkanQueue::endDebugRegion() const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot end debug region in a queue on a released device instance.");

	std::lock_guard<std::mutex> lock{ device->queueMutex(m_impl->m_familyId, m_impl->m_queueId) };

	if (::vkQueueEndDebugUtilsLabel)
		::vkQueueEndDebugUtilsLabel(this->handle());
}

void VulkanQueue::setDebugMarker(const String& label, const Vectors::ByteVector3& color) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot set debug marker in a queue on a released device instance.");

	std::lock_guard<std::mutex> lock{ device->queueMutex(m_impl->m_familyId, m_impl->m_queueId) };

	VkDebugUtilsLabelEXT labelInfo {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = label.c_str(),
		.color = { static_cast<float>(color.x()) / static_cast<float>(std::numeric_limits<UInt8>::max()), static_cast<float>(color.y()) / static_cast<float>(std::numeric_limits<UInt8>::max()), static_cast<float>(color.z()) / static_cast<float>(std::numeric_limits<UInt8>::max()), 1.0f }
	};

	if (::vkQueueInsertDebugUtilsLabel)
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

UInt64 VulkanQueue::submit(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot submit command buffer to a queue on a released device instance.");

	if (commandBuffer == nullptr) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "The command buffer must be initialized.");

	if (commandBuffer->isSecondary()) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "The command buffer must be a primary command buffer.");

	// Begin event.
	this->submitting(this, { { std::static_pointer_cast<const ICommandBuffer>(commandBuffer) } });

	std::unique_lock<std::mutex> lock(device->queueMutex(m_impl->m_familyId, m_impl->m_queueId));

	// Remove all previously submitted command buffers, that have already finished.
	UInt64 completedValue = 0;
	::vkGetSemaphoreCounterValue(device->handle(), m_impl->m_timelineSemaphore, &completedValue);
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	commandBuffer->end();

	// Submit the command buffer. Here we wait for the current fence on the queue, which enforces command buffers to be executed sequentially. This isn't necessary in Vulkan, but it 
	// matches the D3D12 behavior. This allows us to make portable assumptions for barriers (e.g., first-use barriers can use ResourceAccess::None, etc.).
	auto currentFence = m_impl->m_fenceValue.fetch_add(1u);
	auto signalFence = currentFence + 1u;

	VkSemaphoreSubmitInfo waitSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = currentFence,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	m_impl->m_crossQueueWaitInfos.push_back(waitSemaphoreInfo);

	VkSemaphoreSubmitInfo signalSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = signalFence,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	VkCommandBufferSubmitInfo commandBufferInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = commandBuffer->handle()
	};

	VkSubmitInfo2 submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = static_cast<UInt32>(m_impl->m_crossQueueWaitInfos.size()),
		.pWaitSemaphoreInfos = m_impl->m_crossQueueWaitInfos.data(),
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	// Submit the command buffer to the queue.
	try
	{
		raiseIfFailed(::vkQueueSubmit2(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");
	}
	catch (...)
	{
		// Prevent deadlocks if this fails.
		m_impl->m_fenceValue.fetch_sub(1u);
		throw;
	}

	// Add the command buffer to the submitted command buffers list.
	m_impl->m_submittedCommandBuffers.emplace_back(signalFence, commandBuffer);

	// Clear the wait list.
	m_impl->m_crossQueueWaitInfos.clear();

	// Fire end event.
	lock.release();
	this->submitted(this, { signalFence });

	return signalFence;
}

UInt64 VulkanQueue::submit(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot submit command buffer to a queue on a released device instance.");

	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return buffer != nullptr; })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is not initialized.");

	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return !buffer->isSecondary(); })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is a secondary command buffer, which is not allowed to be submitted to a command queue.");

	// Begin event.
	this->submitting(this, { commandBuffers 
		| std::views::transform([](const SharedPtr<const VulkanCommandBuffer>& buffer) { return std::static_pointer_cast<const ICommandBuffer>(buffer); })
		| std::ranges::to<Array<SharedPtr<const ICommandBuffer>>>() });

	std::unique_lock<std::mutex> lock(device->queueMutex(m_impl->m_familyId, m_impl->m_queueId));

	// Remove all previously submitted command buffers, that have already finished.
	UInt64 completedValue = 0;
	::vkGetSemaphoreCounterValue(device->handle(), m_impl->m_timelineSemaphore, &completedValue);
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	auto commandBufferInfos = commandBuffers | std::views::transform([](const SharedPtr<const VulkanCommandBuffer>& buffer) {
		buffer->end();

		return VkCommandBufferSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = buffer->handle()
		};
	}) | std::ranges::to<Array<VkCommandBufferSubmitInfo>>();

	// Submit the command buffer. Here we wait for the current fence on the queue, which enforces command buffers to be executed sequentially. This isn't necessary in Vulkan, but it 
	// matches the D3D12 behavior. This allows us to make portable assumptions for barriers (e.g., first-use barriers can use ResourceAccess::None, etc.).
	auto currentFence = m_impl->m_fenceValue.fetch_add(1u);
	auto signalFence = currentFence + 1u;
	
	VkSemaphoreSubmitInfo waitSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = currentFence,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	m_impl->m_crossQueueWaitInfos.push_back(waitSemaphoreInfo);

	VkSemaphoreSubmitInfo signalSemaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_impl->m_timelineSemaphore,
		.value = signalFence,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	VkSubmitInfo2 submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = static_cast<UInt32>(m_impl->m_crossQueueWaitInfos.size()),
		.pWaitSemaphoreInfos = m_impl->m_crossQueueWaitInfos.data(),
		.commandBufferInfoCount = static_cast<UInt32>(commandBufferInfos.size()),
		.pCommandBufferInfos = commandBufferInfos.data(),
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	// Submit the command buffer to the queue.
	try
	{
		raiseIfFailed(::vkQueueSubmit2(this->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit command buffer to queue.");
	}
	catch (...)
	{
		// Prevent deadlocks if this fails.
		m_impl->m_fenceValue.fetch_sub(1u);
		throw;
	}

	// Add the command buffers to the submitted command buffers list.
	std::ranges::for_each(commandBuffers, [this, &signalFence](const SharedPtr<const VulkanCommandBuffer>& buffer) { m_impl->m_submittedCommandBuffers.emplace_back(signalFence, buffer); });

	// Clear the wait list.
	m_impl->m_crossQueueWaitInfos.clear();

	// Fire end event.
	lock.release();
	this->submitted(this, { signalFence });

	return signalFence;
}

void VulkanQueue::waitFor(UInt64 fence) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot wait for fence on a released device instance.");

	UInt64 completedValue{ 0 };
	raiseIfFailed(::vkGetSemaphoreCounterValue(device->handle(), m_impl->m_timelineSemaphore, &completedValue), "Unable to query current queue timeline semaphore value.");

	if (completedValue < fence)
	{
		VkSemaphoreWaitInfo waitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_impl->m_timelineSemaphore,
			.pValues = &fence
		};

		raiseIfFailed(::vkWaitSemaphores(device->handle(), &waitInfo, std::numeric_limits<UInt64>::max()), "Unable to wait for queue timeline semaphore.");
	}

	// Synchronize command buffer release.
	std::lock_guard<std::mutex> lock{ device->queueMutex(m_impl->m_familyId, m_impl->m_queueId) };

	m_impl->releaseCommandBuffers(*this, fence);
}

void VulkanQueue::waitFor(const VulkanQueue& queue, UInt64 fence) const
{
	// Waiting for a future fence value is possible, but most certainly not intended, since there's a risk of deadlocking the queue, if the fence never is submitted.
	if (fence > queue.currentFence()) [[unlikely]] 
	{
		LITEFX_ERROR(VULKAN_LOG, "Cannot wait for fence {0}, which has not been submitted to the queue yet. The current fence is at {1}.", fence, queue.currentFence());
		return;
	}
	else if (fence == 0)
	{
		return;
	}

	// We cannot directly push an empty submission with only synchronization primitives to the queue, since Vulkan doesn't define how they should be serialized by the 
	// driver. Instead we cache the fence and semaphore and append it to the next submission's wait list. This also means, that the other queue must remain alive as 
	// long as such waits are pending, but we shouldn't be able to delete queues at runtime anyway.
	auto device = m_impl->m_device.lock();
	
	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot wait for fence on a released device instance.");

	std::lock_guard<std::mutex> lock(device->queueMutex(m_impl->m_familyId, m_impl->m_queueId));

	VkSemaphoreSubmitInfo waitSemaphoreInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = queue.m_impl->m_timelineSemaphore,
		.value = fence,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	m_impl->m_crossQueueWaitInfos.push_back(waitSemaphoreInfo);
}

UInt64 VulkanQueue::currentFence() const noexcept
{
	return m_impl->m_fenceValue.load(std::memory_order_acquire);
}

UInt64 VulkanQueue::lastCompletedFence() const noexcept
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		return std::numeric_limits<UInt64>::max();

	UInt64 completedValue{ 0 };
	vkGetSemaphoreCounterValue(device->handle(), m_impl->m_timelineSemaphore, &completedValue);

	return completedValue;
}