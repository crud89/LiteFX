#include "image.h"

using namespace LiteFX::Rendering::Backends;

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class VulkanImage::VulkanImageImpl : public Implement<VulkanImage> {
public:
	friend class VulkanImage;

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocationInfo;
	Array<VkImageView> m_views;
	Format m_format;
	Size3d m_extent;
	UInt32 m_elements{ 1 }, m_layers, m_levels;
	ImageDimensions m_dimensions;
	bool m_writable;

public:
	VulkanImageImpl(VulkanImage* parent, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const bool& writable, VmaAllocator allocator, VmaAllocation allocation) :
		base(parent), m_allocator(allocator), m_allocationInfo(allocation), m_extent(extent), m_format(format), m_dimensions(dimensions), m_levels(levels), m_layers(layers), m_writable(writable)
	{
		VkImageViewCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = m_parent->handle(),
			.viewType = ::getImageViewType(dimensions),
			.format = ::getFormat(m_format),
			.components = VkComponentMapping {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = VkImageSubresourceRange {
				.baseMipLevel = 0,
				.levelCount = m_levels,
				.baseArrayLayer = 0,
				.layerCount = m_layers
			}
		};

		if (!::hasDepth(m_format) && !::hasStencil(m_format))
		{
			VkImageView imageView;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			raiseIfFailed<RuntimeException>(::vkCreateImageView(m_parent->getDevice()->handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
			m_views.push_back(imageView);
		}
		else
		{
			VkImageView imageView;

			if (::hasDepth(m_format))
			{
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				raiseIfFailed<RuntimeException>(::vkCreateImageView(m_parent->getDevice()->handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
				m_views.push_back(imageView);
			}

			if (::hasStencil(m_format))
			{
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
				raiseIfFailed<RuntimeException>(::vkCreateImageView(m_parent->getDevice()->handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
				m_views.push_back(imageView);
			}
		}
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

VulkanImage::VulkanImage(const VulkanDevice& device, VkImage image, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const bool& writable, VmaAllocator allocator, VmaAllocation allocation) :
	m_impl(makePimpl<VulkanImageImpl>(this, extent, format, dimensions, levels, layers, writable, allocator, allocation)), VulkanRuntimeObject<VulkanDevice>(device, &device), Resource<VkImage>(image)
{
}

VulkanImage::~VulkanImage() noexcept 
{
	for (auto& view : m_impl->m_views)
		::vkDestroyImageView(this->getDevice()->handle(), view, nullptr);

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
	if (m_impl->m_allocationInfo) [[likely]]
		return m_impl->m_allocationInfo->GetSize();
	else
	{
		auto elementSize = ::getSize(m_impl->m_format) * m_impl->m_extent.width() * m_impl->m_extent.height() * m_impl->m_extent.depth() * m_impl->m_layers;
		auto totalSize = elementSize;

		for (int l(1); l < m_impl->m_levels; ++l)
		{
			elementSize /= 2;
			totalSize += elementSize;
		}

		return totalSize * m_impl->m_elements;
	}
}

size_t VulkanImage::elementSize() const noexcept
{
	return this->size() / m_impl->m_elements;
}

size_t VulkanImage::elementAlignment() const noexcept
{
	if (m_impl->m_allocationInfo) [[likely]]
		return m_impl->m_allocationInfo->GetAlignment();
	else
		return 0;	// Not sure about the alignment. Probably need to query from device limits.
}

size_t VulkanImage::alignedElementSize() const noexcept
{
	return this->elementSize();
}

const bool& VulkanImage::writable() const noexcept
{
	return m_impl->m_writable;
}

size_t VulkanImage::size(const UInt32& level) const noexcept
{
	if (level >= m_impl->m_levels)
		return 0;

	auto size = this->extent(level);

	switch (this->dimensions())
	{
	case ImageDimensions::DIM_1: return ::getSize(this->format()) * size.width();
	case ImageDimensions::CUBE:
	case ImageDimensions::DIM_2: return ::getSize(this->format()) * size.width() * size.height();
	default:
	case ImageDimensions::DIM_3: return ::getSize(this->format()) * size.width() * size.height() * size.depth();
	}
}

Size3d VulkanImage::extent(const UInt32& level) const noexcept
{
	if (level >= m_impl->m_levels)
		return Size3d{ 0, 0, 0 };

	Size3d size = m_impl->m_extent;
	
	for (size_t l(0); l < level; ++l)
		size /= 2;

	size.width()  = std::max<size_t>(size.width(),  1);
	size.height() = std::max<size_t>(size.height(), 1);
	size.depth()  = std::max<size_t>(size.depth(),  1);

	return size;
}

const Format& VulkanImage::format() const noexcept
{
	return m_impl->m_format;
}

const ImageDimensions& VulkanImage::dimensions() const noexcept
{
	return m_impl->m_dimensions;
}

const UInt32& VulkanImage::levels() const noexcept
{
	return m_impl->m_levels;
}

const UInt32& VulkanImage::layers() const noexcept
{
	return m_impl->m_layers;
}

const VkImageView& VulkanImage::imageView(const UInt32& plane) const
{
	if (plane >= m_impl->m_views.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The image does not have a plane {0}.", plane);

	return m_impl->m_views[plane];
}

VmaAllocator& VulkanImage::allocator() const noexcept
{
	return m_impl->m_allocator;
}

VmaAllocation& VulkanImage::allocationInfo() const noexcept
{
	return m_impl->m_allocationInfo;
}

VkImageView& VulkanImage::imageView(const UInt32& plane)
{
	if (plane >= m_impl->m_views.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The image does not have a plane {0}.", plane);

	return m_impl->m_views[plane];
}

UniquePtr<VulkanImage> VulkanImage::allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const bool& writable, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkImage image;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Writable: {7} }}", fmt::ptr(reinterpret_cast<void*>(image)), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, writable);

	return makeUnique<VulkanImage>(device, image, extent, format, dimensions, levels, layers, writable, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Texture shared implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl : public Implement<VulkanTexture> {
public:
	friend class VulkanTexture;

private:
	VkImageLayout m_imageLayout;
	MultiSamplingLevel m_samples;

public:
	VulkanTextureImpl(VulkanTexture* parent, VkImageLayout imageLayout, const MultiSamplingLevel& samples) :
		base(parent), m_imageLayout(imageLayout), m_samples(samples)
	{
	}

private:
	VkImageAspectFlags getAspectMask(const UInt32& plane = std::numeric_limits<UInt32>::max()) const
	{
		if (plane == std::numeric_limits<UInt32>::max())
		{
			if (::hasDepth(m_parent->format()) && ::hasStencil(m_parent->format()))
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			else [[likely]]
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else
		{
			if (::hasDepth(m_parent->format()) && plane == 0)
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			else if (::hasStencil(m_parent->format()) && plane == 1)
				return VK_IMAGE_ASPECT_STENCIL_BIT;
			else if (plane > 0)
				throw InvalidArgumentException("Unsupported plane {0} for format {1}.", plane, m_parent->format());
			else [[likely]]
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
};

// ------------------------------------------------------------------------------------------------
// Texture shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture(const VulkanDevice& device, VkImage image, const VkImageLayout& imageLayout, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, VmaAllocator allocator, VmaAllocation allocation) :
	VulkanImage(device, image, extent, format, dimensions, levels, layers, writable, allocator, allocation), m_impl(makePimpl<VulkanTextureImpl>(this, imageLayout, samples))
{
}

VulkanTexture::~VulkanTexture() noexcept = default;

const MultiSamplingLevel& VulkanTexture::samples() const noexcept
{
	return m_impl->m_samples;
}

const VkImageLayout& VulkanTexture::imageLayout() const noexcept
{
	return m_impl->m_imageLayout;
}

void VulkanTexture::generateMipMaps(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	VkImageMemoryBarrier barrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = this->handle(),
		.subresourceRange = VkImageSubresourceRange { .aspectMask = m_impl->getAspectMask() }
	};

	auto layout = m_impl->m_imageLayout;

	for (UInt32 layer(0); layer < this->layers(); ++layer)
	{
		Int32 mipWidth = static_cast<Int32>(this->extent().width());
		Int32 mipHeight = static_cast<Int32>(this->extent().height());
		Int32 mipDepth = static_cast<Int32>(this->extent().depth());

		for (UInt32 level(1); level < this->levels(); ++level)
		{
			// Transition the previous level to transfer source.
			barrier.subresourceRange.aspectMask = m_impl->getAspectMask();
			barrier.subresourceRange.baseArrayLayer = layer;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.baseMipLevel = level - 1;
			barrier.subresourceRange.levelCount = 1;
			barrier.oldLayout = m_impl->m_imageLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Blit the image of the previous level into the current level.
			VkImageBlit blit {
				.srcSubresource = VkImageSubresourceLayers {
					.aspectMask = m_impl->getAspectMask(),
					.mipLevel = level - 1,
					.baseArrayLayer = layer,
					.layerCount = 1
				},
				.dstSubresource = VkImageSubresourceLayers {
					.aspectMask = m_impl->getAspectMask(),
					.mipLevel = level,
					.baseArrayLayer = layer,
					.layerCount = 1
				}
			};

			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 };

			::vkCmdBlitImage(commandBuffer.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			// Compute the new size.
			mipWidth  = std::max(mipWidth  / 2, 1);
			mipHeight = std::max(mipHeight / 2, 1);
			mipDepth  = std::max(mipDepth  / 2, 1);
		}
	}

	// Finally, transition all the levels back to the original layout.
	if (layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = m_impl->getAspectMask();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = this->layers();
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = this->levels();
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}

void VulkanTexture::receiveData(const VulkanCommandBuffer& commandBuffer, const bool& receive) const noexcept
{
	if ((receive && m_impl->m_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) || (!receive && m_impl->m_imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
		return;

	if (receive)
	{
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = m_impl->m_imageLayout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = this->handle(),
			.subresourceRange = VkImageSubresourceRange {
				.aspectMask = m_impl->getAspectMask(),
				.baseMipLevel = 0,
				.levelCount = this->levels(),
				.baseArrayLayer = 0,
				.layerCount = this->layers()
			}
		};

		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		m_impl->m_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	else
	{
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = m_impl->m_imageLayout,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = this->handle(),
			.subresourceRange = VkImageSubresourceRange {
				.aspectMask = m_impl->getAspectMask(),
				.baseMipLevel = 0,
				.levelCount = this->levels(),
				.baseArrayLayer = 0,
				.layerCount = this->layers()
			}
		};

		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		m_impl->m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}

void VulkanTexture::sendData(const VulkanCommandBuffer& commandBuffer, const bool& emit) const noexcept
{
	if ((emit && m_impl->m_imageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) || (!emit && m_impl->m_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))
		return;

	if (emit)
	{
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = m_impl->m_imageLayout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = this->handle(),
			.subresourceRange = VkImageSubresourceRange {
				.aspectMask = m_impl->getAspectMask(),
				.baseMipLevel = 0,
				.levelCount = this->levels(),
				.baseArrayLayer = 0,
				.layerCount = this->layers()
			}
		};

		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		m_impl->m_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	else
	{
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = m_impl->m_imageLayout,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = this->handle(),
			.subresourceRange = VkImageSubresourceRange {
				.aspectMask = m_impl->getAspectMask(),
				.baseMipLevel = 0,
				.levelCount = this->levels(),
				.baseArrayLayer = 0,
				.layerCount = this->layers()
			}
		};

		::vkCmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		m_impl->m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}

void VulkanTexture::transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (this->levels() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The image has only {0} mip-map levels, but a transfer for {1} levels starting from level {2} has been requested. For transfers of multiple layers or planes, use multiple transfer commands instead.", this->levels(), elements, targetElement);

	source.sendData(commandBuffer, true);
	this->receiveData(commandBuffer, true);

	// Create a copy command and add it to the command buffer.
	Array<VkBufferImageCopy> copyInfos(elements);
	std::ranges::generate(copyInfos, [&, this, i = targetElement]() mutable {
		return VkBufferImageCopy {
			.bufferOffset = source.alignedElementSize() * sourceElement,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = VkImageSubresourceLayers {
				.aspectMask = m_impl->getAspectMask(plane),
				.mipLevel = i++,
				.baseArrayLayer = layer,
				.layerCount = 1
			}, 
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { static_cast<UInt32>(this->extent().width()), static_cast<UInt32>(this->extent().height()), static_cast<UInt32>(this->extent().depth()) }
		};
	});

	::vkCmdCopyBufferToImage(commandBuffer.handle(), source.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyInfos.size(), copyInfos.data());

	if (!leaveSourceState)
		source.sendData(commandBuffer, false);

	if (!leaveTargetState)
		this->receiveData(commandBuffer, false);
}

void VulkanTexture::transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	if (target.elements() <= targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	if (this->levels() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The image has only {0} mip-map levels, but a transfer for {1} levels starting from level {2} has been requested. For transfers of multiple layers or planes, use multiple transfer commands instead.", this->levels(), elements, sourceElement);

	this->sendData(commandBuffer, true);
	target.receiveData(commandBuffer, true);

	// Create a copy command and add it to the command buffer.
	Array<VkBufferImageCopy> copyInfos(elements);
	std::ranges::generate(copyInfos, [&, this, i = sourceElement]() mutable {
		return VkBufferImageCopy{
			.bufferOffset = target.alignedElementSize() * targetElement,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = VkImageSubresourceLayers {
				.aspectMask = m_impl->getAspectMask(plane),
				.mipLevel = i++,
				.baseArrayLayer = layer,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { static_cast<UInt32>(this->extent().width()), static_cast<UInt32>(this->extent().height()), static_cast<UInt32>(this->extent().depth()) }
		};
	});

	::vkCmdCopyImageToBuffer(commandBuffer.handle(), this->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target.handle(), copyInfos.size(), copyInfos.data());

	if (!leaveSourceState)
		this->sendData(commandBuffer, false);

	if (!leaveTargetState)
		target.receiveData(commandBuffer, false);
}

UniquePtr<VulkanTexture> VulkanTexture::allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	// Allocate the buffer.
	VkImage image;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {7}, Samples: {6}, Writable: {7} }}", fmt::ptr(reinterpret_cast<void*>(image)), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, samples, layers, writable);

	return makeUnique<VulkanTexture>(device, image, createInfo.initialLayout, extent, format, dimensions, levels, layers, samples, writable, allocator, allocation);
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSampler::VulkanSamplerImpl : public Implement<VulkanSampler> {
public:
	friend class VulkanSampler;

private:
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;

public:
	VulkanSamplerImpl(VulkanSampler* parent, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
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

VulkanSampler::VulkanSampler(const VulkanDevice& device, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	Resource<VkSampler>(VK_NULL_HANDLE), VulkanRuntimeObject<VulkanDevice>(device, &device), m_impl(makePimpl<VulkanSamplerImpl>(this, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
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