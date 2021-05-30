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
	UniquePtr<DirectX12Queue> m_graphicsQueue, m_transferQueue, m_bufferQueue;
	ComPtr<ID3D12InfoQueue1> m_eventQueue;
	UniquePtr<DirectX12SwapChain> m_swapChain;
	DWORD m_debugCallbackCookie = 0;

public:
	DirectX12DeviceImpl(DirectX12Device* parent, const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend) :
		base(parent), m_adapter(adapter), m_surface(surface), m_backend(backend)
	{
	}

	virtual ~DirectX12DeviceImpl() noexcept
	{
		if (m_eventQueue != nullptr & m_debugCallbackCookie != 0)
			m_eventQueue->UnregisterMessageCallback(m_debugCallbackCookie);

		m_swapChain = nullptr;
	}

private:
	HWND getSurface() const noexcept
	{
		auto surface = dynamic_cast<const DirectX12Surface*>(m_parent->getBackend()->getSurface());
		return surface ? surface->handle() : nullptr;
	}

	ComPtr<IDXGIAdapter4> getAdapter() const noexcept
	{
		auto adapter = dynamic_cast<const DirectX12GraphicsAdapter*>(m_parent->getBackend()->getAdapter());
		return adapter ? adapter->handle() : nullptr;
	}

	ComPtr<IDXGIFactory7> getInstance() const noexcept
	{
		auto backend = dynamic_cast<const DirectX12Backend*>(m_parent->getBackend());
		return backend ? backend->handle() : nullptr;
	}

#ifndef NDEBUG
private:
	static void onDebugMessage(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void* /*context*/)
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

		auto adapter = this->getAdapter();
		auto a = adapter.Get();

		raiseIfFailed<RuntimeException>(::D3D12CreateDevice(this->getAdapter().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)), "Unable to create DirectX 12 device.");			

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
			D3D12_MESSAGE_ID suppressIds[] = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE };
			D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };	// Somehow it is required to deny info-level messages. Otherwise strange pointer issues are occurring.

			D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
			infoQueueFilter.DenyList.NumIDs = _countof(suppressIds);
			infoQueueFilter.DenyList.pIDList = suppressIds;
			infoQueueFilter.DenyList.NumSeverities = _countof(severities);
			infoQueueFilter.DenyList.pSeverityList = severities;

			raiseIfFailed<RuntimeException>(infoQueue->PushStorageFilter(&infoQueueFilter), "Unable to push message filter to info queue.");

			// Try to register event callback.
			// TODO: Change message when build finally hits production.
			if (FAILED(infoQueue.As(&m_eventQueue)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to query debug message callback queue. Native event logging will be disabled. Note that it requires Windows 10 version 21H1 or later.");
			else if (FAILED(m_eventQueue->RegisterMessageCallback(&DirectX12Device::DirectX12DeviceImpl::onDebugMessage, D3D12_MESSAGE_CALLBACK_FLAGS::D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &m_debugCallbackCookie)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to register debug message callback with info queue. Native event logging will be disabled.");
		}
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
		//m_graphicsQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Graphics, QueuePriority::Realtime);
		m_graphicsQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Graphics, QueuePriority::High);
		m_transferQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Transfer, QueuePriority::Normal);
		m_bufferQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Transfer, QueuePriority::High);
	}

	void createCommandQueues()
	{
		//m_graphicsQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Graphics, QueuePriority::Realtime);
		m_graphicsQueue = makeUnique<DirectX12Queue>(m_parent, QueueType::Graphics, QueuePriority::High);
	}

	void createSwapChain(const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		m_swapChain = makeUnique<DirectX12SwapChain>(m_parent, frameBufferSize, frameBuffers, format);
	}

	void resize(int width, int height)
	{
		// Reset the swap chain.
		m_swapChain->reset(Size2d(width, height), m_swapChain->getBuffers());
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

DirectX12Device::DirectX12Device(const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers) :
	ComResource<ID3D12Device5>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this, adapter, surface, backend)), GraphicsDevice(backend)
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {0}, Adapter: {1}, Extensions: {2} }}...", fmt::ptr(&surface), adapter.getDeviceId(), Join(this->enabledExtensions(), ", "));
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(VULKAN_LOG, "Vendor: {0:#0x}", adapter.getVendorId());
	LITEFX_DEBUG(VULKAN_LOG, "Driver Version: {0:#0x}", adapter.getDriverVersion());
	LITEFX_DEBUG(VULKAN_LOG, "API Version: {0:#0x}", adapter.getApiVersion());
	LITEFX_DEBUG(VULKAN_LOG, "Dedicated Memory: {0} Bytes", adapter.getDedicatedMemory());
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

	this->handle() = m_impl->initialize();
	m_impl->createQueues();
	m_impl->createFactory();
	m_impl->createSwapChain(format, frameBufferSize, frameBuffers);
}

DirectX12Device::~DirectX12Device() noexcept = default;

const DirectX12Backend& DirectX12Device::backend() const noexcept
{
	return m_impl->m_backend;
}

DirectX12RenderPassBuilder DirectX12Device::buildRenderPass() const
{
	return DirectX12RenderPassBuilder(*this);
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

void DirectX12Device::wait() const
{
	// TODO: Wait for all queues.
	//raiseIfFailed<RuntimeException>(::vkDeviceWaitIdle(this->handle()), "Unable to wait for the device.");
	throw;
}