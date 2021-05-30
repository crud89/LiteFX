#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12SwapChain::DirectX12SwapChainImpl : public Implement<DirectX12SwapChain> {
public:
	friend class DirectX12SwapChain;

private:
	Size2d m_renderArea{ };
	Format m_format{ Format::None };
	UInt32 m_buffers{ };
	std::atomic_uint32_t m_currentImage{ };
	//Array<UniquePtr<IDirectX12Image>> m_presentImages{ };
	bool m_supportsVariableRefreshRates{ false };

public:
	DirectX12SwapChainImpl(DirectX12SwapChain* parent) : 
		base(parent) 
	{
	}

private:
	bool supportsVariableRefreshRates(const DirectX12Backend* backend) const
	{
		BOOL allowTearing = FALSE;
		
		if (FAILED(backend->handle()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			return false;
		
		return static_cast<bool>(allowTearing);
	}

public:
	[[nodiscard]]
	ComPtr<IDXGISwapChain4> initialize(const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		if (format == Format::Other || format == Format::None)
			throw InvalidArgumentException("The provided surface format it not a valid value. It must not equal {0} or {1}.", Format::None, Format::Other);

		auto adapter = m_parent->getDevice()->adapter().handle();
		auto surface = m_parent->getDevice()->surface().handle();
		auto graphicsQueue = m_parent->getDevice()->graphicsQueue().handle();
		auto backend = m_parent->getDevice()->backend();

		// Create the swap chain.
		LITEFX_TRACE(DIRECTX12_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px }}...", fmt::ptr(m_parent->getDevice()), frameBuffers, frameBufferSize.width(), frameBufferSize.height());
		
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = static_cast<UInt32>(frameBufferSize.width());
		swapChainDesc.Height = static_cast<UInt32>(frameBufferSize.height());
		swapChainDesc.Format = ::getFormat(format);
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = std::max<UInt32>(2, frameBuffers);
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = (m_supportsVariableRefreshRates = supportsVariableRefreshRates(backend)) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChainBase;
		ComPtr<IDXGISwapChain4> swapChain;
		raiseIfFailed<RuntimeException>(backend->handle()->CreateSwapChainForHwnd(graphicsQueue->handle().Get(), surface->handle(), &swapChainDesc, nullptr, nullptr, &swapChainBase), "Unable to create swap chain for device.");
		raiseIfFailed<RuntimeException>(swapChainBase.As(&swapChain), "The swap chain does not implement the IDXGISwapChain4 interface.");

		// Disable Alt+Enter shortcut for fullscreen-toggle.
		backend->handle()->MakeWindowAssociation(surface->handle(), DXGI_MWA_NO_ALT_ENTER);

		m_format = format;
		m_renderArea = frameBufferSize;
		m_buffers = frameBuffers;

		return swapChain;
	}

	void cleanup()
	{
		throw;
	}

	UInt32 swapBackBuffer()
	{
		throw;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12SwapChain::DirectX12SwapChain(const DirectX12Device& device, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers) :
	m_impl(makePimpl<DirectX12SwapChainImpl>(this)), DirectX12RuntimeObject(device, &device), ComResource<IDXGISwapChain4>(nullptr)
{
	this->handle() = m_impl->initialize(format, frameBufferSize, frameBuffers);
}

DirectX12SwapChain::~DirectX12SwapChain() noexcept = default;

const Format& DirectX12SwapChain::surfaceFormat() const noexcept
{
	return m_impl->m_format;
}

const UInt32& DirectX12SwapChain::buffers() const noexcept
{
	return m_impl->m_buffers;
}

const Size2d& DirectX12SwapChain::renderArea() const noexcept
{
	return m_impl->m_renderArea;
}

Array<const IDirectX12Image*> DirectX12SwapChain::images() const noexcept
{
	//return m_impl->m_presentImages | std::views::transform([](const UniquePtr<IDirectX12Image>& image) { return image.get(); }) | ranges::to<Array<const IDirectX12Image*>>();
	throw;
}

Array<Format> DirectX12SwapChain::getSurfaceFormats() const noexcept
{
	// TODO: DX only supports a pre-defined set of surface formats and afaik has no way to query them.
	//return m_impl->getSurfaceFormats(this->getDevice()->adapter().handle(), this->getDevice()->surface().handle());
	throw;
}

void DirectX12SwapChain::reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers)
{
	// Cleanup and re-initialize.
	m_impl->cleanup();
	this->handle() = m_impl->initialize(surfaceFormat, renderArea, buffers);
}

UInt32 DirectX12SwapChain::swapBackBuffer() const
{
	return m_impl->swapBackBuffer();
}