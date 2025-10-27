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
	D3D12_RESOURCE_DESC1 m_resourceDesc;

public:
	DirectX12BufferImpl(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const D3D12_RESOURCE_DESC1& resourceDesc) :
		m_allocator(std::move(allocator)), m_allocation(std::move(allocation)), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_usage(usage), m_resourceDesc(resourceDesc)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Buffer::DirectX12Buffer(ComPtr<ID3D12Resource>&& buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	ComResource<ID3D12Resource>(nullptr), m_impl(type, elements, elementSize, alignment, usage, std::move(allocator), std::move(allocation), resourceDesc)
{
	this->handle() = std::move(buffer);

	if (!name.empty())
	{
		this->name() = name;

#ifndef NDEBUG
		this->handle()->SetName(Widen(name).c_str());
#endif
	}

	if (m_impl->m_allocation != nullptr)
		m_impl->m_allocation->SetPrivateData(static_cast<IDeviceMemory*>(this));
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

	this->write(data, size, element * this->alignedElementSize());
}

void DirectX12Buffer::map(Span<const void* const> data, size_t elementSize, UInt32 firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](auto mem) mutable { this->map(mem, elementSize, i++); });
}

void DirectX12Buffer::map(void* data, size_t size, UInt32 element, bool write)
{
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	if (write)
		this->write(data, size, element * this->alignedElementSize());
	else
		this->read(data, size, element * this->alignedElementSize());
}

void DirectX12Buffer::map(Span<void*> data, size_t elementSize, UInt32 firstElement, bool write)
{
	std::ranges::for_each(data, [this, &elementSize, &write, i = firstElement](auto mem) mutable { this->map(mem, elementSize, i++, write); });
}

void DirectX12Buffer::write(const void* const data, size_t size, size_t offset)
{
	D3D12_RANGE mappedRange{ };
	void* buffer{ nullptr };
	raiseIfFailed(this->handle()->Map(0, &mappedRange, &buffer), "Unable to map buffer memory.");
	std::memcpy(std::next(static_cast<Byte*>(buffer), offset), data, size); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
	this->handle()->Unmap(0, nullptr);
}

void DirectX12Buffer::read(void* data, size_t size, size_t offset)
{
	D3D12_RANGE mappedRange{ };
	void* buffer{ nullptr };
	raiseIfFailed(this->handle()->Map(0, &mappedRange, &buffer), "Unable to map buffer memory.");
	std::memcpy(data, std::next(static_cast<Byte*>(buffer), offset), size); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
	this->handle()->Unmap(0, nullptr);
}

AllocatorPtr DirectX12Buffer::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Buffer::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

void DirectX12Buffer::reset(ComPtr<ID3D12Resource>&& image, AllocationPtr&& allocation)
{
	this->handle() = std::move(image);
	m_impl->m_allocation = std::move(allocation);
	m_impl->m_allocation->SetPrivateData(static_cast<IDeviceMemory*>(this));
}

SharedPtr<IDirectX12Buffer> DirectX12Buffer::allocate(const String& name, AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, type, elements, elementSize, elements * elementSize, usage);

	return SharedObject::create<DirectX12Buffer>(std::move(resource), type, elements, elementSize, alignment, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
}

bool DirectX12Buffer::tryAllocate(SharedPtr<IDirectX12Buffer>& buffer, const String& name, AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	buffer = nullptr;

	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	auto result = allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource));

	if (FAILED(result))
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocation for buffer {0} with {4} bytes failed: 0x{6:08X} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, type, elements, elementSize, elements * elementSize, usage, result);
		return false;
	}
	else
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, type, elements, elementSize, elements * elementSize, usage);

		buffer = SharedObject::create<DirectX12Buffer>(std::move(resource), type, elements, elementSize, alignment, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
		return true;
	}
}

bool DirectX12Buffer::move(SharedPtr<IDirectX12Buffer> buffer, D3D12MA::Allocation* to, const DirectX12CommandBuffer& commandBuffer) // NOLINT(performance-unnecessary-value-param)
{
	// NOTES: If this method returns true, the command buffer must be executed and all bindings to the image must be updated afterwards, otherwise the result of this operation is undefined behavior.
	
	if (buffer == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("buffer");

	if (to == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("to");

	auto& source = dynamic_cast<DirectX12Buffer&>(*buffer);
	const auto device = commandBuffer.queue()->device();
	const auto& resourceDesc = source.m_impl->m_resourceDesc;
	auto allocator = source.m_impl->m_allocator;

	ComPtr<ID3D12Resource> resource;
	auto result = (*device).handle()->CreatePlacedResource2(to->GetHeap(), to->GetOffset(), std::addressof(resourceDesc), D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, IID_PPV_ARGS(&resource));

	if (FAILED(result)) [[unlikely]]
		return false;

	to->SetResource(resource.Get());

	// NOTE: Buffers in the READBACK heap can be copied synchronously using a usual `memcpy`
	// if (to->GetHeap()->GetDesc().Properties.Type == D3D12_HEAP_TYPE_READBACK) ...

	if (!buffer->volatileMove())
		commandBuffer.handle()->CopyResource(resource.Get(), source.handle().Get());

	// Reset the resource and return.
	// NOTE: At this point, the previous resource does still exist, but is inaccessible through the current instance. The only remaining reference should be stored by the source allocation during 
	//       defragmentation. After it gets released, the resource should also be removed. If a reference is stored somewhere else this leaks, but you should never store the reference obtained by
	//       calling `handle` manually.
	//       The new resource handle is valid beyond this point, but may contain uninitialized data. Any attempt of using the resource must be properly synchronized to execute after the submission
	//       of `commandBuffer`.
	source.reset(std::move(resource), AllocationPtr(to));
	return true;
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBuffer::DirectX12VertexBufferImpl {
public:
	friend class DirectX12VertexBuffer;

private:
	SharedPtr<const DirectX12VertexBufferLayout> m_layout;
	D3D12_VERTEX_BUFFER_VIEW m_view{};

public:
	DirectX12VertexBufferImpl(const DirectX12VertexBufferLayout& layout) :
		m_layout(layout.shared_from_this())
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

DirectX12VertexBuffer::DirectX12VertexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	DirectX12Buffer(std::move(buffer), BufferType::Vertex, elements, layout.elementSize(), 0, usage, resourceDesc, std::move(allocator), std::move(allocation), name), m_impl(layout)
{
	m_impl->initialize(*this);
}

DirectX12VertexBuffer::~DirectX12VertexBuffer() noexcept = default;

const DirectX12VertexBufferLayout& DirectX12VertexBuffer::layout() const noexcept
{
	return *m_impl->m_layout;
}

const D3D12_VERTEX_BUFFER_VIEW& DirectX12VertexBuffer::view() const noexcept
{
	return m_impl->m_view;
}

SharedPtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const String& name, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<DirectX12VertexBuffer>(std::move(resource), layout, elements, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
}

bool DirectX12VertexBuffer::tryAllocate(SharedPtr<IDirectX12VertexBuffer>& buffer, const String& name, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	buffer = nullptr;

	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	auto result = allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource));

	if (FAILED(result))
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocation for buffer {0} with {4} bytes failed: 0x{6:08X} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage, result);
		return false;
	}
	else
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage);
		
		buffer = SharedObject::create<DirectX12VertexBuffer>(std::move(resource), layout, elements, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
		return true;
	}
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12IndexBuffer::DirectX12IndexBufferImpl  {
public:
	friend class DirectX12IndexBuffer;

private:
	SharedPtr<const DirectX12IndexBufferLayout> m_layout;
	D3D12_INDEX_BUFFER_VIEW m_view{};

public:
	DirectX12IndexBufferImpl(const DirectX12IndexBufferLayout& layout) :
		m_layout(layout.shared_from_this())
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

DirectX12IndexBuffer::DirectX12IndexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12IndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	DirectX12Buffer(std::move(buffer), BufferType::Index, elements, layout.elementSize(), 0, usage, resourceDesc, std::move(allocator), std::move(allocation), name), m_impl(layout)
{
	m_impl->initialize(*this);
}

DirectX12IndexBuffer::~DirectX12IndexBuffer() noexcept = default;

const DirectX12IndexBufferLayout& DirectX12IndexBuffer::layout() const noexcept
{
	return *m_impl->m_layout;
}

const D3D12_INDEX_BUFFER_VIEW& DirectX12IndexBuffer::view() const noexcept
{
	return m_impl->m_view;
}

SharedPtr<IDirectX12IndexBuffer> DirectX12IndexBuffer::allocate(const String& name, const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<DirectX12IndexBuffer>(std::move(resource), layout, elements, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
}

bool DirectX12IndexBuffer::tryAllocate(SharedPtr<IDirectX12IndexBuffer>& buffer, const String& name, const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	buffer = nullptr;

	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	auto result = allocator->CreateResource3(&allocationDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource));

	if (FAILED(result))
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocation for buffer {0} with {4} bytes failed: 0x{6:08X} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage, result);
		return false;
	}
	else
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage);

		buffer = SharedObject::create<DirectX12IndexBuffer>(std::move(resource), layout, elements, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
		return true;
	}
}