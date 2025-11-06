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
		std::function<void(VkDevice)> deleter;
		SharedPtr<IDeviceMemory> resource;
	};

	VmaDefragmentationContext m_defragmentationContext{ nullptr };
	VmaDefragmentationPassMoveInfo m_defragmentationPass{};
	SharedPtr<VulkanCommandBuffer> m_defragmentationCommandBuffer{ nullptr };
	Queue<DefragResource> m_destroyedResources{};
	UInt64 m_defragmentationFence{ 0u };
	Array<UInt32> m_queueIds;

public:
	VulkanGraphicsFactoryImpl(const VulkanDevice& device) :
		m_device(device.weak_from_this()), m_queueIds(device.queueFamilyIndices() | std::ranges::to<std::vector>())
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
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

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
	inline VmaAllocationCreateInfo getAllocationCreateInfo(ResourceHeap heap, AllocationBehavior allocationBehavior, bool manualAlloc = false) const
	{
		VmaAllocationCreateInfo allocInfo{};

		if (allocationBehavior == AllocationBehavior::StayWithinBudget)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		else if (allocationBehavior == AllocationBehavior::DontExpandCache)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;

		switch (heap)
		{
		case ResourceHeap::Staging:
			if (!manualAlloc)
				allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		case ResourceHeap::Resource:
			if (manualAlloc)
				allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			else
				allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case ResourceHeap::Dynamic:
			if (!manualAlloc)
				allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::Readback:
			if (manualAlloc)
				allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			else
				allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case ResourceHeap::GPUUpload:
			if (manualAlloc)
				allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			else
				allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		}

		return allocInfo;
	}

	inline VkBufferCreateInfo getCreateInfo(const ResourceAllocationInfo::BufferInfo& bufferInfo, ResourceUsage usage, UInt64& elementSize, UInt64& alignment) const
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot acquire buffer information from a released device instance.");

		// Create new buffer description.
		VkBufferCreateInfo bufferDescription = { 
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		};

		// Compute element size, alignment and usage flags.
		elementSize = static_cast<UInt64>(bufferInfo.ElementSize);
		alignment = 1;

		switch (bufferInfo.Type)
		{
		case BufferType::Vertex:
			bufferDescription.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			if (bufferInfo.VertexBufferLayout != nullptr)
				elementSize = bufferInfo.VertexBufferLayout->elementSize();

			break;
		case BufferType::Index:
			bufferDescription.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

			if (bufferInfo.IndexBufferLayout != nullptr)
				elementSize = bufferInfo.IndexBufferLayout->elementSize();

			break;
		case BufferType::Uniform:
			bufferDescription.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			alignment = device->adapter().limits().minUniformBufferOffsetAlignment;
			break;
		case BufferType::Storage:
			bufferDescription.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
			break;
		case BufferType::Texel:
			if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite))
				bufferDescription.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
			else
				bufferDescription.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

			alignment = device->adapter().limits().minTexelBufferOffsetAlignment;
			break;
		case BufferType::AccelerationStructure:
			bufferDescription.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
			alignment = device->adapter().limits().minUniformBufferOffsetAlignment;
			break;
		case BufferType::ShaderBindingTable:
			bufferDescription.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
			alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
			break;
		case BufferType::Indirect:
			bufferDescription.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			alignment = device->adapter().limits().minStorageBufferOffsetAlignment;
			break;
		default:
			break;
		}

		if (alignment > 1)
			elementSize = (elementSize + alignment - 1) & ~(alignment - 1);

		bufferDescription.size = elementSize * static_cast<UInt64>(bufferInfo.Elements);

		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
			bufferDescription.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
			bufferDescription.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput))
			bufferDescription.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

		// Set sharing mode based on queue families.
		bufferDescription.sharingMode = m_queueIds.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
		bufferDescription.queueFamilyIndexCount = static_cast<UInt32>(m_queueIds.size());
		bufferDescription.pQueueFamilyIndices = m_queueIds.data();

		return bufferDescription;
	}

	inline VkImageCreateInfo getCreateInfo(const ResourceAllocationInfo::ImageInfo& imageInfo, ResourceUsage usage) const
	{
		// Create image description.
		VkImageCreateInfo imageDescription = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = VK_IMAGE_CREATE_ALIAS_BIT,
			.imageType = Vk::getImageType(imageInfo.Dimensions),
			.format = Vk::getFormat(imageInfo.Format),
			.extent = { 
				.width = static_cast<UInt32>(imageInfo.Size.width()), 
				.height = static_cast<UInt32>(imageInfo.Size.height()), 
				.depth = static_cast<UInt32>(imageInfo.Size.depth()) 
			},
			.mipLevels = imageInfo.Levels,
			.arrayLayers = imageInfo.Layers,
			.samples = Vk::getSamples(imageInfo.Samples),
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		// Properly setup usage flags.
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite))
			imageDescription.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
			imageDescription.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
			imageDescription.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::RenderTarget))
		{
			if (::hasDepth(imageInfo.Format) || ::hasStencil(imageInfo.Format))
				imageDescription.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				imageDescription.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		// Set sharing mode based on queue families.
		imageDescription.sharingMode = m_queueIds.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
		imageDescription.queueFamilyIndexCount = static_cast<UInt32>(m_queueIds.size());
		imageDescription.pQueueFamilyIndices = m_queueIds.data();

		return imageDescription;
	}

	inline VkMemoryRequirements getMemoryRequirements(const VulkanDevice& device, const ResourceAllocationInfo::BufferInfo& bufferInfo, ResourceUsage usage) const
	{
		// Get the buffer description.
		UInt64 elementSize{}, elementAlignment{};
		auto bufferDescription = getCreateInfo(bufferInfo, usage, elementSize, elementAlignment);

		// Query the memory requirements.
		VkDeviceBufferMemoryRequirements deviceRequirements = { .sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS, .pCreateInfo = &bufferDescription };
		VkMemoryRequirements2 memoryRequirements = { .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, };
		::vkGetDeviceBufferMemoryRequirements(device.handle(), &deviceRequirements, &memoryRequirements);

		return memoryRequirements.memoryRequirements;
	}

	inline VkMemoryRequirements getMemoryRequirements(const VulkanDevice& device, const ResourceAllocationInfo::ImageInfo& imageInfo, ResourceUsage usage) const
	{
		// Get the image description.
		auto imageDescription = getCreateInfo(imageInfo, usage);

		// Query the memory requirements.
		VkDeviceImageMemoryRequirements deviceRequirements = { .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS, .pCreateInfo = &imageDescription };
		VkMemoryRequirements2 memoryRequirements = { .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, };
		::vkGetDeviceImageMemoryRequirements(device.handle(), &deviceRequirements, &memoryRequirements);

		return memoryRequirements.memoryRequirements;
	}

	inline VkMemoryRequirements getMemoryRequirements(const ResourceAllocationInfo& allocationInfo) const
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot allocate buffer from a released device instance.");

		if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
			return this->getMemoryRequirements(*device, std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo), allocationInfo.Usage);
		else if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
			return this->getMemoryRequirements(*device, std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo), allocationInfo.Usage);
		else
			std::unreachable();
	}

public:
	template <typename TAllocator, typename... TArgs>
	inline auto allocateBuffer(const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Validate inputs.
		if ((bufferInfo.Type == BufferType::Vertex || bufferInfo.Type == BufferType::Index || bufferInfo.Type == BufferType::Uniform) && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex, index and uniform/constant buffers cannot be written to.");

		if (bufferInfo.Type == BufferType::AccelerationStructure && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: acceleration structures cannot be used as build inputs for other acceleration structures.");

		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot allocate buffer from a released device instance.");

		// Set heap-default usages.
		if (bufferInfo.Heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
			usage |= ResourceUsage::TransferSource;
		else if (bufferInfo.Heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
			usage |= ResourceUsage::TransferDestination;
	
		// Get a buffer and allocation create info.
		UInt64 elementSize{}, elementAlignment{};
		auto bufferDescription = getCreateInfo(bufferInfo, usage, elementSize, elementAlignment);
		auto allocationDescription = getAllocationCreateInfo(bufferInfo.Heap, allocationBehavior);

		// Create the buffer and return.
		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, bufferInfo, static_cast<size_t>(elementAlignment), usage, *device, m_allocator, bufferDescription, allocationDescription, &allocationResult);
	}

	template <typename TAllocator, typename... TArgs>
	inline auto allocateImage(const String& name, const ResourceAllocationInfo::ImageInfo& imageInfo, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Validate usage flags.
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: image resources cannot be used as build inputs for other acceleration structures.");

		// Validate dimensions.
		constexpr UInt32 CUBE_SIDES = 6u;

		if (imageInfo.Dimensions == ImageDimensions::CUBE && imageInfo.Layers != CUBE_SIDES) [[unlikely]]
			throw ArgumentOutOfRangeException("imageInfo", std::make_pair(CUBE_SIDES, CUBE_SIDES), imageInfo.Layers, "A cube map must be defined with 6 layers, but {0} are provided.", imageInfo.Layers);

		if (imageInfo.Dimensions == ImageDimensions::DIM_3 && imageInfo.Layers != 1) [[unlikely]]
			throw ArgumentOutOfRangeException("imageInfo", std::make_pair(1u, 1u), imageInfo.Layers, "A 3D texture can only have one layer, but {0} are provided.", imageInfo.Layers);

		// Get a image and allocation create info.
		auto imageDescription = getCreateInfo(imageInfo, usage);
		auto allocationDescription = getAllocationCreateInfo(ResourceHeap::Resource, allocationBehavior);

		// Create the image and return.
		VmaAllocationInfo allocationResult{};
		return allocator(std::forward<TArgs>(args)..., name, imageInfo.Size, imageInfo.Format, imageInfo.Dimensions, imageInfo.Levels, imageInfo.Layers, imageInfo.Samples, usage, m_allocator, imageDescription, allocationDescription, &allocationResult);
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

	// Begin recording a command buffer for defragmentation.
	Array<IDeviceMemory*> resources;
	auto& commandBuffer = *m_impl->m_defragmentationCommandBuffer;
	commandBuffer.begin();

	// Prepare the move operation on each resource, i.e., create a barrier to allow then to synchronize the move with their current usage.
	VulkanBarrier barrier(PipelineStage::All, PipelineStage::Transfer);
	IDeviceMemory::PrepareMoveEventArgs eventArgs(barrier);

	for (UInt32 i{ 0u }; i < pass.moveCount; ++i)
	{
		// Get the source allocation.
		auto sourceAllocation = pass.pMoves[i].srcAllocation;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		// Acquire the underlying resource device memory instance.
		VmaAllocationInfo allocationInfo{};
		::vmaGetAllocationInfo(m_impl->m_allocator, sourceAllocation, &allocationInfo);

		// Invoke the `prepareMove` event.
		static_cast<IDeviceMemory*>(allocationInfo.pUserData)->prepareMove(this, eventArgs);
	}

	// Issue a barrier to transition the resources that requested it.
	commandBuffer.barrier(barrier);

	// Perform the actual move operations.
	for (UInt32 i{ 0u }; i < pass.moveCount; ++i)
	{
		// Get the source allocation.
		auto sourceAllocation = pass.pMoves[i].srcAllocation;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		auto targetAllocation = pass.pMoves[i].dstTmpAllocation; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		VmaAllocationInfo allocationInfo{};
		::vmaGetAllocationInfo(m_impl->m_allocator, sourceAllocation, &allocationInfo);

		// Acquire the underlying resource device memory instance and add it to the list of moved-from resources.
		auto deviceMemory = static_cast<IDeviceMemory*>(allocationInfo.pUserData);
		resources.emplace_back(deviceMemory);

		// Figure out the resource type.
		if (auto buffer = dynamic_cast<VulkanBuffer*>(deviceMemory); buffer != nullptr)
		{
			auto oldHandle = std::as_const(*buffer).handle();

			if (VulkanBuffer::move(buffer->shared_from_this(), targetAllocation, commandBuffer))
				m_impl->m_destroyedResources.emplace([oldHandle](VkDevice device) { ::vkDestroyBuffer(device, oldHandle, nullptr); }, buffer->shared_from_this());
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

				if (VulkanImage::move(image->shared_from_this(), targetAllocation, commandBuffer))
					m_impl->m_destroyedResources.emplace([oldHandle](VkDevice device) { ::vkDestroyImage(device, oldHandle, nullptr); }, image->shared_from_this());
				else
					pass.pMoves[i].operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			}
		}
	}

	// Submit de command buffer and store the fence.
	auto fence = m_impl->m_defragmentationFence = commandBuffer.submit();

	// Invoke the `moving` event.
	for (auto resource : resources)
		resource->moving(this, { commandBuffer.queue(), fence });

	// Return the current fence value.
	return fence;
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
		resource.deleter(device->handle());

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
	VkBufferCreateInfo bufferDescription = { 
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
	auto& queueFamilies = m_impl->m_queueIds;

	bufferDescription.sharingMode = queueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE; // Does not matter anyway if only one queue family is present, but satisfies validation layers.
	bufferDescription.queueFamilyIndexCount = static_cast<UInt32>(queueFamilies.size());
	bufferDescription.pQueueFamilyIndices = queueFamilies.data();

	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Other,
		.ElementSize = heapSize,
		.Elements = 1u,
		.Heap = ResourceHeap::Resource
	};

#ifndef NDEBUG
	auto buffer = VulkanBuffer::allocate(name, bufferInfo, 1u, ResourceUsage::Default, *device, m_impl->m_allocator, bufferDescription, allocInfo);

	if (!name.empty())
		device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);

	return buffer;
#else
	return VulkanBuffer::allocate(name, bufferInfo, 1u, ResourceUsage::Default, *device, m_impl->m_allocator, bufferDescription, allocInfo);
#endif
}

Generator<ResourceAllocationResult> VulkanGraphicsFactory::allocate(Enumerable<const ResourceAllocationInfo&> ai, AllocationBehavior allocationBehavior, bool alias) const
{
	auto allocationInfos = ai | std::ranges::to<std::vector>();

	if (allocationInfos.empty())
		co_return;

	auto device = m_impl->m_device.lock();

	if (device == nullptr)
		throw RuntimeException("Unable to allocate resources from a device that has already been released.");

	if (!alias)
	{
		// Allocate all resources individually.
		for (auto& allocationInfo : allocationInfos)
			co_yield this->allocate(allocationInfo, allocationBehavior);
	}
	else
	{
		// NOTE: It is assumed that before calling this method, support for aliasing has been checked by calling `canAlias`. Here we simply use the first resource heap
		//       we can find, as it is assumed that they are all equal anyway.
		auto resourceHeaps = allocationInfos
			| std::views::transform([](auto& allocationInfo) {
				if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
					return ResourceHeap::Resource;
				else if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
					return std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo).Heap;
				else
					std::unreachable();
			})
			| std::views::take(1)
			| std::ranges::to<std::vector>();

		auto allocationDesc = m_impl->getAllocationCreateInfo(resourceHeaps.front(), allocationBehavior, true);

		// Acquire memory requirements for each resource.
		auto memoryRequirements = std::ranges::fold_left_first(
			allocationInfos | std::views::transform([this](auto& allocationInfo) { return m_impl->getMemoryRequirements(allocationInfo); }) | std::ranges::to<std::vector>(),
			[](const VkMemoryRequirements& accumulated, const VkMemoryRequirements& current) -> VkMemoryRequirements {
				return { std::max(accumulated.size, current.size), std::max(accumulated.alignment, current.alignment), accumulated.memoryTypeBits & current.memoryTypeBits };
			}).value_or({});
		 
		// Allocate the memory.
		VmaAllocation allocation{};
		auto result = ::vmaAllocateMemory(m_impl->m_allocator, &memoryRequirements, &allocationDesc, &allocation, nullptr);

		if (result != VK_SUCCESS) [[unlikely]]
			throw VulkanPlatformException(result, "Unable to allocate memory for aliasing resources.");

		// Create the buffers and images on the allocation.
		AllocationPtr allocationPtr(allocation, VmaAllocationDeleter{ m_impl->m_allocator });

		for (auto& allocationInfo : allocationInfos)
		{
			if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
			{
				const auto& bufferInfo = std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo);

				VkBuffer buffer{};
				UInt64 elementSize{}, elementAlignment{};
				auto resourceDescription = m_impl->getCreateInfo(bufferInfo, allocationInfo.Usage, elementSize, elementAlignment);
				result = ::vmaCreateAliasingBuffer2(m_impl->m_allocator, allocation, allocationInfo.AliasingOffset, &resourceDescription, &buffer);

				if (result != VK_SUCCESS) [[unlikely]]
					throw VulkanPlatformException(result, "Unable to allocate resource from memory reserved for aliasing resource block.");

				if (bufferInfo.Type == BufferType::Vertex && bufferInfo.VertexBufferLayout != nullptr)
					co_yield std::dynamic_pointer_cast<IBuffer>(VulkanVertexBuffer::create(buffer, dynamic_cast<const VulkanVertexBufferLayout&>(*bufferInfo.VertexBufferLayout), bufferInfo.Elements, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, *device, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
				else if (bufferInfo.Type == BufferType::Index && bufferInfo.IndexBufferLayout != nullptr)
					co_yield std::dynamic_pointer_cast<IBuffer>(VulkanIndexBuffer::create(buffer, dynamic_cast<const VulkanIndexBufferLayout&>(*bufferInfo.IndexBufferLayout), bufferInfo.Elements, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, *device, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
				else [[likely]]
					co_yield std::dynamic_pointer_cast<IBuffer>(VulkanBuffer::create(buffer, bufferInfo.Type, bufferInfo.Elements, bufferInfo.ElementSize, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, *device, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
			}
			else if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
			{
				const auto& imageInfo = std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo);

				VkImage image{};
				auto resourceDescription = m_impl->getCreateInfo(imageInfo, allocationInfo.Usage);
				result = ::vmaCreateAliasingImage2(m_impl->m_allocator, allocation, allocationInfo.AliasingOffset, &resourceDescription, &image);

				if (result != VK_SUCCESS) [[unlikely]]
					throw VulkanPlatformException(result, "Unable to allocate resource from memory reserved for aliasing resource block.");

				co_yield std::dynamic_pointer_cast<IImage>(VulkanImage::create(image, imageInfo.Size, imageInfo.Format, imageInfo.Dimensions, imageInfo.Levels, imageInfo.Layers, imageInfo.Samples, allocationInfo.Usage, resourceDescription, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
			}
		}
	}
}

bool VulkanGraphicsFactory::canAlias(Enumerable<const ResourceAllocationInfo&> allocationInfos) const
{
	// Check if all resources are on the same heap.
	auto heaps = allocationInfos
		| std::views::transform([](const ResourceAllocationInfo& allocationInfo) -> ResourceHeap {
			if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
				return ResourceHeap::Resource;
			else if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
				return std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo).Heap;
			else
				std::unreachable();
		}) 
		| std::ranges::to<std::vector>();
	
	std::ranges::sort(heaps);	
	auto heapCount = std::unique(heaps.begin(), heaps.end()) - heaps.begin();

	if (heapCount > 1)
		return false;

	// Find if there's at least one memory type that can store all requested resources.
	auto memoryType = std::ranges::fold_left_first(
		allocationInfos 
			| std::views::transform([this](auto& allocationInfo) { 
					auto requirements = m_impl->getMemoryRequirements(allocationInfo); return requirements.memoryTypeBits; 
				}) 
			| std::ranges::to<std::vector>(), std::bit_and{});

	return memoryType.value_or(0) != 0;
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createBuffer("", type, heap, elementSize, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanBuffer> VulkanGraphicsFactory::createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = type,
		.ElementSize = elementSize,
		.Elements = elements,
		.Heap = heap,
	};

#ifndef NDEBUG
	auto buffer = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();
		
		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanBuffer::allocate);
#endif
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createVertexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanVertexBuffer> VulkanGraphicsFactory::createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Vertex,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.VertexBufferLayout = layout.shared_from_this()
	};

#ifndef NDEBUG
	auto buffer = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanVertexBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanVertexBuffer::allocate);
#endif
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createIndexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IVulkanIndexBuffer> VulkanGraphicsFactory::createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Index,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.IndexBufferLayout = layout.shared_from_this()
	};

#ifndef NDEBUG
	auto buffer = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanIndexBuffer::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return buffer;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanIndexBuffer::allocate);
#endif
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

SharedPtr<IVulkanImage> VulkanGraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::ImageInfo imageInfo = {
		.Format = format,
		.Dimensions = dimension,
		.Size = size,
		.Levels = levels,
		.Layers = layers,
		.Samples = samples
	};

#ifndef NDEBUG
	auto image = m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, VulkanImage::allocate);

	if (!name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
	}

	return image;
#else 
	return m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, VulkanImage::allocate);
#endif
}

bool VulkanGraphicsFactory::tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateBuffer(buffer, "", type, heap, elementSize, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = type,
		.ElementSize = elementSize,
		.Elements = elements,
		.Heap = heap,
	};

#ifndef NDEBUG
	auto result = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateVertexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Vertex,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.VertexBufferLayout = layout.shared_from_this()
	};

#ifndef NDEBUG
	auto result = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanVertexBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanVertexBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateIndexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Index,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.IndexBufferLayout = layout.shared_from_this()
	};

#ifndef NDEBUG
	auto result = m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanIndexBuffer::tryAllocate, buffer);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*buffer).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	return result;
#else
	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, VulkanIndexBuffer::tryAllocate, buffer);
#endif
}

bool VulkanGraphicsFactory::tryCreateTexture(SharedPtr<IVulkanImage>& image, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateTexture(image, "", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

bool VulkanGraphicsFactory::tryCreateTexture(SharedPtr<IVulkanImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::ImageInfo imageInfo = {
		.Format = format,
		.Dimensions = dimension,
		.Size = size,
		.Levels = levels,
		.Layers = layers,
		.Samples = samples
	};

#ifndef NDEBUG
	auto result = m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, VulkanImage::tryAllocate, image);

	if (result && !name.empty())
	{
		auto device = m_impl->m_device.lock();

		if (device != nullptr) [[likely]]
			device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
	}

	return result;
#else 
	return m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, VulkanImage::tryAllocate, image);
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