#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Buffer::DirectX12BufferImpl {
public:
	friend class DirectX12Buffer;

private:
	AllocatorPtr m_allocator;
	AllocationPtr m_allocation;
	BufferType m_type;
	UInt32 m_elements;
	size_t m_elementSize, m_alignment;
	ResourceUsage m_usage;

public:
	DirectX12BufferImpl(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation) :
		m_allocator(std::move(allocator)), m_allocation(std::move(allocation)), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_usage(usage)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Buffer::DirectX12Buffer(ComPtr<ID3D12Resource>&& buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	ComResource<ID3D12Resource>(nullptr), m_impl(type, elements, elementSize, alignment, usage, std::move(allocator), std::move(allocation))
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

BufferType DirectX12Buffer::type() const noexcept
{
	return m_impl->m_type;
}

UInt32 DirectX12Buffer::elements() const noexcept
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

ResourceUsage DirectX12Buffer::usage() const noexcept
{
	return m_impl->m_usage;
}

UInt64 DirectX12Buffer::virtualAddress() const noexcept
{
	return this->handle()->GetGPUVirtualAddress();
}

void DirectX12Buffer::map(const void* const data, size_t size, UInt32 element)
{
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	D3D12_RANGE mappedRange = { };
	char* buffer{};
	void* bufferPtr = static_cast<void*>(buffer);
	raiseIfFailed(this->handle()->Map(0, &mappedRange, &bufferPtr), "Unable to map buffer memory.");
	std::memcpy(static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), data, size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	this->handle()->Unmap(0, nullptr);
}

void DirectX12Buffer::map(Span<const void* const> data, size_t elementSize, UInt32 firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

void DirectX12Buffer::map(void* data, size_t size, UInt32 element, bool write)
{
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	D3D12_RANGE mappedRange = { };
	char* buffer{};
	void* bufferPtr = static_cast<void*>(buffer);
	raiseIfFailed(this->handle()->Map(0, &mappedRange, &bufferPtr), "Unable to map buffer memory.");

	if (write)
		std::memcpy(static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), data, size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	else
		std::memcpy(data, static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

	this->handle()->Unmap(0, nullptr);
}

void DirectX12Buffer::map(Span<void*> data, size_t elementSize, UInt32 firstElement, bool write)
{
	std::ranges::for_each(data, [this, &elementSize, &write, i = firstElement](void* mem) mutable { this->map(mem, elementSize, i++, write); });
}

AllocatorPtr DirectX12Buffer::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Buffer::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

SharedPtr<IDirectX12Buffer> DirectX12Buffer::allocate(AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12Buffer::allocate("", std::move(allocator), type, elements, elementSize, alignment, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12Buffer> DirectX12Buffer::allocate(const String& name, AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, type, elements, elementSize, elements * elementSize, usage);

	return SharedObject::create<DirectX12Buffer>(std::move(resource), type, elements, elementSize, alignment, usage, std::move(allocator), AllocationPtr(allocation), name);
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBuffer::DirectX12VertexBufferImpl {
public:
	friend class DirectX12VertexBuffer;

private:
	const DirectX12VertexBufferLayout& m_layout;
	D3D12_VERTEX_BUFFER_VIEW m_view{};

public:
	DirectX12VertexBufferImpl(const DirectX12VertexBufferLayout& layout) :
		m_layout(layout)
	{
	}

public:
	void initialize(const DirectX12VertexBuffer& vertexBuffer)
	{
		m_view = D3D12_VERTEX_BUFFER_VIEW
		{
			.BufferLocation = vertexBuffer.handle()->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UInt32>(vertexBuffer.size()),
			.StrideInBytes = static_cast<UInt32>(vertexBuffer.elementSize())
		};
	}
};

// ------------------------------------------------------------------------------------------------
// Vertex buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12VertexBuffer::DirectX12VertexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	DirectX12Buffer(std::move(buffer), BufferType::Vertex, elements, layout.elementSize(), 0, usage, std::move(allocator), std::move(allocation), name), m_impl(layout)
{
	m_impl->initialize(*this);
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

SharedPtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12VertexBuffer::allocate("", layout, std::move(allocator), elements, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const String& name, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<DirectX12VertexBuffer>(std::move(resource), layout, elements, usage, std::move(allocator), AllocationPtr(allocation), name);
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12IndexBuffer::DirectX12IndexBufferImpl  {
public:
	friend class DirectX12IndexBuffer;

private:
	const DirectX12IndexBufferLayout& m_layout;
	D3D12_INDEX_BUFFER_VIEW m_view{};

public:
	DirectX12IndexBufferImpl(const DirectX12IndexBufferLayout& layout) :
		m_layout(layout)
	{
	}

public:
	void initialize(const DirectX12IndexBuffer& indexBuffer)
	{
		m_view = D3D12_INDEX_BUFFER_VIEW
		{
			.BufferLocation = indexBuffer.handle()->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UInt32>(indexBuffer.size()),
			.Format = indexBuffer.layout().indexType() == IndexType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
		};
	}
};

// ------------------------------------------------------------------------------------------------
// Index buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12IndexBuffer::DirectX12IndexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12IndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	DirectX12Buffer(std::move(buffer), BufferType::Index, elements, layout.elementSize(), 0, usage, std::move(allocator), std::move(allocation), name), m_impl(layout)
{
	m_impl->initialize(*this);
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

SharedPtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12IndexBuffer::allocate("", layout, std::move(allocator), elements, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const String& name, const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<DirectX12IndexBuffer>(std::move(resource), layout, elements, usage, std::move(allocator), AllocationPtr(allocation), name);
}