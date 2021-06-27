#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class VulkanImage::VulkanImageImpl : public Implement<VulkanImage> {
public:
	friend class VulkanImage;

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocationInfo;
	VkImageView m_view;
	Format m_format;
	Size2d m_extent;
	UInt32 m_elements{ 1 };
	ImageDimensions m_dimensions;

public:
	VulkanImageImpl(VulkanImage* parent, const Size2d& extent, const Format& format, const ImageDimensions& dimensions, VmaAllocator allocator, VmaAllocation allocation) :
		base(parent), m_allocator(allocator), m_allocationInfo(allocation), m_extent(extent), m_format(format), m_dimensions(dimensions)
	{
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_parent->handle();
		createInfo.viewType = ::getImageViewType(dimensions);
		createInfo.format = ::getFormat(m_format);
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (!::hasDepth(m_format))
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		else
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		// NOTE: This is not allowed - depending on what's read/written, a separate image view should be created.
		//if (::hasStencil(format))
		//	createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		raiseIfFailed<RuntimeException>(::vkCreateImageView(m_parent->getDevice()->handle(), &createInfo, nullptr, &m_view), "Unable to create image view.");
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

VulkanImage::VulkanImage(const VulkanDevice& device, VkImage image, const Size2d& extent, const Format& format, const ImageDimensions& dimensions, VmaAllocator allocator, VmaAllocation allocation) :
	m_impl(makePimpl<VulkanImageImpl>(this, extent, format, dimensions, allocator, allocation)), VulkanRuntimeObject<VulkanDevice>(device, &device), Resource<VkImage>(image)
{
}

VulkanImage::~VulkanImage() noexcept 
{
	::vkDestroyImageView(this->getDevice()->handle(), m_impl->m_view, nullptr);

	if (m_impl->m_allocator != nullptr && m_impl->m_allocationInfo != nullptr)
	{
		::vmaDestroyImage(m_impl->m_allocator, this->handle(), m_impl->m_allocationInfo);
		LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(reinterpret_cast<void*>(this->handle())));
	}
}

const UInt32& VulkanImage::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t VulkanImage::size() const noexcept
{
	return this->elementSize() * m_impl->m_elements;
}

size_t VulkanImage::elementSize() const noexcept
{
	// Rough estimation, that does not include alignment.
	return ::getSize(m_impl->m_format) * m_impl->m_extent.width() * m_impl->m_extent.height();
}

size_t VulkanImage::elementAlignment() const noexcept
{
	return 0;
}

size_t VulkanImage::alignedElementSize() const noexcept
{
	return this->elementSize();
}

const Size2d& VulkanImage::extent() const noexcept
{
	return m_impl->m_extent;
}

const Format& VulkanImage::format() const noexcept
{
	return m_impl->m_format;
}

const ImageDimensions& VulkanImage::dimensions() const noexcept
{
	return m_impl->m_dimensions;
}

const VkImageView& VulkanImage::imageView() const noexcept
{
	return m_impl->m_view;
}

VmaAllocator& VulkanImage::allocator() const noexcept
{
	return m_impl->m_allocator;
}

VmaAllocation& VulkanImage::allocationInfo() const noexcept
{
	return m_impl->m_allocationInfo;
}

VkImageView& VulkanImage::imageView() noexcept
{
	return m_impl->m_view;
}

UniquePtr<VulkanImage> VulkanImage::allocate(const VulkanDevice& device, const Size2d& extent, const Format& format, const ImageDimensions& dimensions, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkImage image;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4} }}", fmt::ptr(reinterpret_cast<void*>(image)), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format);

	return makeUnique<VulkanImage>(device, image, extent, format, dimensions, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Texture shared implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl : public Implement<VulkanTexture> {
public:
	friend class VulkanTexture;

private:
	const VulkanDescriptorLayout& m_descriptorLayout;
	VkImageLayout m_imageLayout;
	MultiSamplingLevel m_samples;
	UInt32 m_levels;

public:
	VulkanTextureImpl(VulkanTexture* parent, VkImageLayout imageLayout, const VulkanDescriptorLayout& descriptorLayout, const UInt32& levels, const MultiSamplingLevel& samples) :
		base(parent), m_imageLayout(imageLayout), m_descriptorLayout(descriptorLayout), m_samples(samples), m_levels(levels)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Texture shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture(const VulkanDevice& device, const VulkanDescriptorLayout& layout, VkImage image, const VkImageLayout& imageLayout, const Size2d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator allocator, VmaAllocation allocation) :
	VulkanImage(device, image, extent, format, dimensions, allocator, allocation), m_impl(makePimpl<VulkanTextureImpl>(this, imageLayout, layout, levels, samples))
{
}

VulkanTexture::~VulkanTexture() noexcept = default;

const UInt32& VulkanTexture::binding() const noexcept
{
	return m_impl->m_descriptorLayout.binding();
}

const VulkanDescriptorLayout& VulkanTexture::layout() const noexcept
{
	return m_impl->m_descriptorLayout;
}

const MultiSamplingLevel& VulkanTexture::samples() const noexcept
{
	return m_impl->m_samples;
}

const UInt32& VulkanTexture::levels() const noexcept
{
	return m_impl->m_levels;
}

const VkImageLayout& VulkanTexture::imageLayout() const noexcept
{
	return m_impl->m_imageLayout;
}

void VulkanTexture::transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (elements != 1 || targetElement != 0)
		throw ArgumentOutOfRangeException("Textures currently do not support transforms for more than one element. The target element is required to be 0 (is {0}) and the number of elements is required to be 1 (is {1}).", targetElement, elements);

	if (source.elements() < sourceElement + elements)
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

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
	//beginTransitionBarrier.subresourceRange.levelCount = m_impl->m_levels;
	beginTransitionBarrier.subresourceRange.levelCount = 1;
	beginTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	beginTransitionBarrier.subresourceRange.layerCount = 1;
	beginTransitionBarrier.srcAccessMask = 0;
	beginTransitionBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

	// Create a copy command and add it to the command buffer.
	VkBufferImageCopy copyInfo{};

	// TODO: Support padded buffer formats.
	copyInfo.bufferOffset = source.alignedElementSize() * sourceElement;
	copyInfo.bufferRowLength = 0;
	copyInfo.bufferImageHeight = 0;

	copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.imageSubresource.mipLevel = 0;
	copyInfo.imageSubresource.baseArrayLayer = 0;
	copyInfo.imageSubresource.layerCount = 1;

	copyInfo.imageOffset = { 0, 0, 0 };
	copyInfo.imageExtent = { static_cast<UInt32>(this->extent().width()), static_cast<UInt32>(this->extent().height()), 1 };

	::vkCmdCopyBufferToImage(commandBuffer.handle(), source.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

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
	auto shaderStages = m_impl->m_descriptorLayout.parent().shaderStages();

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

	::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);
}

void VulkanTexture::transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (elements != 1 || sourceElement != 0)
		throw ArgumentOutOfRangeException("Textures currently do not support transforms for more than one element. The source element is required to be 0 (is {0}) and the number of elements is required to be 1 (is {1}).", targetElement, elements);

	if (target.elements() <= targetElement + elements)
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

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

	::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &beginTransitionBarrier);

	// Create a copy command and add it to the command buffer.
	VkBufferImageCopy copyInfo{};

	// TODO: Support padded buffer formats.
	copyInfo.bufferOffset = target.alignedElementSize() * targetElement;
	copyInfo.bufferRowLength = 0;
	copyInfo.bufferImageHeight = 0;

	copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.imageSubresource.mipLevel = 0;
	copyInfo.imageSubresource.baseArrayLayer = 0;
	copyInfo.imageSubresource.layerCount = 1;

	copyInfo.imageOffset = { 0, 0, 0 };
	copyInfo.imageExtent = { static_cast<UInt32>(this->extent().width()), static_cast<UInt32>(this->extent().height()), 1 };

	::vkCmdCopyImageToBuffer(commandBuffer.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, target.handle(), 1, &copyInfo);

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
	auto shaderStages = m_impl->m_descriptorLayout.parent().shaderStages();

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

	::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, targetStages, 0, 0, nullptr, 0, nullptr, 1, &endTransitionBarrier);
}

UniquePtr<VulkanTexture> VulkanTexture::allocate(const VulkanDevice& device, const VulkanDescriptorLayout& layout, const Size2d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	// Allocate the buffer.
	VkImage image;
	VmaAllocation allocation;
	
	raiseIfFailed<RuntimeException>(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Samples: {6} }}", fmt::ptr(reinterpret_cast<void*>(image)), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, samples);

	return makeUnique<VulkanTexture>(device, layout, image, createInfo.initialLayout, extent, format, dimensions, levels, samples, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSampler::VulkanSamplerImpl : public Implement<VulkanSampler> {
public:
	friend class VulkanSampler;

private:
	const VulkanDescriptorLayout& m_layout;
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;

public:
	VulkanSamplerImpl(VulkanSampler* parent, const VulkanDescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_layout(layout), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
	{
	}

private:
	VkFilter getFilterMode(const FilterMode& mode)
	{
		switch (mode)
		{
		case FilterMode::Linear: return VK_FILTER_LINEAR;
		case FilterMode::Nearest: return VK_FILTER_NEAREST;
		default: throw std::invalid_argument("Invalid filter mode.");
		}
	}

	VkSamplerMipmapMode getMipMapMode(const MipMapMode& mode)
	{
		switch (mode)
		{
		case MipMapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case MipMapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		default: throw std::invalid_argument("Invalid mip map mode.");
		}
	}

	VkSamplerAddressMode getBorderMode(const BorderMode& mode)
	{
		switch (mode)
		{
		case BorderMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case BorderMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case BorderMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case BorderMode::RepeatMirrored: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case BorderMode::ClampToEdgeMirrored: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		default: throw std::invalid_argument("Invalid border mode.");
		}
	}

public:
	VkSampler initialize()
	{
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = getFilterMode(m_magFilter);
		samplerInfo.minFilter = getFilterMode(m_minFilter);
		samplerInfo.addressModeU = getBorderMode(m_borderU);
		samplerInfo.addressModeV = getBorderMode(m_borderV);
		samplerInfo.addressModeW = getBorderMode(m_borderW);
		samplerInfo.anisotropyEnable = m_anisotropy > 0.f ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = m_anisotropy;
		samplerInfo.mipmapMode = getMipMapMode(m_mipMapMode);
		samplerInfo.mipLodBias = m_mipMapBias;
		samplerInfo.minLod = m_minLod;
		samplerInfo.maxLod = m_maxLod;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		VkSampler sampler;
		raiseIfFailed<RuntimeException>(::vkCreateSampler(m_parent->getDevice()->handle(), &samplerInfo, nullptr, &sampler), "Unable to create sampler.");

		return sampler;
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSampler::VulkanSampler(const VulkanDevice& device, const VulkanDescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	Resource<VkSampler>(VK_NULL_HANDLE), VulkanRuntimeObject<VulkanDevice>(device, &device), m_impl(makePimpl<VulkanSamplerImpl>(this, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
{
	this->handle() = m_impl->initialize();
}

VulkanSampler::~VulkanSampler() noexcept
{
	::vkDestroySampler(this->getDevice()->handle(), this->handle(), nullptr);
}

const FilterMode& VulkanSampler::getMinifyingFilter() const noexcept
{
	return m_impl->m_minFilter;
}

const FilterMode& VulkanSampler::getMagnifyingFilter() const noexcept
{
	return m_impl->m_magFilter;
}

const BorderMode& VulkanSampler::getBorderModeU() const noexcept
{
	return m_impl->m_borderU;
}

const BorderMode& VulkanSampler::getBorderModeV() const noexcept
{
	return m_impl->m_borderV;
}

const BorderMode& VulkanSampler::getBorderModeW() const noexcept
{
	return m_impl->m_borderW;
}

const Float& VulkanSampler::getAnisotropy() const noexcept
{
	return m_impl->m_anisotropy;
}

const MipMapMode& VulkanSampler::getMipMapMode() const noexcept
{
	return m_impl->m_mipMapMode;
}

const Float& VulkanSampler::getMipMapBias() const noexcept
{
	return m_impl->m_mipMapBias;
}

const Float& VulkanSampler::getMaxLOD() const noexcept
{
	return m_impl->m_maxLod;
}

const Float& VulkanSampler::getMinLOD() const noexcept
{
	return m_impl->m_minLod;
}

const UInt32& VulkanSampler::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

const VulkanDescriptorLayout& VulkanSampler::layout() const noexcept
{
	return m_impl->m_layout;
}