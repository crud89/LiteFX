#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Device::DirectX12DeviceImpl : public Implement<DirectX12Device> {
public:
	friend class DirectX12Device;

private:
	UniquePtr<DirectX12Queue> m_graphicsQueue;
	ComPtr<ID3D12InfoQueue1> m_eventQueue;
	UniquePtr<DirectX12SwapChain> m_swapChain;
	DWORD m_debugCallbackCookie = 0;

public:
	DirectX12DeviceImpl(DirectX12Device* parent) :
		base(parent)
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
	ComPtr<ID3D12Device5> initialize(const Format& format)
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
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to query debug message callback queue. Native event logging will be disabled. Note that it requires Windows 10 build 20236 or later.");
			else if (FAILED(m_eventQueue->RegisterMessageCallback(&DirectX12Device::DirectX12DeviceImpl::onDebugMessage, D3D12_MESSAGE_CALLBACK_FLAGS::D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &m_debugCallbackCookie)))
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to register debug message callback with info queue. Native event logging will be disabled.");
		}
#endif

		return device;
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

DirectX12Device::DirectX12Device(const IRenderBackend* backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers) :
	IComResource<ID3D12Device5>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this)), GraphicsDevice(backend)
{
	LITEFX_DEBUG(DIRECTX12_LOG, "Creating device on backend {0} {{ Surface: {1}, Adapter: {2}, Format: {3} }}...", fmt::ptr(backend), fmt::ptr(backend->getSurface()), backend->getAdapter()->getDeviceId(), format);

	this->handle() = m_impl->initialize(format);
	m_impl->createCommandQueues();
	m_impl->createSwapChain(format, frameBufferSize, frameBuffers);
}

DirectX12Device::~DirectX12Device() noexcept
{
	//// Release the command queues first.
	//this->graphicsQueue()->release();
	//this->transferQueue()->release();

	// Destroy the implementation.
	m_impl.destroy();

	//// Destroy the device.
	//this->handle() = nullptr;
}

size_t DirectX12Device::getBufferWidth() const noexcept
{
	return m_impl->m_swapChain->getWidth();
}

size_t DirectX12Device::getBufferHeight() const noexcept
{
	return m_impl->m_swapChain->getHeight();
}

const ICommandQueue* DirectX12Device::graphicsQueue() const noexcept
{
	return m_impl->m_graphicsQueue.get();
}

const ICommandQueue* DirectX12Device::transferQueue() const noexcept
{
	//return m_impl->m_graphicsQueue.get();
	throw;
}

const ICommandQueue* DirectX12Device::bufferQueue() const noexcept
{
	//return m_impl->m_graphicsQueue.get();
	throw;
}

void DirectX12Device::wait()
{
	// TODO: If actually required, we should use a fence here and wait on it.
	return;
}

void DirectX12Device::resize(int width, int height)
{
	m_impl->resize(width, height);
}

UniquePtr<IBuffer> DirectX12Device::createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements) const
{
	throw;
}

UniquePtr<IVertexBuffer> DirectX12Device::createVertexBuffer(const IVertexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IIndexBuffer> DirectX12Device::createIndexBuffer(const IIndexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IConstantBuffer> DirectX12Device::createConstantBuffer(const IDescriptorLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IImage> DirectX12Device::createImage(const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	throw;
}

UniquePtr<IImage> DirectX12Device::createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const
{
	throw;
}

UniquePtr<ITexture> DirectX12Device::createTexture(const IDescriptorLayout* layout, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	throw;
}

UniquePtr<ISampler> DirectX12Device::createSampler(const IDescriptorLayout* layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	throw;
}

UniquePtr<IShaderModule> DirectX12Device::loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint) const
{
	throw;
}

Array<Format> DirectX12Device::getSurfaceFormats() const 
{
	return m_impl->getSurfaceFormats();
}

const ISwapChain* DirectX12Device::getSwapChain() const noexcept 
{ 
	return m_impl->m_swapChain.get();
}

DirectX12RenderPassBuilder DirectX12Device::buildRenderPass() const
{
	return this->build<DirectX12RenderPass>();
}