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
	UInt64 m_virtualAddress{0};

public:
	VulkanBufferImpl(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_usage(usage), m_allocator(allocator), m_allocation(allocation)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBuffer::VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	Resource<VkBuffer>(buffer), m_impl(type, elements, elementSize, alignment, usage, allocator, allocation)
{
	if (!name.empty())
		this->name() = name;

	// Store the virtual address.
	VkBufferDeviceAddressInfo info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = this->handle()
	};

	m_impl->m_virtualAddress = static_cast<UInt64>(::vkGetBufferDeviceAddress(device.handle(), &info));
}

VulkanBuffer::~VulkanBuffer() noexcept
{
	::vmaDestroyBuffer(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", Vk::handleAddress(this->handle()));
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
	return m_impl->m_virtualAddress;
}

void VulkanBuffer::map(const void* const data, size_t size, UInt32 element)
{
	if (data == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("data", "The data pointer must be initialized.");

	if (element >= m_impl->m_elements) [[unlikely]]
		throw ArgumentOutOfRangeException("element", std::make_pair(0u, m_impl->m_elements), element, "The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);
		
	if (this->size() - (element * this->alignedElementSize()) < size) [[unlikely]]
		throw InvalidArgumentException("size", "The provided data size would overflow the buffer (buffer offset: 0x{1:X}; {2} bytes remaining but size was set to {0}).", size, element * this->alignedElementSize(), this->size() - (element * this->alignedElementSize()));

	this->write(data, size, element * this->alignedElementSize());
}

void VulkanBuffer::map(Span<const void* const> data, size_t elementSize, UInt32 firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](auto mem) mutable { this->map(mem, elementSize, i++); });
}

void VulkanBuffer::map(void* data, size_t size, UInt32 element, bool write)
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

void VulkanBuffer::map(Span<void*> data, size_t elementSize, UInt32 firstElement, bool write)
{
	std::ranges::for_each(data, [this, &elementSize, &write, i = firstElement](auto mem) mutable { this->map(mem, elementSize, i++, write); });
}

void VulkanBuffer::write(const void* const data, size_t size, size_t offset)
{
	raiseIfFailed(::vmaCopyMemoryToAllocation(m_impl->m_allocator, data, m_impl->m_allocation, offset, size), "Unable to write to buffer.");
}

void VulkanBuffer::read(void* data, size_t size, size_t offset)
{
	raiseIfFailed(::vmaCopyAllocationToMemory(m_impl->m_allocator, m_impl->m_allocation, offset, data, size), "Unable to read from buffer.");
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
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, type, elements, elementSize, elements * elementSize, usage);

	return SharedObject::create<VulkanBuffer>(buffer, type, elements, elementSize, alignment, usage, device, allocator, allocation, name);
}

// ------------------------------------------------------------------------------------------------
// Vertex buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBuffer::VulkanVertexBufferImpl {
public:
	friend class VulkanVertexBuffer;

private:
	SharedPtr<const VulkanVertexBufferLayout> m_layout;

public:
	VulkanVertexBufferImpl(const VulkanVertexBufferLayout& layout) :
		m_layout(layout.shared_from_this())
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
	return *m_impl->m_layout;
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
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<VulkanVertexBuffer>(buffer, layout, elements, usage, device, allocator, allocation, name);
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBuffer::VulkanIndexBufferImpl {
public:
	friend class VulkanIndexBuffer;

private:
	SharedPtr<const VulkanIndexBufferLayout> m_layout;

public:
	VulkanIndexBufferImpl(const VulkanIndexBufferLayout& layout) :
		m_layout(layout.shared_from_this())
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
	return *m_impl->m_layout;
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
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}", name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage);

	return SharedObject::create<VulkanIndexBuffer>(buffer, layout, elements, usage, device, allocator, allocation, name);
}