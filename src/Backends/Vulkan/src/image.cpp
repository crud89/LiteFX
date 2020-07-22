#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image.
// ------------------------------------------------------------------------------------------------
_VMAImage::_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format)
	: _VMAImage(device, image, elements, size, extent, format, nullptr, nullptr)
{
}

_VMAImage::_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format, VmaAllocator allocator, VmaAllocation allocation) :
    _VMAImageBase(image, allocator, allocation), Image(elements, size, extent, format), VulkanRuntimeObject(device)
{
	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = ::getFormat(format);
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (::vkCreateImageView(this->getDevice()->handle(), &createInfo, nullptr, &m_view) != VK_SUCCESS)
		throw std::runtime_error("Unable to create image view!");
}

_VMAImage::~_VMAImage() noexcept
{
	::vkDestroyImageView(this->getDevice()->handle(), m_view, nullptr);
}

const VkImageView& _VMAImage::getImageView() const noexcept
{
	return m_view;
}

UniquePtr<IImage> _VMAImage::allocate(const VulkanDevice* device, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    if (device == nullptr)
        throw std::invalid_argument("The device must be initialized.");

    // Allocate the buffer.
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo result;

    if (::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, &result) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate texture.");
    
    LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4} }}", fmt::ptr(image), size, extent.width(), extent.height(), format);

    if (allocationResult != nullptr)
        *allocationResult = result;

    return makeUnique<_VMAImage>(device, image, elements, size, extent, format, allocator, allocation);
}