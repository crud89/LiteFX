#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsFactory::VulkanGraphicsFactoryImpl {
public:
	friend class VulkanGraphicsFactory;

private:
	WeakPtr<const VulkanDevice> m_device;
	VmaAllocator m_allocator{ nullptr };

public:
	VulkanGraphicsFactoryImpl(const VulkanDevice& device) :
		m_device(device.weak_from_this())
	{
		// Create an buffer allocator.
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = device.adapter().handle();
		allocatorInfo.instance = device.surface().instance();
		allocatorInfo.device = device.handle();
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

		raiseIfFailed(::vmaCreateAllocator(&allocatorInfo, &m_allocator), "Unable to create Vulkan memory allocator.");
	}

	VulkanGraphicsFactoryImpl(VulkanGraphicsFactoryImpl&&) noexcept = default;
	VulkanGraphicsFactoryImpl(const VulkanGraphicsFactoryImpl&) = delete;
	VulkanGraphicsFactoryImpl& operator=(VulkanGraphicsFactoryImpl&&) noexcept = default;
	VulkanGraphicsFactoryImpl& operator=(const VulkanGraphicsFactoryImpl&) = delete;

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
	m_impl(device)
{
}

VulkanGraphicsFactory::~VulkanGraphicsFactory() noexcept = default;

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const
{
	return this->createBuffer("", type, heap, elementSize, elements, usage);
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const
{
	// Validate inputs.
	if ((type == BufferType::Vertex || type == BufferType::Index || type == BufferType::Uniform) && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex, index and uniform/constant buffers cannot be written to.");

	if (type == BufferType::AccelerationStructure && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: acceleration structures cannot be used as build inputs for other acceleration structures.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate buffer from a released device instance.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;
	
	// Create the buffer.
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	VkBufferUsageFlags usageFlags = { VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };

	UInt64 alignedSize = static_cast<UInt64>(elementSize);
	UInt64 alignment = 0;

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
		alignment = device->adapter().limits().minUniformBufferOffsetAlignment;
		break;
	case BufferType::Storage:
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
		break;
	case BufferType::Texel:
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite))
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		else
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		alignment = device->adapter().limits().minTexelBufferOffsetAlignment;
		break;
	case BufferType::AccelerationStructure:
		usageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		alignment = device->adapter().limits().minUniformBufferOffsetAlignment;
		break;
	case BufferType::ShaderBindingTable:
		usageFlags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
		break;
	case BufferType::Indirect:
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
		break;
	default:
		break;
	}

	if (alignment > 0)
		alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);

	bufferInfo.size = alignedSize * static_cast<size_t>(elements);

	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput))
		usageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (heap)
	{
	case ResourceHeap::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case ResourceHeap::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case ResourceHeap::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case ResourceHeap::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

#ifndef NDEBUG
	auto buffer = VulkanBuffer::allocate(name, type, elements, elementSize, static_cast<size_t>(alignment), usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);

	return buffer;
#else
	return VulkanBuffer::allocate(name, type, elements, elementSize, static_cast<size_t>(alignment), usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);
#endif
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	return this->createVertexBuffer("", layout, heap, elements, usage);
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	// Validate usage.
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex buffers cannot be written to.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate vertex buffer from a released device instance.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout.elementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput))
		usageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (heap)
	{
	case ResourceHeap::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case ResourceHeap::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case ResourceHeap::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case ResourceHeap::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

#ifndef NDEBUG
	auto buffer = VulkanVertexBuffer::allocate(name, layout, elements, usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);

	return buffer;
#else
	return VulkanVertexBuffer::allocate(name, layout, elements, usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);
#endif
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	return this->createIndexBuffer("", layout, heap, elements, usage);
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	// Validate usage.
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: index buffers cannot be written to.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate index from a released device instance.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout.elementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput))
		usageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};

	switch (heap)
	{
	case ResourceHeap::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case ResourceHeap::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case ResourceHeap::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case ResourceHeap::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// NOTE: Resource sharing between queue families leaves room for optimization. Currently we simply allow concurrent access by all queue families, so that the driver
	//       needs to ensure that resource state is valid. Ideally, we would set sharing mode to exclusive and detect queue family switches where we need to insert a 
	//       barrier for queue family ownership transfer. This would allow to further optimize workloads between queues to minimize resource ownership transfers (i.e.,
	//       prefer executing workloads that depend on one resource on the same queue, even if it could be run in parallel).
	auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

#ifndef NDEBUG
	auto buffer = VulkanIndexBuffer::allocate(name, layout, elements, usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);

	return buffer;
#else
	return VulkanIndexBuffer::allocate(name, layout, elements, usage, *device, m_impl->m_allocator, bufferInfo, allocInfo);
#endif
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, usage);
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	// Validate usage flags
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: image resources cannot be used as build inputs for other acceleration structures.");

	constexpr UInt32 CUBE_SIDES = 6u;

	if (dimension == ImageDimensions::CUBE && layers != CUBE_SIDES) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", std::make_pair(CUBE_SIDES, CUBE_SIDES), layers, "A cube map must be defined with 6 layers, but {0} are provided.", layers);

	if (dimension == ImageDimensions::DIM_3 && layers != 1) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", std::make_pair(1u, 1u), layers, "A 3D texture can only have one layer, but {0} are provided.", layers);

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate texture from a released device instance.");

	auto width = std::max<UInt32>(1, static_cast<UInt32>(size.width()));
	auto height = std::max<UInt32>(1, static_cast<UInt32>(size.height()));
	auto depth = std::max<UInt32>(1, static_cast<UInt32>(size.depth()));

	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = Vk::getImageType(dimension),
		.format = Vk::getFormat(format),
		.extent = VkExtent3D { .width = width, .height = height, .depth = depth },
		.mipLevels = levels, 
		.arrayLayers = layers,
		.samples = Vk::getSamples(samples),
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite))
		imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::RenderTarget))
	{
		if (::hasDepth(format) || ::hasStencil(format))
			imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();
	imageInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	imageInfo.pQueueFamilyIndices = queueFamilies.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#ifndef NDEBUG
	auto image = VulkanImage::allocate(name, { width, height, depth }, format, dimension, levels, layers, samples, usage, m_impl->m_allocator, imageInfo, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);

	return image;
#else 
	return VulkanImage::allocate(name, { width, height, depth }, format, dimension, levels, layers, samples, usage, m_impl->m_allocator, imageInfo, allocInfo);
#endif
}

Enumerable<SharedPtr<IVulkanImage>> VulkanGraphicsFactory::createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	return [](SharedPtr<const VulkanGraphicsFactory> factory, UInt32 elements, Format format, Size3d size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) -> std::generator<SharedPtr<IVulkanImage>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield factory->createTexture(format, size, dimension, levels, layers, samples, usage);
	}(this->shared_from_this(), elements, format, size, dimension, levels, layers, samples, usage) | std::views::as_rvalue;
}

SharedPtr<IVulkanSampler> VulkanGraphicsFactory::createSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate sampler from a released device instance.");

	return VulkanSampler::allocate(*device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

SharedPtr<IVulkanSampler> VulkanGraphicsFactory::createSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate sampler from a released device instance.");

#ifndef NDEBUG
	auto sampler = VulkanSampler::allocate(*device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);

	if (!name.empty())
		device->setDebugName(std::as_const(*sampler).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);

	return sampler;
#else
	return VulkanSampler::allocate(*device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
#endif
}

Enumerable<SharedPtr<IVulkanSampler>> VulkanGraphicsFactory::createSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [](SharedPtr<const VulkanGraphicsFactory> factory, UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) -> std::generator<SharedPtr<IVulkanSampler>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield factory->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}(this->shared_from_this(), elements, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy) | std::views::as_rvalue;
}

UniquePtr<VulkanBottomLevelAccelerationStructure> VulkanGraphicsFactory::createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<VulkanBottomLevelAccelerationStructure>(flags, name);
}

UniquePtr<VulkanTopLevelAccelerationStructure> VulkanGraphicsFactory::createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<VulkanTopLevelAccelerationStructure>(flags, name);
}