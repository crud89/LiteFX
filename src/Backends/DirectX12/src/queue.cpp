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
	bool m_bound;

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