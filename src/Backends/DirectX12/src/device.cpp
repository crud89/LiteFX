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
	ComPtr<ID3D12Device>&& initialize(const Format& format)
	{
		ComPtr<ID3D12Device> device;
		HRESULT hr;

		auto adapter = this->getAdapter();
		auto a = adapter.Get();

		if (FAILED(hr = ::D3D12CreateDevice(this->getAdapter().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
			throw std::runtime_error(fmt::format("Unable to create DirectX 12 device: {0:#010x}.", static_cast<unsigned>(hr)));

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
	IComResource<ID3D12Device>(nullptr), m_impl(makePimpl<DirectX12DeviceImpl>(this)), GraphicsDevice(backend)
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
	//::vkDestroyDevice(this->handle(), nullptr);
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