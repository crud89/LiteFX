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

public:
	DirectX12CommandBufferImpl(DirectX12CommandBuffer* parent, const DirectX12Queue* queue) :
		base(parent), m_queue(queue)
	{
		if (queue == nullptr)
			throw ArgumentNotInitializedException("The command queue must be initialized.");
	}

public:
	ComPtr<ID3D12CommandList> initialize()
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
		ComPtr<ID3D12GraphicsCommandList> commandList;
		// TODO: Also pass the pipeline state, if possible. 
		raiseIfFailed<RuntimeException>(device->handle()->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");
		raiseIfFailed<RuntimeException>(commandList->Close(), "Unable to close command list.");

		return commandList;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12CommandBuffer::DirectX12CommandBuffer(const DirectX12Queue* queue) :
	m_impl(makePimpl<DirectX12CommandBufferImpl>(this, queue)), IComResource<ID3D12CommandList>(nullptr)
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
	throw;
}

void DirectX12CommandBuffer::end() const
{
	throw;
}

void DirectX12CommandBuffer::submit(const bool& waitForQueue) const
{
	throw;
}