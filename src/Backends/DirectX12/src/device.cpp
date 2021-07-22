#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Device::DirectX12DeviceImpl : public Implement<DirectX12Device> {
public:
	friend class DirectX12Device;

private:
	const DirectX12GraphicsAdapter& m_adapter;
	const DirectX12Surface& m_surface;
	const DirectX12Backend& m_backend;
	UniquePtr<DirectX12Queue> m_graphicsQueue, m_transferQueue, m_bufferQueue, m_computeQueue;
	UniquePtr<DirectX12GraphicsFactory> m_factory;
	UniquePtr<DirectX12ComputePipeline> m_blitPipeline;
	ComPtr<ID3D12InfoQueue1> m_eventQueue;
	UniquePtr<DirectX12SwapChain> m_swapChain;
	DWORD m_debugCallbackCookie = 0;
	UInt32 m_globalBufferHeapSize, m_globalSamplerHeapSize, m_bufferDescriptorIncrement, m_samplerDescriptorIncrement, m_bufferOffset{ 0 }, m_samplerOffset{ 0 };
	ComPtr<ID3D12DescriptorHeap> m_globalBufferHeap, m_globalSamplerHeap;
	mutable std::mutex m_bufferBindMutex;

public:
	DirectX12DeviceImpl(DirectX12Device* parent, const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend, const UInt32& globalBufferHeapSize, const UInt32& globalSamplerHeapSize) :
		base(parent), m_adapter(adapter), m_surface(surface), m_backend(backend), m_globalBufferHeapSize(globalBufferHeapSize), m_globalSamplerHeapSize(globalSamplerHeapSize)
	{
		if (globalSamplerHeapSize > 2048)
			throw ArgumentOutOfRangeException("Only 2048 samplers are allowed in the global sampler heap, but {0} have been specified.", globalSamplerHeapSize);
	}

	~DirectX12DeviceImpl() noexcept
	{
		if (m_eventQueue != nullptr & m_debugCallbackCookie != 0)
			m_eventQueue->UnregisterMessageCallback(m_debugCallbackCookie);

		m_swapChain = nullptr;
		m_graphicsQueue = nullptr;
		m_transferQueue = nullptr;
		m_bufferQueue = nullptr;
		m_computeQueue = nullptr;
	}

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

public:
	[[nodiscard]]
	ComPtr<ID3D12Device5> initialize()
	{
		ComPtr<ID3D12Device5> device;
		HRESULT hr;

		raiseIfFailed<RuntimeException>(::D3D12CreateDevice(m_adapter.handle().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)), "Unable to create DirectX 12 device.");

#ifndef NDEBUG
		// Try to query an info queue to forward log messages.
		ComPtr<ID3D12InfoQueue> infoQueue;

		if (FAILED(device.As(&infoQueue)))
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query info queue. Debugger support will be disabled disabled.");
		else
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);
			
			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID suppressIds[] = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE };
			D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };	// Somehow it is required to deny info-level messages. Otherwise strange pointer issues are occurring.

			D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
			infoQueueFilter.DenyList.NumIDs = _countof(suppressIds);
			infoQueueFilter.DenyList.pIDList = suppressIds;
			infoQueueFilter.DenyList.NumSeverities = _countof(severities);
			infoQueueFilter.DenyList.pSeverityList = severities;

			raiseIfFailed<RuntimeException>(infoQueue->PushStorageFilter(&infoQueueFilter), "Unable to push message filter to info queue.");

			// Try to register event callback.
			if (FAILED(infoQueue.As(&m_eventQueue)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to query debug message callback queue. Native event logging will be disabled. Note that it requires Windows 10 SDK 10.0.20236 or later.");
			else if (FAILED(m_eventQueue->RegisterMessageCallback(&DirectX12Device::DirectX12DeviceImpl::onDebugMessage, D3D12_MESSAGE_CALLBACK_FLAGS::D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &m_debugCallbackCookie)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to register debug message callback with info queue. Native event logging will be disabled.");
		}
#endif

		// Create global buffer and sampler descriptor heaps.
		D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {};
		bufferHeapDesc.NumDescriptors = m_globalBufferHeapSize;
		bufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		bufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		raiseIfFailed<RuntimeException>(device->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&m_globalBufferHeap)), "Unable create global GPU descriptor heap for buffers.");
		m_bufferDescriptorIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = m_globalSamplerHeapSize;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		raiseIfFailed<RuntimeException>(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_globalSamplerHeap)), "Unable create global GPU descriptor heap for samplers.");
		m_samplerDescriptorIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

#ifndef NDEBUG
		m_globalBufferHeap->SetName(L"Global Descriptor Heap");
		m_globalSamplerHeap->SetName(L"Global Sampler Heap");
#endif

		return device;
	}

	void createFactory()
	{
		m_factory = makeUnique<DirectX12GraphicsFactory>(*m_parent);
	}

	void createSwapChain(const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		m_swapChain = makeUnique<DirectX12SwapChain>(*m_parent, format, frameBufferSize, frameBuffers);
	}

	void createQueues()
	{
		//m_graphicsQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Graphics, QueuePriority::Realtime);
		m_graphicsQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Graphics, QueuePriority::High);
		m_transferQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Transfer, QueuePriority::Normal);
		m_bufferQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Transfer, QueuePriority::High);
		m_computeQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Compute, QueuePriority::High);
	}

	void createBlitPipeline()
	{
		try
		{
			//m_blitPipeline = m_parent->buildComputePipeline()
			//	.layout()
			//		.shaderProgram()
			//			.addComputeShaderModule("shaders/blit.dxi")
			//			.go()
			//		.addDescriptorSet(0)
			//			.addUniform(0, 16, 1)
			//			.addImage(1)
			//			.addStorage(2)
			//			.go()
			//		.addDescriptorSet(1)
			//			.addSampler(0)
			//			.go()
			//		.go()
			//	.go();
		}
		catch (Exception& ex)
		{
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to create blit pipeline. Blitting will not be available. Error: {0}", ex.what());
		}	
	}

public:
	Array<Format> getSurfaceFormats() const
	{
		// NOTE: Those formats are actually the only ones that are supported for flip-model swap chains, which is currently the only 
		//       supported swap effect. If other swap effects are used, this function may require redesign. For more information see: 
		//       https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1#remarks.
		Array<Format> surfaceFormats = {
			::getFormat(DXGI_FORMAT_R16G16B16A16_FLOAT),
			::getFormat(DXGI_FORMAT_R10G10B10A2_UNORM),
			::getFormat(DXGI_FORMAT_B8G8R8A8_UNORM)
		};

		return surfaceFormats;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Device::DirectX12Device(const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend) :
	DirectX12Device(adapter, surface, backend, Format::B8G8R8A8_SRGB, { 800, 600 }, 3)
{
}

DirectX12Device::DirectX12Device(const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, const UInt32& globalBufferHeapSize, const UInt32& globalSamplerHeapSize) :
	ComResource<ID3D12Device5>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this, adapter, surface, backend, globalBufferHeapSize, globalSamplerHeapSize))
{
	LITEFX_DEBUG(DIRECTX12_LOG, "Creating DirectX 12 device {{ Surface: {0}, Adapter: {1} }}...", fmt::ptr(&surface), adapter.getDeviceId());
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(DIRECTX12_LOG, "Vendor: {0:#0x} (\"{1}\")", adapter.getVendorId(), ::getVendorName(adapter.getVendorId()).c_str());
	LITEFX_DEBUG(DIRECTX12_LOG, "Driver Version: {0:#0x}", adapter.getDriverVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "API Version: {0:#0x}", adapter.getApiVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "Dedicated Memory: {0} Bytes", adapter.getDedicatedMemory());
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(DIRECTX12_LOG, "Descriptor Heap Size: {0}", globalBufferHeapSize);
	LITEFX_DEBUG(DIRECTX12_LOG, "Sampler Heap Size: {0}", globalSamplerHeapSize);
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");

	this->handle() = m_impl->initialize();
	m_impl->createQueues();
	m_impl->createFactory();
	m_impl->createSwapChain(format, frameBufferSize, frameBuffers);
	m_impl->createBlitPipeline();
}

DirectX12Device::~DirectX12Device() noexcept = default;

const DirectX12Backend& DirectX12Device::backend() const noexcept
{
	return m_impl->m_backend;
}

const ID3D12DescriptorHeap* DirectX12Device::globalBufferHeap() const noexcept
{
	return m_impl->m_globalBufferHeap.Get();
}

const ID3D12DescriptorHeap* DirectX12Device::globalSamplerHeap() const noexcept
{
	return m_impl->m_globalSamplerHeap.Get();
}

void DirectX12Device::updateGlobalDescriptors(const DirectX12CommandBuffer& commandBuffer, const DirectX12DescriptorSet& descriptorSet) const noexcept
{
	// TODO: Ring-buffer may not work here - we need to track which descriptor sets are bound to which descriptor offset, since otherwise we might overwrite static descriptors. 
	//       We could for example use a map for this and discard descriptor sets when they are `free`d.
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);
	
	// Get the current descriptor sizes and compute the offsets.
	UInt32 buffers{ 0 }, samplers{ 0 }, bufferOffset{ m_impl->m_bufferOffset }, samplerOffset{ m_impl->m_samplerOffset };

	if (descriptorSet.bufferHeap() != nullptr)
		buffers = descriptorSet.bufferHeap()->GetDesc().NumDescriptors;

	if (descriptorSet.samplerHeap() != nullptr)
		samplers = descriptorSet.samplerHeap()->GetDesc().NumDescriptors;

	if (bufferOffset + buffers > m_impl->m_globalBufferHeapSize) [[unlikely]]
		bufferOffset = 0;

	if (samplerOffset + samplers > m_impl->m_globalSamplerHeapSize) [[unlikely]]
		samplerOffset = 0;
	
	// Get the descriptor handles.
	CD3DX12_CPU_DESCRIPTOR_HANDLE targetBufferHandle(m_impl->m_globalBufferHeap->GetCPUDescriptorHandleForHeapStart(), bufferOffset, m_impl->m_bufferDescriptorIncrement);
	CD3DX12_CPU_DESCRIPTOR_HANDLE targetSamplerHandle(m_impl->m_globalSamplerHeap->GetCPUDescriptorHandleForHeapStart(), samplerOffset, m_impl->m_samplerDescriptorIncrement);

	// Copy the descriptors to the global heaps and set the root table parameters.
	if (buffers > 0)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.bufferHeap()->GetCPUDescriptorHandleForHeapStart(), 0, m_impl->m_bufferDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(buffers, targetBufferHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// The parameter index equals the target descriptor set space.
		CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalBufferHeap->GetGPUDescriptorHandleForHeapStart(), bufferOffset, m_impl->m_bufferDescriptorIncrement);
		commandBuffer.handle()->SetGraphicsRootDescriptorTable(descriptorSet.parent().rootParameterIndex(), targetGpuHandle);

		// Store the updated offset.
		m_impl->m_bufferOffset = bufferOffset + buffers;
	}

	if (samplers > 0)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.samplerHeap()->GetCPUDescriptorHandleForHeapStart(), 0, m_impl->m_samplerDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(samplers, targetSamplerHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// The parameter index equals the target descriptor set space.
		CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalSamplerHeap->GetGPUDescriptorHandleForHeapStart(), samplerOffset, m_impl->m_samplerDescriptorIncrement);
		commandBuffer.handle()->SetGraphicsRootDescriptorTable(descriptorSet.parent().rootParameterIndex(), targetGpuHandle);

		// Store the updated offset.
		m_impl->m_samplerOffset = samplerOffset + samplers;
	}
}

void DirectX12Device::bindGlobalDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	const std::array<ID3D12DescriptorHeap*, 2> globalHeaps{ m_impl->m_globalBufferHeap.Get(), m_impl->m_globalSamplerHeap.Get() };
	commandBuffer.handle()->SetDescriptorHeaps(globalHeaps.size(), globalHeaps.data());
}

const DirectX12ComputePipeline& DirectX12Device::blitPipeline() const noexcept
{
	return *m_impl->m_blitPipeline;
}

DirectX12RenderPassBuilder DirectX12Device::buildRenderPass(const MultiSamplingLevel& samples) const
{
	return DirectX12RenderPassBuilder(*this, samples);
}

DirectX12ComputePipelineBuilder DirectX12Device::buildComputePipeline() const
{
	return DirectX12ComputePipelineBuilder(*this);
}

DirectX12SwapChain& DirectX12Device::swapChain() noexcept
{
	return *m_impl->m_swapChain;
}

const DirectX12SwapChain& DirectX12Device::swapChain() const noexcept
{
	return *m_impl->m_swapChain;
}

const DirectX12Surface& DirectX12Device::surface() const noexcept
{
	return m_impl->m_surface;
}

const DirectX12GraphicsAdapter& DirectX12Device::adapter() const noexcept
{
	return m_impl->m_adapter;
}

const DirectX12GraphicsFactory& DirectX12Device::factory() const noexcept
{
	return *m_impl->m_factory;
}

const DirectX12Queue& DirectX12Device::graphicsQueue() const noexcept
{
	return *m_impl->m_graphicsQueue;
}

const DirectX12Queue& DirectX12Device::transferQueue() const noexcept
{
	return *m_impl->m_transferQueue;
}

const DirectX12Queue& DirectX12Device::bufferQueue() const noexcept
{
	return *m_impl->m_bufferQueue;
}

const DirectX12Queue& DirectX12Device::computeQueue() const noexcept
{
	return *m_impl->m_computeQueue;
}

MultiSamplingLevel DirectX12Device::maximumMultiSamplingLevel(const Format& format) const noexcept
{
	constexpr std::array<MultiSamplingLevel, 7> allLevels = { MultiSamplingLevel::x64, MultiSamplingLevel::x32, MultiSamplingLevel::x16, MultiSamplingLevel::x8, MultiSamplingLevel::x4, MultiSamplingLevel::x2, MultiSamplingLevel::x1 };
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{ .Format = ::getFormat(format) };

	for (int level(0); level < allLevels.size(); ++level)
	{
		levels.SampleCount = static_cast<UInt32>(allLevels[level]);
		
		if (FAILED(this->handle()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))))
			continue;

		if (levels.NumQualityLevels > 0)
			return allLevels[level];
	}

	LITEFX_WARNING(DIRECTX12_LOG, "No supported multi-sampling levels could be queried. Assuming that multi-sampling is disabled.");
	return MultiSamplingLevel::x1;
}

void DirectX12Device::wait() const
{
	// NOTE: Currently we are only waiting for the graphics queue here - all other queues may continue their work.
	// Create a fence.
	ComPtr<ID3D12Fence> fence;
	raiseIfFailed<RuntimeException>(this->handle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Unable to create queue synchronization fence.");
	
	// Create a signal event.
	HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
	HRESULT hr = fence->SetEventOnCompletion(1, eventHandle);

	if (FAILED(hr))
	{
		::CloseHandle(eventHandle);
		raiseIfFailed<RuntimeException>(hr, "Unable to register queue synchronization fence completion event.");
	}

	// Signal the event value on the graphics queue.
	hr = std::as_const(*m_impl->m_graphicsQueue).handle()->Signal(fence.Get(), 1);
	
	if (FAILED(hr))
	{
		::CloseHandle(eventHandle);
		raiseIfFailed<RuntimeException>(hr, "Unable to wait for queue synchronization fence.");
	}

	// Wait for the fence signal.
	if (fence->GetCompletedValue() < 1)
		::WaitForSingleObject(eventHandle, INFINITE);

	::CloseHandle(eventHandle);
}