#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Device::DirectX12DeviceImpl : public Implement<DirectX12Device> {
public:
	friend class DirectX12Device;

private:

public:
	DirectX12DeviceImpl(DirectX12Device* parent) :
		base(parent)
	{
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

public:
	ComPtr<ID3D12Device5>&& initialize(const Format& format)
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
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query info queue. Problem reporting will be disabled.");
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
		}
#endif

		return std::move(device);
	}

	//void createSwapChain(const Format& format)
	//{
	//	m_swapChain = makeUnique<DirectX12SwapChain>(m_parent, format);
	//}

	//void wait()
	//{
	//	if (::vkDeviceWaitIdle(m_parent->handle()) != VK_SUCCESS)
	//		throw std::runtime_error("Unable to wait for the device.");
	//}

	//void resize(int width, int height)
	//{
	//	// Wait for the device to be idle.
	//	this->wait();

	//	// Reset the swap chain.
	//	m_swapChain->reset();
	//}

public:
	//Array<Format> getSurfaceFormats() const
	//{
	//	auto adapter = this->getAdapter();
	//	auto surface = this->getSurface();

	//	if (adapter == nullptr)
	//		throw std::runtime_error("The adapter is not a valid DirectX12 adapter.");

	//	if (surface == nullptr)
	//		throw std::runtime_error("The surface is not a valid DirectX12 surface.");

	//	uint32_t formats;
	//	::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

	//	Array<VkSurfaceFormatKHR> availableFormats(formats);
	//	Array<Format> surfaceFormats(formats);

	//	::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());
	//	std::generate(surfaceFormats.begin(), surfaceFormats.end(), [&availableFormats, i = 0]() mutable { return getFormat(availableFormats[i++].format); });

	//	return surfaceFormats;
	//}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Device::DirectX12Device(const IRenderBackend* backend, const Format& format) :
	IComResource<ID3D12Device5>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this)), GraphicsDevice(backend)
{
	LITEFX_DEBUG(DIRECTX12_LOG, "Creating device on backend {0} {{ Surface: {1}, Adapter: {2}, Format: {3} }}...", fmt::ptr(backend), fmt::ptr(backend->getSurface()), backend->getAdapter()->getDeviceId(), format);

	this->handle() = m_impl->initialize(format);
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
	//return m_impl->m_swapChain->getWidth();
	throw;
}

size_t DirectX12Device::getBufferHeight() const noexcept
{
	//return m_impl->m_swapChain->getHeight();
	throw;
}

void DirectX12Device::wait()
{
	throw;
}

void DirectX12Device::resize(int width, int height)
{
	throw;
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

Array<UniquePtr<IImage>> DirectX12Device::createSwapChainImages(const ISwapChain* swapChain) const
{
	throw;
}

Array<Format> DirectX12Device::getSurfaceFormats() const 
{
	throw;
}

const ISwapChain* DirectX12Device::getSwapChain() const noexcept 
{ 
	throw; 
}

//DirectX12RenderPassBuilder DirectX12Device::buildRenderPass() const
//{
//	throw;
//}