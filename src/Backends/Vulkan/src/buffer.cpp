#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

_VMABuffer::_VMABuffer(const BufferLayout* layout, VkBuffer buffer, VmaAllocator& allocator, VmaAllocation allocation) :
    VulkanBuffer(layout, buffer), m_allocator(allocator), m_allocationInfo(allocation)
{
}

_VMABuffer::~_VMABuffer() noexcept
{
    ::vmaDestroyBuffer(m_allocator, this->handle(), m_allocationInfo);
}

void _VMABuffer::map(const void* const data, const size_t& size)
{
    void* buffer;
    
    if (::vmaMapMemory(m_allocator, m_allocationInfo, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Unable to map buffer memory.");

    // TODO: Figure out why calling m_allocationInfo->GetSize() produces a C2027 error. 
    //auto result = ::memcpy_s(buffer, m_allocationInfo->GetSize(), data, size);
    ::memcpy(buffer, data, size);
    ::vmaUnmapMemory(m_allocator, m_allocationInfo);

    //if (result != 0)
    //{
    //    LITEFX_ERROR(VULKAN_LOG, "Error mapping buffer to device memory: {#X}", result);
    //    throw std::runtime_error("Error mapping buffer to device memory.");
    //}
}

void _VMABuffer::transfer(IBuffer* target) const
{
    throw;
}

// ------------------------------------------------------------------------------------------------
// Factory.
// ------------------------------------------------------------------------------------------------

UniquePtr<IBuffer> _VMABuffer::makeBuffer(const BufferLayout* layout, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    if (layout == nullptr)
        throw std::invalid_argument("The buffer layout must be initialized.");

	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

    return makeUnique<_VMABuffer>(layout, buffer, allocator, allocation);
}