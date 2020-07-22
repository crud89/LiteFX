#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Buffer Base.
// ------------------------------------------------------------------------------------------------
_VMABufferBase::_VMABufferBase(VkBuffer buffer, VmaAllocator& allocator, VmaAllocation allocation) :
	IResource(buffer), m_allocator(allocator), m_allocationInfo(allocation) 
{
}

_VMABufferBase::~_VMABufferBase() noexcept
{
	::vmaDestroyBuffer(m_allocator, this->handle(), m_allocationInfo);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0}", fmt::ptr(this->handle()));
}

void _VMABufferBase::map(const void* const data, const size_t& size)
{
	void* buffer;

	if (::vmaMapMemory(m_allocator, m_allocationInfo, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Unable to map buffer memory.");

	auto result = ::memcpy_s(buffer, this->getSize(), data, size);

	if (result != 0) {
		LITEFX_ERROR(VULKAN_LOG, "Error mapping buffer to device memory: {#X}.", result);
		throw std::runtime_error("Error mapping buffer to device memory.");
	}

	::vmaUnmapMemory(m_allocator, m_allocationInfo);
}

void _VMABufferBase::transfer(const ICommandQueue* commandQueue, IBuffer* source, const size_t& size, const size_t& offset, const size_t& targetOffset) const
{
	auto transferQueue = dynamic_cast<const VulkanQueue*>(commandQueue);
	auto sourceBuffer = dynamic_cast<const IResource<VkBuffer>*>(source);

	if (sourceBuffer == nullptr)
		throw std::invalid_argument("The transfer source buffer must be initialized and a valid Vulkan buffer.");

	if (transferQueue == nullptr)
		throw std::invalid_argument("The transfer queue must be initialized and a valid Vulkan command queue.");

	auto device = dynamic_cast<const VulkanDevice*>(transferQueue->getDevice());

	if (device == nullptr)
		throw std::runtime_error("The transfer queue must be bound to a valid Vulkan device.");

	auto commandBuffer = makeUnique<const VulkanCommandBuffer>(transferQueue);

	// Begin the transfer recording.
	commandBuffer->begin();

	// Create a copy command and add it to the command buffer.
	VkBufferCopy copyInfo{};
	copyInfo.size = size;
	copyInfo.srcOffset = offset;
	copyInfo.dstOffset = targetOffset;
	::vkCmdCopyBuffer(commandBuffer->handle(), sourceBuffer->handle(), this->handle(), 1, &copyInfo);

	// End the transfer recording and submit the buffer.
	commandBuffer->end();
	commandBuffer->submit(true);
}

// ------------------------------------------------------------------------------------------------
// Generic Buffer.
// ------------------------------------------------------------------------------------------------

_VMABuffer::_VMABuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, VmaAllocation allocation) :
	_VMABufferBase(buffer, allocator, allocation), Buffer(type, elements, size)
{
}

_VMABuffer::~_VMABuffer() noexcept = default;

UniquePtr<IBuffer> _VMABuffer::allocate(const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), type, elements, size / elements, size);

	return makeUnique<_VMABuffer>(buffer, type, elements, size, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Vertex Buffer.
// ------------------------------------------------------------------------------------------------

_VMAVertexBuffer::_VMAVertexBuffer(VkBuffer buffer, const IVertexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation) :
	_VMABufferBase(buffer, allocator, allocation), VertexBuffer(layout, elements)
{
}

_VMAVertexBuffer::~_VMAVertexBuffer() noexcept = default;

UniquePtr<IVertexBuffer> _VMAVertexBuffer::allocate(const IVertexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	if (layout == nullptr)
		throw std::invalid_argument("The layout must be initialized.");

	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), BufferType::Vertex, elements, layout->getElementSize(), layout->getElementSize() * elements);

	return makeUnique<_VMAVertexBuffer>(buffer, layout, elements, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Index Buffer.
// ------------------------------------------------------------------------------------------------

_VMAIndexBuffer::_VMAIndexBuffer(VkBuffer buffer, const IIndexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation) :
	_VMABufferBase(buffer, allocator, allocation), IndexBuffer(layout, elements)
{
}

_VMAIndexBuffer::~_VMAIndexBuffer() noexcept = default;

UniquePtr<IIndexBuffer> _VMAIndexBuffer::allocate(const IIndexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	if (layout == nullptr)
		throw std::invalid_argument("The layout must be initialized.");

	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), BufferType::Index, elements, layout->getElementSize(), layout->getElementSize() * elements);

	return makeUnique<_VMAIndexBuffer>(buffer, layout, elements, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Constant Buffer.
// ------------------------------------------------------------------------------------------------

_VMAConstantBuffer::_VMAConstantBuffer(VkBuffer buffer, const IDescriptorLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation) :
	_VMABufferBase(buffer, allocator, allocation), ConstantBuffer(layout, elements)
{
}

_VMAConstantBuffer::~_VMAConstantBuffer() noexcept = default;

UniquePtr<IConstantBuffer> _VMAConstantBuffer::allocate(const IDescriptorLayout * layout, const UInt32 & elements, VmaAllocator & allocator, const VkBufferCreateInfo & createInfo, const VmaAllocationCreateInfo & allocationInfo, VmaAllocationInfo * allocationResult)
{
	if (layout == nullptr)
		throw std::invalid_argument("The layout must be initialized.");

	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

	LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), layout->getType(), elements, layout->getElementSize(), layout->getElementSize() * elements);

	return makeUnique<_VMAConstantBuffer>(buffer, layout, elements, allocator, allocation);
}