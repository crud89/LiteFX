#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>
#include <shader_resources.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Device::DirectX12DeviceImpl : public Implement<DirectX12Device> {
public:
	friend class DirectX12Device;

private:
	const DirectX12GraphicsAdapter& m_adapter;
	const DirectX12Backend& m_backend;
	DeviceState m_deviceState;
	UniquePtr<DirectX12Surface> m_surface;
	DirectX12Queue* m_graphicsQueue, * m_transferQueue, * m_computeQueue;
	Array<UniquePtr<DirectX12Queue>> m_queues;
	UniquePtr<DirectX12GraphicsFactory> m_factory;
	UniquePtr<DirectX12ComputePipeline> m_blitPipeline;
	ComPtr<ID3D12InfoQueue1> m_eventQueue;
	UniquePtr<DirectX12SwapChain> m_swapChain;
	DWORD m_debugCallbackCookie = 0;
	UInt32 m_globalBufferHeapSize, m_globalSamplerHeapSize, m_bufferDescriptorIncrement, m_samplerDescriptorIncrement, m_bufferOffset{ 0 }, m_samplerOffset{ 0 };
	ComPtr<ID3D12DescriptorHeap> m_globalBufferHeap, m_globalSamplerHeap;
	mutable std::mutex m_bufferBindMutex;
	Array<std::pair<UInt32, UInt32>> m_bufferDescriptorFragments, m_samplerDescriptorFragments;

public:
	DirectX12DeviceImpl(DirectX12Device* parent, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, const DirectX12Backend& backend, UInt32 globalBufferHeapSize, UInt32 globalSamplerHeapSize) :
		base(parent), m_adapter(adapter), m_surface(std::move(surface)), m_backend(backend), m_globalBufferHeapSize(globalBufferHeapSize), m_globalSamplerHeapSize(globalSamplerHeapSize)
	{
		if (m_surface == nullptr)
			throw ArgumentNotInitializedException("surface", "The surface must be initialized.");

		if (globalSamplerHeapSize > 2048) [[unlikely]]
			throw ArgumentOutOfRangeException("globalSamplerHeapSize", 0u, 2048u, globalSamplerHeapSize, "Only 2048 samplers are allowed in the global sampler heap, but {0} have been specified.", globalSamplerHeapSize);
	}

	~DirectX12DeviceImpl() noexcept
	{
		// Clear the device state.
		m_deviceState.clear();

		// Unregister the event queue.
		if (m_eventQueue != nullptr && m_debugCallbackCookie != 0)
			m_eventQueue->UnregisterMessageCallback(m_debugCallbackCookie);

		// Release queues and swap chain.
		m_swapChain = nullptr;
		m_queues.clear();
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

private:
	bool checkRequiredExtensions(ID3D12Device10* device)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS12 options {};
		raiseIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options, sizeof(options)), "Unable to query device extensions.");
		
		bool result = options.EnhancedBarriersSupported; // && ...

		return result;
	}

public:
	[[nodiscard]]
	ComPtr<ID3D12Device10> initialize()
	{
		ComPtr<ID3D12Device10> device;
		HRESULT hr;

		raiseIfFailed(::D3D12CreateDevice(m_adapter.handle().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)), "Unable to create DirectX 12 device.");

		if (!this->checkRequiredExtensions(device.Get()))
			throw RuntimeException("Not all required extensions are supported by this device. A driver update may resolve this problem.");

#ifndef NDEBUG
		// Try to query an info queue to forward log messages.
		ComPtr<ID3D12InfoQueue> infoQueue;

		if (FAILED(device.As(&infoQueue)))
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query info queue. Debugger support will be disabled.");
		else
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);
			
			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID suppressIds[] = { 
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // Mismatch in clear value is intended.
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE, // Mismatch in clear value is intended.
			};
			D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };	// Somehow it is required to deny info-level messages. Otherwise strange pointer issues are occurring.

			D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
			infoQueueFilter.DenyList.NumIDs = _countof(suppressIds);
			infoQueueFilter.DenyList.pIDList = suppressIds;
			infoQueueFilter.DenyList.NumSeverities = _countof(severities);
			infoQueueFilter.DenyList.pSeverityList = severities;

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

		return device;
	}

	void createFactory()
	{
		m_factory = makeUnique<DirectX12GraphicsFactory>(*m_parent);
	}

	void createSwapChain(Format format, const Size2d& frameBufferSize, UInt32 frameBuffers)
	{
		m_swapChain = makeUnique<DirectX12SwapChain>(*m_parent, format, frameBufferSize, frameBuffers);
	}

	void createQueues()
	{
		//m_graphicsQueue = makeUnique<DirectX12Queue>(*m_parent, QueueType::Graphics, QueuePriority::Realtime);
		m_graphicsQueue = this->createQueue(QueueType::Graphics, QueuePriority::High);
		m_transferQueue = this->createQueue(QueueType::Transfer, QueuePriority::High);
		m_computeQueue  = this->createQueue(QueueType::Compute,  QueuePriority::High);
	}

	DirectX12Queue* createQueue(QueueType type, QueuePriority priority)
	{
		auto queue = makeUnique<DirectX12Queue>(*m_parent, type, priority);
		auto result = queue.get();
		m_queues.push_back(std::move(queue));
		return result;
	}

	void createBlitPipeline()
	{
		try
		{
			// Allocate shader module.
			Array<UniquePtr<DirectX12ShaderModule>> modules;
			auto blitShader = LiteFX::Backends::DirectX12::Shaders::blit_dxi::open();
			modules.push_back(std::move(makeUnique<DirectX12ShaderModule>(*m_parent, ShaderStage::Compute, blitShader, LiteFX::Backends::DirectX12::Shaders::blit_dxi::name(), "main")));
			auto shaderProgram = makeShared<DirectX12ShaderProgram>(*m_parent, std::move(modules | std::views::as_rvalue));

			// Allocate descriptor set layouts.
			UniquePtr<DirectX12PushConstantsLayout> pushConstantsLayout = nullptr;
			auto bufferLayouts = Enumerable<UniquePtr<DirectX12DescriptorLayout>>(
				makeUnique<DirectX12DescriptorLayout>(DescriptorType::ConstantBuffer, 0, 16), 
				makeUnique<DirectX12DescriptorLayout>(DescriptorType::Texture, 1, 0),
				makeUnique<DirectX12DescriptorLayout>(DescriptorType::RWTexture, 2, 0)
			);
			auto samplerLayouts = Enumerable<UniquePtr<DirectX12DescriptorLayout>>(
				makeUnique<DirectX12DescriptorLayout>(DescriptorType::Sampler, 0, 0) 
			);
			auto descriptorSetLayouts = Enumerable<UniquePtr<DirectX12DescriptorSetLayout>>(
				makeUnique<DirectX12DescriptorSetLayout>(*m_parent, std::move(bufferLayouts), 0, ShaderStage::Compute),
				makeUnique<DirectX12DescriptorSetLayout>(*m_parent, std::move(samplerLayouts), 1, ShaderStage::Compute)
			);
			
			// Create a pipeline layout.
			auto pipelineLayout = makeShared<DirectX12PipelineLayout>(*m_parent, std::move(descriptorSetLayouts), std::move(pushConstantsLayout));

			// Create the pipeline.
			m_blitPipeline = makeUnique<DirectX12ComputePipeline>(*m_parent, pipelineLayout, shaderProgram, "Blit");
		}
		catch (const Exception& ex)
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
			DX12::getFormat(DXGI_FORMAT_R16G16B16A16_FLOAT),
			DX12::getFormat(DXGI_FORMAT_R10G10B10A2_UNORM),
			DX12::getFormat(DXGI_FORMAT_B8G8R8A8_UNORM)
		};

		return surfaceFormats;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Device::DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface) :
	DirectX12Device(backend, adapter, std::move(surface), Format::B8G8R8A8_SRGB, { 800, 600 }, 3)
{
}

DirectX12Device::DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, Format format, const Size2d& frameBufferSize, UInt32 frameBuffers, UInt32 globalBufferHeapSize, UInt32 globalSamplerHeapSize) :
	ComResource<ID3D12Device10>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this, adapter, std::move(surface), backend, globalBufferHeapSize, globalSamplerHeapSize))
{
	LITEFX_DEBUG(DIRECTX12_LOG, "Creating DirectX 12 device {{ Surface: {0}, Adapter: {1} }}...", fmt::ptr(&surface), adapter.deviceId());
	LITEFX_DEBUG(DIRECTX12_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(DIRECTX12_LOG, "Vendor: {0:#0x} (\"{1}\")", adapter.vendorId(), DX12::getVendorName(adapter.vendorId()).c_str());
	LITEFX_DEBUG(DIRECTX12_LOG, "Driver Version: {0:#0x}", adapter.driverVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "API Version: {0:#0x}", adapter.apiVersion());
	LITEFX_DEBUG(DIRECTX12_LOG, "Dedicated Memory: {0} Bytes", adapter.dedicatedMemory());
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

void DirectX12Device::allocateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32& bufferOffset, UInt32& samplerOffset) const
{
	// NOTE: Freeing descriptor sets with leaves the heap(s) in fragmented state. This should be prevented, however we also keep track of the released offset/count pairs to re-allocate 
	//       them later. Re-allocation could follow those steps:
	//       - First, try to append to the current descriptor range.
	//       - If we're overflowing: find perfect offset/pair matches for the requested set.
	//       - If none is available: allocate from a fragmented area. Resize it afterwards with a new offset and reduced count.
	//       - If all of the above fail, then we're out of descriptors.
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	// Get the current descriptor sizes and compute the offsets.
	UInt32 buffers{ 0 }, samplers{ 0 };

	if (descriptorSet.bufferHeap() != nullptr)
		buffers = descriptorSet.bufferHeap()->GetDesc().NumDescriptors;

	if (descriptorSet.samplerHeap() != nullptr)
		samplers = descriptorSet.samplerHeap()->GetDesc().NumDescriptors;

	if (m_impl->m_bufferOffset + buffers <= m_impl->m_globalBufferHeapSize) [[likely]]
	{
		bufferOffset = m_impl->m_bufferOffset;
		m_impl->m_bufferOffset += buffers;
	}
	else [[unlikely]]
	{
		m_impl->m_bufferOffset = m_impl->m_globalBufferHeapSize;

		// Find a fitting offset from the fragment heap.
		if (auto match = std::ranges::find_if(m_impl->m_bufferDescriptorFragments, [&buffers](const auto& pair) { return pair.second == buffers; }); match != m_impl->m_bufferDescriptorFragments.end())
		{
			bufferOffset = match->first;
			m_impl->m_bufferDescriptorFragments.erase(match);
		}
		else if (auto match = std::ranges::find_if(m_impl->m_bufferDescriptorFragments, [&buffers](const auto& pair) { return pair.second > buffers; }); match != m_impl->m_bufferDescriptorFragments.end())
		{
			bufferOffset = match->first;
			match->first += buffers;
			match->second -= buffers;
		}
		else [[unlikely]]
		{
			throw RuntimeException("Unable to allocate more descriptors.");
		}
	}

	if (m_impl->m_samplerOffset + samplers <= m_impl->m_globalSamplerHeapSize) [[likely]]
	{
		samplerOffset = m_impl->m_samplerOffset;
		m_impl->m_samplerOffset += samplers;
	}
	else [[unlikely]]
	{
		m_impl->m_samplerOffset = m_impl->m_globalSamplerHeapSize;

		// Find a fitting offset from the fragment heap.
		if (auto match = std::ranges::find_if(m_impl->m_samplerDescriptorFragments, [&samplers](const auto& pair) { return pair.second == samplers; }); match != m_impl->m_samplerDescriptorFragments.end())
		{
			samplerOffset = match->first;
			m_impl->m_samplerDescriptorFragments.erase(match);
		}
		else if (auto match = std::ranges::find_if(m_impl->m_samplerDescriptorFragments, [&samplers](const auto& pair) { return pair.second > samplers; }); match != m_impl->m_samplerDescriptorFragments.end())
		{
			samplerOffset = match->first;
			match->first += samplers;
			match->second -= samplers;
		}
		else [[unlikely]]
		{
			throw RuntimeException("Unable to allocate more descriptors.");
		}
	}
}

void DirectX12Device::releaseGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet) const noexcept
{
	std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);

	if (descriptorSet.bufferHeap() != nullptr)
		m_impl->m_bufferDescriptorFragments.push_back(std::make_pair(descriptorSet.bufferOffset(), descriptorSet.bufferHeap()->GetDesc().NumDescriptors));

	if (descriptorSet.samplerHeap() != nullptr)
		m_impl->m_samplerDescriptorFragments.push_back(std::make_pair(descriptorSet.samplerOffset(), descriptorSet.samplerHeap()->GetDesc().NumDescriptors));
}

void DirectX12Device::updateBufferDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 firstDescriptor, UInt32 descriptors) const noexcept
{
	if (descriptors > 0) [[likely]]
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(m_impl->m_globalBufferHeap->GetCPUDescriptorHandleForHeapStart(), descriptorSet.bufferOffset() + firstDescriptor, m_impl->m_bufferDescriptorIncrement);
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.bufferHeap()->GetCPUDescriptorHandleForHeapStart(), firstDescriptor, m_impl->m_bufferDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(descriptors, targetHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

void DirectX12Device::updateSamplerDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 firstDescriptor, UInt32 descriptors) const noexcept
{
	if (descriptors > 0) [[likely]]
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(m_impl->m_globalSamplerHeap->GetCPUDescriptorHandleForHeapStart(), descriptorSet.samplerOffset() + firstDescriptor, m_impl->m_samplerDescriptorIncrement);
		CD3DX12_CPU_DESCRIPTOR_HANDLE sourceHandle(descriptorSet.samplerHeap()->GetCPUDescriptorHandleForHeapStart(), firstDescriptor, m_impl->m_samplerDescriptorIncrement);
		this->handle()->CopyDescriptorsSimple(descriptors, targetHandle, sourceHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
}

void DirectX12Device::bindDescriptorSet(const DirectX12CommandBuffer& commandBuffer, const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept
{
	UInt32 buffers = 0, samplers = 0;

	if (descriptorSet.bufferHeap() != nullptr)
		buffers = descriptorSet.bufferHeap()->GetDesc().NumDescriptors;

	if (descriptorSet.samplerHeap() != nullptr)
		samplers = descriptorSet.samplerHeap()->GetDesc().NumDescriptors;

	// Deduct, whether to set the graphics or compute descriptor tables.
	// TODO: Maybe we could store a simple boolean on the pipeline state to make this easier.
	const bool isGraphicsSet = dynamic_cast<const DirectX12RenderPipeline*>(&pipeline) != nullptr;
	 
	// Copy the descriptors to the global heaps and set the root table parameters.
	if (buffers > 0)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalBufferHeap->GetGPUDescriptorHandleForHeapStart(), descriptorSet.bufferOffset(), m_impl->m_bufferDescriptorIncrement);

		if (isGraphicsSet)
			commandBuffer.handle()->SetGraphicsRootDescriptorTable(descriptorSet.layout().rootParameterIndex(), targetGpuHandle);
		else
			commandBuffer.handle()->SetComputeRootDescriptorTable(descriptorSet.layout().rootParameterIndex(), targetGpuHandle);
	}

	if (samplers > 0)
	{
		// The parameter index equals the target descriptor set space.
		CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(m_impl->m_globalSamplerHeap->GetGPUDescriptorHandleForHeapStart(), descriptorSet.samplerOffset(), m_impl->m_samplerDescriptorIncrement);

		if (isGraphicsSet)
			commandBuffer.handle()->SetGraphicsRootDescriptorTable(descriptorSet.layout().rootParameterIndex(), targetGpuHandle);
		else
			commandBuffer.handle()->SetComputeRootDescriptorTable(descriptorSet.layout().rootParameterIndex(), targetGpuHandle);
	}
}

void DirectX12Device::bindGlobalDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	const std::array<ID3D12DescriptorHeap*, 2> globalHeaps{ m_impl->m_globalBufferHeap.Get(), m_impl->m_globalSamplerHeap.Get() };
	commandBuffer.handle()->SetDescriptorHeaps(globalHeaps.size(), globalHeaps.data());
}

DirectX12ComputePipeline& DirectX12Device::blitPipeline() const noexcept
{
	return *m_impl->m_blitPipeline;
}

#if defined(BUILD_DEFINE_BUILDERS)
DirectX12RenderPassBuilder DirectX12Device::buildRenderPass(MultiSamplingLevel samples, UInt32 commandBuffers) const
{
	return DirectX12RenderPassBuilder(*this, commandBuffers, samples);
}

DirectX12RenderPassBuilder DirectX12Device::buildRenderPass(const String& name, MultiSamplingLevel samples, UInt32 commandBuffers) const
{
	return DirectX12RenderPassBuilder(*this, commandBuffers, samples, name);
}

DirectX12RenderPipelineBuilder DirectX12Device::buildRenderPipeline(const DirectX12RenderPass& renderPass, const String& name) const
{
	return DirectX12RenderPipelineBuilder(renderPass, name);
}

DirectX12ComputePipelineBuilder DirectX12Device::buildComputePipeline(const String& name) const
{
	return DirectX12ComputePipelineBuilder(*this, name);
}

DirectX12PipelineLayoutBuilder DirectX12Device::buildPipelineLayout() const
{
	return DirectX12PipelineLayoutBuilder(*this);
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
#endif // defined(BUILD_DEFINE_BUILDERS)

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
	return m_impl->m_adapter;
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

const DirectX12Queue* DirectX12Device::createQueue(QueueType type, QueuePriority priority) noexcept
{
	return m_impl->createQueue(type, priority);
}

UniquePtr<DirectX12Barrier> DirectX12Device::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept
{
	return makeUnique<DirectX12Barrier>(syncBefore, syncAfter);
}

MultiSamplingLevel DirectX12Device::maximumMultiSamplingLevel(Format format) const noexcept
{
	constexpr std::array<MultiSamplingLevel, 7> allLevels = { MultiSamplingLevel::x64, MultiSamplingLevel::x32, MultiSamplingLevel::x16, MultiSamplingLevel::x8, MultiSamplingLevel::x4, MultiSamplingLevel::x2, MultiSamplingLevel::x1 };
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{ .Format = DX12::getFormat(format) };

	for (int level(0); level < allLevels.size(); ++level)
	{
		levels.SampleCount = std::to_underlying(allLevels[level]);
		
		if (FAILED(this->handle()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))))
			continue;

		if (levels.NumQualityLevels > 0)
			return allLevels[level];
	}

	LITEFX_WARNING(DIRECTX12_LOG, "No supported multi-sampling levels could be queried. Assuming that multi-sampling is disabled.");
	return MultiSamplingLevel::x1;
}

double DirectX12Device::ticksPerMillisecond() const noexcept
{
	UInt64 frequency = 1000000u;
	std::as_const(*m_impl->m_graphicsQueue).handle()->GetTimestampFrequency(&frequency);
	return static_cast<double>(frequency) / 1000.0;
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

		return { std::move(eventHandle), std::move(fence) };
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