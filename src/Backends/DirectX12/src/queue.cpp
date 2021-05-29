#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Queue::DirectX12QueueImpl : public Implement<DirectX12Queue> {
public:
	friend class DirectX12Queue;

private:
	const DirectX12Device* m_device;
	QueueType m_type;
	QueuePriority m_priority;
	bool m_bound;

public:
	DirectX12QueueImpl(DirectX12Queue* parent) :
		base(parent), m_bound(false) { }

	~DirectX12QueueImpl()
	{
		this->release();
	}

public:
	[[nodiscard]]
	ComPtr<ID3D12CommandQueue> initialize(const IGraphicsDevice* device, const QueueType& type, const QueuePriority& priority)
	{
		if (device == nullptr)
			throw ArgumentNotInitializedException("The device must be initialized.");

		m_device = dynamic_cast<const DirectX12Device*>(device);

		if (m_device == nullptr)
			throw InvalidArgumentException("The device must be a valid DirectX 12 device.");

		ComPtr<ID3D12CommandQueue> commandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		if (LITEFX_FLAG_IS_SET(type, QueueType::Graphics))
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		else if (LITEFX_FLAG_IS_SET(type, QueueType::Compute))
			desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		else if (LITEFX_FLAG_IS_SET(type, QueueType::Transfer))
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

		switch (priority)
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

		raiseIfFailed<RuntimeException>(m_device->handle()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)), "Unable to create command queue of type {0} with priority {1}.", type, priority);

		m_type = type;
		m_priority = priority;

		return commandQueue;
	}

	void release()
	{
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

DirectX12Queue::DirectX12Queue(const IGraphicsDevice* device, const QueueType& type, const QueuePriority& priority) :
	ComResource<ID3D12CommandQueue>(nullptr), m_impl(makePimpl<DirectX12QueueImpl>(this))
{
	this->handle() = m_impl->initialize(device, type, priority);
}

DirectX12Queue::~DirectX12Queue() noexcept
{
	this->release();
}

void DirectX12Queue::bind()
{
	m_impl->bind();
}

void DirectX12Queue::release()
{
	m_impl->release();
}

bool DirectX12Queue::isBound() const noexcept
{
	return m_impl->m_bound;
}

QueueType DirectX12Queue::getType() const noexcept
{
	return m_impl->m_type;
}

QueuePriority DirectX12Queue::getPriority() const noexcept
{
	return m_impl->m_priority;
}

const IGraphicsDevice* DirectX12Queue::getDevice() const noexcept
{
	return m_impl->m_device;
}

UniquePtr<ICommandBuffer> DirectX12Queue::createCommandBuffer() const
{
	return makeUnique<DirectX12CommandBuffer>(this);
}