#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"

// Include Vulkan Memory Allocator.
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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

		raiseIfFailed<RuntimeException>(::vmaCreateAllocator(&allocatorInfo, &m_allocator), "Unable to create Vulkan memory allocator.");
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

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createImage(const Format& format, const Size2d& size, const ImageDimensions& dimensions, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = ::getImageType(dimensions);
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

	if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
		queues.push_back(m_impl->m_device.transferQueue().familyId());

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
	imageInfo.pQueueFamilyIndices = queues.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return VulkanImage::allocate(m_impl->m_device, size, format, dimensions, m_impl->m_allocator, imageInfo, allocInfo);
}

UniquePtr<IVulkanImage> VulkanGraphicsFactory::createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = (::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	UInt32 queues[] = { m_impl->m_device.graphicsQueue().familyId() };
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 1;
	imageInfo.pQueueFamilyIndices = queues;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return VulkanImage::allocate(m_impl->m_device, size, format, ImageDimensions::DIM_2, m_impl->m_allocator, imageInfo, allocInfo);
}

UniquePtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements) const
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
		alignment = m_impl->m_device.adapter().getLimits().minUniformBufferOffsetAlignment;
		break;
	case BufferType::Storage: 
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = m_impl->m_device.adapter().getLimits().minStorageBufferOffsetAlignment;
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
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;

		return VulkanBuffer::allocate(m_impl->m_device, type, elements, elementSize, alignment, m_impl->m_allocator, bufferInfo, allocInfo);
	}
	else
	{
		Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

		if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
			queues.push_back(m_impl->m_device.transferQueue().familyId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();

		return VulkanBuffer::allocate(m_impl->m_device, type, elements, elementSize, alignment, m_impl->m_allocator, bufferInfo, allocInfo);
	}
}

UniquePtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const VulkanVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
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
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;

		return VulkanVertexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
	}
	else
	{
		Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

		if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
			queues.push_back(m_impl->m_device.transferQueue().familyId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();

		return VulkanVertexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
	}
}

UniquePtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const VulkanIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
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

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;

		return VulkanIndexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
	}
	else
	{
		Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

		if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
			queues.push_back(m_impl->m_device.transferQueue().familyId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();

		return VulkanIndexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
	}
}

UniquePtr<IVulkanConstantBuffer> VulkanGraphicsFactory::createConstantBuffer(const VulkanDescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	VkBufferUsageFlags usageFlags = {};

	size_t alignedSize = static_cast<size_t>(layout.elementSize());
	size_t alignment = 0;

	switch (layout.type())
	{
	case BufferType::Uniform: 
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; 
		alignment = m_impl->m_device.adapter().getLimits().minUniformBufferOffsetAlignment;
		break;
	case BufferType::Storage: 
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = m_impl->m_device.adapter().getLimits().minStorageBufferOffsetAlignment;
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
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;

		return VulkanConstantBuffer::allocate(layout, elements, alignment, m_impl->m_allocator, bufferInfo, allocInfo);
	}
	else
	{
		Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

		if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
			queues.push_back(m_impl->m_device.transferQueue().familyId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();

		return VulkanConstantBuffer::allocate(layout, elements, alignment, m_impl->m_allocator, bufferInfo, allocInfo);
	}
}

UniquePtr<IVulkanTexture> VulkanGraphicsFactory::createTexture(const VulkanDescriptorLayout& layout, const Format& format, const Size2d& size, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = ::getImageType(dimension);
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | (::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_SAMPLED_BIT);

	Array<UInt32> queues{ m_impl->m_device.graphicsQueue().familyId() };

	if (m_impl->m_device.transferQueue().familyId() != m_impl->m_device.graphicsQueue().familyId())
		queues.push_back(m_impl->m_device.transferQueue().familyId());

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
	imageInfo.pQueueFamilyIndices = queues.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return VulkanTexture::allocate(m_impl->m_device, layout, size, format, dimension, levels, samples, m_impl->m_allocator, imageInfo, allocInfo);
}

Array<UniquePtr<IVulkanTexture>> VulkanGraphicsFactory::createTextures(const VulkanDescriptorLayout& layout, const UInt32& elements, const Format& format, const Size2d& size, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	Array<UniquePtr<IVulkanTexture>> textures(elements);
	std::ranges::generate(textures, [&, this]() { return this->createTexture(layout, format, size, dimension, levels, samples); });
	return textures;
}

UniquePtr<IVulkanSampler> VulkanGraphicsFactory::createSampler(const VulkanDescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	return makeUnique<VulkanSampler>(m_impl->m_device, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

Array<UniquePtr<IVulkanSampler>> VulkanGraphicsFactory::createSamplers(const VulkanDescriptorLayout& layout, const UInt32& elements, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	Array<UniquePtr<IVulkanSampler>> samplers(elements);
	std::ranges::generate(samplers, [&, this]() { return this->createSampler(layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy); });
	return samplers;
}