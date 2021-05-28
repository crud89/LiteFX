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
	size_t m_elementSize;
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;

public:
	VulkanBufferImpl(VulkanBuffer* parent, const BufferType& type, const UInt32& elements, const size_t& elementSize, const VmaAllocator& allocator, const VmaAllocation& allocation) :
		base(parent), m_type(type), m_elements(elements), m_elementSize(elementSize), m_allocator(allocator), m_allocation(allocation)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Buffer shared interface.
// ------------------------------------------------------------------------------------------------
VulkanBuffer::VulkanBuffer(const VulkanDevice& device, VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanBufferImpl>(this, type, elements, elementSize, allocator, allocation)), VulkanRuntimeObject<VulkanDevice>(device, &device), Resource<VkBuffer>(buffer)
{
}

VulkanBuffer::~VulkanBuffer() noexcept
{
	::vmaDestroyBuffer(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", fmt::ptr(this->handle()));
}

void VulkanBuffer::transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset)
{
	// TODO: Don't ignore alignment!!!
	// TODO: Use multiple VkBufferCopys to allow to copy multiple array elements. Currently only one contiguous block can be copied at a time.
	VkBufferCopy copyInfo{};
	copyInfo.size = size;
	copyInfo.srcOffset = sourceOffset;
	copyInfo.dstOffset = targetOffset;
	::vkCmdCopyBuffer(commandBuffer.handle(), source.handle(), this->handle(), 1, &copyInfo);
}

void VulkanBuffer::transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset) const
{
	// TODO: Don't ignore alignment!!!
	// TODO: Use multiple VkBufferCopys to allow to copy multiple array elements. Currently only one contiguous block can be copied at a time.
	VkBufferCopy copyInfo{};
	copyInfo.size = size;
	copyInfo.srcOffset = sourceOffset;
	copyInfo.dstOffset = targetOffset;
	::vkCmdCopyBuffer(commandBuffer.handle(), this->handle(), target.handle(), 1, &copyInfo);
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
	return static_cast<size_t>(m_impl->m_elements) * m_impl->m_elementSize;
}

size_t VulkanBuffer::elementSize() const noexcept
{
	return m_impl->m_elementSize;
}

void VulkanBuffer::map(const void* const data, const size_t& size, const UInt32& element)
{
	if (element >= m_impl->m_elements)
		throw ArgumentOutOfRangeException("The element {0} is out of range. The buffer only contains {1} elements.", element, m_impl->m_elements);

	size_t alignedSize = size;
	size_t alignment = 0;

	if (m_impl->m_type == BufferType::Uniform)
		alignment = this->getDevice()->adapter().getLimits().minUniformBufferOffsetAlignment;
	else if (m_impl->m_type == BufferType::Storage)
		alignment = this->getDevice()->adapter().getLimits().minStorageBufferOffsetAlignment;

	if (alignment > 0)
		alignedSize = (size + alignment - 1) & ~(alignment - 1);

	char* buffer;		// A pointer to the whole (aligned) buffer memory.
	raiseIfFailed<RuntimeException>(::vmaMapMemory(m_impl->m_allocator, m_impl->m_allocation, reinterpret_cast<void**>(&buffer)), "Unable to map buffer memory.");
	auto result = ::memcpy_s(reinterpret_cast<void*>(buffer + (element * alignedSize)), alignedSize, data, size);

	if (result != 0)
		throw RuntimeException("Error mapping buffer to device memory: {#X}.", result);

	::vmaUnmapMemory(m_impl->m_allocator, m_impl->m_allocation);
}

void VulkanBuffer::map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement)
{
	std::ranges::for_each(data, [this, &elementSize, i = firstElement](const void* const mem) mutable { this->map(mem, elementSize, i++); });
}

UniquePtr<IVulkanBuffer> VulkanBuffer::allocate(const VulkanDevice& device, const BufferType& type, const UInt32& elements, const size_t& elementSize, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), type, elements, elementSize, elements * elementSize);

	return makeUnique<VulkanBuffer>(device, buffer, type, elements, elementSize, allocator, allocation);
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

VulkanVertexBuffer::VulkanVertexBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanVertexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanVertexBufferImpl>(this, layout)), VulkanBuffer(device, buffer, BufferType::Vertex, elements, layout.elementSize(), allocator, allocation)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer() noexcept = default;

const VulkanVertexBufferLayout& VulkanVertexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const UInt32& VulkanVertexBuffer::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

UniquePtr<IVulkanVertexBuffer> VulkanVertexBuffer::allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate vertex buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), BufferType::Vertex, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanVertexBuffer>(*layout.getDevice(), buffer, layout, elements, allocator, allocation);
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

VulkanIndexBuffer::VulkanIndexBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanIndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanIndexBufferImpl>(this, layout)), VulkanBuffer(device, buffer, BufferType::Index, elements, layout.elementSize(), allocator, allocation)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;

const VulkanIndexBufferLayout& VulkanIndexBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

UniquePtr<IVulkanIndexBuffer> VulkanIndexBuffer::allocate(const VulkanIndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate index buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), BufferType::Index, elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanIndexBuffer>(*layout.getDevice(), buffer, layout, elements, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Constant buffer implementation.
// ------------------------------------------------------------------------------------------------

class VulkanConstantBuffer::VulkanConstantBufferImpl : public Implement<VulkanConstantBuffer> {
public:
	friend class VulkanConstantBuffer;

private:
	const VulkanDescriptorLayout& m_layout;

public:
	VulkanConstantBufferImpl(VulkanConstantBuffer* parent, const VulkanDescriptorLayout& layout) :
		base(parent), m_layout(layout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Constant buffer shared interface.
// ------------------------------------------------------------------------------------------------

VulkanConstantBuffer::VulkanConstantBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanDescriptorLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation) :
	m_impl(makePimpl<VulkanConstantBufferImpl>(this, layout)), VulkanBuffer(device, buffer, layout.type(), elements, layout.elementSize(), allocator, allocation)
{
}

VulkanConstantBuffer::~VulkanConstantBuffer() noexcept = default;

const VulkanDescriptorLayout& VulkanConstantBuffer::layout() const noexcept
{
	return m_impl->m_layout;
}

const UInt32& VulkanConstantBuffer::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

UniquePtr<IVulkanConstantBuffer> VulkanConstantBuffer::allocate(const VulkanDescriptorLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo * allocationResult)
{
	VkBuffer buffer;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult), "Unable to allocate constant buffer.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), layout.type(), elements, layout.elementSize(), layout.elementSize() * elements);

	return makeUnique<VulkanConstantBuffer>(*layout.getDevice(), buffer, layout, elements, allocator, allocation);
}