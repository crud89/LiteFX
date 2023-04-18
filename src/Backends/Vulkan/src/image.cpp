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
	VmaAllocation m_allocation;
	Array<VkImageView> m_views;
	Format m_format;
	Size3d m_extent;
	UInt32 m_elements, m_layers, m_levels, m_planes;
	ImageDimensions m_dimensions;
	bool m_writable;
	Array<ImageLayout> m_layouts;
	MultiSamplingLevel m_samples;
	const VulkanDevice& m_device;

public:
	VulkanImageImpl(VulkanImage* parent, const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ImageLayout& initialLayout, VmaAllocator allocator, VmaAllocation allocation) :
		base(parent), m_device(device), m_allocator(allocator), m_allocation(allocation), m_extent(extent), m_format(format), m_dimensions(dimensions), m_levels(levels), m_layers(layers), m_writable(writable), m_samples(samples)
	{	
		VkImageViewCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = m_parent->handle(),
			.viewType = Vk::getImageViewType(dimensions),
			.format = Vk::getFormat(m_format),
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
			raiseIfFailed<RuntimeException>(::vkCreateImageView(m_device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
			m_views.push_back(imageView);
		}
		else
		{
			VkImageView imageView;

			if (::hasDepth(m_format))
			{
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				raiseIfFailed<RuntimeException>(::vkCreateImageView(m_device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
				m_views.push_back(imageView);
			}

			if (::hasStencil(m_format))
			{
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
				raiseIfFailed<RuntimeException>(::vkCreateImageView(m_device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
				m_views.push_back(imageView);
			}
		}

		// Note: Currently no multi-planar images are supported. Planes have a two-fold meaning in this context. Multi-planar images are images, which have a format with `_2PLANE` or `_3PLANE` in the name, or
		//       which are listed here: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#formats-requiring-sampler-ycbcr-conversion.
		//       More info: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VkFormatProperties (See "Multi-planar").
		//       All those formats are currently not supported. To stay in line with DX12 plane indexing, depth and stencil parts of an depth/stencil image are also separated by planes. Depending on the format,
		//       the proper aspect is selected based on the plane.
		m_planes = static_cast<UInt32>(m_views.size());
		m_elements = m_levels * m_layers * m_planes;
		m_layouts.resize(m_elements, initialLayout);
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

VulkanImage::VulkanImage(const VulkanDevice& device, VkImage image, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ImageLayout& initialLayout, VmaAllocator allocator, VmaAllocation allocation, const String& name) :
	m_impl(makePimpl<VulkanImageImpl>(this, device, extent, format, dimensions, levels, layers, samples, writable, initialLayout, allocator, allocation)), Resource<VkImage>(image)
{
	if (!name.empty())
		this->name() = name;

	if (allocation != nullptr)
	{
		// Store the image on the allocation.
		::vmaSetAllocationUserData(allocator, allocation, this);
		::vmaSetAllocationName(allocator, allocation, name.c_str());
	}
}

VulkanImage::~VulkanImage() noexcept 
{
	for (auto& view : m_impl->m_views)
		::vkDestroyImageView(m_impl->m_device.handle(), view, nullptr);

	if (m_impl->m_allocator != nullptr && m_impl->m_allocation != nullptr)
	{
		::vmaDestroyImage(m_impl->m_allocator, this->handle(), m_impl->m_allocation);
		LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(reinterpret_cast<void*>(this->handle())));
	}
}

const UInt32& VulkanImage::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t VulkanImage::size() const noexcept
{
	if (m_impl->m_allocation) [[likely]]
	{
		VmaAllocationInfo allocationInfo;
		::vmaGetAllocationInfo(m_impl->m_allocator, m_impl->m_allocation, &allocationInfo);
		return allocationInfo.size;
	}
	else
	{
		auto elementSize = ::getSize(m_impl->m_format) * m_impl->m_extent.width() * m_impl->m_extent.height() * m_impl->m_extent.depth() * m_impl->m_layers;
		auto totalSize = elementSize;

		for (int l(1); l < m_impl->m_levels; ++l)
		{
			elementSize /= 2;
			totalSize += elementSize;
		}

		return totalSize * m_impl->m_planes;
	}
}

size_t VulkanImage::elementSize() const noexcept
{
	return this->size();
}

size_t VulkanImage::elementAlignment() const noexcept
{
	// Same alignment as enforced by D3D. Actual alignment might differ when image gets allocated.
	return 256;
}

size_t VulkanImage::alignedElementSize() const noexcept
{
	// TODO: Align this by `elementAlignment`.
	return this->elementSize();
}

const bool& VulkanImage::writable() const noexcept
{
	return m_impl->m_writable;
}

const ImageLayout& VulkanImage::layout(const UInt32& subresource) const
{
	if (subresource >= m_impl->m_layouts.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_layouts[subresource];
}

ImageLayout& VulkanImage::layout(const UInt32& subresource)
{
	if (subresource >= m_impl->m_layouts.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_layouts[subresource];
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

const UInt32& VulkanImage::planes() const noexcept
{
	return m_impl->m_planes;
}

const MultiSamplingLevel& VulkanImage::samples() const noexcept
{
	return m_impl->m_samples;
}

VkImageAspectFlags VulkanImage::aspectMask() const noexcept
{
	// Get the aspect mask for all sub-resources.
	if (::hasDepth(m_impl->m_format) && ::hasStencil(m_impl->m_format))
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if (::hasDepth(m_impl->m_format))
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (::hasStencil(m_impl->m_format))
	{
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	//else if (::isMultiPlanar(m_impl->m_format))
	else if (m_impl->m_planes > 1)
	{
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;

		if (m_impl->m_planes > 1)
			aspectMask |= VK_IMAGE_ASPECT_PLANE_1_BIT;
		if (m_impl->m_planes > 2)
			aspectMask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
		if (m_impl->m_planes > 3) [[unlikely]]
			LITEFX_ERROR(VULKAN_LOG, "An image resource with a multi-planar format has {0} planes, which is not supported (maximum is {1}).", m_impl->m_planes, 3);

		return aspectMask;
	}
	else [[likely]]
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

VkImageAspectFlags VulkanImage::aspectMask(const UInt32& plane) const 
{
	if (::hasDepth(m_impl->m_format) && ::hasStencil(m_impl->m_format))
	{
		if (plane > 2) [[unlikely]]		// Should actually never happen.
			throw RuntimeException("An image resource with a depth/stencil format has more than two planes, which is not supported.");

		return plane == 1 ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (::hasDepth(m_impl->m_format))
	{
		if (plane > 1) [[unlikely]]		// Should actually never happen.
			throw RuntimeException("An image resource with a depth-only format has more than one planes, which is not supported.");

		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (::hasStencil(m_impl->m_format))
	{
		if (plane > 1) [[unlikely]]		// Should actually never happen.
			throw RuntimeException("An image resource with a stencil-only format has more than one planes, which is not supported.");

		return VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if (m_impl->m_planes > 1)
	{
		if (plane == 0)
			return VK_IMAGE_ASPECT_PLANE_0_BIT;
		else if (plane == 1)
			return VK_IMAGE_ASPECT_PLANE_1_BIT;
		else if (plane == 2)
			return VK_IMAGE_ASPECT_PLANE_2_BIT;
		else [[unlikely]]		// Should actually never happen.
			throw RuntimeException("An image resource with a multi-planar format has more than three planes, which is not supported.");
	}
	else [[likely]]
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
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
	return m_impl->m_allocation;
}

VkImageView& VulkanImage::imageView(const UInt32& plane)
{
	if (plane >= m_impl->m_views.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("The image does not have a plane {0}.", plane);

	return m_impl->m_views[plane];
}

UniquePtr<VulkanImage> VulkanImage::allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ImageLayout& initialLayout, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanImage::allocate("", device, extent, format, dimensions, levels, layers, samples, writable, initialLayout, allocator, createInfo, allocationInfo, allocationResult);
}

UniquePtr<VulkanImage> VulkanImage::allocate(const String& name, const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ImageLayout& initialLayout, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkImage image;
	VmaAllocation allocation;

	raiseIfFailed<RuntimeException>(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Writable: {7} }}", name.empty() ? fmt::to_string(fmt::ptr(reinterpret_cast<void*>(image))) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, writable, samples);

	return makeUnique<VulkanImage>(device, image, extent, format, dimensions, levels, layers, samples, writable, initialLayout, allocator, allocation, name);
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
	const VulkanDevice& m_device;

public:
	VulkanSamplerImpl(VulkanSampler* parent, const VulkanDevice& device, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_device(device), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
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
		raiseIfFailed<RuntimeException>(::vkCreateSampler(m_device.handle(), &samplerInfo, nullptr, &sampler), "Unable to create sampler.");

		return sampler;
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSampler::VulkanSampler(const VulkanDevice& device, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy, const String& name) :
	Resource<VkSampler>(VK_NULL_HANDLE), m_impl(makePimpl<VulkanSamplerImpl>(this, device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
{
	this->handle() = m_impl->initialize();

	if (!name.empty())
		this->name() = name;
}

VulkanSampler::~VulkanSampler() noexcept
{
	::vkDestroySampler(m_impl->m_device.handle(), this->handle(), nullptr);
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