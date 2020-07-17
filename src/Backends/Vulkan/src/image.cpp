#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

_VMAImage::_VMAImage(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size, const UInt32& binding, VmaAllocator& allocator, VmaAllocation allocation) :
    VulkanTexture(device, image, format, size, binding), m_allocator(allocator), m_allocationInfo(allocation)
{
}

_VMAImage::~_VMAImage() noexcept
{
    ::vmaDestroyImage(m_allocator, this->handle(), m_allocationInfo);
    LITEFX_TRACE(VULKAN_LOG, "Destroyed texture {0}", fmt::ptr(this->handle()));
}

void _VMAImage::map(const void* const data, const size_t& size)
{
    void* buffer;

    if (::vmaMapMemory(m_allocator, m_allocationInfo, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Unable to map texture memory.");

    auto result = ::memcpy_s(buffer, this->getSize(), data, size);

    if (result != 0)
    {
        LITEFX_ERROR(VULKAN_LOG, "Error mapping texture to device memory: {#X}.", result);
        throw std::runtime_error("Error mapping texture to device memory.");
    }

    ::vmaUnmapMemory(m_allocator, m_allocationInfo);
}

// ------------------------------------------------------------------------------------------------
// Factory.
// ------------------------------------------------------------------------------------------------

UniquePtr<ITexture> _VMAImage::makeImage(const VulkanDevice* device, const Format& format, const Size2d& size, const UInt32& binding, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    if (device == nullptr)
        throw std::invalid_argument("The device must be initialized.");

    // Allocate the buffer.
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo result;

    if (::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, &result) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate texture.");
    
    LITEFX_DEBUG(VULKAN_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Binding: {5} }}", fmt::ptr(image), static_cast<size_t>(result.size), size.width(), size.height(), format, binding);

    if (allocationResult != nullptr)
        *allocationResult = result;

    return makeUnique<_VMAImage>(device, image, format, size, binding, allocator, allocation);
}