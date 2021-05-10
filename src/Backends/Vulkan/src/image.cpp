#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class _VMAImageBase::_VMAImageBaseImpl : public Implement<_VMAImageBase> {
public:
	friend class _VMAImageBase;

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocationInfo;
	VkImageView m_view;

public:
	_VMAImageBaseImpl(_VMAImageBase* parent, VmaAllocator allocator, VmaAllocation allocation) :
		base(parent), m_allocator(allocator), m_allocationInfo(allocation)
	{
	}

public:
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

_VMAImageBase::_VMAImageBase(const VulkanDevice& device, VkImage image, VmaAllocator allocator, VmaAllocation allocation) :
	IResource(image), VulkanRuntimeObject<VulkanDevice>(device, &device), m_impl(makePimpl<_VMAImageBaseImpl>(this, allocator, allocation))
{
}

_VMAImageBase::~_VMAImageBase() noexcept 
{
}

VmaAllocator& _VMAImageBase::allocator() const noexcept
{
	return m_impl->m_allocator;
}

VmaAllocation& _VMAImageBase::allocationInfo() const noexcept
{
	return m_impl->m_allocationInfo;
}

VkImageView& _VMAImageBase::view() const noexcept
{
	return m_impl->m_view;
}

// ------------------------------------------------------------------------------------------------
// Image.
// ------------------------------------------------------------------------------------------------

_VMAImage::_VMAImage(const VulkanDevice& device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format)
	: _VMAImage(device, image, elements, extent, format, nullptr, nullptr)
{
}

_VMAImage::_VMAImage(const VulkanDevice& device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator allocator, VmaAllocation allocation) :
    _VMAImageBase(device, image, allocator, allocation), Image(elements, ::getSize(format) * extent.width() * extent.height(), extent, format)
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

	if (!::hasDepth(format))
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	// NOTE: This is not allowed - depending on what's read/written, a separate image view should be created.
	//if (::hasStencil(format))
	//	createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (::vkCreateImageView(device.handle(), &createInfo, nullptr, &this->view()) != VK_SUCCESS)
		throw std::runtime_error("Unable to create image view!");
}

_VMAImage::~_VMAImage() noexcept
{
	::vkDestroyImageView(this->getDevice()->handle(), this->view(), nullptr);

	if (this->allocator() != nullptr && this->allocationInfo() != nullptr)
	{
		::vmaDestroyImage(this->allocator(), this->handle(), this->allocationInfo());
		LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(this->handle()));
	}
}

const VkImageView& _VMAImage::getImageView() const noexcept
{
	return this->view();
}

UniquePtr<IImage> _VMAImage::allocate(const VulkanDevice& device, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    // Allocate the buffer.
    VkImage image;
    VmaAllocation allocation;

    if (::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate texture.");
    
    LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4} }}", fmt::ptr(image), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format);

    return makeUnique<_VMAImage>(device, image, elements, extent, format, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Texture shared implementation.
// ------------------------------------------------------------------------------------------------

class _VMATexture::_VMATextureImpl : public Implement<_VMATexture> {
public:
	friend class _VMATexture;

private:
	VkImageLayout m_imageLayout;
	const VulkanDescriptorLayout* m_descriptorLayout;

public:
	_VMATextureImpl(_VMATexture* parent, VkImageLayout imageLayout, const VulkanDescriptorLayout* descriptorLayout) :
		base(parent), m_imageLayout(imageLayout), m_descriptorLayout(descriptorLayout)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Texture shared interface.
// ------------------------------------------------------------------------------------------------

_VMATexture::_VMATexture(const VulkanDevice& device, const VulkanDescriptorLayout* layout, VkImage image, const VkImageLayout& imageLayout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator allocator, VmaAllocation allocation) :
	_VMAImageBase(device, image, allocator, allocation), Texture(layout, elements, ::getSize(format)* extent.width()* extent.height(), extent, format, levels, samples), m_impl(makePimpl<_VMATextureImpl>(this, imageLayout, layout))
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

	if (!::hasDepth(format))
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	// NOTE: This is not allowed - depending on what's read/written, a separate image view should be created.
	//if (::hasStencil(format))
	//	createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	createInfo.subresourceRange.baseMipLevel = 0;
	//createInfo.subresourceRange.levelCount = levels;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (::vkCreateImageView(device.handle(), &createInfo, nullptr, &this->view()) != VK_SUCCESS)
		throw std::runtime_error("Unable to create image view!");
}

_VMATexture::~_VMATexture() noexcept
{
	::vkDestroyImageView(this->getDevice()->handle(), this->view(), nullptr);
	::vmaDestroyImage(this->allocator(), this->handle(), this->allocationInfo());
	LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(this->handle()));
}

const VkImageView& _VMATexture::getImageView() const noexcept
{
	return this->view();
}

void _VMATexture::transferFrom(const ICommandBuffer* commandBuffer, IBuffer* source, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset)
{
	auto transferBuffer = dynamic_cast<const VulkanCommandBuffer*>(commandBuffer);
	auto sourceBuffer = dynamic_cast<const IResource<VkBuffer>*>(source);

	if (sourceBuffer == nullptr)
		throw std::invalid_argument("The transfer source buffer must be initialized and a valid Vulkan buffer.");

	if (transferBuffer == nullptr)
		throw std::invalid_argument("The command buffer must be initialized and a valid Vulkan command buffer.");

	// First, transition the image into a fitting layout, so that it can receive transfers.
	VkImageMemoryBarrier beginTransitionBarrier = {};
	beginTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	beginTransitionBarrier.oldLayout = m_impl->m_imageLayout;
	beginTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	beginTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	beginTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	beginTransitionBarrier.image = this->handle();
	beginTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	beginTransitionBarrier.subresourceRange.baseMipLevel = 0;
	//beginTransitionBarrier.subresourceRange.levelCount = this->getLevels();
	beginTransitionBarrier.subresourceRange.levelCount = 1;
	beginTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	beginTransitionBarrier.subresourceRange.layerCount = 1;
	beginTransitionBarrier.srcAccessMask = 0;
	beginTransitionBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	::vkCmdPipelineBarrier(transferBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

	// Create a copy command and add it to the command buffer.
	VkBufferImageCopy copyInfo{};
	copyInfo.bufferOffset = sourceOffset;

	// TODO: Support padded buffer formats.
	copyInfo.bufferRowLength = 0;
	copyInfo.bufferImageHeight = 0;

	copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.imageSubresource.mipLevel = 0;
	copyInfo.imageSubresource.baseArrayLayer = 0;
	copyInfo.imageSubresource.layerCount = 1;

	copyInfo.imageOffset = { 0, 0, 0 };
	copyInfo.imageExtent = { static_cast<UInt32>(this->getExtent().width()), static_cast<UInt32>(this->getExtent().height()), 1 };

	::vkCmdCopyBufferToImage(transferBuffer->handle(), sourceBuffer->handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

	// Last, transition the image back into the required layout for rendering.
	VkImageMemoryBarrier endTransitionBarrier = {};
	endTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	endTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	endTransitionBarrier.newLayout = m_impl->m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	endTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	endTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	endTransitionBarrier.image = this->handle();
	endTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	endTransitionBarrier.subresourceRange.baseMipLevel = 0;
	//endTransitionBarrier.subresourceRange.levelCount = this->getLevels();
	endTransitionBarrier.subresourceRange.levelCount = 1;
	endTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	endTransitionBarrier.subresourceRange.layerCount = 1;
	endTransitionBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	endTransitionBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkPipelineStageFlags targetStages = {};
	auto shaderStages = m_impl->m_descriptorLayout->parent().getShaderStages();

	if ((shaderStages & ShaderStage::Vertex) == ShaderStage::Vertex)
		targetStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	if ((shaderStages & ShaderStage::Geometry) == ShaderStage::Geometry)
		targetStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	if ((shaderStages & ShaderStage::Fragment) == ShaderStage::Fragment)
		targetStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if ((shaderStages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
		targetStages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	if ((shaderStages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
		targetStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	if ((shaderStages & ShaderStage::Compute) == ShaderStage::Compute)
		targetStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	::vkCmdPipelineBarrier(transferBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);
}

void _VMATexture::transferTo(const ICommandBuffer* commandBuffer, IBuffer* target, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset) const
{
	auto transferBuffer = dynamic_cast<const VulkanCommandBuffer*>(commandBuffer);
	auto targetBuffer = dynamic_cast<const IResource<VkBuffer>*>(target);
	auto layout = dynamic_cast<const VulkanDescriptorLayout*>(this->getLayout());
	assert(layout != nullptr);

	if (targetBuffer == nullptr)
		throw std::invalid_argument("The transfer target buffer must be initialized and a valid Vulkan buffer.");

	if (transferBuffer == nullptr)
		throw std::invalid_argument("The command buffer must be initialized and a valid Vulkan command buffer.");

	// First, transition the image into a fitting layout, so that it can receive transfers.
	VkImageMemoryBarrier beginTransitionBarrier = {};
	beginTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	beginTransitionBarrier.oldLayout = m_impl->m_imageLayout;
	beginTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	beginTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	beginTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	beginTransitionBarrier.image = this->handle();
	beginTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	beginTransitionBarrier.subresourceRange.baseMipLevel = 0;
	//beginTransitionBarrier.subresourceRange.levelCount = this->getLevels();
	beginTransitionBarrier.subresourceRange.levelCount = 1;
	beginTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	beginTransitionBarrier.subresourceRange.layerCount = 1;
	beginTransitionBarrier.srcAccessMask = 0;
	beginTransitionBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	::vkCmdPipelineBarrier(transferBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

	// Create a copy command and add it to the command buffer.
	VkBufferImageCopy copyInfo{};
	copyInfo.bufferOffset = 0;
	copyInfo.bufferRowLength = 0;
	copyInfo.bufferImageHeight = 0;

	copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.imageSubresource.mipLevel = 0;
	copyInfo.imageSubresource.baseArrayLayer = 0;
	copyInfo.imageSubresource.layerCount = 1;

	copyInfo.imageOffset = { 0, 0, 0 };
	copyInfo.imageExtent = { static_cast<UInt32>(this->getExtent().width()), static_cast<UInt32>(this->getExtent().height()), 1 };

	::vkCmdCopyImageToBuffer(transferBuffer->handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetBuffer->handle(), 1, &copyInfo);

	// Last, transition the image back into the required layout for rendering.
	VkImageMemoryBarrier endTransitionBarrier = {};
	endTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	endTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	endTransitionBarrier.newLayout = m_impl->m_imageLayout;
	endTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	endTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	endTransitionBarrier.image = this->handle();
	endTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	endTransitionBarrier.subresourceRange.baseMipLevel = 0;
	//endTransitionBarrier.subresourceRange.levelCount = this->getLevels();
	endTransitionBarrier.subresourceRange.levelCount = 1;
	endTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	endTransitionBarrier.subresourceRange.layerCount = 1;
	endTransitionBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	endTransitionBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkPipelineStageFlags targetStages = {};
	auto shaderStages = m_impl->m_descriptorLayout->parent().getShaderStages();

	if ((shaderStages & ShaderStage::Vertex) == ShaderStage::Vertex)
		targetStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	if ((shaderStages & ShaderStage::Geometry) == ShaderStage::Geometry)
		targetStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	if ((shaderStages & ShaderStage::Fragment) == ShaderStage::Fragment)
		targetStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if ((shaderStages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
		targetStages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	if ((shaderStages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
		targetStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	if ((shaderStages & ShaderStage::Compute) == ShaderStage::Compute)
		targetStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	::vkCmdPipelineBarrier(transferBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);
}

UniquePtr<ITexture> _VMATexture::allocate(const VulkanDevice& device, const VulkanDescriptorLayout* layout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	if (layout == nullptr)
		throw std::invalid_argument("The layout must be initialized.");

	// Allocate the buffer.
	VkImage image;
	VmaAllocation allocation;
	
	if (::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate texture.");

	LITEFX_DEBUG(VULKAN_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Elements: {5}, Levels: {6}, Samples: {7} }}", fmt::ptr(image), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, elements, levels, samples);

	return makeUnique<_VMATexture>(device, layout, image, createInfo.initialLayout, elements, extent, format, levels, samples, allocator, allocation);
}