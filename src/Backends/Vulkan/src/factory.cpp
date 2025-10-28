#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"
#include "virtual_allocator.hpp"

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

	// Defragmentation state.
	struct DefragResource {
		Variant<VkBuffer, VkImage> resourceHandle;
		SharedPtr<IDeviceMemory> resource;
	};

	VmaDefragmentationContext m_defragmentationContext{ nullptr };
	VmaDefragmentationPassMoveInfo m_defragmentationPass{};
	SharedPtr<VulkanCommandBuffer> m_defragmentationCommandBuffer{ nullptr };
	Queue<DefragResource> m_destroyedResources{};
	UInt64 m_defragmentationFence{ 0u };

public:
	VulkanGraphicsFactoryImpl(const VulkanDevice& device) :
		m_device(device.weak_from_this())
	{
		// Setup VMA flags according to enabled device extensions.
		VmaAllocatorCreateFlags createFlags{ VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT };

		auto supportedExtensions = device.enabledExtensions();

		if (std::ranges::any_of(supportedExtensions, [](auto& extension) { return extension == VK_KHR_MAINTENANCE_5_EXTENSION_NAME; }))
			createFlags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;

		if (std::ranges::any_of(supportedExtensions, [](auto& extension) { return extension == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME; }))
			createFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

		if (std::ranges::any_of(supportedExtensions, [](auto& extension) { return extension == VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME; }))
			createFlags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;

		// Create an buffer allocator.
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = device.adapter().handle();
		allocatorInfo.instance = device.surface().instance();
		allocatorInfo.device = device.handle();
		allocatorInfo.flags = createFlags;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

		raiseIfFailed(::vmaCreateAllocator(&allocatorInfo, &m_allocator), "Unable to create Vulkan memory allocator.");

		// Listen to swap chain buffer swap events, in order to call `vmaSetCurrentFrameIndex`.
		device.swapChain().swapped += std::bind(&VulkanGraphicsFactory::VulkanGraphicsFactoryImpl::onBackBufferSwap, this, std::placeholders::_1, std::placeholders::_2);
	}

	VulkanGraphicsFactoryImpl(VulkanGraphicsFactoryImpl&&) noexcept = default;
	VulkanGraphicsFactoryImpl(const VulkanGraphicsFactoryImpl&) = delete;
	VulkanGraphicsFactoryImpl& operator=(VulkanGraphicsFactoryImpl&&) noexcept = default;
	VulkanGraphicsFactoryImpl& operator=(const VulkanGraphicsFactoryImpl&) = delete;

	~VulkanGraphicsFactoryImpl() {
		if (m_allocator != nullptr)
			::vmaDestroyAllocator(m_allocator);
	}

private:
	void onBackBufferSwap([[maybe_unused]] const void* sender, const ISwapChain::BackBufferSwapEventArgs& e) {
		::vmaSetCurrentFrameIndex(m_allocator, e.backBuffer());
	}

public:
	template <typename TAllocator, typename... TArgs>
	inline auto allocateBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Validate inputs.
		if ((type == BufferType::Vertex || type == BufferType::Index || type == BufferType::Uniform) && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex, index and uniform/constant buffers cannot be written to.");

		if (type == BufferType::AccelerationStructure && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: acceleration structures cannot be used as build inputs for other acceleration structures.");

		// Check if the device is still valid.
		auto device = m_device.lock();

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
	
		if (allocationBehavior == AllocationBehavior::StayWithinBudget)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		else if (allocationBehavior == AllocationBehavior::DontExpandCache)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;

		switch (heap)
		{
		case ResourceHeap::Staging:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		case ResourceHeap::Resource:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case ResourceHeap::Dynamic:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::Readback:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::GPUUpload:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		}

		// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
		auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

		bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
		bufferInfo.pQueueFamilyIndices = queueFamilies.data();

		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, type, elements, elementSize, static_cast<size_t>(alignment), usage, *device, m_allocator, bufferInfo, allocInfo, &allocationResult);
	}

	template <typename TAllocator, typename... TArgs>
	inline auto allocateVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Validate usage.
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex buffers cannot be written to.");

		// Check if the device is still valid.
		auto device = m_device.lock();

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

		if (allocationBehavior == AllocationBehavior::StayWithinBudget)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		else if (allocationBehavior == AllocationBehavior::DontExpandCache)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;

		switch (heap)
		{
		case ResourceHeap::Staging:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		case ResourceHeap::Resource:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case ResourceHeap::Dynamic:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::Readback:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::GPUUpload:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		}

		// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
		auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

		bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
		bufferInfo.pQueueFamilyIndices = queueFamilies.data();

		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, layout, elements, usage, *device, m_allocator, bufferInfo, allocInfo, &allocationResult);
	}

	template <typename TAllocator, typename... TArgs>
	inline auto allocateIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Validate usage.
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: index buffers cannot be written to.");

		// Check if the device is still valid.
		auto device = m_device.lock();

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

		if (allocationBehavior == AllocationBehavior::StayWithinBudget)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		else if (allocationBehavior == AllocationBehavior::DontExpandCache)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;

		switch (heap)
		{
		case ResourceHeap::Staging:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		case ResourceHeap::Resource:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case ResourceHeap::Dynamic:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::Readback:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::GPUUpload:
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		}

		// NOTE: Resource sharing between queue families leaves room for optimization. Currently we simply allow concurrent access by all queue families, so that the driver
		//       needs to ensure that resource state is valid. Ideally, we would set sharing mode to exclusive and detect queue family switches where we need to insert a 
		//       barrier for queue family ownership transfer. This would allow to further optimize workloads between queues to minimize resource ownership transfers (i.e.,
		//       prefer executing workloads that depend on one resource on the same queue, even if it could be run in parallel).
		auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

		bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
		bufferInfo.pQueueFamilyIndices = queueFamilies.data();

		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, layout, elements, usage, *device, m_allocator, bufferInfo, allocInfo, &allocationResult);
	}

	template <typename TAllocator, typename... TArgs>
	inline auto allocateImage(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
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
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot allocate texture from a released device instance.");

		auto width = std::max<UInt32>(1, static_cast<UInt32>(size.width()));
		auto height = std::max<UInt32>(1, static_cast<UInt32>(size.height()));
		auto depth = std::max<UInt32>(1, static_cast<UInt32>(size.depth()));

		VkImageCreateInfo imageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = Vk::getImageType(dimension),
			.format = Vk::getFormat(format),
			.extent = VkExtent3D {.width = width, .height = height, .depth = depth },
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

		VmaAllocationCreateInfo allocInfo = { .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE };

		if (allocationBehavior == AllocationBehavior::StayWithinBudget)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		else if (allocationBehavior == AllocationBehavior::DontExpandCache)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;

		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, { width, height, depth }, format, dimension, levels, layers, samples, usage, m_allocator, imageInfo, allocInfo, &allocationResult);
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

VirtualAllocator VulkanGraphicsFactory::createAllocator(UInt64 overallMemory, AllocationAlgorithm algorithm) const
{
	return VirtualAllocator::create<VulkanBackend>(overallMemory, algorithm);
}

void VulkanGraphicsFactory::beginDefragmentation(const ICommandQueue& queue, DefragmentationStrategy strategy, UInt64 maxBytesToMove, UInt32 maxAllocationsToMove) const
{
	if (m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("Another defragmentation process has been previously started and has not yet finished.");

	// Initialize a defragmentation context.
	VmaDefragmentationInfo defragDesc = {
		.maxBytesPerPass = maxBytesToMove,
		.maxAllocationsPerPass = maxAllocationsToMove
	};

	switch (strategy)
	{
	case DefragmentationStrategy::Fast:
		defragDesc.flags = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST_BIT;
		break;
	case DefragmentationStrategy::Balanced:
		defragDesc.flags = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED_BIT;
		break;
	case DefragmentationStrategy::Full:
		defragDesc.flags = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL_BIT;
		break;
	}

	auto result = ::vmaBeginDefragmentation(m_impl->m_allocator, &defragDesc, &m_impl->m_defragmentationContext);

	if (result != VK_SUCCESS)
		throw VulkanPlatformException(result, "Unable to start defragmentation process.");

	// Allocate a command buffer to record the transfer commands to.
	m_impl->m_defragmentationCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(queue.createCommandBuffer(false));
}

UInt64 VulkanGraphicsFactory::beginDefragmentationPass() const
{
	if (!m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("There is currently no active defragmentation process.");

	auto& pass = m_impl->m_defragmentationPass;

	auto result = ::vmaBeginDefragmentationPass(m_impl->m_allocator, m_impl->m_defragmentationContext, &pass);

	if (result == VK_SUCCESS)
		return 0u;
	else if (result != VK_INCOMPLETE) [[unlikely]]
		throw VulkanPlatformException(result, "Unable to begin new defragmentation pass.");

	m_impl->m_defragmentationCommandBuffer->begin();
	Array<IDeviceMemory*> resources;

	for (UInt32 i{ 0u }; i < pass.moveCount; ++i)
	{
		// Get the source allocation.
		auto sourceAllocation = pass.pMoves[i].srcAllocation;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		auto targetAllocation = pass.pMoves[i].dstTmpAllocation; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		VmaAllocationInfo allocationInfo{};
		::vmaGetAllocationInfo(m_impl->m_allocator, sourceAllocation, &allocationInfo);
		
		auto deviceMemory = static_cast<IDeviceMemory*>(allocationInfo.pUserData);
		resources.emplace_back(deviceMemory);

		// Figure out the resource type.
		if (auto buffer = dynamic_cast<VulkanBuffer*>(deviceMemory); buffer != nullptr)
		{
			auto oldHandle = std::as_const(*buffer).handle();

			if (VulkanBuffer::move(buffer->shared_from_this(), targetAllocation, *m_impl->m_defragmentationCommandBuffer))
				m_impl->m_destroyedResources.emplace(oldHandle, buffer->shared_from_this());
			else
				pass.pMoves[i].operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}
		else if (auto image = dynamic_cast<VulkanImage*>(deviceMemory); image != nullptr)
		{
			// TODO: Moving render targets is currently unsupported, as it introduces way to many unpredictable synchronization issues. We should 
			//       improve this in the future. As an alternative, we could create render targets from a separate pool.
			if (LITEFX_FLAG_IS_SET(image->usage(), ResourceUsage::RenderTarget))
				pass.pMoves[i].operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			else
			{
				auto oldHandle = std::as_const(*image).handle();

				if (VulkanImage::move(image->shared_from_this(), targetAllocation, *m_impl->m_defragmentationCommandBuffer))
					m_impl->m_destroyedResources.emplace(oldHandle, image->shared_from_this());
				else
					pass.pMoves[i].operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			}
		}
	}

	m_impl->m_defragmentationFence = m_impl->m_defragmentationCommandBuffer->submit();

	for (auto resource : resources)
		resource->moving(this, { m_impl->m_defragmentationCommandBuffer->queue(), m_impl->m_defragmentationFence });

	return m_impl->m_defragmentationFence;
}

bool VulkanGraphicsFactory::endDefragmentationPass() const
{
	if (!m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("There is currently no active defragmentation process.");

	auto device = m_impl->m_device.lock();

	if (device == nullptr)
		throw RuntimeException("Unable to acquire instance from an already released device.");

	m_impl->m_defragmentationCommandBuffer->queue()->waitFor(m_impl->m_defragmentationFence);

	auto result = ::vmaEndDefragmentationPass(m_impl->m_allocator, m_impl->m_defragmentationContext, &m_impl->m_defragmentationPass);

	if (result != VK_SUCCESS && result != VK_INCOMPLETE) [[unlikely]]
		throw VulkanPlatformException(result, "Unable to end defragmentation pass.");

	while (!m_impl->m_destroyedResources.empty())
	{
		// Get the resource to remove.
		auto& resource = m_impl->m_destroyedResources.front();

		// Invoke the `moved` event.
		resource.resource->moved(this, {});

		// Destroy the old resource.
		std::visit(type_switch{
			[device](VkBuffer buffer) { ::vkDestroyBuffer(device->handle(), buffer, nullptr); },
			[device](VkImage image) { ::vkDestroyImage(device->handle(), image, nullptr); }
		}, resource.resourceHandle);

		// Erase the allocation from the queue.
		m_impl->m_destroyedResources.pop();
	}

	if (result == VK_SUCCESS)
	{
		vmaEndDefragmentation(m_impl->m_allocator, m_impl->m_defragmentationContext, nullptr);
		m_impl->m_defragmentationContext = nullptr;
		return true;
	}
	else // if (result == VK_INCOMPLETE)
	{
		return false;
	}
}

bool VulkanGraphicsFactory::supportsResizableBaseAddressRegister() const noexcept
{
	static constinit UInt32 DEFAULT_BAR_SIZE = 256 * 1024 * 1024; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

	// Query the memory properties from VMA.
	std::array<const VkPhysicalDeviceMemoryProperties*, 1> memProps{};
	::vmaGetMemoryProperties(m_impl->m_allocator, memProps.data());

	// Check the heap sizes for all memory types that are both, DEVICE_LOCAL and HOST_VISIBLE. Default BAR size is 256 Mb. If we found a
	// heap that has equal or less than that, we ignore it, even if it might still be ReBAR-supported, but with that small BAR memory we
	// might as well assume non-support.
	auto memTypes = Span{ memProps[0]->memoryTypes, memProps[0]->memoryTypeCount }; // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

	for (auto& memType : memTypes
		| std::views::filter([](const auto& type) { return
			(type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
			(type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }))
	{
		if (memProps[0]->memoryHeaps[memType.heapIndex].size > DEFAULT_BAR_SIZE) // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
			return true;
	}

	return false;
}

Array<MemoryHeapStatistics> VulkanGraphicsFactory::memoryStatistics() const
{
	// Query the memory properties from VMA to get the number of heaps.
	std::array<const VkPhysicalDeviceMemoryProperties*, 1> memProps{};
	::vmaGetMemoryProperties(m_impl->m_allocator, memProps.data());
	auto memoryTypes = Span{ memProps[0]->memoryTypes, memProps[0]->memoryTypeCount }; // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

	// Allocate array for heap statistics.
	Array<VmaBudget> heapBudgets(memProps[0]->memoryHeapCount); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
	::vmaGetHeapBudgets(m_impl->m_allocator, heapBudgets.data());

	// Convert the heap budgets to the API types.
	return heapBudgets 
		| std::views::transform([&memoryTypes, i = 0](const VmaBudget& budget) mutable -> MemoryHeapStatistics {
				// Find the memory type for the heap.
				UInt32 heapIndex = i++;

				// Return the heap statistics.
				if (auto match = std::ranges::find_if(memoryTypes, [heapIndex](const auto& type) { return type.heapIndex == heapIndex; }); match != memoryTypes.end())
					return {
						.onGpu = (match->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						.cpuVisible = (match->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						.blocks = budget.statistics.blockCount,
						.allocations = budget.statistics.allocationCount,
						.blockSize = budget.statistics.blockBytes,
						.allocationSize = budget.statistics.allocationBytes,
						.usedMemory = budget.usage,
						.availableMemory = budget.budget
					}; 
				else [[unlikely]]
					std::unreachable(); // If we reach here, the driver messed up something real badly.
			})
		| std::ranges::to<Array<MemoryHeapStatistics>>();
}

DetailedMemoryStatistics VulkanGraphicsFactory::detailedMemoryStatistics() const
{
	static auto convertStats = [](const VmaDetailedStatistics& stats, bool onGpu, bool cpuVisible) -> DetailedMemoryStatistics::StatisticsBlock {
		return {
			.onGpu = onGpu,
			.cpuVisible = cpuVisible,
			.blocks = stats.statistics.blockCount,
			.allocations = stats.statistics.allocationCount,
			.blockSize = stats.statistics.blockCount,
			.allocationSize = stats.statistics.allocationBytes,
			.unusedRangeCount = stats.unusedRangeCount,
			.minAllocationSize = stats.allocationSizeMin,
			.maxAllocationSize = stats.allocationSizeMax,
			.minUnusedRangeSize = stats.unusedRangeSizeMin,
			.maxUnusedRangeSize = stats.unusedRangeSizeMax
		};
	};

	// Query the memory properties from VMA to get the number of heaps.
	std::array<const VkPhysicalDeviceMemoryProperties*, 1> memProps{};
	::vmaGetMemoryProperties(m_impl->m_allocator, memProps.data());
	UInt32 heapCount = memProps[0]->memoryHeapCount, typeCount = memProps[0]->memoryTypeCount; // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
	auto memoryTypes = Span{ memProps[0]->memoryTypes, memProps[0]->memoryTypeCount }; // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)

	// Query the total memory statistics.
	VmaTotalStatistics stats{};
	::vmaCalculateStatistics(m_impl->m_allocator, &stats);

	// Convert and return.
	return {
		.perLocation = stats.memoryHeap
			| std::views::take(heapCount)
			| std::views::transform([&, i = 0](const auto& stats) mutable ->DetailedMemoryStatistics::StatisticsBlock {
				// Find the memory type for the heap.
				UInt32 heapIndex = i++;

				// Return the heap statistics.
				if (auto match = std::ranges::find_if(memoryTypes, [heapIndex](const auto& type) { return type.heapIndex == heapIndex; }); match != memoryTypes.end())
					return convertStats(stats, (match->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (match->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				else [[unlikely]]
					std::unreachable(); // If we reach here, the driver messed up something real badly.
			})
		| std::ranges::to<Array<DetailedMemoryStatistics::StatisticsBlock>>(),
		.perResourceHeap = stats.memoryType
			| std::views::take(typeCount)
			| std::views::transform([&, i = 0](const auto& stats) mutable -> DetailedMemoryStatistics::StatisticsBlock {
					const auto& type = memoryTypes[i++];
					return convertStats(stats, (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				})
			| std::ranges::to<Array<DetailedMemoryStatistics::StatisticsBlock>>(),
		.total = convertStats(stats.total, true, true)
	};
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createDescriptorHeap(size_t heapSize) const
{
	return this->createDescriptorHeap("", heapSize);
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createDescriptorHeap(const String& name, size_t heapSize) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate buffer from a released device instance.");

	// Create the buffer.
	// NOTE: D3D12 descriptor heaps do not map exactly to descriptor buffers:
	// - Descriptor heaps bind individual root parameters (we support constants and tables), which a single descriptor set can contain multiple of (one per binding point). Those
	//   can be split over multiple heaps (one for samplers, one for resources).
	// - Descriptor buffers bind whole descriptor sets, which cannot be split over multiple descriptor buffers.
	// For this specific reason, we do not create two descriptor buffers (one for samplers and one for resources), as we would otherwise have to enforce a separation between sets
	// that contain samplers and sets that contain resources. We did this earlier, but this is a quite significant restriction when authoring shaders, so I removed it after 
	// implementing descriptor buffers. 
	// Luckily it appears as if all devices that support VK_EXT_descriptor_buffer provide exactly the same limits for `samplerDescriptorBufferAddressSpaceSize`, 
	// `resourceDescriptorBufferAddressSpaceSize` and `descriptorBufferAddressSpaceSize` in `VkPhysicalDeviceDescriptorBufferPropertiesEXT` (see: 
	// https://vulkan.gpuinfo.org/listpropertiesextensions.php). In other words, it does not make a difference in the available address space, when binding resources and samplers
	// separately, compared to binding them to a single descriptor buffer that can bind all of them. 
	// This might change in the future, in which case I hope we will have a better alternative. Until then, we simply use a single descriptor buffer, supporting mixed sets.
	VkBufferUsageFlags usageFlags = { VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
	VkBufferCreateInfo bufferInfo = { 
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = heapSize,
		.usage = usageFlags
	};

	VmaAllocationCreateInfo allocInfo = {
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
	};

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	auto queueFamilies = device->queueFamilyIndices() | std::ranges::to<std::vector>();

	bufferInfo.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();

#ifndef NDEBUG
	auto buffer = VulkanBuffer::allocate(name, BufferType::Other, 1u, heapSize, 1u, ResourceUsage::Default, *device, m_impl->m_allocator, bufferInfo, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);

	return buffer;
#else
	return VulkanBuffer::allocate(name, BufferType::Other, 1u, heapSize, 1u, ResourceUsage::Default, *device, m_impl->m_allocator, bufferInfo, allocInfo);
#endif
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createBuffer("", type, heap, elementSize, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto buffer = m_impl->allocateBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior, VulkanBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();
		
		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior, VulkanBuffer::allocate);
#endif
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createVertexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto buffer = m_impl->allocateVertexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanVertexBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateVertexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanVertexBuffer::allocate);
#endif
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createIndexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto buffer = m_impl->allocateIndexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanIndexBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateIndexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanIndexBuffer::allocate);
#endif
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto image = m_impl->allocateImage(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior, VulkanImage::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
	}

	return image;
#else 
	return m_impl->allocateImage(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior, VulkanImage::allocate);
#endif
}

bool VulkanGraphicsFactory::tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateBuffer(buffer, "", type, heap, elementSize, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto result = m_impl->allocateBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior, VulkanBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior, VulkanBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateVertexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto result = m_impl->allocateVertexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanVertexBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateVertexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanVertexBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateIndexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto result = m_impl->allocateIndexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanIndexBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateIndexBuffer(name, layout, heap, elements, usage, allocationBehavior, VulkanIndexBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateTexture(SharedPtr<IVulkanImage>& image, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateTexture(image, "", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateTexture(SharedPtr<IVulkanImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
#ifndef NDEBUG
	auto result = m_impl->allocateImage(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior, VulkanImage::tryAllocate, image);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
	}

	return result;
#else 
	return m_impl->allocateImage(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior, VulkanImage::tryAllocate, image);
#endif
}

Generator<SharedPtr<IVulkanImage>> VulkanGraphicsFactory::createTextures(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return [](SharedPtr<const VulkanGraphicsFactory> factory, Format format, Size3d size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) -> Generator<SharedPtr<IVulkanImage>> {
		for (;;)
			co_yield factory->createTexture(format, size, dimension, levels, layers, samples, usage, allocationBehavior);
	}(this->shared_from_this(), format, size, dimension, levels, layers, samples, usage, allocationBehavior);
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

Generator<SharedPtr<IVulkanSampler>> VulkanGraphicsFactory::createSamplers(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [](SharedPtr<const VulkanGraphicsFactory> factory, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) -> Generator<SharedPtr<IVulkanSampler>> {
		for (;;)
			co_yield factory->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}(this->shared_from_this(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

UniquePtr<VulkanBottomLevelAccelerationStructure> VulkanGraphicsFactory::createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<VulkanBottomLevelAccelerationStructure>(flags, name);
}

UniquePtr<VulkanTopLevelAccelerationStructure> VulkanGraphicsFactory::createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<VulkanTopLevelAccelerationStructure>(flags, name);
}