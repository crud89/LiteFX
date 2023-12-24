#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsFactory::VulkanGraphicsFactoryImpl : public Implement<VulkanGraphicsFactory> {
public:
	friend class VulkanGraphicsFactory;

private:
	const VulkanDevice& m_device;
	VmaAllocator m_allocator{ nullptr };

public:
	VulkanGraphicsFactoryImpl(VulkanGraphicsFactory* parent, const VulkanDevice& device) :
		base(parent), m_device(device)
	{
		// Create an buffer allocator.
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = device.adapter().handle();
		allocatorInfo.instance = device.surface().instance();
		allocatorInfo.device = device.handle();

		raiseIfFailed(::vmaCreateAllocator(&allocatorInfo, &m_allocator), "Unable to create Vulkan memory allocator.");
	}

	~VulkanGraphicsFactoryImpl()
	{
		if (m_allocator != nullptr)
			::vmaDestroyAllocator(m_allocator);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanGraphicsFactory::VulkanGraphicsFactory(const VulkanDevice& device) :
	m_impl(makePimpl<VulkanGraphicsFactoryImpl>(this, device))
{
}

VulkanGraphicsFactory::~VulkanGraphicsFactory() noexcept = default;

UniquePtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const
{
	return this->createBuffer("", type, usage, elementSize, elements, allowWrite);
}

UniquePtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(const String& name, BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	VkBufferUsageFlags usageFlags = {};

	size_t alignedSize = static_cast<size_t>(elementSize);
	size_t alignment = 0;

	switch (type)
	{
	case BufferType::Vertex:  
		usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;  
		break;
	case BufferType::Index:   
		usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;   
		break;
	case BufferType::Uniform: 
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		alignment = m_impl->m_device.adapter().limits().minUniformBufferOffsetAlignment;
		break;
	case BufferType::Storage:
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = m_impl->m_device.adapter().limits().minStorageBufferOffsetAlignment;
		break;
	case BufferType::Texel:
		if (allowWrite)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		else
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		alignment = m_impl->m_device.adapter().limits().minTexelBufferOffsetAlignment;
		break;
	}

	if (alignment > 0)
		alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);

	bufferInfo.size = alignedSize * static_cast<size_t>(elements);

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (usage)
	{
	case BufferUsage::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case BufferUsage::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case BufferUsage::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case BufferUsage::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	UniquePtr<IVulkanBuffer> buffer;
	auto queueFamilies = m_impl->m_device.queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

	buffer = VulkanBuffer::allocate(name, type, elements, elementSize, alignment, allowWrite, m_impl->m_allocator, bufferInfo, allocInfo);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*buffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
#endif

	return buffer;
}

UniquePtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const VulkanVertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	return this->createVertexBuffer("", layout, usage, elements);
}

UniquePtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout.elementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (usage)
	{
	case BufferUsage::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case BufferUsage::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case BufferUsage::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case BufferUsage::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	UniquePtr<IVulkanVertexBuffer> buffer;
	auto queueFamilies = m_impl->m_device.queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

	buffer = VulkanVertexBuffer::allocate(name, layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*buffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
#endif

	return buffer;
}

UniquePtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const VulkanIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	return this->createIndexBuffer("", layout, usage, elements);
}

UniquePtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout.elementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (usage)
	{
	case BufferUsage::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case BufferUsage::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case BufferUsage::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case BufferUsage::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// NOTE: Resource sharing between queue families leaves room for optimization. Currently we simply allow concurrent access by all queue families, so that the driver
	//       needs to ensure that resource state is valid. Ideally, we would set sharing mode to exclusive and detect queue family switches where we need to insert a 
	//       barrier for queue family ownership transfer. This would allow to further optimize workloads between queues to minimize resource ownership transfers (i.e.,
	//       prefer executing workloads that depend on one resource on the same queue, even if it could be run in parallel).
	UniquePtr<IVulkanIndexBuffer> buffer;
	auto queueFamilies = m_impl->m_device.queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

	buffer = VulkanIndexBuffer::allocate(name, layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*buffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
#endif

	return buffer;
}

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createAttachment(Format format, const Size2d& size, MultiSamplingLevel samples) const
{
	return this->createAttachment("", format, size, samples);
}

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createAttachment(const String& name, Format format, const Size2d& size, MultiSamplingLevel samples) const
{
	auto width = std::max<UInt32>(1, size.width());
	auto height = std::max<UInt32>(1, size.height());

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = Vk::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = Vk::getSamples(samples);
	imageInfo.usage = (::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	auto queueFamilies = m_impl->m_device.queueFamilyIndices() | std::ranges::to<std::vector>();
	imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	imageInfo.pQueueFamilyIndices = queueFamilies.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	UniquePtr<IVulkanImage> image;

	if (::hasDepth(format)) [[unlikely]]
		image = VulkanImage::allocate(name, m_impl->m_device, Size3d{ width, height, 1 }, format, ImageDimensions::DIM_2, 1, 1, samples, false, ImageLayout::DepthRead, m_impl->m_allocator, imageInfo, allocInfo);
	else
		image = VulkanImage::allocate(name, m_impl->m_device, Size3d{ width, height, 1 }, format, ImageDimensions::DIM_2, 1, 1, samples, false, ImageLayout::Common, m_impl->m_allocator, imageInfo, allocInfo);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*image).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
#endif

	return image;
}

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, allowWrite);
}

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	if (dimension == ImageDimensions::CUBE && layers != 6) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", 6u, 6u, layers, "A cube map must be defined with 6 layers, but {0} are provided.", layers);

	if (dimension == ImageDimensions::DIM_3 && layers != 1) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", 1u, 1u, layers, "A 3D texture can only have one layer, but {0} are provided.", layers);

	auto width = std::max<UInt32>(1, size.width());
	auto height = std::max<UInt32>(1, size.height());
	auto depth = std::max<UInt32>(1, size.depth());

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = Vk::getImageType(dimension);
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = depth;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = layers;
	imageInfo.format = Vk::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = Vk::getSamples(samples);
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	
	if (allowWrite)
		imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	
	if (::hasDepth(format) || ::hasStencil(format))
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	auto queueFamilies = m_impl->m_device.queueFamilyIndices() | std::ranges::to<std::vector>();
	imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	imageInfo.pQueueFamilyIndices = queueFamilies.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	auto image = VulkanImage::allocate(name, m_impl->m_device, { width, height, depth }, format, dimension, levels, layers, samples, allowWrite, ImageLayout::Undefined, m_impl->m_allocator, imageInfo, allocInfo);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*image).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
#endif

	return image;
}

Enumerable<UniquePtr<IVulkanImage>> VulkanGraphicsFactory::createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	return [&, this]() -> std::generator<UniquePtr<IVulkanImage>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield this->createTexture(format, size, dimension, levels, layers, samples, allowWrite);
	}() | std::views::as_rvalue;
}

UniquePtr<IVulkanSampler> VulkanGraphicsFactory::createSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return makeUnique<VulkanSampler>(m_impl->m_device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

UniquePtr<IVulkanSampler> VulkanGraphicsFactory::createSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	auto sampler = makeUnique<VulkanSampler>(m_impl->m_device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);

#ifndef NDEBUG
	if (!name.empty())
		m_impl->m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*sampler).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
#endif

	return sampler;
}

Enumerable<UniquePtr<IVulkanSampler>> VulkanGraphicsFactory::createSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [&, this]() -> std::generator<UniquePtr<IVulkanSampler>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield this->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}() | std::views::as_rvalue;
}