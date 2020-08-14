#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image Base.
// ------------------------------------------------------------------------------------------------

_VMAImageBase::_VMAImageBase(const VulkanDevice* device, VkImage image, VmaAllocator allocator, VmaAllocation allocation) :
	VulkanRuntimeObject(device), IResource(image), m_allocator(allocator), m_allocationInfo(allocation)
{
}

_VMAImageBase::~_VMAImageBase() noexcept 
{
}

// ------------------------------------------------------------------------------------------------
// Image.
// ------------------------------------------------------------------------------------------------

_VMAImage::_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format)
	: _VMAImage(device, image, elements, extent, format, nullptr, nullptr)
{
}

_VMAImage::_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator allocator, VmaAllocation allocation) :
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

	if (::vkCreateImageView(this->getDevice()->handle(), &createInfo, nullptr, &m_view) != VK_SUCCESS)
		throw std::runtime_error("Unable to create image view!");
}

_VMAImage::~_VMAImage() noexcept
{
	::vkDestroyImageView(this->getDevice()->handle(), m_view, nullptr);

	if (m_allocator != nullptr && m_allocationInfo != nullptr)
	{
		::vmaDestroyImage(m_allocator, this->handle(), m_allocationInfo);
		LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(this->handle()));
	}
}

const VkImageView& _VMAImage::getImageView() const noexcept
{
	return m_view;
}

UniquePtr<IImage> _VMAImage::allocate(const VulkanDevice* device, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
    if (device == nullptr)
        throw std::invalid_argument("The device must be initialized.");

    // Allocate the buffer.
    VkImage image;
    VmaAllocation allocation;

    if (::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate texture.");
    
    LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4} }}", fmt::ptr(image), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format);

    return makeUnique<_VMAImage>(device, image, elements, extent, format, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Image.
// ------------------------------------------------------------------------------------------------

_VMATexture::_VMATexture(const VulkanDevice* device, const IDescriptorLayout* layout, VkImage image, const VkImageLayout& imageLayout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator allocator, VmaAllocation allocation) :
	_VMAImageBase(device, image, allocator, allocation), Texture(layout, elements, ::getSize(format)* extent.width()* extent.height(), extent, format, levels, samples), m_imageLayout(imageLayout)
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

	if (::vkCreateImageView(this->getDevice()->handle(), &createInfo, nullptr, &m_view) != VK_SUCCESS)
		throw std::runtime_error("Unable to create image view!");
}

_VMATexture::~_VMATexture() noexcept
{
	::vkDestroyImageView(this->getDevice()->handle(), m_view, nullptr);
	::vmaDestroyImage(m_allocator, this->handle(), m_allocationInfo);
	LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(this->handle()));
}

const VkImageView& _VMATexture::getImageView() const noexcept
{
	return m_view;
}

void _VMATexture::transferFrom(const ICommandQueue* commandQueue, IBuffer* source, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset)
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

	// First, transition the image into a fitting layout, so that it can receive transfers.
	VkImageMemoryBarrier beginTransitionBarrier = {};
	beginTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	beginTransitionBarrier.oldLayout = m_imageLayout;
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

	::vkCmdPipelineBarrier(commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

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

	::vkCmdCopyBufferToImage(commandBuffer->handle(), sourceBuffer->handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

	// Last, transition the image back into the required layout for rendering.
	VkImageMemoryBarrier endTransitionBarrier = {};
	endTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	endTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	endTransitionBarrier.newLayout = m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
	auto shaderStages = this->getLayout()->getDescriptorSet()->getShaderStages();

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

	::vkCmdPipelineBarrier(commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);
	
	// End the transfer recording and submit the buffer.
	commandBuffer->end();
	commandBuffer->submit(true);
}

void _VMATexture::transferTo(const ICommandQueue* commandQueue, IBuffer* target, const size_t& size, const size_t& sourceOffset, const size_t& targetOffset) const
{
	auto transferQueue = dynamic_cast<const VulkanQueue*>(commandQueue);
	auto targetBuffer = dynamic_cast<const IResource<VkBuffer>*>(target);

	if (targetBuffer == nullptr)
		throw std::invalid_argument("The transfer target buffer must be initialized and a valid Vulkan buffer.");

	if (transferQueue == nullptr)
		throw std::invalid_argument("The transfer queue must be initialized and a valid Vulkan command queue.");

	auto device = dynamic_cast<const VulkanDevice*>(transferQueue->getDevice());

	if (device == nullptr)
		throw std::runtime_error("The transfer queue must be bound to a valid Vulkan device.");

	auto commandBuffer = makeUnique<const VulkanCommandBuffer>(transferQueue);

	// Begin the transfer recording.
	commandBuffer->begin();

	// First, transition the image into a fitting layout, so that it can receive transfers.
	VkImageMemoryBarrier beginTransitionBarrier = {};
	beginTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	beginTransitionBarrier.oldLayout = m_imageLayout;
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

	::vkCmdPipelineBarrier(commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

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

	::vkCmdCopyImageToBuffer(commandBuffer->handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetBuffer->handle(), 1, &copyInfo);

	// Last, transition the image back into the required layout for rendering.
	VkImageMemoryBarrier endTransitionBarrier = {};
	endTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	endTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	endTransitionBarrier.newLayout = m_imageLayout;
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
	auto shaderStages = this->getLayout()->getDescriptorSet()->getShaderStages();

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

	::vkCmdPipelineBarrier(commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);

	// End the transfer recording and submit the buffer.
	commandBuffer->end();
	commandBuffer->submit(true);
}

UniquePtr<ITexture> _VMATexture::allocate(const VulkanDevice* device, const IDescriptorLayout* layout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	if (device == nullptr)
		throw std::invalid_argument("The device must be initialized.");

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