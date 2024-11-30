#include "image.h"

using namespace LiteFX::Rendering::Backends;

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class VulkanImage::VulkanImageImpl {
public:
	friend class VulkanImage;

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocationInfo;
	Format m_format;
	Size3d m_extent;
	UInt32 m_elements, m_layers, m_levels, m_planes;
	ImageDimensions m_dimensions;
	ResourceUsage m_usage;
	MultiSamplingLevel m_samples;

public:
	VulkanImageImpl(Size3d extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator allocator, VmaAllocation allocation) :
		m_allocator(allocator), m_allocationInfo(allocation), m_format(format), m_extent(std::move(extent)), m_layers(layers), m_levels(levels), m_planes(::hasDepth(format) && ::hasStencil(format) ? 2 : 1), m_dimensions(dimensions), m_usage(usage), m_samples(samples)
	{
		// Note: Currently no multi-planar images are supported. Planes have a two-fold meaning in this context. Multi-planar images are images, which have a format with `_2PLANE` or `_3PLANE` in the name, or
		//       which are listed here: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#formats-requiring-sampler-ycbcr-conversion.
		//       More info: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VkFormatProperties (See "Multi-planar").
		//       All those formats are currently not supported. To stay in line with DX12 plane indexing, depth and stencil parts of an depth/stencil image are also separated by planes. Depending on the format,
		//       the proper aspect is selected based on the plane.
		m_elements = m_levels * m_layers * m_planes;
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

VulkanImage::VulkanImage(VkImage image, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator allocator, VmaAllocation allocation, const String& name) :
	Resource<VkImage>(image), m_impl(extent, format, dimensions, levels, layers, samples, usage, allocator, allocation)
{
	if (!name.empty())
		this->name() = name;
}

VulkanImage::~VulkanImage() noexcept 
{
	if (m_impl->m_allocator != nullptr && m_impl->m_allocationInfo != nullptr)
	{
		::vmaDestroyImage(m_impl->m_allocator, this->handle(), m_impl->m_allocationInfo);
		LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", static_cast<void*>(this->handle()));
	}
}

UInt32 VulkanImage::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t VulkanImage::size() const noexcept
{
	if (m_impl->m_allocationInfo) [[likely]]
		return m_impl->m_allocationInfo->GetSize();
	else
	{
		size_t pixelSize{ 0 };

		try
		{
			pixelSize = ::getSize(m_impl->m_format);
		}
		catch (const InvalidArgumentException&)
		{
			LITEFX_ERROR(VULKAN_LOG, "Unsupported pixel format detected: {}.", std::to_underlying(m_impl->m_format));
			return 0;
		}
		catch (...)
		{
			return 0;
		}

		auto elementSize = pixelSize * m_impl->m_extent.width() * m_impl->m_extent.height() * m_impl->m_extent.depth() * m_impl->m_layers;
		auto totalSize = elementSize;

		for (UInt32 l(1); l < m_impl->m_levels; ++l)
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
	if (m_impl->m_allocationInfo) [[likely]]
		return m_impl->m_allocationInfo->GetAlignment();
	else
		return 0;	// Not sure about the alignment. Probably need to query from device limits.
}

size_t VulkanImage::alignedElementSize() const noexcept
{
	// TODO: Align this by `elementAlignment`.
	return this->elementSize();
}

ResourceUsage VulkanImage::usage() const noexcept
{
	return m_impl->m_usage;
}

UInt64 VulkanImage::virtualAddress() const noexcept
{
	// NOTE: There is a vendor-specific extension to support this but for the time being, we simply warn (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetImageViewAddressNVX.html).
	LITEFX_WARNING(VULKAN_LOG, "Vulkan does not allow to query virtual addresses of images.");

	return 0ul;
}

size_t VulkanImage::size(UInt32 level) const noexcept
{
	if (level >= m_impl->m_levels)
		return 0;

	auto size = this->extent(level);
	size_t pixelSize{ 0 };

	try
	{
		pixelSize = ::getSize(m_impl->m_format);
	}
	catch (const InvalidArgumentException&)
	{
		LITEFX_ERROR(VULKAN_LOG, "Unsupported pixel format detected: {}.", std::to_underlying(m_impl->m_format));
		return 0;
	}
	catch (...)
	{
		return 0;
	}

	switch (this->dimensions())
	{
	case ImageDimensions::DIM_1: return pixelSize * size.width();
	case ImageDimensions::CUBE:
	case ImageDimensions::DIM_2: return pixelSize * size.width() * size.height();
	default:
	case ImageDimensions::DIM_3: return pixelSize * size.width() * size.height() * size.depth();
	}
}

Size3d VulkanImage::extent(UInt32 level) const noexcept
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

Format VulkanImage::format() const noexcept
{
	return m_impl->m_format;
}

ImageDimensions VulkanImage::dimensions() const noexcept
{
	return m_impl->m_dimensions;
}

UInt32 VulkanImage::levels() const noexcept
{
	return m_impl->m_levels;
}

UInt32 VulkanImage::layers() const noexcept
{
	return m_impl->m_layers;
}

UInt32 VulkanImage::planes() const noexcept
{
	return m_impl->m_planes;
}

MultiSamplingLevel VulkanImage::samples() const noexcept
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

VkImageAspectFlags VulkanImage::aspectMask(UInt32 plane) const 
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

VmaAllocator& VulkanImage::allocator() const noexcept
{
	return m_impl->m_allocator;
}

VmaAllocation& VulkanImage::allocationInfo() const noexcept
{
	return m_impl->m_allocationInfo;
}

SharedPtr<VulkanImage> VulkanImage::allocate(const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	return VulkanImage::allocate("", extent, format, dimensions, levels, layers, samples, usage, allocator, createInfo, allocationInfo, allocationResult);
}

SharedPtr<VulkanImage> VulkanImage::allocate(const String& name, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult)
{
	VkImage image{};
	VmaAllocation allocation{};

	raiseIfFailed(::vmaCreateImage(allocator, &createInfo, &allocationInfo, &image, &allocation, allocationResult), "Unable to allocate texture.");
	LITEFX_DEBUG(VULKAN_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Usage: {7} }}", name.empty() ? std::format("{0}", static_cast<void*>(image)) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, usage, samples);

	return SharedPtr<VulkanImage>(new VulkanImage(image, extent, format, dimensions, levels, layers, samples, usage, allocator, allocation, name));
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSampler::VulkanSamplerImpl {
public:
	friend class VulkanSampler;

private:
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;
	WeakPtr<const VulkanDevice> m_device;

public:
	VulkanSamplerImpl(const VulkanDevice& device, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) :
		m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy), m_device(device.weak_from_this())
	{
	}

private:
	VkFilter getFilterMode(FilterMode mode)
	{
		switch (mode)
		{
		case FilterMode::Linear: return VK_FILTER_LINEAR;
		case FilterMode::Nearest: return VK_FILTER_NEAREST;
		default: throw std::invalid_argument("Invalid filter mode.");
		}
	}

	VkSamplerMipmapMode getMipMapMode(MipMapMode mode)
	{
		switch (mode)
		{
		case MipMapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case MipMapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		default: throw std::invalid_argument("Invalid mip map mode.");
		}
	}

	VkSamplerAddressMode getBorderMode(BorderMode mode)
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
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot allocate sampler from a released device instance.");

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

		VkSampler sampler{};
		raiseIfFailed(::vkCreateSampler(device->handle(), &samplerInfo, nullptr, &sampler), "Unable to create sampler.");

		return sampler;
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSampler::VulkanSampler(const VulkanDevice& device, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy, const String& name) :
	Resource<VkSampler>(VK_NULL_HANDLE), m_impl(device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy)
{
	this->handle() = m_impl->initialize();

	if (!name.empty())
		this->name() = name;
}

VulkanSampler::~VulkanSampler() noexcept
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release sampler after parent device.");
	else
		::vkDestroySampler(device->handle(), this->handle(), nullptr);
}

FilterMode VulkanSampler::getMinifyingFilter() const noexcept
{
	return m_impl->m_minFilter;
}

FilterMode VulkanSampler::getMagnifyingFilter() const noexcept
{
	return m_impl->m_magFilter;
}

BorderMode VulkanSampler::getBorderModeU() const noexcept
{
	return m_impl->m_borderU;
}

BorderMode VulkanSampler::getBorderModeV() const noexcept
{
	return m_impl->m_borderV;
}

BorderMode VulkanSampler::getBorderModeW() const noexcept
{
	return m_impl->m_borderW;
}

Float VulkanSampler::getAnisotropy() const noexcept
{
	return m_impl->m_anisotropy;
}

MipMapMode VulkanSampler::getMipMapMode() const noexcept
{
	return m_impl->m_mipMapMode;
}

Float VulkanSampler::getMipMapBias() const noexcept
{
	return m_impl->m_mipMapBias;
}

Float VulkanSampler::getMaxLOD() const noexcept
{
	return m_impl->m_maxLod;
}

Float VulkanSampler::getMinLOD() const noexcept
{
	return m_impl->m_minLod;
}