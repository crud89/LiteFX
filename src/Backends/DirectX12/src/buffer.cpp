#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Buffer::DirectX12BufferImpl : public Implement<DirectX12Buffer> {
public:
	friend class DirectX12Buffer;

private:
	AllocatorPtr m_allocator;
	AllocationPtr m_allocation;
	BufferType m_type;
	UInt32 m_elements;
	size_t m_elementSize, m_alignment;
	D3D12_RESOURCE_STATES m_state;

public:
	DirectX12BufferImpl(DirectX12Buffer* parent, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_allocator(allocator), m_allocation(std::move(allocation)), m_state(initialState)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Buffer::DirectX12Buffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12BufferImpl>(this, type, elements, elementSize, alignment, initialState, allocator, std::move(allocation))), DirectX12RuntimeObject<DirectX12Device>(device, &device), ComResource<ID3D12Resource>(nullptr)
{
	this->handle() = std::move(buffer);
}

DirectX12Buffer::~DirectX12Buffer() noexcept = default;

void DirectX12Buffer::receiveData(const DirectX12CommandBuffer& commandBuffer, const bool& receive) const noexcept
{
	if ((receive && this->state() & D3D12_RESOURCE_STATE_COPY_DEST) || (!receive && !(this->state() & D3D12_RESOURCE_STATE_COPY_DEST)))
		return;

	D3D12_RESOURCE_STATES targetState = receive ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON;

	if (receive) switch (this->type())
	{
	case BufferType::Index:     targetState |= D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
	case BufferType::Uniform:
	case BufferType::Vertex:    targetState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
	case BufferType::Storage:   targetState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
	case BufferType::Other:
	default:                    break;
	}

	this->transitionTo(commandBuffer, targetState);
}

void DirectX12Buffer::sendData(const DirectX12CommandBuffer& commandBuffer, const bool& emit) const noexcept
{
	if ((emit && this->state() & D3D12_RESOURCE_STATE_COPY_SOURCE) || (!emit && !(this->state() & D3D12_RESOURCE_STATE_COPY_SOURCE)))
		return;

	D3D12_RESOURCE_STATES targetState = emit ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_GENERIC_READ;

	if (emit) switch (this->type())
	{
	case BufferType::Index:     targetState |= D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
	case BufferType::Uniform:
	case BufferType::Vertex:    targetState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
	case BufferType::Storage:   targetState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
	case BufferType::Other: 
	default:                    break;
	}

	this->transitionTo(commandBuffer, targetState);
}

void DirectX12Buffer::transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (this->elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The current buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", this->elements(), elements, targetElement);

#ifndef NDEBUG
	if (layer > 0) [[unlikely]]
		LITEFX_WARNING(DIRECTX12_LOG, "You've specified a buffer copy operation for layer {0}, however layers are ignored for buffer-buffer transfers.", layer);

	if (plane > 0) [[unlikely]]
		LITEFX_WARNING(DIRECTX12_LOG, "You've specified a buffer copy operation for plane {0}, however planes are ignored for buffer-buffer transfers.", plane);
#endif

	source.sendData(commandBuffer, true);	
	this->receiveData(commandBuffer, true);

	commandBuffer.handle()->CopyBufferRegion(this->handle().Get(), targetElement * this->alignedElementSize(), source.handle().Get(), sourceElement * source.alignedElementSize(), elements * source.alignedElementSize());

	if (!leaveSourceState)
		source.sendData(commandBuffer, false);

	if (!leaveTargetState)
		this->receiveData(commandBuffer, false);
}

void DirectX12Buffer::transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	target.transferFrom(commandBuffer, *this, sourceElement, targetElement, elements, leaveSourceState, leaveTargetState, layer, plane);
}

const BufferType& DirectX12Buffer::type() const noexcept
{
	return m_impl->m_type;
}

const UInt32& DirectX12Buffer::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t DirectX12Buffer::size() const noexcept
{
	return static_cast<size_t>(m_impl->m_elements) * this->alignedElementSize();
}

size_t DirectX12Buffer::elementSize() const noexcept
{
	return m_impl->m_elementSize;
}

size_t DirectX12Buffer::elementAlignment() const noexcept
{
	return m_impl->m_alignment;
}

size_t DirectX12Buffer::alignedElementSize() const noexcept
{
	return m_impl->m_alignment == 0 ? m_impl->m_elementSize : (m_impl->m_elementSize + m_impl->m_alignment - 1) & ~(m_impl->m_alignment - 1);
}

void DirectX12Buffer::map(const void* const data, const size_t& size, const UInt32& element)
{
	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	size_t alignedSize = size;
	size_t alignment = this->elementAlignment();

	if (alignment > 0)
		alignedSize = (size + alignment - 1) & ~(alignment - 1);

	D3D12_RANGE mappedRange = {};
	char* buffer;
	raiseIfFailed<RuntimeException>(this->handle()->Map(0, &mappedRange, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	auto result = ::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size);
	this->handle()->Unmap(0, nullptr);

	if (result != 0)
		throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);
}

void DirectX12Buffer::map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

const D3D12_RESOURCE_STATES& DirectX12Buffer::state() const noexcept
{
	return m_impl->m_state;
}

D3D12_RESOURCE_STATES& DirectX12Buffer::state() noexcept
{
	return m_impl->m_state;
}

D3D12_RESOURCE_BARRIER DirectX12Buffer::transitionTo(const D3D12_RESOURCE_STATES& state, const UInt32& element, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const
{
	if (m_impl->m_state == state) [[unlikely]]
		throw InvalidArgumentException("The specified buffer state must be different from the current resource state.");

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(this->handle().Get(), m_impl->m_state, state, element, flags);
	m_impl->m_state = state;
	return barrier;
}

void DirectX12Buffer::transitionTo(const DirectX12CommandBuffer& commandBuffer, const D3D12_RESOURCE_STATES& state, const UInt32& element, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const
{
	if (m_impl->m_state == state)
		return;

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(this->handle().Get(), m_impl->m_state, state, element, flags);
	m_impl->m_state = state;
	commandBuffer.handle()->ResourceBarrier(1, &barrier);
}

AllocatorPtr DirectX12Buffer::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Buffer::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

UniquePtr<IDirectX12Buffer> DirectX12Buffer::allocate(const DirectX12Device& device, AllocatorPtr allocator, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), type, elements, elementSize, elements * elementSize);

	return makeUnique<DirectX12Buffer>(device, std::move(resource), type, elements, elementSize, alignment, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBuffer::DirectX12VertexBufferImpl : public Implement<DirectX12VertexBuffer> {
public:
	friend class DirectX12VertexBuffer;

private:
	const DirectX12VertexBufferLayout& m_layout;
	D3D12_VERTEX_BUFFER_VIEW m_view;

public:
	DirectX12VertexBufferImpl(DirectX12VertexBuffer* parent, const DirectX12VertexBufferLayout& layout) :
		base(parent), m_layout(layout)
	{
	}

public:
	void initialize()
	{
		m_view = D3D12_VERTEX_BUFFER_VIEW
		{
			.BufferLocation = m_parent->handle()->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UInt32>(m_parent->size()),
			.StrideInBytes = static_cast<UInt32>(m_parent->elementSize())
		};
	}
};

// ------------------------------------------------------------------------------------------------
// Vertex buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12VertexBuffer::DirectX12VertexBuffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12VertexBufferImpl>(this, layout)), DirectX12Buffer(device, std::move(buffer), BufferType::Vertex, elements, layout.elementSize(), 0, initialState, allocator, std::move(allocation))
{
	m_impl->initialize();
}

DirectX12VertexBuffer::~DirectX12VertexBuffer() noexcept = default;

const DirectX12VertexBufferLayout& DirectX12VertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const UInt32& DirectX12VertexBuffer::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

const D3D12_VERTEX_BUFFER_VIEW& DirectX12VertexBuffer::view() const noexcept
{
	return m_impl->m_view;
}

UniquePtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const DirectX12Device& device, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12VertexBuffer>(device, std::move(resource), layout, elements, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12IndexBuffer::DirectX12IndexBufferImpl : public Implement<DirectX12IndexBuffer> {
public:
	friend class DirectX12IndexBuffer;

private:
	const DirectX12IndexBufferLayout& m_layout;
	D3D12_INDEX_BUFFER_VIEW m_view;

public:
	DirectX12IndexBufferImpl(DirectX12IndexBuffer* parent, const DirectX12IndexBufferLayout& layout) :
		base(parent), m_layout(layout)
	{
	}

public:
	void initialize()
	{
		m_view = D3D12_INDEX_BUFFER_VIEW
		{
			.BufferLocation = m_parent->handle()->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UInt32>(m_parent->size()),
			.Format = m_parent->layout().indexType() == IndexType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT
		};
	}
};

// ------------------------------------------------------------------------------------------------
// Index buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12IndexBuffer::DirectX12IndexBuffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const DirectX12IndexBufferLayout& layout, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12IndexBufferImpl>(this, layout)), DirectX12Buffer(device, std::move(buffer), BufferType::Index, elements, layout.elementSize(), 0, initialState, allocator, std::move(allocation))
{
	m_impl->initialize();
}

DirectX12IndexBuffer::~DirectX12IndexBuffer() noexcept = default;

const DirectX12IndexBufferLayout& DirectX12IndexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const D3D12_INDEX_BUFFER_VIEW& DirectX12IndexBuffer::view() const noexcept
{
	return m_impl->m_view;
}

UniquePtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const DirectX12Device & device, const DirectX12IndexBufferLayout & layout, AllocatorPtr allocator, const UInt32 & elements, const D3D12_RESOURCE_STATES & initialState, const D3D12_RESOURCE_DESC & resourceDesc, const D3D12MA::ALLOCATION_DESC & allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12IndexBuffer>(device, std::move(resource), layout, elements, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Constant buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ConstantBuffer::DirectX12ConstantBufferImpl : public Implement<DirectX12ConstantBuffer> {
public:
	friend class DirectX12ConstantBuffer;

private:
	const DirectX12DescriptorLayout& m_layout;

public:
	DirectX12ConstantBufferImpl(DirectX12ConstantBuffer* parent, const DirectX12DescriptorLayout& layout) :
		base(parent), m_layout(layout)
	{
	}

public:
	void initialize()
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Constant buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12ConstantBuffer::DirectX12ConstantBuffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const DirectX12DescriptorLayout& layout, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12ConstantBufferImpl>(this, layout)), DirectX12Buffer(device, std::move(buffer), BufferType::Uniform, elements, layout.elementSize(), 256, initialState, allocator, std::move(allocation))
{
	m_impl->initialize();
}

DirectX12ConstantBuffer::~DirectX12ConstantBuffer() noexcept = default;

const DirectX12DescriptorLayout& DirectX12ConstantBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const UInt32& DirectX12ConstantBuffer::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

UniquePtr<IDirectX12ConstantBuffer> DirectX12ConstantBuffer::allocate(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, AllocatorPtr allocator, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), BufferType::Uniform, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12ConstantBuffer>(device, std::move(resource), layout, elements, initialState, allocator, AllocationPtr(allocation));
}