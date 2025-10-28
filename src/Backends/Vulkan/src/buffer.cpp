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
	VkBufferCreateInfo m_createInfo;
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	UInt64 m_virtualAddress{0};

public:
	VulkanBufferImpl(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		m_type(type), m_elements(elements), m_elementSize(elementSize), m_alignment(alignment), m_usage(usage), m_createInfo(createInfo), m_allocator(allocator), m_allocation(allocation)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBuffer::VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	Resource<VkBuffer>(buffer), m_impl(type, elements, elementSize, alignment, usage, createInfo, allocator, allocation)
{
	if (!name.empty())
	{
		this->name() = name;
#ifndef NDEBUG
		::vmaSetAllocationName(allocator, allocation, name.c_str());
#endif
	}

	// Store the virtual address.
	VkBufferDeviceAddressInfo info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = this->handle()
	};

	m_impl->m_virtualAddress = static_cast<UInt64>(::vkGetBufferDeviceAddress(device.handle(), &info));

	if (m_impl->m_allocator != nullptr && m_impl->m_allocation != nullptr)
		::vmaSetAllocationUserData(m_impl->m_allocator, m_impl->m_allocation, static_cast<IDeviceMemory*>(this));
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

VmaAllocator& VulkanBuffer::allocator() const noexcept
{
	return m_impl->m_allocator;
}

VmaAllocation& VulkanBuffer::allocationInfo() const noexcept
{
	return m_impl->m_allocation;
}

SharedPtr<IVulkanBuffer> VulkanBuffer::allocate(const String& name, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate buffer.");

#ifndef NDEBUG
	VkMemoryPropertyFlags memoryProperties{};
	::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}", 
		name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, type, elements, elementSize, elements * elementSize, usage, 
		(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

	return SharedObject::create<VulkanBuffer>(buffer, type, elements, elementSize, alignment, usage, createInfo, device, allocator, allocation, name);
}

bool VulkanBuffer::tryAllocate(SharedPtr<IVulkanBuffer>& buffer, const String& name, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	buffer = nullptr;
	VkBuffer bufferHandle{};
	VmaAllocation allocation{};

	auto result = ::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &bufferHandle, &allocation, allocationResult);

	if (result != VK_SUCCESS)
	{
		LITEFX_DEBUG(VULKAN_LOG, "Allocation for buffer {0} with {4} bytes failed: {6} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, type, elements, elementSize, elements * elementSize, usage, result);
		return false;
	}
	else
	{
#ifndef NDEBUG
		VkMemoryPropertyFlags memoryProperties{};
		::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);

		LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, type, elements, elementSize, elements * elementSize, usage,
			(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

		buffer = SharedObject::create<VulkanBuffer>(bufferHandle, type, elements, elementSize, alignment, usage, createInfo, device, allocator, allocation, name);
		return true;
	}
}

bool VulkanBuffer::move(SharedPtr<IVulkanBuffer> buffer, VmaAllocation to, const VulkanCommandBuffer& commandBuffer) // NOLINT(performance-unnecessary-value-param)
{
	// NOTES: If this method returns true, the command buffer must be executed and all bindings to the image must be updated afterwards, otherwise the result of this operation is undefined behavior.
	// TODO: Handle host-visible copies

	if (buffer == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("buffer");

	if (to == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("to");

	auto& source = dynamic_cast<VulkanBuffer&>(*buffer);
	const auto device = commandBuffer.queue()->device();
	const auto& createInfo = source.m_impl->m_createInfo;
	auto allocator = source.m_impl->m_allocator;

	VkBuffer bufferHandle{};
	auto result = ::vkCreateBuffer(device->handle(), &createInfo, nullptr, &bufferHandle);

	if (result != VK_SUCCESS) [[unlikely]]
		return false;

	result = ::vmaBindBufferMemory(allocator, to, bufferHandle);

	if (result != VK_SUCCESS) [[unlikely]]
	{
		::vkDestroyBuffer(device->handle(), bufferHandle, nullptr);
		return false;
	}

	// NOTE: Buffers in the HOST_VISIBLE and HOST_CACHED memory can be copied synchronously using a usual `memcpy`
	
	// Transfer the buffer.
	if (!buffer->volatileMove())
	{
		VkBufferCopy copyInfo { .size = buffer->elements() * buffer->alignedElementSize() };
		::vkCmdCopyBuffer(commandBuffer.handle(), std::as_const(*buffer).handle(), bufferHandle, 1u, &copyInfo);
	}

	// Reset the buffer.
	// NOTE: At this point, the previous resource does still exist, but is inaccessible through the current instance. The only remaining reference should be stored by the source allocation during 
	//       defragmentation. After the command buffer executed, the resource will be destroyed. If a reference is stored somewhere else it will get invalid, but you should never store the 
	//       reference obtained by calling `handle` manually.
	//       The new resource handle is valid beyond this point, but may contain uninitialized data. Any attempt of using the resource must be properly synchronized to execute after the submission
	//       of `commandBuffer`.
	source.handle() = bufferHandle;
	return true;
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

VulkanVertexBuffer::VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	VulkanBuffer(buffer, BufferType::Vertex, elements, layout.elementSize(), 0, usage, createInfo, device, allocator, allocation, name), m_impl(layout)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer() noexcept = default;

const VulkanVertexBufferLayout& VulkanVertexBuffer::layout() const noexcept
{
	return *m_impl->m_layout;
}

SharedPtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const String& name, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate vertex buffer.");

#ifndef NDEBUG
	VkMemoryPropertyFlags memoryProperties{};
	::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);
	
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}", 
		name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage,
		(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

	return SharedObject::create<VulkanVertexBuffer>(buffer, layout, elements, usage, createInfo, device, allocator, allocation, name);
}

bool VulkanVertexBuffer::tryAllocate(SharedPtr<IVulkanVertexBuffer>& buffer, const String& name, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer bufferHandle{};
	VmaAllocation allocation{};

	auto result = ::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &bufferHandle, &allocation, allocationResult);

	if (result != VK_SUCCESS)
	{
		LITEFX_DEBUG(VULKAN_LOG, "Allocation for buffer {0} with {4} bytes failed: {6} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage, result);
		return false;
	}
	else
	{
#ifndef NDEBUG
		VkMemoryPropertyFlags memoryProperties{};
		::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);

		LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements, usage,
			(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

		buffer = SharedObject::create<VulkanVertexBuffer>(bufferHandle, layout, elements, usage, createInfo, device, allocator, allocation, name);
		return true;
	}
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

VulkanIndexBuffer::VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name) :
	VulkanBuffer(buffer, BufferType::Index, elements, layout.elementSize(), 0, usage, createInfo, device, allocator, allocation, name), m_impl(layout)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;

const VulkanIndexBufferLayout& VulkanIndexBuffer::layout() const noexcept
{
	return *m_impl->m_layout;
}

SharedPtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const String& name, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate index buffer.");

#ifndef NDEBUG
	VkMemoryPropertyFlags memoryProperties{};
	::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}", 
		name.empty() ? std::format("{0}", Vk::handleAddress(buffer)) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage,
		(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

	return SharedObject::create<VulkanIndexBuffer>(buffer, layout, elements, usage, createInfo, device, allocator, allocation, name);
}

bool VulkanIndexBuffer::tryAllocate(SharedPtr<IVulkanIndexBuffer>& buffer, const String& name, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer bufferHandle{};
	VmaAllocation allocation{};

	auto result = ::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &bufferHandle, &allocation, allocationResult);

	if (result != VK_SUCCESS)
	{
		LITEFX_DEBUG(VULKAN_LOG, "Allocation for buffer {0} with {4} bytes failed: {6} {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage, result);
		return false;
	}
	else
	{
#ifndef NDEBUG
		VkMemoryPropertyFlags memoryProperties{};
		::vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);

		LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3}, Usage: {5}, On GPU: {6}, CPU Access: {7} }}",
			name.empty() ? std::format("{0}", Vk::handleAddress(bufferHandle)) : name, BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements, usage,
			(memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#endif

		buffer = SharedObject::create<VulkanIndexBuffer>(bufferHandle, layout, elements, usage, createInfo, device, allocator, allocation, name);
		return true;
	}
}