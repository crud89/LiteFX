#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

_VMABuffer::_VMABuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const UInt32& elementSize, VmaAllocator& allocator, VmaAllocation allocation) :
    VulkanBuffer(buffer, type, elements, elementSize), m_allocator(allocator), m_allocationInfo(allocation)
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

    auto result = ::memcpy_s(buffer, this->getSize(), data, size);

    if (result != 0)
    {
        LITEFX_ERROR(VULKAN_LOG, "Error mapping buffer to device memory: {#X}.", result);
        throw std::runtime_error("Error mapping buffer to device memory.");
    }

    ::vmaUnmapMemory(m_allocator, m_allocationInfo);
    LITEFX_TRACE(VULKAN_LOG, "Mapped {1} bytes to buffer {0} {{ Type: {2} }}", fmt::ptr(this->handle()), size, this->getType());
}

void _VMABuffer::transfer(const ICommandQueue* q, IBuffer* t, const size_t& size, const size_t& offset, const size_t& targetOffset) const
{
    auto transferQueue = dynamic_cast<const VulkanQueue*>(q);
    auto target = dynamic_cast<const VulkanBuffer*>(t);

    if (target == nullptr)
        throw std::invalid_argument("The transfer target buffer must be initialized and a valid Vulkan buffer.");

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
    ::vkCmdCopyBuffer(commandBuffer->handle(), this->handle(), target->handle(), 1, &copyInfo);
    
    // End the transfer recording and submit the buffer.
    commandBuffer->end();
    commandBuffer->submit(true);
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
    auto elementSize = this->getElementSize();

    switch (this->getType())
    {
    case BufferType::Vertex:
        ::vkCmdBindVertexBuffers(commandBuffer->handle(), 0, 1, &this->handle(), offsets);
        break;
    case BufferType::Index:
        if (elementSize != 4 && elementSize != 2)
            throw std::runtime_error("Unsupported index buffer element size.");

        ::vkCmdBindIndexBuffer(commandBuffer->handle(), this->handle(), 0, elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        break;
    default:
        throw std::runtime_error("The buffer could not be bound: unsupported buffer type.");
    }
}

// ------------------------------------------------------------------------------------------------
// Factory.
// ------------------------------------------------------------------------------------------------

UniquePtr<IBuffer> _VMABuffer::makeBuffer(const BufferType& type, const UInt32& elements, const UInt32& elementSize, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	// Allocate the buffer.
	VkBuffer buffer;
	VmaAllocation allocation;

	if (::vmaCreateBuffer(allocator, &createInfo, &allocationInfo, &buffer, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate buffer.");

    LITEFX_DEBUG(VULKAN_LOG, "Allocated buffer {0} with {4} bytes {{ Type: {1}, Elements: {2}, Element Size: {3} }}", fmt::ptr(buffer), type, elements, elementSize, elements * elementSize);

    return makeUnique<_VMABuffer>(buffer, type, elements, elementSize, allocator, allocation);
}