#include <litefx/backends/dx12.hpp>

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
#include <pix3.h>
#endif

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Command allocator pool.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandAllocatorPool final : public std::enable_shared_from_this<DirectX12CommandAllocatorPool> {
public:
	struct Allocator {
		ComPtr<ID3D12CommandAllocator> CommandAllocator{};
		ComPtr<ID3D12GraphicsCommandList7> CommandList{};
		bool Dirty{ false };
	};

private:
	static constexpr std::size_t MAX_FREE_ALLOCATORS = 32;

	std::mutex m_mutex;
	WeakPtr<const DirectX12Device> m_device;
	D3D12_COMMAND_LIST_TYPE m_type;
	Array<UniquePtr<Allocator>> m_freeAllocators{};

public:
	DirectX12CommandAllocatorPool(const DirectX12Device& device, D3D12_COMMAND_LIST_TYPE type) :
		m_device(device.weak_from_this()), m_type(type)
	{
	}

	DirectX12CommandAllocatorPool(const DirectX12CommandAllocatorPool&) = delete;
	DirectX12CommandAllocatorPool(DirectX12CommandAllocatorPool&&) noexcept = delete;
	DirectX12CommandAllocatorPool& operator=(const DirectX12CommandAllocatorPool&) = delete;
	DirectX12CommandAllocatorPool& operator=(DirectX12CommandAllocatorPool&&) noexcept = delete;

	~DirectX12CommandAllocatorPool() noexcept = default;

public:
	UniquePtr<IDirectX12CommandAllocatorHandle> acquire();

	void release(UniquePtr<Allocator>&& allocator) noexcept // NOLINT(bugprone-exception-escape)
	{
		allocator->Dirty = true;
		std::lock_guard lock(m_mutex);

		if (m_freeAllocators.size() < MAX_FREE_ALLOCATORS)
			m_freeAllocators.push_back(std::move(allocator));

		// Allocators beyond the `MAX_FREE_ALLOCATORS` are released when `allocator` goes out of scope.
	}
};

// ------------------------------------------------------------------------------------------------
// Command allocator handle.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandAllocatorHandle final : public IDirectX12CommandAllocatorHandle {
private:
	SharedPtr<DirectX12CommandAllocatorPool> m_pool;
	UniquePtr<DirectX12CommandAllocatorPool::Allocator> m_allocator;

public:
	DirectX12CommandAllocatorHandle(SharedPtr<DirectX12CommandAllocatorPool> pool, UniquePtr<DirectX12CommandAllocatorPool::Allocator>&& allocator) noexcept :
		m_pool(std::move(pool)), m_allocator(std::move(allocator))
	{
	}

	DirectX12CommandAllocatorHandle(const DirectX12CommandAllocatorHandle&) = delete;
	DirectX12CommandAllocatorHandle(DirectX12CommandAllocatorHandle&&) noexcept = delete;
	DirectX12CommandAllocatorHandle& operator=(const DirectX12CommandAllocatorHandle&) = delete;
	DirectX12CommandAllocatorHandle& operator=(DirectX12CommandAllocatorHandle&&) noexcept = delete;

	~DirectX12CommandAllocatorHandle() noexcept override
	{
		m_pool->release(std::move(m_allocator));
	}

	ComPtr<ID3D12GraphicsCommandList7> commandBuffer() const noexcept override
	{
		return m_allocator->CommandList;
	}
};

UniquePtr<IDirectX12CommandAllocatorHandle> DirectX12CommandAllocatorPool::acquire()
{
	auto device = m_device.lock();
	bool opened{ false };

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot acquire a command allocator from a released device instance.");

	// Check if there are any free allocators and acquire one from the list in this case.
	UniquePtr<Allocator> allocator{};

	{
		std::lock_guard lock(m_mutex);

		if (!m_freeAllocators.empty())
		{
			allocator = std::move(m_freeAllocators.back());
			m_freeAllocators.pop_back();
		}
	}

	// If there was no free allocator, create a new one.
	if (allocator == nullptr)
	{
		allocator = makeUnique<Allocator>();
		raiseIfFailed(device->handle()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&allocator->CommandAllocator)), "Unable to create command allocator.");
	}

	try
	{
		// If the allocator is dirty, reset the underlying command pool.
		if (allocator->Dirty)
		{
			raiseIfFailed(allocator->CommandAllocator->Reset(), "Unable to reset command allocator.");
			allocator->Dirty = false;
		}

		// Allocate a command buffer and return it.
		if (allocator->CommandList == nullptr)
			raiseIfFailed(device->handle()->CreateCommandList(0, m_type, allocator->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&allocator->CommandList)), "Unable to create command list.");
		else
			raiseIfFailed(allocator->CommandList->Reset(allocator->CommandAllocator.Get(), nullptr), "Unable to reset command list.");

#ifndef NDEBUG
		// Reset the name of the command buffer, such that they don't leak into re-used ones.
		allocator->CommandList->SetName(L"Unnamed command buffer");
#endif

		opened = true;
		return makeUnique<DirectX12CommandAllocatorHandle>(this->shared_from_this(), std::move(allocator));
	}
	catch (...)
	{
		// Make sure the list is closed again, before the allocator is reused.
		if (allocator->CommandList != nullptr && opened) 
		{
			auto result = allocator->CommandList->Close();

			if (FAILED(result))
				LITEFX_FATAL_ERROR(DIRECTX12_LOG, "Unable to reset command buffer after a failed allocation: {0}", result);
		}

		this->release(std::move(allocator));
		throw;
	}
}

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Queue::DirectX12QueueImpl {
public:
	friend class DirectX12Queue;

private:
	WeakPtr<const DirectX12Device> m_device;
	QueueType m_type;
	QueuePriority m_priority;
	ComPtr<ID3D12Fence> m_fence;
	std::atomic<UInt64> m_fenceValue{ 0 };
	mutable std::mutex m_mutex;
	Array<Tuple<UInt64, SharedPtr<const DirectX12CommandBuffer>>> m_submittedCommandBuffers;
	SharedPtr<DirectX12CommandAllocatorPool> m_allocators, m_bundleAllocators;

public:
	DirectX12QueueImpl(const DirectX12Device& device, QueueType type, QueuePriority priority) :
		m_device(device.weak_from_this()), m_type(type), m_priority(priority)
	{
	}

	DirectX12QueueImpl(const DirectX12QueueImpl&) = delete;
	DirectX12QueueImpl(DirectX12QueueImpl&&) noexcept = delete;
	DirectX12QueueImpl& operator=(const DirectX12QueueImpl&) = delete;
	DirectX12QueueImpl& operator=(DirectX12QueueImpl&&) noexcept = delete;

	~DirectX12QueueImpl()
	{
		m_submittedCommandBuffers.clear();
		m_allocators.reset();
		m_bundleAllocators.reset();
	}

public:
	[[nodiscard]]
	ComPtr<ID3D12CommandQueue> initialize(const DirectX12Device& device)
	{
		ComPtr<ID3D12CommandQueue> commandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		if (m_type == QueueType::Graphics)
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		else if (m_type == QueueType::Compute)
			desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		else if (m_type == QueueType::VideoDecode)
			desc.Type = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
		else if (m_type == QueueType::VideoEncode)
			desc.Type = D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
		else if (m_type == QueueType::Transfer)
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		else // Combinations are not supported here. All queues implicitly support transfer operations, but it is not valid to provide combinations like `QueueType::Graphics | QueueType::VideoEncode`.
			throw InvalidArgumentException("type", "Unsupported combination of queue types. Only specify one queue type, even if the queue needs to support other tasks).");

		switch (m_priority)
		{
		default:
		case QueuePriority::Normal:
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			break;
		case QueuePriority::High:
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
			break;
		case QueuePriority::Realtime:
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
			break;
		}

		raiseIfFailed(device.handle()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)), "Unable to create command queue of type {0} with priority {1}.", m_type, m_priority);
		raiseIfFailed(device.handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create command buffer synchronization fence.");

		// Create the command allocator pools.
		m_allocators = makeShared<DirectX12CommandAllocatorPool>(device, desc.Type);
		m_bundleAllocators = makeShared<DirectX12CommandAllocatorPool>(device, D3D12_COMMAND_LIST_TYPE_BUNDLE);

		return commandQueue;
	}

	void releaseCommandBuffers(const DirectX12Queue& queue, UInt64 beforeFence)
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

DirectX12Queue::DirectX12Queue(const DirectX12Device& device, QueueType type, QueuePriority priority) :
	ComResource<ID3D12CommandQueue>(nullptr), m_impl(device, type, priority)
{
	this->handle() = m_impl->initialize(device);
}

UniquePtr<IDirectX12CommandAllocatorHandle> DirectX12Queue::acquireCommandAllocator(bool bundle) const
{
	if (m_impl->m_type == QueueType::VideoDecode || m_impl->m_type == QueueType::VideoEncode) [[unlikely]]
		throw RuntimeException("Command buffers are not supported on video queues.");

	if (bundle && m_impl->m_type != QueueType::Graphics) [[unlikely]]
		throw RuntimeException("Secondary command buffers are only supported on the graphics queue.");

	return bundle ? m_impl->m_bundleAllocators->acquire() : m_impl->m_allocators->acquire();
}

SharedPtr<const DirectX12Device> DirectX12Queue::device() const noexcept
{
	return m_impl->m_device.lock();
}

QueueType DirectX12Queue::type() const noexcept
{
	return m_impl->m_type;
}

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
void DirectX12Queue::beginDebugRegion(const String& label, const Vectors::ByteVector3& color) const
{
	::PIXBeginEvent(this->handle().Get(), PIX_COLOR(color.x(), color.y(), color.z()), label.c_str());
}

void DirectX12Queue::endDebugRegion() const
{
	::PIXEndEvent(this->handle().Get());
}

void DirectX12Queue::setDebugMarker(const String& label, const Vectors::ByteVector3& color) const
{
	::PIXSetMarker(this->handle().Get(), PIX_COLOR(color.x(), color.y(), color.z()), label.c_str());
}
#endif // defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)

QueuePriority DirectX12Queue::priority() const noexcept
{
	return m_impl->m_priority;
}

SharedPtr<DirectX12CommandBuffer> DirectX12Queue::createCommandBuffer(bool secondary) const
{
	return DirectX12CommandBuffer::create(*this, !secondary);
}

UInt64 DirectX12Queue::submit(const SharedPtr<const DirectX12CommandBuffer>& commandBuffer) const
{
	if (commandBuffer == nullptr)
		throw InvalidArgumentException("commandBuffer", "The command buffer must be initialized.");

	if (commandBuffer->secondary())
		throw InvalidArgumentException("commandBuffer", "The command buffer must be a primary command buffer.");

	if (!commandBuffer->recording()) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "The command buffer has already been submitted.");

	// Begin event.
	this->submitting(this, { { std::static_pointer_cast<const ICommandBuffer>(commandBuffer) } });

	// Remove all previously submitted command buffers, that have already finished.
	std::unique_lock<std::mutex> lock(m_impl->m_mutex);

	auto completedValue = m_impl->m_fence->GetCompletedValue();
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	commandBuffer->end();
	
	// Submit the command buffer.
	Array<ID3D12CommandList*> commandBuffers{ commandBuffer->handle().Get() };
	this->handle()->ExecuteCommandLists(1, commandBuffers.data());

	// Insert a fence and return the value.
	auto fence = m_impl->m_fenceValue.fetch_add(1u) + 1u;
	raiseIfFailed(this->handle()->Signal(m_impl->m_fence.Get(), fence), "Unable to add fence signal to command buffer.");

	// Add the command buffer to the submitted command buffers list.
	m_impl->m_submittedCommandBuffers.emplace_back(fence, commandBuffer);

	// Fire end event.
	lock.unlock();
	this->submitted(this, { fence });

	return fence;
}

UInt64 DirectX12Queue::submit(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const
{
	auto buffers = commandBuffers | std::ranges::to<std::vector>();

	if (!std::ranges::all_of(buffers, [](const auto& buffer) { return buffer != nullptr; }))
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is not initialized.");

	if (!std::ranges::all_of(buffers, [](const auto& buffer) { return !buffer->secondary(); }))
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is a secondary command buffer, which is not allowed to be submitted to a command queue.");

	if (std::ranges::any_of(buffers, [](const auto& buffer) { return !buffer->recording(); })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "At least one command buffer has already been submitted.");

	// Begin event.
	this->submitting(this, { buffers
		| std::views::transform([](const SharedPtr<const DirectX12CommandBuffer>& buffer) { return std::static_pointer_cast<const ICommandBuffer>(buffer); })
		| std::ranges::to<Array<SharedPtr<const ICommandBuffer>>>() });

	// Remove all previously submitted command buffers, that have already finished.
	std::unique_lock<std::mutex> lock(m_impl->m_mutex);

	auto completedValue = m_impl->m_fence->GetCompletedValue();
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End and submit the command buffers.
	for (auto buffer = buffers.begin(); buffer != buffers.end(); ++buffer)
		(*buffer)->end();

	auto handles = buffers
		| std::views::transform([](const SharedPtr<const DirectX12CommandBuffer>& buffer) { return buffer->handle().Get(); })
		| std::ranges::to<Array<ID3D12CommandList*>>();

	this->handle()->ExecuteCommandLists(static_cast<UInt32>(handles.size()), handles.data());

	// Insert a fence and return the value.
	auto fence = m_impl->m_fenceValue.fetch_add(1u) + 1u;
	raiseIfFailed(this->handle()->Signal(m_impl->m_fence.Get(), fence), "Unable to add fence signal to command buffer.");

	// Add the command buffers to the submitted command buffers list.
	std::ranges::for_each(buffers, [this, &fence](const SharedPtr<const DirectX12CommandBuffer>& buffer) { m_impl->m_submittedCommandBuffers.emplace_back(fence, buffer); });

	// Fire end event.
	lock.unlock();
	this->submitted(this, { fence });

	return fence;
}

void DirectX12Queue::waitFor(UInt64 fence) const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot wait for fence on a released device instance.");

	auto completedValue = m_impl->m_fence->GetCompletedValue();

	if (completedValue < fence)
	{
		HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
		HRESULT hr = m_impl->m_fence->SetEventOnCompletion(fence, eventHandle);

		if (SUCCEEDED(hr))
			::WaitForSingleObject(eventHandle, INFINITE);

		::CloseHandle(eventHandle);
		raiseIfFailed(hr, "Unable to register fence completion event.");
	}

	// Synchronize command buffer release.
	std::lock_guard<std::mutex> lock{ m_impl->m_mutex };

	m_impl->releaseCommandBuffers(*this, fence);
}

void DirectX12Queue::waitFor(const DirectX12Queue& queue, UInt64 fence) const
{
	// Waiting for a future fence value is possible, but most certainly not intended, since there's a risk of deadlocking the queue, if the fence never is submitted.
	if (fence > queue.currentFence()) [[unlikely]]
		LITEFX_WARNING(DIRECTX12_LOG, "Waiting for fence {0} which has not been submitted to the queue yet. The current fence is at {1}.", fence, queue.currentFence());
	else if (fence == 0)
		return;

	this->handle()->Wait(queue.m_impl->m_fence.Get(), fence);
}

UInt64 DirectX12Queue::currentFence() const noexcept
{
	return m_impl->m_fenceValue.load(std::memory_order_acquire);
}

UInt64 DirectX12Queue::lastCompletedFence() const noexcept
{
	return m_impl->m_fence->GetCompletedValue();
}