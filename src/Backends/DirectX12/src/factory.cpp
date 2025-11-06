#include <litefx/backends/dx12.hpp>
#include "buffer.h"
#include "image.h"
#include "virtual_allocator.hpp"
#include <bitset>

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

D3D12_RESOURCE_DESC1 getResourceDesc(const ResourceAllocationInfo::BufferInfo& bufferInfo, ResourceUsage usage, UInt64& elementSize, UInt64& elementAlignment) {
	elementAlignment = 1;
	elementSize = bufferInfo.ElementSize;
	D3D12_RESOURCE_FLAGS flags = LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	switch (bufferInfo.Type)
	{
	case BufferType::Uniform: 
		elementAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		elementSize = Math::align(bufferInfo.ElementSize, static_cast<size_t>(elementAlignment));
		break;
	case BufferType::Vertex:
		if (bufferInfo.VertexBufferLayout != nullptr)
			elementSize = bufferInfo.VertexBufferLayout->elementSize();

		break;
	case BufferType::Index:
		if (bufferInfo.IndexBufferLayout != nullptr)
			elementSize = bufferInfo.IndexBufferLayout->elementSize();

		break;
	case BufferType::AccelerationStructure:
		flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
		[[fallthrough]];
	default:
		elementAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT;
		elementSize = Math::align(bufferInfo.ElementSize, static_cast<size_t>(elementAlignment));
		break;
	}

	return {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		//.Alignment = 0, // NOTE: D3D12MA handles alignment by itself.
		.Width = elementSize * bufferInfo.Elements,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = flags
	};
}

D3D12_RESOURCE_DESC1 getResourceDesc(const ResourceAllocationInfo::ImageInfo& imageInfo, ResourceUsage usage) {
	auto width = std::max<UInt32>(1, static_cast<UInt32>(imageInfo.Size.width()));
	auto height = std::max<UInt32>(1, static_cast<UInt32>(imageInfo.Size.height()));
	auto depth = std::max<UInt32>(1, static_cast<UInt32>(imageInfo.Size.depth()));

	D3D12_RESOURCE_FLAGS flags = LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::RenderTarget))
	{
		if (::hasDepth(imageInfo.Format) || ::hasStencil(imageInfo.Format))
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		else
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	return {
		.Dimension = DX12::getImageType(imageInfo.Dimensions),
		//.Alignment = 0, // NOTE: D3D12MA handles alignment by itself.
		.Width = width,
		.Height = height,
		.DepthOrArraySize = static_cast<UInt16>(imageInfo.Dimensions == ImageDimensions::DIM_3 ? depth : imageInfo.Layers),
		.MipLevels = static_cast<UInt16>(imageInfo.Levels),
		.Format = DX12::getFormat(imageInfo.Format),
		.SampleDesc = imageInfo.Samples == MultiSamplingLevel::x1 ? 
			DXGI_SAMPLE_DESC{ 1, 0 } : 
			DXGI_SAMPLE_DESC{ static_cast<UInt32>(imageInfo.Samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN },
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};
}

D3D12MA::ALLOCATION_DESC getAllocationDesc(ResourceHeap heap, AllocationBehavior allocationBehavior) {

	D3D12MA::ALLOCATION_DESC allocationDesc{};

	if (allocationBehavior == AllocationBehavior::DontExpandCache)
		allocationDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_NEVER_ALLOCATE;
	else if (allocationBehavior == AllocationBehavior::StayWithinBudget)
		allocationDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_WITHIN_BUDGET;

	switch (heap)
	{
	case ResourceHeap::Dynamic:
	case ResourceHeap::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case ResourceHeap::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case ResourceHeap::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	case ResourceHeap::GPUUpload:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_GPU_UPLOAD;
		break;
	default:
		throw InvalidArgumentException("heap", "The buffer heap {0} is not supported.", heap);
	}

	return allocationDesc;
}

class DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl {
public:
	friend class DirectX12GraphicsFactory;

private:
	WeakPtr<const DirectX12Device> m_device;
	AllocatorPtr m_allocator; 

	// Defragmentation state.
	struct DefragResource {
		ComPtr<ID3D12Resource> resourceHandle;
		SharedPtr<IDeviceMemory> resource;
	};

	D3D12MA::DefragmentationContext* m_defragmentationContext{ nullptr };
	D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO m_defragmentationPass{};
	SharedPtr<DirectX12CommandBuffer> m_defragmentationCommandBuffer{ nullptr };
	Queue<DefragResource> m_destroyedResources{};
	UInt64 m_defragmentationFence{ 0u };

public:
	DirectX12GraphicsFactoryImpl(const DirectX12Device& device) :
		m_device(device.weak_from_this())
	{
		// Initialize memory allocator.
		D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
		//allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED;
		//allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
		allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
		allocatorDesc.pAdapter = device.adapter().handle().Get();
		allocatorDesc.pDevice = device.handle().Get();
		allocatorDesc.PreferredBlockSize = 0;	// TODO: Make configurable.

		D3D12MA::Allocator* allocator{};
		raiseIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &allocator), "Unable to create D3D12 memory allocator.");
		m_allocator.reset(allocator, D3D12MADeleter{});

		// Listen to swap chain buffer swap events, in order to call `SetCurrentFrameIndex`.
		device.swapChain().swapped += std::bind(&DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl::onBackBufferSwap, this, std::placeholders::_1, std::placeholders::_2);
	}

private:
	void onBackBufferSwap([[maybe_unused]] const void* sender, const ISwapChain::BackBufferSwapEventArgs& e) {
		m_allocator->SetCurrentFrameIndex(e.backBuffer());
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

		// Set heap-default usages.
		if (bufferInfo.Heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
			usage |= ResourceUsage::TransferSource;
		else if (bufferInfo.Heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
			usage |= ResourceUsage::TransferDestination;

		// Get a buffer and allocation create info.
		UInt64 elementSize{}, elementAlignment{};
		D3D12_RESOURCE_DESC1 resourceDescription = getResourceDesc(bufferInfo, usage, elementSize, elementAlignment);
		D3D12MA::ALLOCATION_DESC allocationDescription = getAllocationDesc(bufferInfo.Heap, allocationBehavior);

		// Create the buffer and return.
		return allocator(std::forward<TArgs>(args)..., name, m_allocator, bufferInfo, static_cast<size_t>(elementAlignment), usage, resourceDescription, allocationDescription);
	}

	template <typename TAllocator, typename... TArgs>
	inline auto allocateImage(const String& name, const ResourceAllocationInfo::ImageInfo& imageInfo, ResourceUsage usage, AllocationBehavior allocationBehavior, TAllocator allocator, TArgs&&... args)
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot allocate texture from a released device instance.");

		// Validate usage flags
		if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
			throw InvalidArgumentException("usage", "Invalid resource usage has been specified: image resources cannot be used as build inputs for other acceleration structures.");

		// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
		if (imageInfo.Dimensions == ImageDimensions::CUBE && imageInfo.Layers != 6) [[unlikely]]
			throw ArgumentOutOfRangeException("imageInfo", std::make_pair(6u, 6u), imageInfo.Layers, "A cube map must be defined with 6 layers, but {0} are provided.", imageInfo.Layers);

		if (imageInfo.Dimensions == ImageDimensions::DIM_3 && imageInfo.Layers != 1) [[unlikely]]
			throw ArgumentOutOfRangeException("imageInfo", std::make_pair(1u, 1u), imageInfo.Layers, "A 3D texture can only have one layer, but {0} are provided.", imageInfo.Layers);
		// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

		// Get a image and allocation create info.
		D3D12_RESOURCE_DESC1 resourceDescription = getResourceDesc(imageInfo, usage);
		D3D12MA::ALLOCATION_DESC allocationDescription = getAllocationDesc(ResourceHeap::Resource, allocationBehavior);

		// Create the image and return.
		return allocator(std::forward<TArgs>(args)..., name, *device.get(), m_allocator, imageInfo.Size, imageInfo.Format, imageInfo.Dimensions, imageInfo.Levels, imageInfo.Layers, imageInfo.Samples, usage, resourceDescription, allocationDescription);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsFactory::DirectX12GraphicsFactory(const DirectX12Device& device) :
	m_impl(device)
{
}

DirectX12GraphicsFactory::~DirectX12GraphicsFactory() noexcept = default;

VirtualAllocator DirectX12GraphicsFactory::createAllocator(UInt64 overallMemory, AllocationAlgorithm algorithm) const
{
	return VirtualAllocator::create<DirectX12Backend>(overallMemory, algorithm);
}

void DirectX12GraphicsFactory::beginDefragmentation(const ICommandQueue& queue, DefragmentationStrategy strategy, UInt64 maxBytesToMove, UInt32 maxAllocationsToMove) const
{
	if (m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("Another defragmentation process has been previously started and has not yet finished.");

	// Initialize a defragmentation context.
	D3D12MA::DEFRAGMENTATION_DESC defragDesc = {
		.MaxBytesPerPass = maxBytesToMove,
		.MaxAllocationsPerPass = maxAllocationsToMove
	};

	switch (strategy)
	{
	case DefragmentationStrategy::Fast:
		defragDesc.Flags = D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_FAST;
		break;
	case DefragmentationStrategy::Balanced:
		defragDesc.Flags = D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED;
		break;
	case DefragmentationStrategy::Full:
		defragDesc.Flags = D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_FULL;
		break;
	}

	m_impl->m_allocator->BeginDefragmentation(&defragDesc, &m_impl->m_defragmentationContext);

	// Allocate a command buffer to record the transfer commands to.
	m_impl->m_defragmentationCommandBuffer = std::dynamic_pointer_cast<DirectX12CommandBuffer>(queue.createCommandBuffer(false));
}

UInt64 DirectX12GraphicsFactory::beginDefragmentationPass() const
{
	if (!m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("There is currently no active defragmentation process.");

	auto& pass = m_impl->m_defragmentationPass;
	HRESULT result = m_impl->m_defragmentationContext->BeginPass(&pass);

	if (result == S_OK)
		return 0u;
	else if (result != S_FALSE) [[unlikely]]
		throw DX12PlatformException(result, "Unable to begin new defragmentation pass.");

	// Begin recording a command buffer for defragmentation.
	Array<IDeviceMemory*> resources;
	auto& commandBuffer = *m_impl->m_defragmentationCommandBuffer;
	commandBuffer.begin();

	// Prepare the move operation on each resource, i.e., create a barrier to allow then to synchronize the move with their current usage.
	DirectX12Barrier barrier(PipelineStage::All, PipelineStage::Transfer);
	IDeviceMemory::PrepareMoveEventArgs eventArgs(barrier);

	for (UInt32 i{ 0u }; i < pass.MoveCount; ++i)
	{
		// Get the source allocation.
		auto sourceAllocation = pass.pMoves[i].pSrcAllocation;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		// Acquire the underlying resource device memory instance.
		auto deviceMemory = static_cast<IDeviceMemory*>(sourceAllocation->GetPrivateData());

		// Invoke the `prepareMove` event.
		deviceMemory->prepareMove(this, eventArgs);
	}

	// Issue a barrier to transition the resources that requested it.
	commandBuffer.barrier(barrier);

	// Perform the actual move operations.
	for (UInt32 i{ 0u }; i < pass.MoveCount; ++i)
	{
		// Get the source allocation.
		auto sourceAllocation = pass.pMoves[i].pSrcAllocation;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		auto targetAllocation = pass.pMoves[i].pDstTmpAllocation; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		// Acquire the underlying resource device memory instance and add it to the list of moved-from resources.
		IDeviceMemory* deviceMemory = static_cast<IDeviceMemory*>(sourceAllocation->GetPrivateData());
		resources.emplace_back(deviceMemory);

		// Figure out the resource type.
		if (auto buffer = dynamic_cast<DirectX12Buffer*>(deviceMemory); buffer != nullptr)
		{
			auto oldHandle = std::as_const(*buffer).handle();

			if (DirectX12Buffer::move(buffer->shared_from_this(), targetAllocation, commandBuffer))
				m_impl->m_destroyedResources.emplace(std::move(oldHandle), buffer->shared_from_this());
			else
				pass.pMoves[i].Operation = D3D12MA::DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}
		else if (auto image = dynamic_cast<DirectX12Image*>(deviceMemory); image != nullptr)
		{
			// TODO: Moving render targets is currently unsupported, as it introduces way to many unpredictable synchronization issues. We should 
			//       improve this in the future. As an alternative, we could create render targets from a separate pool.
			if (LITEFX_FLAG_IS_SET(image->usage(), ResourceUsage::RenderTarget))
				pass.pMoves[i].Operation = D3D12MA::DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			else
			{
				auto oldHandle = std::as_const(*image).handle();

				if (DirectX12Image::move(image->shared_from_this(), targetAllocation, commandBuffer))
					m_impl->m_destroyedResources.emplace(std::move(oldHandle), image->shared_from_this());
				else
					pass.pMoves[i].Operation = D3D12MA::DEFRAGMENTATION_MOVE_OPERATION_IGNORE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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

bool DirectX12GraphicsFactory::endDefragmentationPass() const
{
	if (!m_impl->m_defragmentationContext) [[unlikely]]
		throw RuntimeException("There is currently no active defragmentation process.");

	m_impl->m_defragmentationCommandBuffer->queue()->waitFor(m_impl->m_defragmentationFence);

	Array<ComPtr<ID3D12Resource>> resources{};

	while (!m_impl->m_destroyedResources.empty())
	{
		// Get the resource to remove.
		auto& resource = m_impl->m_destroyedResources.front();

		// Invoke the `moved` event.
		resource.resource->moved(this, {});

		// Store the resource just so it can be destroyed after ending the pass.
		resources.emplace_back(std::move(resource.resourceHandle));

		// Erase the allocation from the queue.
		m_impl->m_destroyedResources.pop();
	}

	auto result = m_impl->m_defragmentationContext->EndPass(&m_impl->m_defragmentationPass);

	if (result != S_OK && result != S_FALSE) [[unlikely]]
		throw DX12PlatformException(result, "Unable to end defragmentation pass.");

	resources.clear();

	if (result == S_OK)
	{
		m_impl->m_defragmentationContext->Release();
		m_impl->m_defragmentationContext = nullptr;
		return true;
	}
	else // if (result == S_FALSE)
	{
		return false;
	}
}

bool DirectX12GraphicsFactory::supportsResizableBaseAddressRegister() const noexcept
{
	return m_impl->m_allocator->IsGPUUploadHeapSupported();
}

Array<MemoryHeapStatistics> DirectX12GraphicsFactory::memoryStatistics() const
{
	// Query the current memory statistics.
	auto budgets = std::array<D3D12MA::Budget, 2u>{};
	m_impl->m_allocator->GetBudget(&budgets[0], &budgets[1]);

	// Convert the budgets to the API type.
	return {
		MemoryHeapStatistics {
			.onGpu = !m_impl->m_allocator->IsUMA(),
			.cpuVisible = static_cast<bool>(m_impl->m_allocator->IsUMA()),
			.blocks = budgets[0].Stats.BlockCount,
			.allocations = budgets[0].Stats.AllocationCount,
			.blockSize = budgets[0].Stats.BlockBytes,
			.allocationSize = budgets[0].Stats.AllocationBytes,
			.usedMemory = budgets[0].UsageBytes,
			.availableMemory = budgets[0].BudgetBytes
		},
		MemoryHeapStatistics {
			.onGpu = false,
			.cpuVisible = true,
			.blocks = budgets[1].Stats.BlockCount,
			.allocations = budgets[1].Stats.AllocationCount,
			.blockSize = budgets[1].Stats.BlockBytes,
			.allocationSize = budgets[1].Stats.AllocationBytes,
			.usedMemory = budgets[1].UsageBytes,
			.availableMemory = budgets[1].BudgetBytes
		},
	};
}

DetailedMemoryStatistics DirectX12GraphicsFactory::detailedMemoryStatistics() const
{
	static auto convertStats = [](const D3D12MA::DetailedStatistics& stats, bool onGpu, bool cpuVisible) -> DetailedMemoryStatistics::StatisticsBlock {
		return {
			.onGpu = onGpu,
			.cpuVisible = cpuVisible,
			.blocks = stats.Stats.BlockCount,
			.allocations = stats.Stats.AllocationCount,
			.blockSize = stats.Stats.BlockCount,
			.allocationSize = stats.Stats.AllocationBytes,
			.unusedRangeCount = stats.UnusedRangeCount,
			.minAllocationSize = stats.AllocationSizeMin,
			.maxAllocationSize = stats.AllocationSizeMax,
			.minUnusedRangeSize = stats.UnusedRangeSizeMin,
			.maxUnusedRangeSize = stats.UnusedRangeSizeMax
		};
	};

	// Query the total memory statistics.
	D3D12MA::TotalStatistics stats{};
	m_impl->m_allocator->CalculateStatistics(&stats);

	// Convert and return.
	return {
		.perLocation = stats.MemorySegmentGroup 
			| std::views::transform([&, i = 0](const auto& stats) mutable -> DetailedMemoryStatistics::StatisticsBlock {
					if (i++ == 0)
						return convertStats(stats, !m_impl->m_allocator->IsUMA(), static_cast<bool>(m_impl->m_allocator->IsUMA()));
					else
						return convertStats(stats, false, true);
				}) 
			| std::ranges::to<Array<DetailedMemoryStatistics::StatisticsBlock>>(),
		.perResourceHeap = stats.HeapType 
			| std::views::transform([&, i = 0](const auto& stats) mutable -> DetailedMemoryStatistics::StatisticsBlock {
					switch (i++)
					{
					case 0:  // DEFAULT
						return convertStats(stats, true, false);
					case 1:  // UPLOAD
						return convertStats(stats, false, true);
					case 2:  // READBACK
						return convertStats(stats, true, true);
					case 3:  // CUSTOM
						return convertStats(stats, true, false);
					case 4:  // GPUUPLOAD
						return convertStats(stats, true, true);
					default: // INVALID
						return convertStats(stats, false, false);
					}
				})
			| std::ranges::to<Array<DetailedMemoryStatistics::StatisticsBlock>>(),
		.total = convertStats(stats.Total, true, true)
	};
}

Generator<ResourceAllocationResult> DirectX12GraphicsFactory::allocate(Enumerable<const ResourceAllocationInfo&> ai, AllocationBehavior allocationBehavior, bool alias) const
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
		//       we can find, as it is assumed that they are all equal anyway. Similarly, we pick the first resource type only, if we're on heap tier 0.
		auto resourceUsageInfos = allocationInfos
			| std::views::transform([](auto& allocationInfo) {
				if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
				{
					auto& imageInfo = std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo);

					if (allocationInfo.Usage == ResourceUsage::RenderTarget || ::hasDepth(imageInfo.Format) || ::hasStencil(imageInfo.Format))
						return std::make_tuple(ResourceHeap::Resource, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);
					else
						return std::make_tuple(ResourceHeap::Resource, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);
				}
				else if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
					return std::make_tuple(std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo).Heap, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);
				else
					std::unreachable();
			})
			| std::views::take(1)
			| std::ranges::to<std::vector>();

		auto allocationDesc = getAllocationDesc(std::get<0>(resourceUsageInfos.front()), allocationBehavior);

		// If we are on resource heap tier 0, we need to find the appropriate resource type to allocate.
		if (m_impl->m_allocator->GetD3D12Options().ResourceHeapTier < D3D12_RESOURCE_HEAP_TIER_1)
			allocationDesc.ExtraHeapFlags = std::get<1>(resourceUsageInfos.front());

		// We need to manually perform the allocation, so we need to get the resource descriptions.
		auto resourceDescriptionData = allocationInfos
			| std::views::transform([](auto& allocationInfo) {
				UInt64 elementSize{}, elementAlignment{};

				if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
					return std::make_tuple(getResourceDesc(std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo), allocationInfo.Usage, elementSize, elementAlignment), elementSize, elementAlignment);
				else if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
					return std::make_tuple(getResourceDesc(std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo), allocationInfo.Usage), elementSize, elementAlignment);
				else
					std::unreachable();
			})
			| std::ranges::to<std::vector>();

		auto resourceDescriptions = resourceDescriptionData | std::views::transform([](auto& tuple) { return std::get<0>(tuple); }) | std::ranges::to<std::vector>();
		Array<D3D12_RESOURCE_ALLOCATION_INFO1> allocationDescriptions(resourceDescriptions.size());
		auto finalDesc = std::as_const(*device).handle()->GetResourceAllocationInfo2(0u, static_cast<UInt32>(resourceDescriptions.size()), resourceDescriptions.data(), allocationDescriptions.data());

		// Do the allocation.
		D3D12MA::Allocation* allocation{};
		auto result = m_impl->m_allocator->AllocateMemory(&allocationDesc, &finalDesc, &allocation);

		if (FAILED(result))
			throw DX12PlatformException(result, "Unable to allocate memory for aliasing resources.");

		if (allocation == nullptr || allocation->GetHeap() == nullptr)
			throw RuntimeException("Unable to allocate memory for aliasing resources.");

		auto allocationPtr = AllocationPtr(allocation, D3D12MADeleter{});

		for (const auto& [allocationInfo, resourceDescriptionInfo] : std::views::zip(allocationInfos, resourceDescriptionData))
		{
			ComPtr<ID3D12Resource> resource{};
			auto resourceDescription = std::get<0>(resourceDescriptionInfo);
			auto elementAlignment = std::get<2>(resourceDescriptionInfo);
			
			result = m_impl->m_allocator->CreateAliasingResource2(allocation, allocationInfo.AliasingOffset, &resourceDescription, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0u, nullptr, IID_PPV_ARGS(&resource));

			if (FAILED(result))
				throw DX12PlatformException(result, "Unable to allocate resource from memory reserved for aliasing resource block.");

			if (std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo))
			{
				const auto& bufferInfo = std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo);

				if (bufferInfo.Type == BufferType::Vertex && bufferInfo.VertexBufferLayout != nullptr)
					co_yield std::dynamic_pointer_cast<IBuffer>(DirectX12VertexBuffer::create(std::move(resource), dynamic_cast<const DirectX12VertexBufferLayout&>(*bufferInfo.VertexBufferLayout), bufferInfo.Elements, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
				else if (bufferInfo.Type == BufferType::Index && bufferInfo.IndexBufferLayout != nullptr)
					co_yield std::dynamic_pointer_cast<IBuffer>(DirectX12IndexBuffer::create(std::move(resource), dynamic_cast<const DirectX12IndexBufferLayout&>(*bufferInfo.IndexBufferLayout), bufferInfo.Elements, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
				else [[likely]]
					co_yield std::dynamic_pointer_cast<IBuffer>(DirectX12Buffer::create(std::move(resource), bufferInfo.Type, bufferInfo.Elements, bufferInfo.ElementSize, static_cast<size_t>(elementAlignment), allocationInfo.Usage, resourceDescription, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
			}
			else if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
			{
				const auto& imageInfo = std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo);
				co_yield std::dynamic_pointer_cast<IImage>(DirectX12Image::create(*device, std::move(resource), imageInfo.Size, imageInfo.Format, imageInfo.Dimensions, imageInfo.Levels, imageInfo.Layers, imageInfo.Samples, allocationInfo.Usage, resourceDescription, m_impl->m_allocator, allocationPtr, allocationInfo.Name));
			}
		}
	}
}

bool DirectX12GraphicsFactory::canAlias(Enumerable<const ResourceAllocationInfo&> allocationInfos) const
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

	// Resource heap tier 2 and above allow aliasing of arbitrary resource types.
	if (m_impl->m_allocator->GetD3D12Options().ResourceHeapTier >= D3D12_RESOURCE_HEAP_TIER_2)
		return true;

	// Count resource types to check if aliasing is allowed.
	std::bitset<3> resourceTypes;
	resourceTypes.set(0, std::ranges::any_of(allocationInfos, [](auto& allocationInfo) { return std::holds_alternative<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo); }));
	resourceTypes.set(1, std::ranges::any_of(allocationInfos, [](auto& allocationInfo) { return std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo); }));

	if (resourceTypes.test(1))
		resourceTypes.set(2, std::ranges::any_of(allocationInfos, [](auto& allocationInfo) {
			if (allocationInfo.Usage == ResourceUsage::RenderTarget)
				return true;

			if (!std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
				return false;

			auto& imageInfo = std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo);
			return ::hasDepth(imageInfo.Format) || ::hasStencil(imageInfo.Format);
		}));

	// If there's more than 1 resource type in the allocations, it's an unsupported mixing scenario.
	return resourceTypes.count() <= 1;
}

SharedPtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createBuffer("", type, heap, elementSize, elements, usage, allocationBehavior);
}

SharedPtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return m_impl->allocateBuffer(name, { type, elementSize, elements, heap }, usage, allocationBehavior, DirectX12Buffer::allocate);
}

SharedPtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createVertexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Vertex,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.VertexBufferLayout = layout.shared_from_this()
	};

	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, DirectX12VertexBuffer::allocate);
}

SharedPtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createIndexBuffer("", layout, heap, elements, usage, allocationBehavior);
}

SharedPtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Index,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.IndexBufferLayout = layout.shared_from_this()
	};

	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, DirectX12IndexBuffer::allocate);
}

SharedPtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

SharedPtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::ImageInfo imageInfo = {
		.Format = format,
		.Dimensions = dimension,
		.Size = size,
		.Levels = levels,
		.Layers = layers,
		.Samples = samples
	};

	return m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, DirectX12Image::allocate);
}

bool DirectX12GraphicsFactory::tryCreateBuffer(SharedPtr<IDirectX12Buffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateBuffer(buffer, "", type, heap, elementSize, elements, usage, allocationBehavior);
}

bool DirectX12GraphicsFactory::tryCreateBuffer(SharedPtr<IDirectX12Buffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return m_impl->allocateBuffer(name, { type, elementSize, elements, heap }, usage, allocationBehavior, DirectX12Buffer::tryAllocate, buffer);
}

bool DirectX12GraphicsFactory::tryCreateVertexBuffer(SharedPtr<IDirectX12VertexBuffer>& buffer, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateVertexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool DirectX12GraphicsFactory::tryCreateVertexBuffer(SharedPtr<IDirectX12VertexBuffer>& buffer, const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Vertex,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.VertexBufferLayout = layout.shared_from_this()
	};

	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, DirectX12VertexBuffer::tryAllocate, buffer);
}

bool DirectX12GraphicsFactory::tryCreateIndexBuffer(SharedPtr<IDirectX12IndexBuffer>& buffer, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateIndexBuffer(buffer, "", layout, heap, elements, usage, allocationBehavior);
}

bool DirectX12GraphicsFactory::tryCreateIndexBuffer(SharedPtr<IDirectX12IndexBuffer>& buffer, const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::BufferInfo bufferInfo = {
		.Type = BufferType::Index,
		.ElementSize = layout.elementSize(),
		.Elements = elements,
		.Heap = heap,
		.IndexBufferLayout = layout.shared_from_this()
	};

	return m_impl->allocateBuffer(name, bufferInfo, usage, allocationBehavior, DirectX12IndexBuffer::tryAllocate, buffer);
}

bool DirectX12GraphicsFactory::tryCreateTexture(SharedPtr<IDirectX12Image>& image, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return this->tryCreateTexture(image, "", format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

bool DirectX12GraphicsFactory::tryCreateTexture(SharedPtr<IDirectX12Image>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	ResourceAllocationInfo::ImageInfo imageInfo = {
		.Format = format,
		.Dimensions = dimension,
		.Size = size,
		.Levels = levels,
		.Layers = layers,
		.Samples = samples
	};

	return m_impl->allocateImage(name, imageInfo, usage, allocationBehavior, DirectX12Image::tryAllocate, image);
}

Generator<SharedPtr<IDirectX12Image>> DirectX12GraphicsFactory::createTextures(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const
{
	return [](SharedPtr<const DirectX12GraphicsFactory> factory, Format format, Size3d size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) -> Generator<SharedPtr<IDirectX12Image>> {
		for (;;)
			co_yield factory->createTexture(format, size, dimension, levels, layers, samples, usage, allocationBehavior);
	}(this->shared_from_this(), format, size, dimension, levels, layers, samples, usage, allocationBehavior);
}

SharedPtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return DirectX12Sampler::allocate(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

SharedPtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return DirectX12Sampler::allocate(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
}

Generator<SharedPtr<IDirectX12Sampler>> DirectX12GraphicsFactory::createSamplers(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [](SharedPtr<const DirectX12GraphicsFactory> factory, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) -> Generator<SharedPtr<IDirectX12Sampler>> {
		for (;;)
			co_yield factory->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}(this->shared_from_this(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

UniquePtr<DirectX12BottomLevelAccelerationStructure> DirectX12GraphicsFactory::createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<DirectX12BottomLevelAccelerationStructure>(flags, name);
}

UniquePtr<DirectX12TopLevelAccelerationStructure> DirectX12GraphicsFactory::createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<DirectX12TopLevelAccelerationStructure>(flags, name);
}