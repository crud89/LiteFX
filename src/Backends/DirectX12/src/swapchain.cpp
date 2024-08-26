#include <litefx/backends/dx12.hpp>
#include "image.h"

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
	UInt32 m_currentImage{ };
	Array<UniquePtr<IDirectX12Image>> m_presentImages{ };
	Array<UInt64> m_presentFences{ };
	bool m_supportsVariableRefreshRates{ false };
	bool m_vsync{ false };
	const DirectX12Device& m_device;

	Array<SharedPtr<TimingEvent>> m_timingEvents;
	Array<UInt64> m_timestamps;
	Array<ComPtr<ID3D12QueryHeap>> m_timingQueryHeaps;
	Array<UniquePtr<IDirectX12Buffer>> m_timingQueryReadbackBuffers;
	ID3D12QueryHeap* m_currentQueryHeap;

public:
	DirectX12SwapChainImpl(DirectX12SwapChain* parent, const DirectX12Device& device) :
		base(parent), m_device(device)
	{
	}

private:
	bool supportsVariableRefreshRates(const DirectX12Backend& backend) const
	{
		BOOL allowTearing = FALSE;
		
		if (FAILED(backend.handle()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			return false;
		
		return static_cast<bool>(allowTearing);
	}

public:
	[[nodiscard]]
	ComPtr<IDXGISwapChain4> initialize(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync)
	{
		if (!std::ranges::any_of(m_parent->getSurfaceFormats(), [&format](Format surfaceFormat) { return surfaceFormat == format; }))
			throw InvalidArgumentException("format", "The provided surface format {0} it not a supported. Must be one of the following: {1}.", format, this->joinSupportedSurfaceFormats());

		auto adapter = m_device.adapter().handle();
		auto surface = m_device.surface().handle();
		auto graphicsQueue = m_device.defaultQueue(QueueType::Graphics).handle();
		const auto& backend = m_device.backend();

		// Create the swap chain.
		auto size = Size2d{ std::max<UInt32>(1, renderArea.width()), std::max<UInt32>(1, renderArea.height()) };
		LITEFX_TRACE(DIRECTX12_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4}, VSync: {5} }}...", reinterpret_cast<void*>(m_device.handle().Get()), backBuffers, size.width(), size.height(), format, enableVsync);
		
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = static_cast<UInt32>(size.width());
		swapChainDesc.Height = static_cast<UInt32>(size.height());
		swapChainDesc.Format = DX12::getFormat(format);
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // NOTE: D3D12 does no longer allow UAV access to back buffers, so binding swap chain images to UAV in compute/ray-tracing shaders does not work.
		swapChainDesc.BufferCount = std::max<UInt32>(2, backBuffers);
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = (m_supportsVariableRefreshRates = supportsVariableRefreshRates(backend)) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		swapChainDesc.SampleDesc = { 1, 0 };

		ComPtr<IDXGISwapChain1> swapChainBase;
		ComPtr<IDXGISwapChain4> swapChain;
		raiseIfFailed(backend.handle()->CreateSwapChainForHwnd(graphicsQueue.Get(), surface, &swapChainDesc, nullptr, nullptr, &swapChainBase), "Unable to create swap chain for device.");
		raiseIfFailed(swapChainBase.As(&swapChain), "The swap chain does not implement the IDXGISwapChain4 interface.");

		// Acquire the swap chain images.
		m_presentImages.resize(swapChainDesc.BufferCount);
		m_presentFences.resize(swapChainDesc.BufferCount);
		std::ranges::generate(m_presentImages, [this, &size, &format, &swapChain, i = 0]() mutable {
			ComPtr<ID3D12Resource> resource;
			raiseIfFailed(swapChain->GetBuffer(i++, IID_PPV_ARGS(&resource)), "Unable to acquire image resource from swap chain back buffer {0}.", i);
			return makeUnique<DirectX12Image>(m_device, std::move(resource), size, format, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination);
		});

		// Disable Alt+Enter shortcut for fullscreen-toggle.
		backend.handle()->MakeWindowAssociation(surface, DXGI_MWA_NO_ALT_ENTER);

		m_format = format;
		m_renderArea = size;
		m_buffers = swapChainDesc.BufferCount;
		m_vsync = enableVsync;

		return swapChain;
	}

	void reset(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync)
	{
		if (!std::ranges::any_of(m_parent->getSurfaceFormats(), [&format](Format surfaceFormat) { return surfaceFormat == format; }))
			throw InvalidArgumentException("format", "The provided surface format {0} it not a supported. Must be one of the following: {1}.", format, this->joinSupportedSurfaceFormats());

		// Release all back buffers.
		m_presentImages.clear();
		m_presentFences.clear();	// No need to wait on them, since we are not going to hand out those back-buffers again anyway.

		// Store a backend reference.
		const auto& backend = m_device.backend();

		// Resize the buffers.
		UInt32 buffers = std::max<UInt32>(2, backBuffers);
		auto size = Size2d{ std::max<UInt32>(1, renderArea.width()), std::max<UInt32>(1, renderArea.height()) };
		raiseIfFailed(m_parent->handle()->ResizeBuffers(buffers, static_cast<UInt32>(size.width()), static_cast<UInt32>(size.height()), DX12::getFormat(format), m_supportsVariableRefreshRates ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0), "Unable to resize swap chain back buffers.");
		LITEFX_TRACE(DIRECTX12_LOG, "Resetting swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4}, VSync: {5} }}...", reinterpret_cast<void*>(m_device.handle().Get()), backBuffers, size.width(), size.height(), format, enableVsync);

		// Acquire the swap chain images.
		m_presentImages.resize(buffers);
		m_presentFences.resize(buffers);
		std::ranges::generate(m_presentImages, [this, &size, &format, i = 0]() mutable {
			ComPtr<ID3D12Resource> resource;
			raiseIfFailed(m_parent->handle()->GetBuffer(i++, IID_PPV_ARGS(&resource)), "Unable to acquire image resource from swap chain back buffer {0}.", i);
			return makeUnique<DirectX12Image>(m_device, std::move(resource), size, format, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination);
		});

		m_format = format;
		m_renderArea = size;
		m_buffers = buffers;
		m_currentImage = 0;
		m_vsync = enableVsync;

		// Initialize the query pools.
		if (m_timingQueryHeaps.size() != buffers)
			this->resetQueryHeaps(m_timingEvents);
	}

	void resetQueryHeaps(const Array<SharedPtr<TimingEvent>>& timingEvents)
	{
		// No events - no pools.
		if (timingEvents.empty())
			return;

		// Release the existing query heaps and buffers.
		m_timingQueryHeaps.clear();
		m_timingQueryReadbackBuffers.clear();

		// Resize the query heaps array and allocate a heap for each back buffer.
		m_timingQueryHeaps.resize(m_buffers);
		std::ranges::generate(m_timingQueryHeaps, [this, &timingEvents]() {
			D3D12_QUERY_HEAP_DESC heapInfo {
				.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
				.Count = static_cast<UInt32>(timingEvents.size()),
				.NodeMask = 0x01
			};

			ComPtr<ID3D12QueryHeap> heap;
			raiseIfFailed(m_device.handle()->CreateQueryHeap(&heapInfo, IID_PPV_ARGS(&heap)), "Unable to create timestamp query heap.");
			return heap;
		});

		// Create a readback buffer for each heap.
		m_timingQueryReadbackBuffers.resize(m_buffers);
		std::ranges::generate(m_timingQueryReadbackBuffers, [this, &timingEvents]() { return m_device.factory().createBuffer(BufferType::Other, ResourceHeap::Readback, sizeof(UInt64) * timingEvents.size()); });

		// Store the event and resize the time stamp collection.
		m_timingEvents = timingEvents;
		m_timestamps.resize(timingEvents.size());
	}

	UInt32 swapBackBuffer()
	{
		// Get the next image index.
		m_currentImage = m_parent->handle()->GetCurrentBackBufferIndex();

		// Wait for all rendering commands to finish on the image index (otherwise we would not be able to re-use the command buffers).
		m_device.defaultQueue(QueueType::Graphics).waitFor(m_presentFences[m_currentImage]);

		// Read back the timestamps.
		if (!m_timestamps.empty())
			m_timingQueryReadbackBuffers[m_currentImage]->map(m_timestamps.data(), sizeof(UInt64) * m_timestamps.size(), 0, false);

		return m_currentImage;
	}

private:
	String joinSupportedSurfaceFormats() const noexcept 
	{
		auto formats = m_parent->getSurfaceFormats();

		return Join(formats |
			std::views::transform([](Format format) { return std::format("{0}", format); }) |
			std::ranges::to<Array<String>>(), ", ");
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12SwapChain::DirectX12SwapChain(const DirectX12Device& device, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync) :
	m_impl(makePimpl<DirectX12SwapChainImpl>(this, device)), ComResource<IDXGISwapChain4>(nullptr)
{
	this->handle() = m_impl->initialize(format, renderArea, backBuffers, enableVsync);
}

DirectX12SwapChain::~DirectX12SwapChain() noexcept = default;

bool DirectX12SwapChain::supportsVariableRefreshRate() const noexcept
{
	return m_impl->m_supportsVariableRefreshRates;
}

ID3D12QueryHeap* DirectX12SwapChain::timestampQueryHeap() const noexcept
{
	return m_impl->m_timingQueryHeaps[m_impl->m_currentImage].Get();
}

Enumerable<SharedPtr<TimingEvent>> DirectX12SwapChain::timingEvents() const noexcept
{
	return m_impl->m_timingEvents;
}

SharedPtr<TimingEvent> DirectX12SwapChain::timingEvent(UInt32 queryId) const
{
	if (queryId >= m_impl->m_timingEvents.size())
		throw ArgumentOutOfRangeException("queryId", std::make_pair(0ull, (UInt64)m_impl->m_timingEvents.size()), queryId, "No timing event has been registered for query ID {0}.", queryId);

	return m_impl->m_timingEvents[queryId];
}

UInt64 DirectX12SwapChain::readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	if (auto match = std::find(m_impl->m_timingEvents.begin(), m_impl->m_timingEvents.end(), timingEvent); match != m_impl->m_timingEvents.end()) [[likely]]
		return m_impl->m_timestamps[std::distance(m_impl->m_timingEvents.begin(), match)];

	throw InvalidArgumentException("timingEvent", "The timing event is not registered on the swap chain.");
}

UInt32 DirectX12SwapChain::resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	if (auto match = std::find(m_impl->m_timingEvents.begin(), m_impl->m_timingEvents.end(), timingEvent); match != m_impl->m_timingEvents.end()) [[likely]]
		return static_cast<UInt32>(std::distance(m_impl->m_timingEvents.begin(), match));

	throw InvalidArgumentException("timingEvent", "The timing event is not registered on the swap chain.");
}

Format DirectX12SwapChain::surfaceFormat() const noexcept
{
	return m_impl->m_format;
}

UInt32 DirectX12SwapChain::buffers() const noexcept
{
	return m_impl->m_buffers;
}

const Size2d& DirectX12SwapChain::renderArea() const noexcept
{
	return m_impl->m_renderArea;
}

bool DirectX12SwapChain::verticalSynchronization() const noexcept
{
	return m_impl->m_vsync;
}

IDirectX12Image* DirectX12SwapChain::image(UInt32 backBuffer) const
{
	if (backBuffer >= m_impl->m_presentImages.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("backBuffer", std::make_pair(0ull, (UInt64)m_impl->m_presentImages.size()), backBuffer, "The back buffer must be a valid index.");

	return m_impl->m_presentImages[backBuffer].get();
}

const IDirectX12Image& DirectX12SwapChain::image() const noexcept
{
	return *m_impl->m_presentImages[m_impl->m_currentImage];
}

Enumerable<IDirectX12Image*> DirectX12SwapChain::images() const noexcept
{
	return m_impl->m_presentImages | std::views::transform([](UniquePtr<IDirectX12Image>& image) { return image.get(); });
}

void DirectX12SwapChain::present(UInt64 fence) const
{
	// Store the last fence here that marks the end of the rendering to this frame buffer. Presenting is queued after rendering anyway, but when swapping the back buffers buffers,
	// we need to wait for all commands to finish before being able to re-use the command buffers associated with queued commands.
	m_impl->m_presentFences[m_impl->m_currentImage] = fence;

	if (m_impl->m_vsync)
		raiseIfFailed(this->handle()->Present(1, 0), "Unable to present swap chain");
	else
		raiseIfFailed(this->handle()->Present(0, this->supportsVariableRefreshRate() ? DXGI_PRESENT_ALLOW_TEARING : 0), "Unable to present swap chain");
}

Enumerable<Format> DirectX12SwapChain::getSurfaceFormats() const noexcept
{
	// NOTE: Those formats are actually the only ones that are supported for flip-model swap chains, which is currently the only 
	//       supported swap effect. If other swap effects are used, this function may require redesign. For more information see: 
	//       https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1#remarks.
	return Enumerable<Format> {
		DX12::getFormat(DXGI_FORMAT_R16G16B16A16_FLOAT),
		DX12::getFormat(DXGI_FORMAT_R10G10B10A2_UNORM),
		DX12::getFormat(DXGI_FORMAT_B8G8R8A8_UNORM),
		DX12::getFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
	};
}

void DirectX12SwapChain::addTimingEvent(SharedPtr<TimingEvent> timingEvent)
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	LITEFX_DEBUG(DIRECTX12_LOG, "Registering timing event: \"{0}\".", timingEvent->name());

	auto events = m_impl->m_timingEvents;
	events.push_back(timingEvent);
	m_impl->resetQueryHeaps(events);
}

void DirectX12SwapChain::reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync)
{
	m_impl->reset(surfaceFormat, renderArea, buffers, enableVsync);
	this->reseted(this, { surfaceFormat, renderArea, buffers, enableVsync });
}

UInt32 DirectX12SwapChain::swapBackBuffer() const
{
	return m_impl->swapBackBuffer();
}

void DirectX12SwapChain::resolveQueryHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	if (m_impl->m_timingEvents.empty())
		return;

	commandBuffer.handle()->ResolveQueryData(
		m_impl->m_timingQueryHeaps[m_impl->m_currentImage].Get(), 
		D3D12_QUERY_TYPE_TIMESTAMP, 0, m_impl->m_timestamps.size(), 
		std::as_const(*m_impl->m_timingQueryReadbackBuffers[m_impl->m_currentImage]).handle().Get(), 0);
}