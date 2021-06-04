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

public:
	DirectX12BufferImpl(DirectX12Buffer* parent, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_allocator(allocator), m_allocation(std::move(allocation))
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Buffer::DirectX12Buffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12BufferImpl>(this, type, elements, elementSize, alignment, allocator, std::move(allocation))), DirectX12RuntimeObject<DirectX12Device>(device, &device), ComResource<ID3D12Resource>(nullptr)
{
	this->handle() = std::move(buffer);
}

DirectX12Buffer::~DirectX12Buffer() noexcept = default;

void DirectX12Buffer::transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	//if (source.elements() < sourceElement + elements)
	//	throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	//if (this->elements() < targetElement + elements)
	//	throw ArgumentOutOfRangeException("The current buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", this->elements(), elements, targetElement);

	//// Depending on the alignment, the transfer can be combined into a single copy command.
	//Array<VkBufferCopy> copyInfos;

	//if (this->elementAlignment() == 0 && source.elementAlignment() == 0)
	//{
	//	// Copy from unaligned to unaligned memory, so we can simply do one copy for the whole memory chunk.
	//	VkBufferCopy copyInfo{};
	//	copyInfo.size = source.elementSize() * elements;
	//	copyInfo.srcOffset = sourceElement * source.elementSize();
	//	copyInfo.dstOffset = targetElement * this->alignedElementSize();
	//	copyInfos.push_back(copyInfo);
	//}
	//else
	//{
	//	// All other options require us to record one command per element. Since `alignedElementSize` returns `elementSize`, if there's no alignment, it does 
	//	// not matter which buffer is aligned or unaligned.
	//	copyInfos.resize(elements);
	//	std::ranges::generate(copyInfos, [this, &source, &sourceElement, &targetElement, i = 0]() mutable {
	//		VkBufferCopy copyInfo{};
	//		copyInfo.size = source.alignedElementSize();
	//		copyInfo.srcOffset = (sourceElement + i) * source.alignedElementSize();
	//		copyInfo.dstOffset = (targetElement + i) * this->alignedElementSize();
	//		i++;
	//		return copyInfo;
	//	});
	//}

	//::vkCmdCopyBuffer(commandBuffer.handle(), source.handle(), this->handle(), static_cast<UInt32>(copyInfos.size()), copyInfos.data());
	throw;
}

void DirectX12Buffer::transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	target.transferFrom(commandBuffer, *this, sourceElement, targetElement, elements);
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
	//if (element >= m_impl->m_elements)
	//	throw ArgumentOutOfRangeException("The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	//size_t alignedSize = size;
	//size_t alignment = this->elementAlignment();

	//if (alignment > 0)
	//	alignedSize = (size + alignment - 1) & ~(alignment - 1);

	//char* buffer;		// A pointer to the whole (aligned) buffer memory.
	//raiseIfFailed<RuntimeException>(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	//auto result = ::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size);

	//if (result != 0)
	//	throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);

	//::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);
	throw;
}

void DirectX12Buffer::map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

UniquePtr<IDirectX12Buffer> DirectX12Buffer::allocate(const DirectX12Device& device, AllocatorPtr allocator, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), type, elements, elementSize, elements * elementSize);

	return makeUnique<DirectX12Buffer>(device, std::move(resource), type, elements, elementSize, alignment, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBuffer::DirectX12VertexBufferImpl : public Implement<DirectX12VertexBuffer> {
public:
	friend class DirectX12VertexBuffer;

private:
	const DirectX12VertexBufferLayout& m_layout;

public:
	DirectX12VertexBufferImpl(DirectX12VertexBuffer* parent, const DirectX12VertexBufferLayout& layout) :
		base(parent), m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Vertex buffer shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12VertexBuffer::DirectX12VertexBuffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, const UInt32& elements, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12VertexBufferImpl>(this, layout)), DirectX12Buffer(device, std::move(buffer), BufferType::Vertex, elements, layout.elementSize(), 0, allocator, std::move(allocation))
{
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

UniquePtr<IDirectX12VertexBuffer> DirectX12VertexBuffer::allocate(const DirectX12Device& device, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(resource.Get()), BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<DirectX12VertexBuffer>(device, std::move(resource), layout, elements, allocator, AllocationPtr(allocation));
}