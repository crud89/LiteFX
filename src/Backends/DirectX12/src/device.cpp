#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Device::DirectX12DeviceImpl {
public:
	friend class DirectX12Device;

private:
	SharedPtr<const DirectX12GraphicsAdapter> m_adapter;
	DeviceState m_deviceState;
	UniquePtr<DirectX12Surface> m_surface;
	SharedPtr<DirectX12Queue> m_graphicsQueue{}, m_transferQueue{}, m_computeQueue{};
	Array<SharedPtr<DirectX12Queue>> m_queues;
	SharedPtr<DirectX12GraphicsFactory> m_factory;
	ComPtr<ID3D12InfoQueue1> m_eventQueue;
	UniquePtr<DirectX12SwapChain> m_swapChain;
	DWORD m_debugCallbackCookie = 0;
	UInt32 m_globalBufferHeapSize, m_globalSamplerHeapSize, m_bufferDescriptorIncrement{ 0 }, m_samplerDescriptorIncrement{ 0 }, m_bufferOffset{ 0 }, m_samplerOffset{ 0 };
	ComPtr<ID3D12DescriptorHeap> m_globalBufferHeap, m_globalSamplerHeap;
	mutable std::mutex m_bufferBindMutex;
	Array<std::pair<UInt32, UInt32>> m_bufferDescriptorFragments, m_samplerDescriptorFragments;
	Array<Tuple<DescriptorHeapType, UInt32, UInt32>> m_externallyAllocatedDescriptorRanges;
	ComPtr<ID3D12CommandSignature> m_dispatchSignature, m_drawSignature, m_drawIndexedSignature, m_dispatchMeshSignature;

public:
	DirectX12DeviceImpl(const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, UInt32 globalBufferHeapSize, UInt32 globalSamplerHeapSize) :
		m_adapter(adapter.shared_from_this()), m_surface(std::move(surface)), m_globalBufferHeapSize(globalBufferHeapSize), m_globalSamplerHeapSize(globalSamplerHeapSize)
	{
		if (m_surface == nullptr)
			throw ArgumentNotInitializedException("surface", "The surface must be initialized.");

		if (globalSamplerHeapSize > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE) [[unlikely]]
			throw ArgumentOutOfRangeException("globalSamplerHeapSize", std::make_pair<UInt32, UInt32>(0, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE), globalSamplerHeapSize, "Only 2048 samplers are allowed in the global sampler heap, but {0} have been specified.", globalSamplerHeapSize);
	}

	DirectX12DeviceImpl(DirectX12DeviceImpl&&) noexcept = delete;
	DirectX12DeviceImpl(const DirectX12DeviceImpl&) = delete;
	DirectX12DeviceImpl& operator=(DirectX12DeviceImpl&&) noexcept = delete;
	DirectX12DeviceImpl& operator=(const DirectX12DeviceImpl&) = delete;
	~DirectX12DeviceImpl() noexcept = default;

#ifndef NDEBUG
private:
	static void __stdcall onDebugMessage(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void* /*context*/)
	{
		String t = "";

		switch (category)
		{
			case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED: t = "APPLICATION"; break;
			case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS: t = "MISCELLANEOUS"; break;
			case D3D12_MESSAGE_CATEGORY_INITIALIZATION: t = "INITIALIZATION"; break;
			case D3D12_MESSAGE_CATEGORY_CLEANUP: t = "CLEANUP"; break;
			case D3D12_MESSAGE_CATEGORY_COMPILATION: t = "COMPILER"; break;
			case D3D12_MESSAGE_CATEGORY_STATE_CREATION: t = "CREATE_STATE"; break;
			case D3D12_MESSAGE_CATEGORY_STATE_SETTING: t = "SET_STATE"; break;
			case D3D12_MESSAGE_CATEGORY_STATE_GETTING: t = "GET_STATE"; break;
			case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION: t = "RESOURCE"; break;
			case D3D12_MESSAGE_CATEGORY_EXECUTION: t = "EXECUTION"; break;
			case D3D12_MESSAGE_CATEGORY_SHADER: t = "SHADER"; break;
			default: t = "OTHER";  break;
		}

		switch (severity)
		{
		case D3D12_MESSAGE_SEVERITY_CORRUPTION: LITEFX_FATAL_ERROR(DIRECTX12_LOG, "{1} ({2}): {0}", description, t, id); break;
		case D3D12_MESSAGE_SEVERITY_ERROR: LITEFX_ERROR(DIRECTX12_LOG, "{1} ({2}): {0}", description, t, id); break;
		case D3D12_MESSAGE_SEVERITY_WARNING: LITEFX_WARNING(DIRECTX12_LOG, "{1} ({2}): {0}", description, t, id); break;
		case D3D12_MESSAGE_SEVERITY_INFO: LITEFX_INFO(DIRECTX12_LOG, "{1} ({2}): {0}", description, t, id); break;
		default:
		case D3D12_MESSAGE_SEVERITY_MESSAGE: LITEFX_TRACE(DIRECTX12_LOG, "{1} ({2}): {0}", description, t, id); break;
		}
	}
#endif

private:
	void checkRequiredExtensions(ID3D12Device10* device, const GraphicsDeviceFeatures& features)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 {};
		raiseIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)), "Unable to query device extensions.");
		raiseIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)), "Unable to query device extensions.");
		raiseIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)), "Unable to query device extensions.");
		
		if (features.RayTracing && options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
			throw RuntimeException("The device does not support hardware ray-tracing.");
		if (features.RayQueries && options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_1)
			throw RuntimeException("The device does not support ray-queries and inline ray-tracing.");
		if (features.MeshShaders && options7.MeshShaderTier < D3D12_MESH_SHADER_TIER_1)
			throw RuntimeException("The device does not support mesh shaders.");
	}

public:
	[[nodiscard]]
	ComPtr<ID3D12Device10> initialize(const GraphicsDeviceFeatures& features)
	{
		ComPtr<ID3D12Device10> device;

		// Require feature level 12.1 and express optional features of higher feature levels as device features.
		raiseIfFailed(::D3D12CreateDevice(m_adapter->handle().Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)), "Unable to create DirectX 12 device.");
		this->checkRequiredExtensions(device.Get(), features);

#ifndef NDEBUG
		// Try to query an info queue to forward log messages.
		ComPtr<ID3D12InfoQueue> infoQueue;

		if (FAILED(device.As(&infoQueue)))
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query info queue. Debugger support will be disabled.");
		else
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, FALSE);
			
			// Suppress individual messages by their ID
			auto suppressIds = std::array { 
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // Mismatch in clear value is intended.
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE, // Mismatch in clear value is intended.
			};
			auto severities = std::array { D3D12_MESSAGE_SEVERITY_INFO };	// Somehow it is required to deny info-level messages. Otherwise strange pointer issues are occurring.

			D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
			infoQueueFilter.DenyList.NumIDs = static_cast<UINT>(suppressIds.size());
			infoQueueFilter.DenyList.pIDList = suppressIds.data();
			infoQueueFilter.DenyList.NumSeverities = static_cast<UINT>(severities.size());
			infoQueueFilter.DenyList.pSeverityList = severities.data();

			raiseIfFailed(infoQueue->PushStorageFilter(&infoQueueFilter), "Unable to push message filter to info queue.");

			// Try to register event callback.
			if (FAILED(infoQueue.As(&m_eventQueue)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to query debug message callback queue. Native event logging will be disabled. Note that it requires at least Windows 11.");
			else if (FAILED(m_eventQueue->RegisterMessageCallback(&DirectX12Device::DirectX12DeviceImpl::onDebugMessage, D3D12_MESSAGE_CALLBACK_FLAGS::D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_debugCallbackCookie)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to register debug message callback with info queue. Native event logging will be disabled.");
		}
#endif

		// Create global buffer and sampler descriptor heaps.
		D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {};
		bufferHeapDesc.NumDescriptors = m_globalBufferHeapSize;
		bufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		bufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		raiseIfFailed(device->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&m_globalBufferHeap)), "Unable create global GPU descriptor heap for buffers.");
		m_bufferDescriptorIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = m_globalSamplerHeapSize;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		raiseIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_globalSamplerHeap)), "Unable create global GPU descriptor heap for samplers.");
		m_samplerDescriptorIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

#ifndef NDEBUG
		m_globalBufferHeap->SetName(L"Global Descriptor Heap");
		m_globalSamplerHeap->SetName(L"Global Sampler Heap");
#endif

		// Initialize command signatures for indirect drawing.
		D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = { .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH };
		D3D12_COMMAND_SIGNATURE_DESC signatureDesc = { .ByteStride = sizeof(IndirectDispatchBatch), .NumArgumentDescs = 1, .pArgumentDescs = &argumentDesc, .NodeMask = 0x00 };
		raiseIfFailed(device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_dispatchSignature)), "Unable to create indirect dispatch command signature.");
		argumentDesc = { .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH };
		signatureDesc = { .ByteStride = sizeof(IndirectIndexedBatch), .NumArgumentDescs = 1, .pArgumentDescs = &argumentDesc, .NodeMask = 0x00 };
		raiseIfFailed(device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_dispatchMeshSignature)), "Unable to create indirect mesh shader dispatch command signature.");
		argumentDesc = { .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW };
		signatureDesc = { .ByteStride = sizeof(IndirectBatch), .NumArgumentDescs = 1, .pArgumentDescs = &argumentDesc, .NodeMask = 0x00 };
		raiseIfFailed(device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawSignature)), "Unable to create indirect draw command signature.");
		argumentDesc = { .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED };
		signatureDesc = { .ByteStride = sizeof(IndirectIndexedBatch), .NumArgumentDescs = 1, .pArgumentDescs = &argumentDesc, .NodeMask = 0x00 };
		raiseIfFailed(device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawIndexedSignature)), "Unable to create indirect indexed draw command signature.");

		return device;
	}

	void createQueues(const DirectX12Device& device)
	{
		//m_graphicsQueue = this->createQueue(device, QueueType::Graphics, QueuePriority::Realtime);
		m_graphicsQueue = this->createQueue(device, QueueType::Graphics, QueuePriority::High);
		m_transferQueue = this->createQueue(device, QueueType::Transfer, QueuePriority::High);
		m_computeQueue  = this->createQueue(device, QueueType::Compute,  QueuePriority::High);
	}

	SharedPtr<DirectX12Queue> createQueue(const DirectX12Device& device, QueueType type, QueuePriority priority)
	{
		return m_queues.emplace_back(DirectX12Queue::create(device, type, priority));
	}

public:
	Array<Format> getSurfaceFormats() const
	{
		// NOTE: Those formats are actually the only ones that are supported for flip-model swap chains, which is currently the only 
		//       supported swap effect. If other swap effects are used, this function may require redesign. For more information see: 
		//       https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1#remarks.
		Array<Format> surfaceFormats = {
			DX12::getFormat(DXGI_FORMAT_R16G16B16A16_FLOAT),
			DX12::getFormat(DXGI_FORMAT_R10G10B10A2_UNORM),
			DX12::getFormat(DXGI_FORMAT_B8G8R8A8_UNORM)
		};

		return surfaceFormats;
	}

	Tuple<UInt32, UInt32> allocateDescriptors(DescriptorHeapType heap, UInt32 descriptors, bool external)
	{
		UInt32 offset{};

		switch (heap)
		{
		case DescriptorHeapType::Resource:
			if (m_bufferOffset + descriptors <= m_globalBufferHeapSize) [[likely]]
			{
				offset = m_bufferOffset;
				m_bufferOffset += descriptors;
			}
			else [[unlikely]]
			{
				// Find a fitting offset from the fragment heap.
				if (auto match = std::ranges::find_if(m_bufferDescriptorFragments, [&descriptors](const auto& pair) { return pair.second == descriptors; }); match != m_bufferDescriptorFragments.end())
				{
					offset = match->first;
					m_bufferDescriptorFragments.erase(match);
				}
				else if (match = std::ranges::find_if(m_bufferDescriptorFragments, [&descriptors](const auto& pair) { return pair.second > descriptors; }); match != m_bufferDescriptorFragments.end())
				{
					offset = match->first;
					match->first += descriptors;
					match->second -= descriptors;
				}
				else [[unlikely]]
				{
					throw RuntimeException("Unable to allocate more descriptors on global buffer heap.");
				}
			}

			break;
		case DescriptorHeapType::Sampler:
			if (m_samplerOffset + descriptors <= m_globalSamplerHeapSize) [[likely]]
			{
				offset = m_samplerOffset;
				m_samplerOffset += descriptors;
			}
			else [[unlikely]]
			{
				// Find a fitting offset from the fragment heap.
				if (auto match = std::ranges::find_if(m_samplerDescriptorFragments, [&descriptors](const auto& pair) { return pair.second == descriptors; }); match != m_samplerDescriptorFragments.end())
				{
					offset = match->first;
					m_samplerDescriptorFragments.erase(match);
				}
				else if (match = std::ranges::find_if(m_samplerDescriptorFragments, [&descriptors](const auto& pair) { return pair.second > descriptors; }); match != m_samplerDescriptorFragments.end())
				{
					offset = match->first;
					match->first += descriptors;
					match->second -= descriptors;
				}
				else [[unlikely]]
				{
					throw RuntimeException("Unable to allocate more descriptors on global sampler heap.");
				}
			}

			break;
		default:
			LITEFX_WARNING(DIRECTX12_LOG, "The descriptor heap type must be one of the following: {{ `Resource`, `Sampler` }}, but it was: `{0}`.", heap);
			return { std::numeric_limits<UInt32>::max(), 0u };
		}

		// Remember the segments allocated for external use.
		if (external)
			m_externallyAllocatedDescriptorRanges.emplace_back(heap, offset, descriptors);

		return { offset, descriptors };
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Device::DirectX12Device(const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, UInt32 globalBufferHeapSize, UInt32 globalSamplerHeapSize) :
	ComResource<ID3D12Device10>(nullptr), m_impl(adapter, std::move(surface), globalBufferHeapSize, globalSamplerHeapSize)
{
	LITEFX_DEBUG(DIRECTX12_LOG, "Creating DirectX 12 device {{ Surface: {0}, Adapter: {1} }}...", static_cast<void*>(&surface), adapter.deviceId());
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(DIRECTX12_LOG, "Vendor: {0:#0x} (\"{1}\")", adapter.vendorId(), DX12::getVendorName(adapter.vendorId()).c_str());
	LITEFX_DEBUG(DIRECTX12_LOG, "Driver Version: {0:#0x}", adapter.driverVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "API Version: {0:#0x}", adapter.apiVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "Dedicated Memory: {0} Bytes", adapter.dedicatedMemory());
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(DIRECTX12_LOG, "Descriptor Heap Size: {0}", globalBufferHeapSize);
	LITEFX_DEBUG(DIRECTX12_LOG, "Sampler Heap Size: {0}", globalSamplerHeapSize);
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
}

DirectX12Device::~DirectX12Device() noexcept = default;

SharedPtr<DirectX12Device> DirectX12Device::initialize(const DirectX12Backend& backend, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync, GraphicsDeviceFeatures features)
{
	this->handle() = m_impl->initialize(features);
	m_impl->createQueues(*this);
	m_impl->m_factory = DirectX12GraphicsFactory::create(*this);
	m_impl->m_swapChain = UniquePtr<DirectX12SwapChain>(new DirectX12SwapChain(*this, backend, format, renderArea, backBuffers, enableVsync));

	return this->shared_from_this();
}

void DirectX12Device::release() noexcept
{
	// Clear the device state.
	m_impl->m_deviceState.clear();

	// Unregister the event queue.
	if (m_impl->m_eventQueue != nullptr && m_impl->m_debugCallbackCookie != 0)
		m_impl->m_eventQueue->UnregisterMessageCallback(m_impl->m_debugCallbackCookie);

	m_impl->m_swapChain.reset();
	m_impl->m_queues.clear();
	m_impl->m_transferQueue.reset();
	m_impl->m_computeQueue.reset();
	m_impl->m_graphicsQueue.reset();
	m_impl->m_surface.reset();
	m_impl->m_factory.reset();
}

const ID3D12DescriptorHeap* DirectX12Device::globalBufferHeap() const noexcept
{
	return m_impl->m_globalBufferHeap.Get();
}

const ID3D12DescriptorHeap* DirectX12Device::globalSamplerHeap() const noexcept
{
	return m_impl->m_globalSamplerHeap.Get();
}

void DirectX12Device::allocateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, DescriptorHeapType heapType, UInt32& offset, UInt32& size) const
{
	// NOTE: Freeing descriptor sets with leaves the heap(s) in fragmented state. This should be prevented, however we also keep track of the released offset/count pairs to re-allocate 
	//       them later. Re-allocation could follow those steps:
	//       - First, try to append to the current descriptor range.
	//       - If we're overflowing: find perfect offset/pair matches for the requested set.
	//       - If none is available: allocate from a fragmented area. Resize it afterwards with a new offset and reduced count.
	//       - If all of the above fail, then we're out of descriptors.
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	// Get the current descriptor sizes and compute the offsets.
	// NOTE: The descriptor set layout checks for invalid mixture of samplers and resources, so we only get one or the other here.
	size = descriptorSet.localHeap(heapType)->GetDesc().NumDescriptors;
	offset = std::numeric_limits<UInt32>::max();

	if (size == 0)
		throw InvalidArgumentException("descriptorSet", "Cannot allocate space for empty descriptor set on global descriptor heap.");

	auto [o, s] = m_impl->allocateDescriptors(heapType, size, false);
	size = s;
	offset = o;
}

Tuple<UInt32, UInt32> DirectX12Device::allocateGlobalDescriptors(UInt32 descriptors, DescriptorHeapType heapType) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	return m_impl->allocateDescriptors(heapType, descriptors, true);
}

void DirectX12Device::releaseGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	if (descriptorSet.layout().bindsSamplers())
		m_impl->m_samplerDescriptorFragments.emplace_back(descriptorSet.globalHeapOffset(DescriptorHeapType::Sampler), descriptorSet.globalHeapAddressRange(DescriptorHeapType::Sampler));

	if (descriptorSet.layout().bindsResources())
		m_impl->m_bufferDescriptorFragments.emplace_back(descriptorSet.globalHeapOffset(DescriptorHeapType::Resource), descriptorSet.globalHeapAddressRange(DescriptorHeapType::Resource));
}

void DirectX12Device::releaseGlobalDescriptors(DescriptorHeapType heapType, UInt32 offset, UInt32 descriptors) const
{
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	// Check if the address range has been externally allocated.
	auto match = std::ranges::find_if(m_impl->m_externallyAllocatedDescriptorRanges, [heapType, offset, descriptors](const auto& range) { 
		auto [t, o, s] = range;
		return t == heapType && o == offset && s == descriptors;
	});

	if (match == m_impl->m_externallyAllocatedDescriptorRanges.end()) [[unlikely]]
		throw InvalidArgumentException("offset", "No externally allocated descriptor range was found at offset {0} with {1} descriptors in heap {2}.", offset, descriptors, heapType);

	if (heapType == DescriptorHeapType::Resource)
		m_impl->m_bufferDescriptorFragments.emplace_back(offset, descriptors);
	else if (heapType == DescriptorHeapType::Sampler)
		m_impl->m_samplerDescriptorFragments.emplace_back(offset, descriptors);

	m_impl->m_externallyAllocatedDescriptorRanges.erase(match);
}

void DirectX12Device::updateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const
{
	auto firstDescriptor = descriptorSet.layout().getDescriptorOffset(binding, offset);

	// Bind the descriptor to the appropriate type. Note that static samplers aren't bound, so effectively this call is invalid. However we simply treat it as a no-op.
	auto descriptorLayout = descriptorSet.layout().descriptor(binding);

	if (descriptorLayout.descriptorType() == DescriptorType::Sampler && descriptorLayout.staticSampler() == nullptr)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(m_impl->m_globalSamplerHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptorSet.globalHeapOffset(DescriptorHeapType::Sampler) + firstDescriptor), m_impl->m_samplerDescriptorIncrement);
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.localHeap(DescriptorHeapType::Sampler)->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(firstDescriptor), m_impl->m_samplerDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(descriptors, targetHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
	else if (descriptorLayout.descriptorType() != DescriptorType::Sampler)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(m_impl->m_globalBufferHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptorSet.globalHeapOffset(DescriptorHeapType::Resource) + firstDescriptor), m_impl->m_bufferDescriptorIncrement);
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.localHeap(DescriptorHeapType::Resource)->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(firstDescriptor), m_impl->m_bufferDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(descriptors, targetHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

void DirectX12Device::bindDescriptorSet(const DirectX12CommandBuffer& commandBuffer, const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept
{
	// Deduct, whether to set the graphics or compute descriptor tables.
	// TODO: Maybe we could store a simple boolean on the pipeline state to make this easier.
	const bool isGraphicsSet = dynamic_cast<const DirectX12RenderPipeline*>(&pipeline) != nullptr;

	// Copy the descriptors to the global heaps and set the root table parameters.
	if (descriptorSet.layout().bindsSamplers())
	{
		// Get the root parameter index.
		auto rootParameterIndex = pipeline.layout()->rootParameterIndex(descriptorSet.layout(), DescriptorHeapType::Sampler);

		if (!rootParameterIndex.has_value()) [[unlikely]]
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to bind descriptor set at space {}, as the parent pipeline was not defined with a descriptor set.", descriptorSet.layout().space());
		else
		{
			// The parameter index equals the target descriptor set space.
			CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalSamplerHeap->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptorSet.globalHeapOffset(DescriptorHeapType::Sampler)), m_impl->m_samplerDescriptorIncrement);

			if (isGraphicsSet)
				commandBuffer.handle()->SetGraphicsRootDescriptorTable(rootParameterIndex.value(), targetGpuHandle);
			else
				commandBuffer.handle()->SetComputeRootDescriptorTable(rootParameterIndex.value(), targetGpuHandle);
		}
	}

	if (descriptorSet.layout().bindsResources())
	{
		// Get the root parameter index.
		auto rootParameterIndex = pipeline.layout()->rootParameterIndex(descriptorSet.layout(), DescriptorHeapType::Resource);

		if (!rootParameterIndex.has_value()) [[unlikely]]
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to bind descriptor set at space {}, as the parent pipeline was not defined with a descriptor set.", descriptorSet.layout().space());
		else
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalBufferHeap->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(descriptorSet.globalHeapOffset(DescriptorHeapType::Resource)), m_impl->m_bufferDescriptorIncrement);

			if (isGraphicsSet)
				commandBuffer.handle()->SetGraphicsRootDescriptorTable(rootParameterIndex.value(), targetGpuHandle);
			else
				commandBuffer.handle()->SetComputeRootDescriptorTable(rootParameterIndex.value(), targetGpuHandle);
		}
	}
}

void DirectX12Device::bindGlobalDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	const auto globalHeaps = std::array { m_impl->m_globalBufferHeap.Get(), m_impl->m_globalSamplerHeap.Get() };
	commandBuffer.handle()->SetDescriptorHeaps(static_cast<UINT>(globalHeaps.size()), globalHeaps.data());
}

void DirectX12Device::indirectDrawSignatures(ComPtr<ID3D12CommandSignature>& dispatchSignature, ComPtr<ID3D12CommandSignature>& dispatchMeshSignature, ComPtr<ID3D12CommandSignature>& drawSignature, ComPtr<ID3D12CommandSignature>& drawIndexedSignature) const noexcept
{
	dispatchSignature = m_impl->m_dispatchSignature;
	dispatchMeshSignature = m_impl->m_dispatchMeshSignature;
	drawSignature = m_impl->m_drawSignature;
	drawIndexedSignature = m_impl->m_drawIndexedSignature;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
DirectX12RenderPassBuilder DirectX12Device::buildRenderPass(UInt32 commandBuffers) const
{
	return DirectX12RenderPassBuilder(*this, commandBuffers);
}

DirectX12RenderPassBuilder DirectX12Device::buildRenderPass(const String& name, UInt32 commandBuffers) const
{
	return DirectX12RenderPassBuilder(*this, commandBuffers, name);
}

DirectX12RenderPipelineBuilder DirectX12Device::buildRenderPipeline(const DirectX12RenderPass& renderPass, const String& name) const
{
	return DirectX12RenderPipelineBuilder(renderPass, name);
}

DirectX12ComputePipelineBuilder DirectX12Device::buildComputePipeline(const String& name) const
{
	return DirectX12ComputePipelineBuilder(*this, name);
}

DirectX12RayTracingPipelineBuilder DirectX12Device::buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const
{
	return this->buildRayTracingPipeline("", std::move(shaderRecords));
}

DirectX12RayTracingPipelineBuilder DirectX12Device::buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const
{
	return DirectX12RayTracingPipelineBuilder(*this, std::move(shaderRecords), name);
}

DirectX12PipelineLayoutBuilder DirectX12Device::buildPipelineLayout() const
{
	return { *this };
}

DirectX12InputAssemblerBuilder DirectX12Device::buildInputAssembler() const
{
	return DirectX12InputAssemblerBuilder();
}

DirectX12RasterizerBuilder DirectX12Device::buildRasterizer() const
{
	return DirectX12RasterizerBuilder();
}

DirectX12ShaderProgramBuilder DirectX12Device::buildShaderProgram() const
{
	return DirectX12ShaderProgramBuilder(*this);
}

DirectX12BarrierBuilder DirectX12Device::buildBarrier() const
{
	return DirectX12BarrierBuilder();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)

DirectX12SwapChain& DirectX12Device::swapChain() noexcept
{
	return *m_impl->m_swapChain;
}

DeviceState& DirectX12Device::state() const noexcept
{
	return m_impl->m_deviceState;
}

const DirectX12SwapChain& DirectX12Device::swapChain() const noexcept
{
	return *m_impl->m_swapChain;
}

const DirectX12Surface& DirectX12Device::surface() const noexcept
{
	return *m_impl->m_surface;
}

const DirectX12GraphicsAdapter& DirectX12Device::adapter() const noexcept
{
	return *m_impl->m_adapter;
}

const DirectX12GraphicsFactory& DirectX12Device::factory() const noexcept
{
	return *m_impl->m_factory;
}

const DirectX12Queue& DirectX12Device::defaultQueue(QueueType type) const
{
	// If the type contains the graphics flag, always return the graphics queue.
	if (LITEFX_FLAG_IS_SET(type, QueueType::Graphics))
		return *m_impl->m_graphicsQueue;
	else if (LITEFX_FLAG_IS_SET(type, QueueType::Compute))
		return *m_impl->m_computeQueue;
	else if (LITEFX_FLAG_IS_SET(type, QueueType::Transfer))
		return *m_impl->m_transferQueue;
	else
		throw InvalidArgumentException("type", "No default queue for the provided queue type has was found.");
}

SharedPtr<const DirectX12Queue> DirectX12Device::createQueue(QueueType type, QueuePriority priority)
{
	return m_impl->createQueue(*this, type, priority);
}

UniquePtr<DirectX12Barrier> DirectX12Device::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const
{
	return makeUnique<DirectX12Barrier>(syncBefore, syncAfter);
}

SharedPtr<DirectX12FrameBuffer> DirectX12Device::makeFrameBuffer(StringView name, const Size2d& renderArea) const
{
	return DirectX12FrameBuffer::create(*this, renderArea, name);
}

MultiSamplingLevel DirectX12Device::maximumMultiSamplingLevel(Format format) const noexcept
{
	constexpr auto allLevels = std::array { MultiSamplingLevel::x64, MultiSamplingLevel::x32, MultiSamplingLevel::x16, MultiSamplingLevel::x8, MultiSamplingLevel::x4, MultiSamplingLevel::x2, MultiSamplingLevel::x1 };
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{ .Format = DX12::getFormat(format) };

	for (size_t level(0); level < allLevels.size(); ++level)
	{
		levels.SampleCount = std::to_underlying(allLevels[level]); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		
		if (FAILED(this->handle()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))))
			continue;

		if (levels.NumQualityLevels > 0)
			return allLevels[level]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
	}

	LITEFX_WARNING(DIRECTX12_LOG, "No supported multi-sampling levels could be queried. Assuming that multi-sampling is disabled.");
	return MultiSamplingLevel::x1;
}

double DirectX12Device::ticksPerMillisecond() const noexcept
{
	UInt64 frequency{};
	std::as_const(*m_impl->m_graphicsQueue).handle()->GetTimestampFrequency(&frequency);
	return static_cast<double>(frequency) / 1000.0; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

void DirectX12Device::wait() const
{
	using event_type = std::tuple<HANDLE, ComPtr<ID3D12Fence>>;

	// Insert a fence into each queue.
	Array<event_type> events(m_impl->m_queues.size());
	std::ranges::generate(events, [this, i = 0]() mutable -> event_type {
	ComPtr<ID3D12Fence> fence;
	raiseIfFailed(this->handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Unable to create queue synchronization fence.");
	
	// Create a signal event.
	HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);

		if (eventHandle == nullptr)
			throw RuntimeException("Unable to create event handle for fence.");

	HRESULT hr = fence->SetEventOnCompletion(1, eventHandle);

	if (FAILED(hr))
	{
		::CloseHandle(eventHandle);
		raiseIfFailed(hr, "Unable to register queue synchronization fence completion event.");
	}

	// Signal the event value on the graphics queue.
		hr = std::as_const(*m_impl->m_queues[i++]).handle()->Signal(fence.Get(), 1);
	
	if (FAILED(hr))
	{
		::CloseHandle(eventHandle);
		raiseIfFailed(hr, "Unable to wait for queue synchronization fence.");
	}

		return { eventHandle, std::move(fence) };
	});

	// Wait for all the fences.
	std::ranges::for_each(events, [](event_type& e) {
		auto eventHandle = std::get<0>(e);

		if (std::get<1>(e)->GetCompletedValue() < 1)
		::WaitForSingleObject(eventHandle, INFINITE);

		// Close the handle.
	::CloseHandle(eventHandle);
	});
}

void DirectX12Device::computeAccelerationStructureSizes(const DirectX12BottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const
{
	auto descriptions = blas.buildInfo();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
		.Flags = std::bit_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(blas.flags()),
		.NumDescs = static_cast<UInt32>(descriptions.size()),
		.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
		.pGeometryDescs = descriptions.data()
	};

	// Get the prebuild info and align the buffer sizes.
	this->handle()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
	bufferSize = (prebuildInfo.ResultDataMaxSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	if (forUpdate)
		scratchSize = (prebuildInfo.UpdateScratchDataSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	else
		scratchSize = (prebuildInfo.ScratchDataSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

void DirectX12Device::computeAccelerationStructureSizes(const DirectX12TopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const 
{
	auto& instances = tlas.instances();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {
        .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
        .Flags = std::bit_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(tlas.flags()),
        .NumDescs = static_cast<UInt32>(instances.size()),
        .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY
    };

	// Get the prebuild info and align the buffer sizes.
	this->handle()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
	bufferSize = (prebuildInfo.ResultDataMaxSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	if (forUpdate)
		scratchSize = (prebuildInfo.UpdateScratchDataSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	else
		scratchSize = (prebuildInfo.ScratchDataSizeInBytes + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}