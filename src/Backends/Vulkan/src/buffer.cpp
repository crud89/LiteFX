#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBuffer::VulkanBufferImpl {
public:
	friend class VulkanBuffer;

private:
	BufferType m_type;
	UInt32 m_elements;
	size_t m_elementSize, m_alignment;
	ResourceUsage m_usage;
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	const VulkanDevice& m_device;

public:
	VulkanBufferImpl(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_usage(usage), m_allocator(allocator), m_allocation(allocation), m_device(device)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBuffer::VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	Resource<VkBuffer>(buffer), m_impl(type, elements, elementSize, alignment, usage, device, allocator, allocation)
{
	if (!name.empty())
		this->name() = name;
}

VulkanBuffer::~VulkanBuffer() noexcept
{
	::vmaDestroyBuffer(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", static_cast<void*>(this->handle()));
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

ResourceUsage VulkanBuffer::usage() const noexcept
{
	return m_impl->m_usage;
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
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	void* buffer{};		// A pointer to the whole (aligned) buffer memory.
	void* bufferPtr = static_cast<void*>(buffer);
	raiseIfFailed(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, &bufferPtr), "Unable to map buffer memory.");
	std::memcpy(static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), data, size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);
}

void VulkanBuffer::map(Span<const void* const> data, size_t elementSize, UInt32 firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

void VulkanBuffer::map(void* data, size_t size, UInt32 element, bool write)
{
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	char* buffer{};		// A pointer to the whole (aligned) buffer memory.
	void* bufferPtr = static_cast<void*>(buffer);
	raiseIfFailed(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, &bufferPtr), "Unable to map buffer memory.");
	
	if (write)
		std::memcpy(static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), data, size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	else
		std::memcpy(data, static_cast<char*>(bufferPtr) + (element * this->alignedElementSize()), size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);
}

void VulkanBuffer::map(Span<void*> data, size_t elementSize, UInt32 firstElement, bool write)
{
	std::ranges::for_each(data, [this, &elementSize, &write, i = firstElement](void* mem) mutable { this->map(mem, elementSize, i++, write); });
}

SharedPtr<IVulkanBuffer> VulkanBuffer::allocate(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanBuffer::allocate("", type, elements, elementSize, alignment, usage, device, allocator, createInfo, allocationInfo, allocationResult);
}

SharedPtr<IVulkanBuffer> VulkanBuffer::allocate(const String& name, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(buffer)) : name, type, elements, elementSize, elements * elementSize, usage);

	return SharedPtr<VulkanBuffer>(new VulkanBuffer(buffer, type, elements, elementSize, alignment, usage, device, allocator, allocation, name));
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBuffer::VulkanVertexBufferImpl {
public:
	friend class VulkanVertexBuffer;

private:
	const VulkanVertexBufferLayout& m_layout;

public:
	VulkanVertexBufferImpl(const VulkanVertexBufferLayout& layout) :
		m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Vertex buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBuffer::VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	VulkanBuffer(buffer, BufferType::Vertex, elements, layout.elementSize(), 0, usage, device, allocator, allocation, name), m_impl(layout)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer() noexcept = default;

const VulkanVertexBufferLayout& VulkanVertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

SharedPtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanVertexBuffer::allocate("", layout, elements, usage, device, allocator, createInfo, allocationInfo, allocationResult);
}

SharedPtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const String& name, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(buffer)) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedPtr<VulkanVertexBuffer>(new VulkanVertexBuffer(buffer, layout, elements, usage, device, allocator, allocation, name));
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBuffer::VulkanIndexBufferImpl {
public:
	friend class VulkanIndexBuffer;

private:
	const VulkanIndexBufferLayout& m_layout;

public:
	VulkanIndexBufferImpl(const VulkanIndexBufferLayout& layout) :
		m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Index buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanIndexBuffer::VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	VulkanBuffer(buffer, BufferType::Index, elements, layout.elementSize(), 0, usage, device, allocator, allocation, name), m_impl(layout)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;

const VulkanIndexBufferLayout& VulkanIndexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

SharedPtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanIndexBuffer::allocate("", layout, elements, usage, device, allocator, createInfo, allocationInfo, allocationResult);
}

SharedPtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const String& name, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate index buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", static_cast<void*>(buffer)) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedPtr<VulkanIndexBuffer>(new VulkanIndexBuffer(buffer, layout, elements, usage, device, allocator, allocation, name));
}