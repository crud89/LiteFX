#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Queue::DirectX12QueueImpl : public Implement<DirectX12Queue> {
public:
	friend class DirectX12Queue;

private:
	QueueType m_type;
	QueuePriority m_priority;
	ComPtr<ID3D12Fence> m_fence;
	UInt64 m_fenceValue{ 0 };
	bool m_bound;
	mutable std::mutex m_mutex;

public:
	DirectX12QueueImpl(DirectX12Queue* parent, const QueueType& type, const QueuePriority& priority) :
		base(parent), m_bound(false), m_type(type), m_priority(priority)
	{
	}

public:
	[[nodiscard]]
	ComPtr<ID3D12CommandQueue> initialize()
	{
		ComPtr<ID3D12CommandQueue> commandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		if (LITEFX_FLAG_IS_SET(m_type, QueueType::Graphics))
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		else if (LITEFX_FLAG_IS_SET(m_type, QueueType::Compute))
			desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		else if (LITEFX_FLAG_IS_SET(m_type, QueueType::Transfer))
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

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

		raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)), "Unable to create command queue of type {0} with priority {1}.", m_type, m_priority);
		raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create command buffer synchronization fence.");

		return commandQueue;
	}

	void release()
	{
		if (!m_bound)
			return;

		// TODO: Destroy command pool, if bound.
		m_bound = false;
	}

	void bind()
	{
		if (m_bound)
			return;

		// TODO: Create a command pool.
		m_bound = true;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Queue::DirectX12Queue(const DirectX12Device& device, const QueueType& type, const QueuePriority& priority) :
	ComResource<ID3D12CommandQueue>(nullptr), DirectX12RuntimeObject(device, &device), m_impl(makePimpl<DirectX12QueueImpl>(this, type, priority))
{
	this->handle() = m_impl->initialize();
}

DirectX12Queue::~DirectX12Queue() noexcept
{
	this->release();
}

bool DirectX12Queue::isBound() const noexcept
{
	return m_impl->m_bound;
}

const QueueType& DirectX12Queue::type() const noexcept
{
	return m_impl->m_type;
}

const QueuePriority& DirectX12Queue::priority() const noexcept
{
	return m_impl->m_priority;
}

void DirectX12Queue::bind()
{
	m_impl->bind();
}

void DirectX12Queue::release()
{
	m_impl->release();
}

UniquePtr<DirectX12CommandBuffer> DirectX12Queue::createCommandBuffer(const bool& beginRecording) const
{
	return makeUnique<DirectX12CommandBuffer>(*this, beginRecording);
}

UInt64 DirectX12Queue::submit(const DirectX12CommandBuffer& commandBuffer) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End the command buffer.
	commandBuffer.end();
	
	// Submit the command buffer.
	Array<ID3D12CommandList*> commandBuffers{ commandBuffer.handle().Get() };
	this->handle()->ExecuteCommandLists(1, commandBuffers.data());

	// Insert a fence and return the value.
	raiseIfFailed<RuntimeException>(this->handle()->Signal(m_impl->m_fence.Get(), ++m_impl->m_fenceValue), "Unable to add fence signal to command buffer.");

	return m_impl->m_fenceValue;
}

UInt64 DirectX12Queue::submit(const Array<const DirectX12CommandBuffer*>& commandBuffers) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_mutex);

	// End and submit the command buffers.
	Array<ID3D12CommandList*> handles(commandBuffers.size());
	std::ranges::generate(handles, [&commandBuffers, i = 0]() mutable {
		const auto commandBuffer = commandBuffers[i++];
		commandBuffer->end();
		return commandBuffer->handle().Get();
	});

	this->handle()->ExecuteCommandLists(static_cast<UInt32>(handles.size()), handles.data());

	// Insert a fence and return the value.
	raiseIfFailed<RuntimeException>(this->handle()->Signal(m_impl->m_fence.Get(), ++m_impl->m_fenceValue), "Unable to add fence signal to command buffer.");

	return m_impl->m_fenceValue;
}

void DirectX12Queue::waitFor(const UInt64& fence) const noexcept
{
	if (m_impl->m_fence->GetCompletedValue() < fence)
	{
		HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
		HRESULT hr = m_impl->m_fence->SetEventOnCompletion(fence, eventHandle);

		if (SUCCEEDED(hr))
			::WaitForSingleObject(eventHandle, INFINITE);

		::CloseHandle(eventHandle);
		raiseIfFailed<RuntimeException>(hr, "Unable to register fence completion event.");
	}
}

UInt64 DirectX12Queue::currentFence() const noexcept
{
	return m_impl->m_fenceValue;
}