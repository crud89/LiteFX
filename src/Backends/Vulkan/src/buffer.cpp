#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBuffer::VulkanBufferImpl : public Implement<VulkanBuffer> {
public:
	friend class VulkanBuffer;

private:
	BufferType m_type;
	UInt32 m_elements;
	size_t m_elementSize, m_alignment;
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	bool m_writable;
	Array<ResourceState> m_states;

public:
	VulkanBufferImpl(VulkanBuffer* parent, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_writable(writable), m_allocator(allocator), m_allocation(allocation)
	{
		m_states.resize(elements, initialState);
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBuffer::VulkanBuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanBufferImpl>(this, type, elements, elementSize, alignment, writable, initialState, allocator, allocation)), Resource<VkBuffer>(buffer)
{
}

VulkanBuffer::~VulkanBuffer() noexcept
{
	::vmaDestroyBuffer(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", fmt::ptr(reinterpret_cast<void*>(this->handle())));
}

const BufferType& VulkanBuffer::type() const noexcept
{
	return m_impl->m_type;
}

const UInt32& VulkanBuffer::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t VulkanBuffer::size() const noexcept
{
	return static_cast<size_t>(m_impl->m_elements) * this->alignedElementSize();
}

size_t VulkanBuffer::elementSize() const noexcept
{
	return m_impl->m_elementSize;
}

size_t VulkanBuffer::elementAlignment() const noexcept
{
	return m_impl->m_alignment;
}

size_t VulkanBuffer::alignedElementSize() const noexcept
{
	return m_impl->m_alignment == 0 ? m_impl->m_elementSize : (m_impl->m_elementSize + m_impl->m_alignment - 1) & ~(m_impl->m_alignment - 1);
}

const bool& VulkanBuffer::writable() const noexcept
{
	return m_impl->m_writable;
}

const ResourceState& VulkanBuffer::state(const UInt32& subresource) const
{
	if (subresource >= m_impl->m_states.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_states[subresource];
}

ResourceState& VulkanBuffer::state(const UInt32& subresource)
{
	if (subresource >= m_impl->m_states.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_states[subresource];
}

void VulkanBuffer::map(const void* const data, const size_t& size, const UInt32& element)
{
	if (element >= m_impl->m_elements)
		throw ArgumentOutOfRangeException("The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	size_t alignedSize = size;
	size_t alignment = this->elementAlignment();

	if (alignment > 0)
		alignedSize = (size + alignment - 1) & ~(alignment - 1);

	char* buffer;		// A pointer to the whole (aligned) buffer memory.
	raiseIfFailed<RuntimeException>(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	auto result = ::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size);

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);

	if (result != 0)
		throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);
}

void VulkanBuffer::map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

UniquePtr<IVulkanBuffer> VulkanBuffer::allocate(const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Writable: {5} }}", fmt::ptr(reinterpret_cast<void*>(buffer)), type, elements, elementSize, elements * elementSize, writable);

	return makeUnique<VulkanBuffer>(buffer, type, elements, elementSize, alignment, writable, initialState, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBuffer::VulkanVertexBufferImpl : public Implement<VulkanVertexBuffer> {
public:
	friend class VulkanVertexBuffer;

private:
	const VulkanVertexBufferLayout& m_layout;

public:
	VulkanVertexBufferImpl(VulkanVertexBuffer* parent, const VulkanVertexBufferLayout& layout) :
		base(parent), m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Vertex buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBuffer::VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanVertexBufferImpl>(this, layout)), VulkanBuffer(buffer, BufferType::Vertex, elements, layout.elementSize(), 0, false, initialState, allocator, allocation)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer() noexcept = default;

const VulkanVertexBufferLayout& VulkanVertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

UniquePtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(reinterpret_cast<void*>(buffer)), BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanVertexBuffer>(buffer, layout, elements, initialState, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBuffer::VulkanIndexBufferImpl : public Implement<VulkanIndexBuffer> {
public:
	friend class VulkanIndexBuffer;

private:
	const VulkanIndexBufferLayout& m_layout;

public:
	VulkanIndexBufferImpl(VulkanIndexBuffer* parent, const VulkanIndexBufferLayout& layout) :
		base(parent), m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Index buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanIndexBuffer::VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanIndexBufferImpl>(this, layout)), VulkanBuffer(buffer, BufferType::Index, elements, layout.elementSize(), 0, false, initialState, allocator, allocation)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;

const VulkanIndexBufferLayout& VulkanIndexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

UniquePtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const VulkanIndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate index buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(reinterpret_cast<void*>(buffer)), BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanIndexBuffer>(buffer, layout, elements, initialState, allocator, allocation);
}