#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12SwapChain::DirectX12SwapChainImpl : public Implement<DirectX12SwapChain> {
public:
	friend class DirectX12SwapChain;

private:
	Format m_format{};
	Size2d m_extent{};
	bool m_supportsVariableRefreshRates{ false };

public:
	DirectX12SwapChainImpl(DirectX12SwapChain* parent) : base(parent) { }

private:
	bool supportsVariableRefreshRates(const DirectX12Backend* backend) const
	{
		BOOL allowTearing = FALSE;
		
		if (FAILED(backend->handle()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)), "Unable to request support for variable refresh rates."))
			return false;
		
		return static_cast<bool>(allowTearing);
	}

public:
	[[nodiscard]]
	ComPtr<IDXGISwapChain4> initialize(const DirectX12Device* device, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		if (device == nullptr)
			throw ArgumentNotInitializedException("The device must be initialized.");

		if (format == Format::Other || format == Format::None)
			throw InvalidArgumentException("The provided surface format it not a valid value. It must not equal {0} or {1}.", Format::None, Format::Other);

		auto backend = dynamic_cast<const DirectX12Backend*>(device->getBackend());
		auto graphicsQueue = dynamic_cast<const DirectX12Queue*>(device->graphicsQueue());

		if (backend == nullptr)
			throw InvalidArgumentException("The device has not been initialized from a DirectX 12 backend.");

		if (graphicsQueue == nullptr)
			throw InvalidArgumentException("The device does not provide a graphics queue.");

		auto surface = dynamic_cast<const DirectX12Surface*>(backend->getSurface());

		if (surface == nullptr)
			throw InvalidArgumentException("The backend has does not provide a valid surface.");

		// Create the swap chain.
		LITEFX_TRACE(DIRECTX12_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px }}...", fmt::ptr(device), frameBuffers, frameBufferSize.width(), frameBufferSize.height());
		
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
		m_extent = frameBufferSize;

		return swapChain;
	}

	void reset(const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		raiseIfFailed<RuntimeException>(m_parent->handle()->ResizeBuffers(frameBuffers, static_cast<UInt32>(frameBufferSize.width()), static_cast<UInt32>(frameBufferSize.height()), ::getFormat(m_format), m_supportsVariableRefreshRates ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0), "Unable to resize frame buffer on swap chain.");
		m_extent = frameBufferSize;
	}

	UInt32 swapBackBuffer()
	{
		throw;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12SwapChain::DirectX12SwapChain(const DirectX12Device* device, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Format& format) :
	m_impl(makePimpl<DirectX12SwapChainImpl>(this)), IResource(nullptr)
{
	this->handle() = m_impl->initialize(device, format, frameBufferSize, frameBuffers);
}

DirectX12SwapChain::~DirectX12SwapChain() noexcept = default;

const Size2d& DirectX12SwapChain::getBufferSize() const noexcept
{
	return m_impl->m_extent;
}

size_t DirectX12SwapChain::getWidth() const noexcept
{
	return m_impl->m_extent.width();
}

size_t DirectX12SwapChain::getHeight() const noexcept
{
	return m_impl->m_extent.height();
}

const Format& DirectX12SwapChain::getFormat() const noexcept
{
	return m_impl->m_format;
}

UInt32 DirectX12SwapChain::getBuffers() const noexcept
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	return FAILED(this->handle()->GetDesc1(&swapChainDesc)) ? 0 : swapChainDesc.BufferCount;
}

UInt32 DirectX12SwapChain::swapBackBuffer() const
{
	return m_impl->swapBackBuffer();
}

void DirectX12SwapChain::reset(const Size2d& frameBufferSize, const UInt32& frameBuffers)
{
	m_impl->reset(frameBufferSize, frameBuffers);
}