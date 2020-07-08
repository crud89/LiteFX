#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

_VMABuffer::_VMABuffer(const BufferType& type, const BufferLayout* layout, VkBuffer buffer, VmaAllocator& allocator, VmaAllocation allocation) :
    VulkanBuffer(type, layout, buffer), m_allocator(allocator), m_allocationInfo(allocation)
{
}

_VMABuffer::~_VMABuffer() noexcept
{
    ::vmaDestroyBuffer(m_allocator, this->handle(), m_allocationInfo);
    LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0} {{ Type: {1} }}", fmt::ptr(this->handle()), this->getType());
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

    LITEFX_TRACE(VULKAN_LOG, "Mapped {1} bytes to buffer {0} {{ Type: {2} }}", fmt::ptr(this->handle()), size, this->getType());
}

void _VMABuffer::transfer(IBuffer* target) const
{
    throw;
}

void _VMABuffer::bind(const IRenderPass* renderPass) const
{
    if (renderPass == nullptr)
        throw std::invalid_argument("The render pass must be initialized.");

    // Get the command buffer from the render pass.
    // TODO: We somehow need to get rid of this dynamic cast.
    auto commandBuffer = dynamic_cast<const VulkanCommandBuffer*>(renderPass->getCommandBuffer());

    if (commandBuffer == nullptr)
        throw std::invalid_argument("The command buffer of the render pass is either not initialized or not a valid Vulkan command buffer.");

    // Depending on the type, bind the buffer accordingly.
    constexpr VkDeviceSize offsets[] = { 0 };

    switch (this->getType())
    {
    case BufferType::Vertex:
        ::vkCmdBindVertexBuffers(commandBuffer->handle(), 0, 1, &this->handle(), offsets);
        break;
    default:
        throw std::runtime_error("The buffer could not be bound: unsupported buffer type.");
    }
}

// ------------------------------------------------------------------------------------------------
// Factory.
// ------------------------------------------------------------------------------------------------

UniquePtr<IBuffer> _VMABuffer::makeBuffer(const BufferType& type, const BufferLayout* layout, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    if (layout == nullptr)
        throw std::invalid_argument("The buffer layout must be initialized.");

	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

    LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} for layout {1} {{ Type: {2} }}", fmt::ptr(buffer), fmt::ptr(layout), type);

    return makeUnique<_VMABuffer>(type, layout, buffer, allocator, allocation);
}