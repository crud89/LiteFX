#include <litefx/backends/dx12.hpp>

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
#include <pix3.h>
#endif

using namespace LiteFX::Rendering::Backends;

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
	UInt64 m_fenceValue{ 0 };
	mutable std::mutex m_mutex;
	Array<Tuple<UInt64, SharedPtr<const DirectX12CommandBuffer>>> m_submittedCommandBuffers;

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

//DirectX12Queue::DirectX12Queue(DirectX12Queue&&) noexcept = default;
//DirectX12Queue& DirectX12Queue::operator=(DirectX12Queue&&) noexcept = default;
DirectX12Queue::~DirectX12Queue() noexcept = default;

SharedPtr<const DirectX12Device> DirectX12Queue::device() const noexcept
{
	return m_impl->m_device.lock();
}

QueueType DirectX12Queue::type() const noexcept
{
	return m_impl->m_type;
}

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
void DirectX12Queue::beginDebugRegion(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	::PIXBeginEvent(this->handle().Get(), PIX_COLOR(color.x(), color.y(), color.z()), label.c_str());
}

void DirectX12Queue::endDebugRegion() const noexcept
{
	::PIXEndEvent(this->handle().Get());
}

void DirectX12Queue::setDebugMarker(const String& label, const Vectors::ByteVector3& color) const noexcept
{
	::PIXSetMarker(this->handle().Get(), PIX_COLOR(color.x(), color.y(), color.z()), label.c_str());
}
#endif // defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)

QueuePriority DirectX12Queue::priority() const noexcept
{
	return m_impl->m_priority;
}

SharedPtr<DirectX12CommandBuffer> DirectX12Queue::createCommandBuffer(bool beginRecording, bool secondary) const
{
	return DirectX12CommandBuffer::create(*this, beginRecording, !secondary);
}

UInt64 DirectX12Queue::submit(const SharedPtr<const DirectX12CommandBuffer>& commandBuffer) const
{
	if (commandBuffer == nullptr)
		throw InvalidArgumentException("commandBuffer", "The command buffer must be initialized.");

	if (commandBuffer->isSecondary())
		throw InvalidArgumentException("commandBuffer", "The command buffer must be a primary command buffer.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// Begin event.
	this->submitting(this, { { std::static_pointer_cast<const ICommandBuffer>(commandBuffer) } });

	// Remove all previously submitted command buffers, that have already finished.
	auto completedValue = m_impl->m_fence->GetCompletedValue();
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End the command buffer.
	commandBuffer->end();
	
	// Submit the command buffer.
	Array<ID3D12CommandList*> commandBuffers{ commandBuffer->handle().Get() };
	this->handle()->ExecuteCommandLists(1, commandBuffers.data());

	// Insert a fence and return the value.
	auto fence = ++m_impl->m_fenceValue;
	raiseIfFailed(this->handle()->Signal(m_impl->m_fence.Get(), fence), "Unable to add fence signal to command buffer.");

	// Add the command buffer to the submitted command buffers list.
	m_impl->m_submittedCommandBuffers.emplace_back(fence, commandBuffer);

	// Fire end event.
	this->submitted(this, { fence });
	return fence;
}

UInt64 DirectX12Queue::submit(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const
{
	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return buffer != nullptr; }))
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is not initialized.");

	if (!std::ranges::all_of(commandBuffers, [](const auto& buffer) { return !buffer->isSecondary(); }))
		throw InvalidArgumentException("commandBuffers", "At least one command buffer is a secondary command buffer, which is not allowed to be submitted to a command queue.");

	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// Begin event.
	this->submitting(this, { commandBuffers 
		| std::views::transform([](const SharedPtr<const DirectX12CommandBuffer>& buffer) { return std::static_pointer_cast<const ICommandBuffer>(buffer); }) 
		| std::ranges::to<Array<SharedPtr<const ICommandBuffer>>>() });

	// Remove all previously submitted command buffers, that have already finished.
	auto completedValue = m_impl->m_fence->GetCompletedValue();
	m_impl->releaseCommandBuffers(*this, completedValue);

	// End and submit the command buffers.
	for (auto buffer = commandBuffers.begin(); buffer != commandBuffers.end(); ++buffer)
		(*buffer)->end();

	auto handles = commandBuffers 
		| std::views::transform([](const SharedPtr<const DirectX12CommandBuffer>& buffer) { return buffer->handle().Get(); })
		| std::ranges::to<Array<ID3D12CommandList*>>();

	this->handle()->ExecuteCommandLists(static_cast<UInt32>(handles.size()), handles.data());

	// Insert a fence and return the value.
	auto fence = ++m_impl->m_fenceValue;
	raiseIfFailed(this->handle()->Signal(m_impl->m_fence.Get(), fence), "Unable to add fence signal to command buffer.");

	// Add the command buffers to the submitted command buffers list.
	std::ranges::for_each(commandBuffers, [this, &fence](const SharedPtr<const DirectX12CommandBuffer>& buffer) { m_impl->m_submittedCommandBuffers.emplace_back(fence, buffer); });

	// Fire end event.
	this->submitted(this, { fence });
	return fence;
}

void DirectX12Queue::waitFor(UInt64 fence) const
{
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

	m_impl->releaseCommandBuffers(*this, fence);
}

void DirectX12Queue::waitFor(const DirectX12Queue& queue, UInt64 fence) const noexcept
{
	this->handle()->Wait(queue.m_impl->m_fence.Get(), fence);
}

UInt64 DirectX12Queue::currentFence() const noexcept
{
	return m_impl->m_fenceValue;
}