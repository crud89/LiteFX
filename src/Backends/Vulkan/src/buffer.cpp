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
	const VulkanDevice& m_device;

public:
	VulkanBufferImpl(VulkanBuffer* parent, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, bool writable, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_writable(writable), m_allocator(allocator), m_allocation(allocation), m_device(device)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBuffer::VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, bool writable, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	m_impl(makePimpl<VulkanBufferImpl>(this, type, elements, elementSize, alignment, writable, device, allocator, allocation)), Resource<VkBuffer>(buffer)
{
	if (!name.empty())
		this->name() = name;
}

VulkanBuffer::~VulkanBuffer() noexcept
{
	::vmaDestroyBuffer(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", fmt::ptr(reinterpret_cast<void*>(this->handle())));
}

BufferType VulkanBuffer::type() const noexcept
{
	return m_impl->m_type;
}

UInt32 VulkanBuffer::elements() const noexcept
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

bool VulkanBuffer::writable() const noexcept
{
	return m_impl->m_writable;
}

UInt64 VulkanBuffer::virtualAddress() const noexcept
{
	VkBufferDeviceAddressInfo info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = this->handle()
	};

	return static_cast<UInt64>(::vkGetBufferDeviceAddress(m_impl->m_device.handle(), &info));
}

void VulkanBuffer::map(const void* const data, size_t size, UInt32 element)
{
	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", 0u, m_impl->m_elements, element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	size_t alignedSize = size;
	size_t alignment = this->elementAlignment();

	if (alignment > 0)
		alignedSize = (size + alignment - 1) & ~(alignment - 1);

	char* buffer;		// A pointer to the whole (aligned) buffer memory.
	raiseIfFailed(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	auto result = ::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size);

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);

	if (result != 0) [[unlikely]]
		throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);
}

void VulkanBuffer::map(Span<const void* const> data, size_t elementSize, UInt32 firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

void VulkanBuffer::map(void* data, size_t size, UInt32 element, bool write)
{
	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", 0u, m_impl->m_elements, element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	size_t alignedSize = size;
	size_t alignment = this->elementAlignment();

	if (alignment > 0)
		alignedSize = (size + alignment - 1) & ~(alignment - 1);

	char* buffer;		// A pointer to the whole (aligned) buffer memory.
	raiseIfFailed(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	auto result = write ?
		::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size) :
		::memcpy_s(data, size, reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize);

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);

	if (result != 0) [[unlikely]]
		throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);
}

void VulkanBuffer::map(Span<void*> data, size_t elementSize, UInt32 firstElement, bool write)
{
	std::ranges::for_each(data, [this, &elementSize, &write, i = firstElement](void* mem) mutable { this->map(mem, elementSize, i++, write); });
}

UniquePtr<IVulkanBuffer> VulkanBuffer::allocate(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, bool writable, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanBuffer::allocate("", type, elements, elementSize, alignment, writable, device, allocator, createInfo, allocationInfo, allocationResult);
}

UniquePtr<IVulkanBuffer> VulkanBuffer::allocate(const String& name, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, bool writable, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Writable: {5} }}", name.empty() ? fmt::to_string(fmt::ptr(reinterpret_cast<void*>(buffer))) : name, type, elements, elementSize, elements * elementSize, writable);

	return makeUnique<VulkanBuffer>(buffer, type, elements, elementSize, alignment, writable, device, allocator, allocation, name);
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

VulkanVertexBuffer::VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	m_impl(makePimpl<VulkanVertexBufferImpl>(this, layout)), VulkanBuffer(buffer, BufferType::Vertex, elements, layout.elementSize(), 0, false, device, allocator, allocation, name)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer() noexcept = default;

const VulkanVertexBufferLayout& VulkanVertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

UniquePtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const VulkanVertexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanVertexBuffer::allocate("", layout, elements, device, allocator, createInfo, allocationInfo, allocationResult);
}

UniquePtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const String& name, const VulkanVertexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", name.empty() ? fmt::to_string(fmt::ptr(reinterpret_cast<void*>(buffer))) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanVertexBuffer>(buffer, layout, elements, device, allocator, allocation, name);
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

VulkanIndexBuffer::VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	m_impl(makePimpl<VulkanIndexBufferImpl>(this, layout)), VulkanBuffer(buffer, BufferType::Index, elements, layout.elementSize(), 0, false, device, allocator, allocation, name)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;

const VulkanIndexBufferLayout& VulkanIndexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

UniquePtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const VulkanIndexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanIndexBuffer::allocate("", layout, elements, device, allocator, createInfo, allocationInfo, allocationResult);
}

UniquePtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const String& name, const VulkanIndexBufferLayout& layout, UInt32 elements, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate index buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", name.empty() ? fmt::to_string(fmt::ptr(reinterpret_cast<void*>(buffer))) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanIndexBuffer>(buffer, layout, elements, device, allocator, allocation, name);
}