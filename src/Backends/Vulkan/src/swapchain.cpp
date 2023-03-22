#include <litefx/backends/vulkan.hpp>
#include <atomic>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

#if !defined(BUILD_DIRECTX_12_BACKEND)
// ------------------------------------------------------------------------------------------------
// Default implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSwapChain::VulkanSwapChainImpl : public Implement<VulkanSwapChain> {
public:
	friend class VulkanSwapChain;

private:
	Size2d m_renderArea { };
	Format m_format { Format::None };
	UInt32 m_buffers { };
	Array<VkSemaphore> m_swapSemaphores { };
	std::atomic_uint32_t m_currentImage { };
	Array<UniquePtr<IVulkanImage>> m_presentImages { };
	const VulkanDevice& m_device;
	VkSwapchainKHR m_handle = VK_NULL_HANDLE;

public:
	VulkanSwapChainImpl(VulkanSwapChain* parent, const VulkanDevice& device) :
		base(parent), m_device(device)
	{ 
	}
	
	~VulkanSwapChainImpl()
	{
		this->cleanup();
	}

public:
	void initialize(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{
		if (format == Format::Other || format == Format::None)
			throw InvalidArgumentException("The provided surface format it not a valid value.");

		auto adapter = m_device.adapter().handle();
		auto surface = m_device.surface().handle();

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(adapter, surface);
		Format selectedFormat{ Format::None };
		
		if (auto match = std::ranges::find_if(surfaceFormats, [format](const Format& surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else
			throw InvalidArgumentException("The requested format is not supported by this device.");

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount);

		// Create a swap chain.
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = images;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageFormat = Vk::getFormat(selectedFormat);
		createInfo.imageColorSpace = this->findColorSpace(adapter, surface, selectedFormat);
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = deviceCaps.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		createInfo.imageExtent.height = std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height);
		createInfo.imageExtent.width = std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width);

		// Set the present mode to VK_PRESENT_MODE_MAILBOX_KHR, since it offers best performance without tearing. For VSync use VK_PRESENT_MODE_FIFO_KHR, which is also the only one guaranteed to be available.
		createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4} }}...", fmt::ptr(&m_device), images, createInfo.imageExtent.width, createInfo.imageExtent.height, selectedFormat);

		// Log if something needed to be changed.
		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		[[unlikely]] if (createInfo.imageExtent.height != static_cast<UInt32>(renderArea.height()) || createInfo.imageExtent.width != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", createInfo.imageExtent.height, createInfo.imageExtent.width, static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Create the swap chain instance.
		VkSwapchainKHR swapChain;
		raiseIfFailed<RuntimeException>(::vkCreateSwapchainKHR(m_device.handle(), &createInfo, nullptr, &swapChain), "Swap chain could not be created.");

		// Create a semaphore for swapping images.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_swapSemaphores.resize(images);
		std::ranges::generate(m_swapSemaphores, [&]() mutable {
			VkSemaphore semaphore;
			raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_device.handle(), &semaphoreInfo, nullptr, &semaphore), "Unable to create swap semaphore.");
			
			return semaphore;
		});

		// Create the swap chain images.
		auto actualRenderArea = Size2d(static_cast<size_t>(createInfo.imageExtent.width), static_cast<size_t>(createInfo.imageExtent.height));
		Array<VkImage> imageChain(images);
		::vkGetSwapchainImagesKHR(m_device.handle(), swapChain, &images, imageChain.data());

		m_presentImages = imageChain |
			std::views::transform([this, &actualRenderArea, &selectedFormat](const VkImage& image) { return makeUnique<VulkanImage>(m_device, image, Size3d{ actualRenderArea.width(), actualRenderArea.height(), 1 }, selectedFormat, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, false, ResourceState::Undefined); }) |
			ranges::to<Array<UniquePtr<IVulkanImage>>>();

		// Store state variables.
		m_renderArea = actualRenderArea;
		m_format = selectedFormat;
		m_buffers = images;
		m_currentImage = 0;
		m_handle = swapChain;
	}

	void reset(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{
		// Cleanup and re-initialize.
		this->cleanup();
		this->initialize(format, renderArea, buffers);
	}

	void cleanup()
	{
		// Destroy the swap chain itself.
		::vkDestroySwapchainKHR(m_device.handle(), m_handle, nullptr);

		// Destroy the image swap semaphores.
		std::ranges::for_each(m_swapSemaphores, [&](const auto& semaphore) { ::vkDestroySemaphore(m_device.handle(), semaphore, nullptr); });

		// Destroy state.
		m_swapSemaphores.clear();
		m_buffers = 0;
		m_renderArea = {};
		m_format = Format::None;
		m_currentImage = 0;
	}

	UInt32 swapBackBuffer()
	{
		UInt32 nextImage;
		m_currentImage++;
		raiseIfFailed<RuntimeException>(::vkAcquireNextImageKHR(m_device.handle(), m_handle, UINT64_MAX, this->currentSemaphore(), VK_NULL_HANDLE, &nextImage), "Unable to swap front buffer.");

		return nextImage;
	}

	void present(const VulkanFrameBuffer& frameBuffer)
	{
		// Draw the frame, if the result of the render pass it should be presented to the swap chain.
		std::array<VkSwapchainKHR, 1> swapChains = { m_handle };
		std::array<VkSemaphore, 1> signalSemaphores = { frameBuffer.semaphore() };
		const auto bufferIndex = frameBuffer.bufferIndex();

		VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<UInt32>(signalSemaphores.size()),
			.pWaitSemaphores = signalSemaphores.data(),
			.swapchainCount = static_cast<UInt32>(swapChains.size()),
			.pSwapchains = swapChains.data(),
			.pImageIndices = &bufferIndex,
			.pResults = nullptr
		};

		raiseIfFailed<RuntimeException>(::vkQueuePresentKHR(m_device.graphicsQueue().handle(), &presentInfo), "Unable to present swap chain.");
	}

	const VkSemaphore& currentSemaphore()
	{
		return m_swapSemaphores[m_currentImage % m_buffers];
	}

public:
	Array<Format> getSurfaceFormats(const VkPhysicalDevice adapter, const VkSurfaceKHR surface) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		return availableFormats | std::views::transform([](const VkSurfaceFormatKHR& format) { return Vk::getFormat(format.format); }) | ranges::to<Array<Format>>();
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, const Format& format) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		if (auto match = std::ranges::find_if(availableFormats, [&format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == Vk::getFormat(format); }); match != availableFormats.end()) [[likely]]
			return match->colorSpace;

		return VK_COLOR_SPACE_MAX_ENUM_KHR;
	}
};
#else
#include <litefx/backends/dx12_api.hpp>

namespace D3D 
{
	template <typename TException, typename ...TArgs>
	inline void raiseIfFailed(HRESULT hr, StringView message, TArgs&&... args)
	{
		if (SUCCEEDED(hr)) [[likely]]
			return;

		_com_error error(hr);

		if (message.empty())
			throw TException(VulkanPlatformException("{1} (HRESULT 0x{0:08X})", static_cast<unsigned>(hr), error.ErrorMessage()));
		else
			throw TException(VulkanPlatformException("{1} (HRESULT 0x{0:08X})", static_cast<unsigned>(hr), error.ErrorMessage()), fmt::format(fmt::runtime(message), std::forward<TArgs>(args)...));
	}
}

// ------------------------------------------------------------------------------------------------
// D3D12 interop implementation (with support for flip-model).
// ------------------------------------------------------------------------------------------------

class VulkanSwapChain::VulkanSwapChainImpl : public Implement<VulkanSwapChain> {
public:
	friend class VulkanSwapChain;

private:
	struct ImageResource {
	public:
		~ImageResource()
		{
			image.Reset();
			::vkFreeMemory(device, memory, nullptr);
			D3D::raiseIfFailed<RuntimeException>(::CloseHandle(handle), "Unable to close back buffer resource handle.");
		}

	public:
		ComPtr<ID3D12Resource> image = nullptr;
		VkDevice device = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		HANDLE handle = nullptr;
	};

private:
	Size2d m_renderArea{ };
	Format m_format{ Format::None };
	UInt32 m_buffers{ };
	Array<VkSemaphore> m_swapSemaphores{ };
	UInt32 m_currentImage{ };
	Array<UniquePtr<IVulkanImage>> m_presentImages{ };
	Array<ImageResource> m_imageResources;
	const VulkanDevice& m_device;
	ComPtr<ID3D12Device> m_d3dDevice;
	ComPtr<IDXGISwapChain4> m_swapChain;
	ComPtr<ID3D12CommandQueue> m_presentQueue;
	bool m_supportsTearing = false;

public:
	VulkanSwapChainImpl(VulkanSwapChain* parent, const VulkanDevice& device) :
		base(parent), m_device(device)
	{
	}

	~VulkanSwapChainImpl()
	{
		this->cleanup();
	}

public:
	void initialize(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{
		if (format == Format::Other || format == Format::None)
			throw InvalidArgumentException("The provided surface format it not a valid value.");

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(m_device.adapter().handle(), m_device.surface().handle());
		Format selectedFormat{ Format::None };

		if (auto match = std::ranges::find_if(surfaceFormats, [format](const Format& surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else
			throw InvalidArgumentException("The requested format is not supported by this device.");

		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.adapter().handle(), m_device.surface().handle(), &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount);

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Fix the render area, if required.
		Size2d extent(
			std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width),
			std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height));

		[[unlikely]] if (extent.height() != static_cast<UInt32>(renderArea.height()) || extent.width() != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", extent.height(), extent.width(), static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		// Start initializing a mini D3D environment that hosts the swap chain.
		LITEFX_TRACE(VULKAN_LOG, "Initializing Direct3D interop for flip-model swap chain support.");
		
		// Create a D3D12 factory.
		ComPtr<IDXGIFactory7> factory;
		UInt32 tearingSupport = 0;
#ifndef NDEBUG
		D3D::raiseIfFailed<RuntimeException>(::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)), "Unable to crate D3D12 factory for interop.");
#else
		D3D::raiseIfFailed<RuntimeException>(::CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "Unable to crate D3D12 factory for interop.");
#endif // NDEBUG
		
		if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupport, sizeof(tearingSupport)))) [[unlikely]]
			LITEFX_WARNING(VULKAN_LOG, "Unable to check support for variable refresh rates. Feature will be disabled.");

		// Query the DXGI adapter.
		ComPtr<IDXGIAdapter1> adapter;
		auto adapterId = m_device.adapter().uniqueId();
		D3D::raiseIfFailed<RuntimeException>(factory->EnumAdapterByLuid(*reinterpret_cast<LUID*>(&adapterId), IID_PPV_ARGS(&adapter)), "Unable to query adapter \"{0:#x}\".", adapterId);

		// Create a D3D device.
		D3D::raiseIfFailed<RuntimeException>(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_d3dDevice)), "Unable to create D3D device.");

#ifndef NDEBUG
		// Try to query an info queue to forward log messages.
		ComPtr<ID3D12InfoQueue> infoQueue;

		if (FAILED(m_d3dDevice.As(&infoQueue)))
			LITEFX_WARNING(VULKAN_LOG, "Unable to query info queue on D3D interop device. Debugger support will be disabled disabled.");
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

			D3D::raiseIfFailed<RuntimeException>(infoQueue->PushStorageFilter(&infoQueueFilter), "Unable to push message filter to info queue of D3D interop device.");
		}
#endif // NDEBUG

		// Create a command queue.
		D3D12_COMMAND_QUEUE_DESC presentQueueDesc {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			//.NodeMask = m_d3dDevice->GetNodeCount() <= 1 ? 0 : pdidProps.deviceNodeMask
		};
		D3D::raiseIfFailed<RuntimeException>(m_d3dDevice->CreateCommandQueue(&presentQueueDesc, IID_PPV_ARGS(&m_presentQueue)), "Unable to create present queue.");

		// Create the swap chain instance.
		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4} }}...", fmt::ptr(&m_device), images, extent.width(), extent.height(), selectedFormat);

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
			.Width = static_cast<UInt32>(extent.width()),
			.Height = static_cast<UInt32>(extent.height()),
			.Format = DX12::getFormat(selectedFormat),
			.Stereo = FALSE,
			.SampleDesc = { 1, 0 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = images,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = (m_supportsTearing = static_cast<bool>(tearingSupport)) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : (UInt32)0,
		};

		ComPtr<IDXGISwapChain1> swapChain;
		auto hwnd = m_device.surface().windowHandle();
		D3D::raiseIfFailed<RuntimeException>(factory->CreateSwapChainForHwnd(m_presentQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain), "Unable to create interop swap chain.");
		D3D::raiseIfFailed<RuntimeException>(swapChain.As(&m_swapChain), "The interop swap chain does not implement the IDXGISwapChain4 interface.");

		// Initialize swap chain images.
		this->createImages(selectedFormat, extent, images);

		// Disable Alt+Enter shortcut for fullscreen-toggle.
		if (FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))) [[unlikely]]
			LITEFX_WARNING(VULKAN_LOG, "Unable disable keyboard control sequence for full-screen switching.");
	}

	void reset(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{
		// Release the image memory of the previously allocated images.
		std::ranges::for_each(m_presentImages, [this](const auto& image) { ::vkDestroyImage(m_device.handle(), std::as_const(*image).handle(), nullptr); });

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(m_device.adapter().handle(), m_device.surface().handle());
		Format selectedFormat{ Format::None };

		if (auto match = std::ranges::find_if(surfaceFormats, [format](const Format& surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else
			throw InvalidArgumentException("The requested format is not supported by this device.");

		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.adapter().handle(), m_device.surface().handle(), &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount);

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Fix the render area, if required.
		Size2d extent(std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width), std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height));

		[[unlikely]] if (extent.height() != static_cast<UInt32>(renderArea.height()) || extent.width() != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", extent.height(), extent.width(), static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		// Reset the swap chain instance.
		LITEFX_TRACE(VULKAN_LOG, "Resetting swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4} }}...", fmt::ptr(&m_device), images, extent.width(), extent.height(), selectedFormat);

		// Wait for both devices to be idle.
		//m_device.graphicsQueue().waitFor(m_lastFence);
		this->waitForInteropDevice();
		m_presentImages.clear();
		m_imageResources.clear();
		D3D::raiseIfFailed<RuntimeException>(m_swapChain->ResizeBuffers(buffers, static_cast<UInt32>(extent.width()), static_cast<UInt32>(extent.height()), DX12::getFormat(format), m_supportsTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0), "Unable to resize interop swap chain back buffers.");

		// Initialize swap chain images.
		this->createImages(selectedFormat, extent, images);
	}

	void createImages(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{	
		// Acquire the swap chain images.
		m_presentImages.resize(buffers);
		m_imageResources.resize(buffers);
		std::ranges::generate(m_presentImages, [this, &renderArea, &format, i = 0]() mutable {
			// Acquire a image resource for the back buffer and create a shared handle for it.
			ComPtr<ID3D12Resource> resource;
			HANDLE resourceHandle = nullptr;
			const int image = i++;
			D3D::raiseIfFailed<RuntimeException>(m_swapChain->GetBuffer(image, IID_PPV_ARGS(&resource)), "Unable to acquire image resource from swap chain back buffer {0}.", image);
			D3D::raiseIfFailed<RuntimeException>(m_d3dDevice->CreateSharedHandle(resource.Get(), nullptr, GENERIC_ALL, nullptr, &resourceHandle), "Unable to create shared handle for interop back buffer.");

			// Wrap the back buffer images in an vulkan image.
			const VkExternalMemoryImageCreateInfo wrapperInfo {
				.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
				.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT
			};

			const VkImageCreateInfo imageInfo {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = &wrapperInfo,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = Vk::getFormat(format),
				.extent = { static_cast<UInt32>(renderArea.width()), static_cast<UInt32>(renderArea.height()), 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			// Create the image.
			VkImage backBuffer;
			raiseIfFailed<RuntimeException>(::vkCreateImage(m_device.handle(), &imageInfo, nullptr, &backBuffer), "Unable to create swap-chain image.");
			
			// Get the memory requirements.
			VkMemoryRequirements memoryRequirements;
			VkPhysicalDeviceMemoryProperties memoryProperties;
			::vkGetImageMemoryRequirements(m_device.handle(), backBuffer, &memoryRequirements);
			::vkGetPhysicalDeviceMemoryProperties(m_device.adapter().handle(), &memoryProperties);

			// Find the a suitable memory type.
			UInt32 memoryType{ std::numeric_limits<UInt32>::max() };

			for (UInt32 m = 0; m < memoryProperties.memoryTypeCount; ++m)
			{
				if ((memoryRequirements.memoryTypeBits & (1 << m)) && memoryProperties.memoryTypes[m].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					memoryType = m;
					break;
				}
			}

			// Create an allocation info for the external resource, import, allocate and bind it.
			VkMemoryDedicatedAllocateInfo memoryInfo {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
				.image = backBuffer
			};

			VkImportMemoryWin32HandleInfoKHR importInfo {
				.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
				.pNext = &memoryInfo,
				.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
				.handle = resourceHandle
			};

			VkMemoryAllocateInfo allocationInfo {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = &importInfo,
				.allocationSize = memoryRequirements.size,
				.memoryTypeIndex = memoryType
			};

			VkDeviceMemory imageMemory;
			raiseIfFailed<RuntimeException>(::vkAllocateMemory(m_device.handle(), &allocationInfo, nullptr, &imageMemory), "Unable to allocate memory for imported interop swap chain buffer.");
			raiseIfFailed<RuntimeException>(::vkBindImageMemory(m_device.handle(), backBuffer, imageMemory, 0), "Unable to bind back-buffer.");

			// Return the image instance.
			m_imageResources[image].device = m_device.handle();
			m_imageResources[image].memory = imageMemory;
			m_imageResources[image].handle = resourceHandle;
			m_imageResources[image].image = std::move(resource);

			return makeUnique<VulkanImage>(m_device, backBuffer, Size3d { renderArea.width(), renderArea.height(), 1 }, format, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, false, ResourceState::Present);
		});

		// Destroy the image swap semaphores.
		std::ranges::for_each(m_swapSemaphores, [&](const auto& semaphore) { ::vkDestroySemaphore(m_device.handle(), semaphore, nullptr); });

		// Create a semaphore for swapping images.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_swapSemaphores.resize(buffers);
		std::ranges::generate(m_swapSemaphores, [&]() mutable {
			VkSemaphore semaphore;
			raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_device.handle(), &semaphoreInfo, nullptr, &semaphore), "Unable to create swap semaphore.");

			return semaphore;
		});

		// Store state variables.
		m_renderArea = renderArea;
		m_format = format;
		m_buffers = buffers;
		m_currentImage = 0;
	}

	void cleanup()
	{
		// Release the image memory of the previously allocated images.
		std::ranges::for_each(m_presentImages, [this](const auto& image) { ::vkDestroyImage(m_device.handle(), std::as_const(*image).handle(), nullptr); });

		m_imageResources.clear();
		m_presentImages.clear();

		// Destroy the swap chain and interop device.
		this->waitForInteropDevice();
		m_swapChain.Reset();
		m_d3dDevice.Reset();

		// Destroy the image swap semaphores.
		std::ranges::for_each(m_swapSemaphores, [&](const auto& semaphore) { ::vkDestroySemaphore(m_device.handle(), semaphore, nullptr); });

		// Destroy state.
		m_swapSemaphores.clear();
		m_buffers = 0;
		m_renderArea = {};
		m_format = Format::None;
		m_currentImage = 0;
	}

	UInt32 swapBackBuffer()
	{
		// Get the current back buffer index.
		m_currentImage = m_swapChain->GetCurrentBackBufferIndex();

		// We need to manually signal the current semaphore on the graphics queue, to inform it that the swap chain image is ready to be written.
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &this->currentSemaphore()
		};

		raiseIfFailed<RuntimeException>(::vkQueueSubmit(m_device.graphicsQueue().handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit the present queue signal.");

		// Return the new back buffer index.
		return m_currentImage;
	}

	void present(const VulkanFrameBuffer& frameBuffer)
	{
		// We need to manually signal the current semaphore on the graphics queue, to inform it, that the swap chain is ready.
		Array<VkPipelineStageFlags> waitForStages = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };

		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &frameBuffer.semaphore(),
			.pWaitDstStageMask = waitForStages.data()
		};

		// Wait for the frame buffer semaphore, as well as for the rendering fence to complete.
		raiseIfFailed<RuntimeException>(::vkQueueSubmit(m_device.graphicsQueue().handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit the present queue signal.");

		// Present needs to happen on UI thread, so we cannot do this asynchronously.
		m_device.graphicsQueue().waitFor(frameBuffer.lastFence());
		D3D::raiseIfFailed<RuntimeException>(m_swapChain->Present(0, m_supportsTearing ? DXGI_PRESENT_ALLOW_TEARING : 0), "Unable to queue present event on swap chain.");
	}

	const VkSemaphore& currentSemaphore()
	{
		return m_swapSemaphores[m_currentImage];
	}

public:
	Array<Format> getSurfaceFormats(const VkPhysicalDevice adapter, const VkSurfaceKHR surface) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		return availableFormats | std::views::transform([](const VkSurfaceFormatKHR& format) { return Vk::getFormat(format.format); }) | ranges::to<Array<Format>>();
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, const Format& format) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		if (auto match = std::ranges::find_if(availableFormats, [&format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == Vk::getFormat(format); }); match != availableFormats.end()) [[likely]]
			return match->colorSpace;

		return VK_COLOR_SPACE_MAX_ENUM_KHR;
	}

private:
	void waitForInteropDevice()
	{
		// Wait for the interop device to finish.
		ComPtr<ID3D12Fence> fence;
		D3D::raiseIfFailed<RuntimeException>(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Unable to create queue synchronization fence on interop device.");

		// Create a signal event.
		HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
		HRESULT hr = fence->SetEventOnCompletion(1, eventHandle);

		if (FAILED(hr))
		{
			::CloseHandle(eventHandle);
			D3D::raiseIfFailed<RuntimeException>(hr, "Unable to register queue synchronization fence completion event.");
		}

		// Signal the event value on the graphics queue.
		hr = m_presentQueue->Signal(fence.Get(), 1);

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
};
#endif // defined(BUILD_DIRECTX_12_BACKEND)

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSwapChain::VulkanSwapChain(const VulkanDevice& device, const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) :
	m_impl(makePimpl<VulkanSwapChainImpl>(this, device))
{
	m_impl->initialize(surfaceFormat, renderArea, buffers);
}

VulkanSwapChain::~VulkanSwapChain() noexcept = default;

const VkSemaphore& VulkanSwapChain::semaphore() const noexcept
{
	return m_impl->currentSemaphore();
}

const Format& VulkanSwapChain::surfaceFormat() const noexcept
{
	return m_impl->m_format;
}

const UInt32& VulkanSwapChain::buffers() const noexcept
{
	return m_impl->m_buffers;
}

const Size2d& VulkanSwapChain::renderArea() const noexcept
{
	return m_impl->m_renderArea;
}

Array<const IVulkanImage*> VulkanSwapChain::images() const noexcept
{
	return m_impl->m_presentImages | std::views::transform([](const UniquePtr<IVulkanImage>& image) { return image.get(); }) | ranges::to<Array<const IVulkanImage*>>();
}

void VulkanSwapChain::present(const VulkanFrameBuffer& frameBuffer) const
{
	m_impl->present(frameBuffer);
}

Array<Format> VulkanSwapChain::getSurfaceFormats() const noexcept
{
	return m_impl->getSurfaceFormats(m_impl->m_device.adapter().handle(), m_impl->m_device.surface().handle());
}

void VulkanSwapChain::reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers)
{
	m_impl->reset(surfaceFormat, renderArea, buffers);
	this->reseted(this, { surfaceFormat, renderArea, buffers });
}

UInt32 VulkanSwapChain::swapBackBuffer() const
{
	auto backBuffer = m_impl->swapBackBuffer();
	this->swapped(this, { });
	return backBuffer;
}