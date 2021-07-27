#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandBuffer::DirectX12CommandBufferImpl : public Implement<DirectX12CommandBuffer> {
public:
	friend class DirectX12CommandBuffer;

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Fence> m_fence;
	UINT32 m_currentSignal;
#ifndef NDEBUG
	bool m_signalQueued{ false };
#endif

public:
	DirectX12CommandBufferImpl(DirectX12CommandBuffer* parent) :
		base(parent), m_currentSignal(0)
	{
	}

public:
	ComPtr<ID3D12GraphicsCommandList4> initialize(const bool& begin)
	{
		// Create a command allocator.
		D3D12_COMMAND_LIST_TYPE type;

		switch (m_parent->parent().type())
		{
		case QueueType::Compute: type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
		case QueueType::Transfer: type = D3D12_COMMAND_LIST_TYPE_COPY; break;
		default:
		case QueueType::Graphics: type = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
		}

		raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator)), "Unable to create command allocator for command buffer.");
		raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create command buffer synchronization fence.");

		// Create the actual command list.
		ComPtr<ID3D12GraphicsCommandList4> commandList;

		if (begin)
			raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");
		else
			raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");

		return commandList;
	}

	HRESULT queueSignal()
	{
#ifndef NDEBUG
		if (m_signalQueued)
			throw RuntimeException("Another signal has already been queued. Don't call 'begin()' on the same command buffer twice, since this may cause race conditions!");

		HRESULT hr = m_parent->parent().handle()->Signal(m_fence.Get(), ++m_currentSignal);
		m_signalQueued = true;
#else
		HRESULT hr = m_parent->parent().handle()->Signal(m_fence.Get(), ++m_currentSignal);
#endif

		return hr;
	}

	void waitForSignal()
	{
		if (m_fence->GetCompletedValue() != m_currentSignal)
		{
			HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
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

DirectX12CommandBuffer::DirectX12CommandBuffer(const DirectX12Queue& queue, const bool& begin) :
	m_impl(makePimpl<DirectX12CommandBufferImpl>(this)), DirectX12RuntimeObject(queue, queue.getDevice()), ComResource<ID3D12GraphicsCommandList4>(nullptr)
{
	this->handle() = m_impl->initialize(begin);
}

DirectX12CommandBuffer::~DirectX12CommandBuffer() noexcept = default;

void DirectX12CommandBuffer::wait() const
{
	m_impl->waitForSignal();
}

void DirectX12CommandBuffer::begin() const
{
	// Wait for the fence to be completed.
	m_impl->waitForSignal();

	// Reset the command buffer.
	m_impl->reset();
}

void DirectX12CommandBuffer::end(const bool& submit, const bool& wait) const
{
	// Close the command list, so that it does not longer record any commands.
	raiseIfFailed<RuntimeException>(this->handle()->Close(), "Unable to close command buffer for recording.");

	if (submit)
		this->submit(wait);
}

void DirectX12CommandBuffer::submit(const bool& wait) const
{
	Array<ID3D12CommandList*> commandLists { this->handle().Get() };
	this->parent().handle()->ExecuteCommandLists(static_cast<UInt32>(commandLists.size()), commandLists.data());

	// Queue a signal that indicates the end of the current list.
	raiseIfFailed<RuntimeException>(m_impl->queueSignal(), "Unable to add fence signal to command buffer.");

	if (wait)
		this->wait();
}

void DirectX12CommandBuffer::barrier(const DirectX12Barrier& barrier, const bool& invert) const noexcept
{
	if (invert)
		barrier.executeInverse(*this);
	else
		barrier.execute(*this);
}

void DirectX12CommandBuffer::transfer(const IDirectX12Buffer& source, const IDirectX12Buffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	this->handle()->CopyBufferRegion(target.handle().Get(), targetElement * target.alignedElementSize(), source.handle().Get(), sourceElement * source.alignedElementSize(), elements * source.alignedElementSize());
}

void DirectX12CommandBuffer::transfer(const IDirectX12Buffer& source, const IDirectX12Image& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& sourceDesc = source.handle()->GetDesc();

	for (int sr(0); sr < elements; ++sr)
	{
		this->getDevice()->handle()->GetCopyableFootprints(&sourceDesc, sourceElement + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source.handle().Get(), footprint), targetLocation(target.handle().Get(), firstSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const IDirectX12Image& source, const IDirectX12Image& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	// TODO: Check if we can possibly do this more efficiently by copying multiple sub-resources at once.
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& sourceDesc = source.handle()->GetDesc();

	for (int sr(0); sr < subresources; ++sr)
	{
		this->getDevice()->handle()->GetCopyableFootprints(&sourceDesc, sourceSubresource + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source.handle().Get(), footprint), targetLocation(target.handle().Get(), targetSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const IDirectX12Image& source, const IDirectX12Buffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const
{
	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& sourceDesc = source.handle()->GetDesc();

	for (int sr(0); sr < subresources; ++sr)
	{
		this->getDevice()->handle()->GetCopyableFootprints(&sourceDesc, firstSubresource + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source.handle().Get(), footprint), targetLocation(target.handle().Get(), targetElement + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}