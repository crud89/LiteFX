#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandBuffer::DirectX12CommandBufferImpl : public Implement<DirectX12CommandBuffer> {
public:
	friend class DirectX12CommandBuffer;

private:
	const DirectX12Queue* m_queue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Fence> m_fence;
	UINT32 m_currentSignal;
#ifndef NDEBUG
	bool m_signalQueued{ false };
#endif

public:
	DirectX12CommandBufferImpl(DirectX12CommandBuffer* parent, const DirectX12Queue* queue) :
		base(parent), m_queue(queue), m_currentSignal(0)
	{
		if (queue == nullptr)
			throw ArgumentNotInitializedException("The command queue must be initialized.");
	}

public:
	ComPtr<ID3D12GraphicsCommandList4> initialize()
	{
		auto device = dynamic_cast<const DirectX12Device*>(m_queue->getDevice());
				
		if (device == nullptr)
			throw InvalidArgumentException("The device is not a valid command queue DirectX12 device.");

		// Create a command allocator.
		D3D12_COMMAND_LIST_TYPE type;

		switch (m_queue->getType())
		{
		case QueueType::Compute: type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
		case QueueType::Transfer: type = D3D12_COMMAND_LIST_TYPE_COPY; break;
		default:
		case QueueType::Graphics: type = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
		}

		raiseIfFailed<RuntimeException>(device->handle()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator)), "Unable to create command allocator for command buffer.");
		raiseIfFailed<RuntimeException>(device->handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create command buffer synchronization fence.");

		// Create the actual command list.
		ComPtr<ID3D12GraphicsCommandList4> commandList;
		// TODO: Also pass the pipeline state, if possible. 
		raiseIfFailed<RuntimeException>(device->handle()->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");
		raiseIfFailed<RuntimeException>(commandList->Close(), "Unable to close command list.");
		
		return commandList;
	}

	HRESULT queueSignal()
	{
#ifndef NDEBUG
		if (m_signalQueued)
			throw RuntimeException("Another signal has already been queued. Don't call 'begin()' on the same command buffer twice, since this may cause race conditions!");

		HRESULT hr = m_queue->handle()->Signal(m_fence.Get(), ++m_currentSignal);
		m_signalQueued = true;
#else
		HRESULT hr = m_queue->handle()->Signal(m_fence.Get(), ++m_currentSignal);
#endif

		return hr;
	}

	void waitForSignal()
	{
		if (m_fence->GetCompletedValue() == m_currentSignal)
		{
			HANDLE eventHandle = ::CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			HRESULT hr = m_fence->SetEventOnCompletion(m_currentSignal, eventHandle);

			if (SUCCEEDED(hr))
				::WaitForSingleObject(eventHandle, INFINITE);
			
			::CloseHandle(eventHandle);

			raiseIfFailed<RuntimeException>(hr, "Unable to register fence completion event.");
		}

#ifndef NDEBUG
		m_signalQueued = false;
#endif
	}

	void reset()
	{
		// TODO: Also pass the pipeline state, if possible. 
		raiseIfFailed<RuntimeException>(m_commandAllocator->Reset(), "Unable to reset command allocator.");
		raiseIfFailed<RuntimeException>(m_parent->handle()->Reset(m_commandAllocator.Get(), nullptr), "Unable to reset command list.");
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12CommandBuffer::DirectX12CommandBuffer(const DirectX12Queue* queue) :
	m_impl(makePimpl<DirectX12CommandBufferImpl>(this, queue)), IComResource<ID3D12GraphicsCommandList4>(nullptr)
{
	this->handle() = m_impl->initialize();
}

DirectX12CommandBuffer::~DirectX12CommandBuffer() noexcept = default;

const ICommandQueue* DirectX12CommandBuffer::getQueue() const noexcept
{
	return m_impl->m_queue;
}

void DirectX12CommandBuffer::begin() const
{
	// Wait for the fence to be completed.
	m_impl->waitForSignal();

	// Reset the command buffer.
	m_impl->reset();
}

void DirectX12CommandBuffer::end() const
{
	// Queue a signal that indicates the end of the current list.
	raiseIfFailed<RuntimeException>(m_impl->queueSignal(), "Unable to add fence signal to command buffer.");

	// Close the command list, so that it does not longer record any commands.
	raiseIfFailed<RuntimeException>(this->handle()->Close(), "Unable to close command buffer for recording.");
}

void DirectX12CommandBuffer::submit(const bool& waitForQueue) const
{
	Array<ID3D12CommandList*> commandLists { this->handle().Get() };
	m_impl->m_queue->handle()->ExecuteCommandLists(static_cast<UInt32>(commandLists.size()), commandLists.data());

	// NOTE: DirectX has no equivalent of waiting for a queue to be idle, neither it is efficient (effectively turning the queue into a synchronous 
	//       operation list). Instead we wait for all commands of the buffer to be executed. If there's only one thread, the queue should be idle
	//       afterwards.
	if (waitForQueue)
		m_impl->waitForSignal();
}