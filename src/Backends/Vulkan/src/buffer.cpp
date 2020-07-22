#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

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