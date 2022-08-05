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
	Array<ResourceState> m_states;
	bool m_writable;

public:
	DirectX12BufferImpl(DirectX12Buffer* parent, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_writable(writable), m_allocator(allocator), m_allocation(std::move(allocation))
	{
		m_states.resize(elements, initialState);
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Buffer::DirectX12Buffer(ComPtr<ID3D12Resource>&& buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	m_impl(makePimpl<DirectX12BufferImpl>(this, type, elements, elementSize, alignment, writable, initialState, allocator, std::move(allocation))), ComResource<ID3D12Resource>(nullptr)
{
	this->handle() = std::move(buffer);

	if (!name.empty())
	{
		this->name() = name;

#ifndef NDEBUG
		this->handle()->SetName(Widen(name).c_str());
#endif
	}
}

DirectX12Buffer::~DirectX12Buffer() noexcept = default;

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

const bool& DirectX12Buffer::writable() const noexcept
{
	return m_impl->m_writable;
}

const ResourceState& DirectX12Buffer::state(const UInt32& subresource) const
{
	if (subresource >= m_impl->m_states.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_states[subresource];
}

ResourceState& DirectX12Buffer::state(const UInt32& subresource)
{
	if (subresource >= m_impl->m_states.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_states[subresource];
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

AllocatorPtr DirectX12Buffer::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Buffer::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

UniquePtr<IDirectX12Buffer> DirectX12Buffer::allocate(AllocatorPtr allocator, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12Buffer::allocate("", allocator, type, elements, elementSize, alignment, writable, initialState, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12Buffer> DirectX12Buffer::allocate(const String& name, AllocatorPtr allocator, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, DX12::getResourceState(initialState), nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Writable: {5} }}", name.empty() ? fmt::to_string(fmt::ptr(resource.Get())) : name, type, elements, elementSize, elements * elementSize, writable);

	return makeUnique<DirectX12Buffer>(std::move(resource), type, elements, elementSize, alignment, writable, initialState, allocator, AllocationPtr(allocation), name);
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

DirectX12VertexBuffer::DirectX12VertexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	m_impl(makePimpl<DirectX12VertexBufferImpl>(this, layout)), DirectX12Buffer(std::move(buffer), BufferType::Vertex, elements, layout.elementSize(), 0, false, initialState, allocator, std::move(allocation), name)
{
	m_impl->initialize();
}

DirectX12VertexBuffer::~DirectX12VertexBuffer() noexcept = default;

const DirectX12VertexBufferLayout& DirectX12VertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const D3D12_VERTEX_BUFFER_VIEW& DirectX12VertexBuffer::view() const noexcept
{
	return m_impl->m_view;
}

UniquePtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12VertexBuffer::allocate("", layout, allocator, elements, initialState, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const String& name, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, DX12::getResourceState(initialState), nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", name.empty() ? fmt::to_string(fmt::ptr(resource.Get())) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12VertexBuffer>(std::move(resource), layout, elements, initialState, allocator, AllocationPtr(allocation), name);
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

DirectX12IndexBuffer::DirectX12IndexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12IndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	m_impl(makePimpl<DirectX12IndexBufferImpl>(this, layout)), DirectX12Buffer(std::move(buffer), BufferType::Index, elements, layout.elementSize(), 0, false, initialState, allocator, std::move(allocation), name)
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

UniquePtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12IndexBuffer::allocate("", layout, allocator, elements, initialState, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const String& name, const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, DX12::getResourceState(initialState), nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", name.empty() ? fmt::to_string(fmt::ptr(resource.Get())) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12IndexBuffer>(std::move(resource), layout, elements, initialState, allocator, AllocationPtr(allocation), name);
}