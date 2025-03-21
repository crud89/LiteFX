#include <litefx/backends/vulkan.hpp>
#include <atomic>
#include "image.h"

#if defined(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN) && defined(LITEFX_BUILD_DIRECTX_12_BACKEND)
#define USE_VULKAN_INTEROP_SWAP_CHAIN
#endif

using namespace LiteFX::Rendering::Backends;

// NOTE: It is important to keep private variable names equal between implementation classes in order for the debug visualizers to work.

#if !defined(USE_VULKAN_INTEROP_SWAP_CHAIN)
// ------------------------------------------------------------------------------------------------
// Default implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSwapChain::VulkanSwapChainImpl {
public:
	friend class VulkanSwapChain;

private:
	Size2d m_renderArea { };
	Format m_format { Format::None };
	UInt32 m_buffers { };
	UInt32 m_currentImage { };
	Array<SharedPtr<IVulkanImage>> m_presentImages { };
	WeakPtr<const VulkanDevice> m_device;
	SharedPtr<const VulkanQueue> m_presentQueue;
	VkSwapchainKHR m_handle = VK_NULL_HANDLE;
	VkFence m_waitForImage{};
	Array<VkSemaphore> m_waitForWorkload;

	Array<SharedPtr<const TimingEvent>> m_timingEvents;
	Array<UInt64> m_timestamps;
	Array<VkQueryPool> m_timingQueryPools;
	VkQueryPool m_currentQueryPool{};
	bool m_supportsTiming = false;
	bool m_vsync = false;

public:
	VulkanSwapChainImpl(const VulkanDevice& device) :
		m_device(device.weak_from_this()), m_presentQueue(device.defaultQueue(QueueType::Graphics).shared_from_this())
	{
		m_supportsTiming = device.adapter().limits().timestampComputeAndGraphics;

		if (!m_supportsTiming)
			LITEFX_WARNING(VULKAN_LOG, "Timestamp queries are not supported and will be disabled. Reading timestamps will always return 0.");
	}

	VulkanSwapChainImpl(VulkanSwapChainImpl&&) noexcept = default;
	VulkanSwapChainImpl(const VulkanSwapChainImpl&) = delete;
	VulkanSwapChainImpl& operator=(VulkanSwapChainImpl&&) noexcept = default;
	VulkanSwapChainImpl& operator=(const VulkanSwapChainImpl&) = delete;
	
	~VulkanSwapChainImpl()
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release swap chain after parent device.");
		else
		{
			// Release the existing query pools.
			if (!m_timingQueryPools.empty())
				std::ranges::for_each(m_timingQueryPools, [device](auto& pool) { ::vkDestroyQueryPool(device->handle(), pool, nullptr); });

			// Release the image memory of the previously allocated images.
			//std::ranges::for_each(m_presentImages, [device](const auto& image) { ::vkDestroyImage(device->handle(), std::as_const(*image).handle(), nullptr); });
			m_presentImages.clear();

			// Destroy the swap chain itself.
			::vkDestroySwapchainKHR(device->handle(), m_handle, nullptr);

			// Destroy the fences and semaphores used to wait for image acquisition.
			::vkDestroyFence(device->handle(), m_waitForImage, nullptr);
			std::ranges::for_each(m_waitForWorkload, [&](VkSemaphore semaphore) { ::vkDestroySemaphore(device->handle(), semaphore, nullptr); });

			// Destroy state.
			m_buffers = 0;
			m_renderArea = {};
			m_format = Format::None;
			m_currentImage = 0;
		}
	}

public:
	void initialize(const VulkanDevice& device, Format format, const Size2d& renderArea, UInt32 buffers, bool vsync)
	{
		if (format == Format::Other || format == Format::None) [[unlikely]]
			throw InvalidArgumentException("format", "The provided surface format it not a valid value.");

		auto adapter = device.adapter().handle();
		auto surface = device.surface().handle();

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(adapter, surface);
		Format selectedFormat{ Format::None };
		
		if (auto match = std::ranges::find_if(surfaceFormats, [format](Format surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else [[unlikely]]
			throw InvalidArgumentException("format", "The requested format is not supported by this device.");

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount == 0 ? buffers : deviceCaps.maxImageCount);

		// Create a swap chain.
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = images;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageFormat = Vk::getFormat(selectedFormat);
		createInfo.imageColorSpace = this->findColorSpace(adapter, surface, selectedFormat);
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = deviceCaps.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		createInfo.imageExtent.height = std::max<UInt32>(1u, std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height));
		createInfo.imageExtent.width = std::max<UInt32>(1u, std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width));

		// Set the present mode to VK_PRESENT_MODE_MAILBOX_KHR, since it offers best performance without tearing. For VSync use VK_PRESENT_MODE_FIFO_KHR, which is also the only one guaranteed to be available.
		createInfo.presentMode = vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;/* VK_PRESENT_MODE_MAILBOX_KHR;*/
		m_vsync = vsync;

		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4}, VSync: {5} }}...", static_cast<const void*>(&m_device), images, createInfo.imageExtent.width, createInfo.imageExtent.height, selectedFormat, vsync);

		// Log if something needed to be changed.
		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		[[unlikely]] if (createInfo.imageExtent.height != static_cast<UInt32>(renderArea.height()) || createInfo.imageExtent.width != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", createInfo.imageExtent.height, createInfo.imageExtent.width, static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Create the swap chain instance.
		VkSwapchainKHR swapChain{};
		raiseIfFailed(::vkCreateSwapchainKHR(device.handle(), &createInfo, nullptr, &swapChain), "Swap chain could not be created.");

		// Initialize the fences used to wait for image access.
		VkFenceCreateInfo fenceInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		raiseIfFailed(::vkCreateFence(device.handle(), &fenceInfo, nullptr, &m_waitForImage), "Unable to create image acquisition fence.");

		// Initialize the semaphores used to wait for workload completion before present.
		VkSemaphoreCreateInfo semaphoreInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		m_waitForWorkload.resize(images);
		std::ranges::generate(m_waitForWorkload, [&]() {
			VkSemaphore semaphore{};
			raiseIfFailed(::vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &semaphore), "Unable to workload synchronization semaphore.");
			return semaphore;
		});

		// Create the swap chain images.
		auto actualRenderArea = Size2d(static_cast<size_t>(createInfo.imageExtent.width), static_cast<size_t>(createInfo.imageExtent.height));
		Array<VkImage> imageChain(images);
		::vkGetSwapchainImagesKHR(device.handle(), swapChain, &images, imageChain.data());

		m_presentImages = imageChain |
			std::views::transform([&actualRenderArea, &selectedFormat](const VkImage& image) { return VulkanImage::create(image, Size3d{ actualRenderArea.width(), actualRenderArea.height(), 1 }, selectedFormat, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination); }) |
			std::ranges::to<Array<SharedPtr<IVulkanImage>>>();

		// Store state variables.
		m_renderArea = actualRenderArea;
		m_format = selectedFormat;
		m_buffers = images;
		m_currentImage = 0;
		m_handle = swapChain;

		// Initialize the query pools.
		if (m_timingQueryPools.size() != images)
			this->resetQueryPools(m_timingEvents);
	}

	void resetQueryPools(const Array<SharedPtr<const TimingEvent>>& timingEvents)
	{
		// No events - no pools.
		if (timingEvents.empty())
			return;

		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot reset query pools on a released device instance.");

		// Release the existing query pools.
		if (!m_timingQueryPools.empty())
			std::ranges::for_each(m_timingQueryPools, [&](auto& pool) { ::vkDestroyQueryPool(device->handle(), pool, nullptr); });

		// Resize the query pools array and allocate a pool for each back buffer.
		m_timingQueryPools.resize(m_buffers);
		std::ranges::generate(m_timingQueryPools, [&]() {
			VkQueryPoolCreateInfo poolInfo {
				.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
				.queryType = VkQueryType::VK_QUERY_TYPE_TIMESTAMP,
				.queryCount = static_cast<UInt32>(timingEvents.size())
			};

			VkQueryPool pool{};
			raiseIfFailed(::vkCreateQueryPool(device->handle(), &poolInfo, nullptr, &pool), "Unable to allocate timestamp query pool.");
			::vkResetQueryPool(device->handle(), pool, 0, static_cast<UInt32>(timingEvents.size()));

			return pool;
		});

		// Store the event and resize the time stamp collection.
		m_timingEvents = timingEvents;
		m_timestamps.resize(timingEvents.size());
	}

	void reset(Format format, const Size2d& renderArea, UInt32 buffers, bool vsync)
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Unable to reset swap chain on a released device instance.");

		// Destroy the swap chain itself.
		::vkDestroySwapchainKHR(device->handle(), m_handle, nullptr);

		// Destroy the fences and semaphores used to wait for image acquisition.
		::vkDestroyFence(device->handle(), m_waitForImage, nullptr);
		std::ranges::for_each(m_waitForWorkload, [&](VkSemaphore semaphore) { ::vkDestroySemaphore(device->handle(), semaphore, nullptr); });

		// Destroy state.
		m_buffers = 0;
		m_renderArea = {};
		m_format = Format::None;
		m_currentImage = 0;

		// Reinitialize the swap chain.
		this->initialize(*device, format, renderArea, buffers, vsync);
	}

	UInt32 swapBackBuffer()
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot swap back buffers on a released device instance.");

		// Queue an image acquisition request, then wait for the fence and reset it for the next iteration. Note how this is similar to the DirectX behavior, where the swap call blocks until the 
		// image is acquired and ready.
		raiseIfFailed(::vkAcquireNextImageKHR(device->handle(), m_handle, UINT64_MAX, VK_NULL_HANDLE, m_waitForImage, &m_currentImage), "Unable to swap front buffer. Make sure that all previously acquired images are actually presented before acquiring another image.");
		raiseIfFailed(::vkWaitForFences(device->handle(), 1, &m_waitForImage, VK_TRUE, UINT64_MAX), "Unable to wait for image acquisition.");
		raiseIfFailed(::vkResetFences(device->handle(), 1, &m_waitForImage), "Unable to reset image acquisition fence.");

		// Query the timing events.
		// TODO: In rare situations, and only when using this swap chain implementation, the validation layers will complain about query pools not being reseted, when writing time stamps. I could
		//       not find out why and when this happens, but maybe waiting explicitly on the last frame's fence (for the respective image) will fix the issue.
		if (m_supportsTiming && !m_timingEvents.empty()) [[likely]]
		{
			m_currentQueryPool = m_timingQueryPools[m_currentImage];
			auto result = ::vkGetQueryPoolResults(device->handle(), m_currentQueryPool, 0, static_cast<UInt32>(m_timestamps.size()), m_timestamps.size() * sizeof(UInt64), m_timestamps.data(), sizeof(UInt64), VK_QUERY_RESULT_64_BIT);
		
			if (result != VK_NOT_READY)	// Initial frames do not yet contain query results.
				raiseIfFailed(result, "Unable to query timing events.");

			// Reset the query pool.
			::vkResetQueryPool(device->handle(), m_currentQueryPool, 0, static_cast<UInt32>(m_timestamps.size()));
		}

		return m_currentImage;
	}

	void present(UInt64 fence) 
	{
		// Draw the frame, if the result of the render pass it should be presented to the swap chain.
		auto swapChains = std::array { m_handle };
		const auto bufferIndex = m_currentImage;

		// Wait for the workload semaphore before performing the actual presentation.
		auto workloadSemaphore = m_waitForWorkload[bufferIndex];
		VkPipelineStageFlags synchronizationPoint = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		
		VkTimelineSemaphoreSubmitInfo workloadFenceInfo = {
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.waitSemaphoreValueCount = 1,
			.pWaitSemaphoreValues = &fence
		};

		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &workloadFenceInfo,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_presentQueue->timelineSemaphore(),
			.pWaitDstStageMask = &synchronizationPoint,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &workloadSemaphore
		};

		raiseIfFailed(::vkQueueSubmit(m_presentQueue->handle(), 1, &submitInfo, VK_NULL_HANDLE), "Unable to submit workload semaphore.");

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &workloadSemaphore,
			.swapchainCount = static_cast<UInt32>(swapChains.size()),
			.pSwapchains = swapChains.data(),
			.pImageIndices = &bufferIndex,
			.pResults = nullptr
		};

		raiseIfFailed(::vkQueuePresentKHR(m_presentQueue->handle(), &presentInfo), "Unable to present swap chain.");
	}

	const VkQueryPool& currentTimestampQueryPool()
	{
		return m_currentQueryPool;
	}

public:
	Array<Format> getSurfaceFormats(const VkPhysicalDevice adapter, const VkSurfaceKHR surface) const
	{
		uint32_t formats{};
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		return availableFormats 
			| std::views::transform([](const VkSurfaceFormatKHR& format) { return Vk::getFormat(format.format); }) 
			| std::ranges::to<Array<Format>>();
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, Format format) const
	{
		uint32_t formats{};
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		if (auto match = std::ranges::find_if(availableFormats, [&format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == Vk::getFormat(format); }); match != availableFormats.end()) [[likely]]
			return match->colorSpace;

		return VK_COLOR_SPACE_MAX_ENUM_KHR;
	}
};
#else // !defined(USE_VULKAN_INTEROP_SWAP_CHAIN)
#include <litefx/backends/dx12_api.hpp>

#if VK_HEADER_VERSION < 268
// Warn about this bug: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5295.
#if __cplusplus >= 202302L
#   warning "Vulkan SDK version is below 1.3.268.0, which will cause false validation errors about invalid command buffer resets. This bug has been fixed in later versions."
#else
#   pragma message("Note: Vulkan SDK version is below 1.3.268.0, which will cause false validation errors about invalid command buffer resets. This bug has been fixed in later versions.")
#endif
#endif

namespace D3D 
{
	/// <summary>
	/// Raises a <see cref="DirectX12PlatformException" />, if <paramref name="hr" /> does not equal `S_OK`.
	/// </summary>
	/// <param name="hr">The error code returned by the operation.</param>
	/// <param name="message">The format string for the error message.</param>
	/// <param name="args">The arguments passed to the error message format string.</param>
	template <typename ...TArgs>
	static inline void raiseIfFailed(HRESULT hr, std::format_string<TArgs...> message, TArgs&&... args) {
		if (SUCCEEDED(hr)) [[likely]]
			return;
		else
			throw DX12PlatformException(hr, message, std::forward<TArgs>(args)...);
	}
}

// ------------------------------------------------------------------------------------------------
// D3D12 interop implementation (with support for flip-model).
// ------------------------------------------------------------------------------------------------

class VulkanSwapChain::VulkanSwapChainImpl {
public:
	friend class VulkanSwapChain;

private:
	struct ImageResource {
	public:
		ImageResource() noexcept = default;
		ImageResource(ImageResource&&) noexcept = default;
		ImageResource(const ImageResource&) = delete;
		ImageResource& operator=(ImageResource&&) noexcept = default;
		ImageResource& operator=(const ImageResource&) = delete;

		~ImageResource() noexcept
		{
			image.Reset();
			::vkFreeMemory(device, memory, nullptr);

			if (!::CloseHandle(handle)) [[unlikely]]
				LITEFX_FATAL_ERROR(VULKAN_LOG, "Unable to close back buffer resource handle.");
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
	UInt32 m_currentImage{ };
	Array<SharedPtr<IVulkanImage>> m_presentImages{ };
	Array<ImageResource> m_imageResources;
	Array<UInt64> m_presentFences;
	WeakPtr<const VulkanDevice> m_device;
	ComPtr<ID3D12Device4> m_d3dDevice;
	ComPtr<IDXGISwapChain4> m_swapChain;
	ComPtr<ID3D12CommandQueue> m_presentQueue;
	ComPtr<ID3D12Fence> m_workloadFence = nullptr, m_presentationFence = nullptr;
	Array<ComPtr<ID3D12CommandAllocator>> m_presentCommandAllocators;
	Array<ComPtr<ID3D12GraphicsCommandList7>> m_presentCommandLists;
	
	bool m_supportsTearing = false;
	bool m_vsync = false;
	HANDLE m_fenceHandle{};

	Array<SharedPtr<const TimingEvent>> m_timingEvents;
	Array<UInt64> m_timestamps;
	Array<VkQueryPool> m_timingQueryPools;
	bool m_supportsTiming = false;

	PFN_vkImportSemaphoreWin32HandleKHR importSemaphoreWin32HandleKHR = nullptr;

public:
	VulkanSwapChainImpl(const VulkanDevice& device) :
		m_device(device.weak_from_this())
	{
		m_supportsTiming = device.adapter().limits().timestampComputeAndGraphics;

		if (!m_supportsTiming)
			LITEFX_WARNING(VULKAN_LOG, "Timestamp queries are not supported and will be disabled. Reading timestamps will always return 0.");

		importSemaphoreWin32HandleKHR = std::bit_cast<PFN_vkImportSemaphoreWin32HandleKHR>(::vkGetDeviceProcAddr(device.handle(), "vkImportSemaphoreWin32HandleKHR"));

		if (importSemaphoreWin32HandleKHR == nullptr) [[unlikely]]
			throw RuntimeException("Semaphore importing is not available. Check if all required extensions are available.");
	}

	VulkanSwapChainImpl(VulkanSwapChainImpl&&) noexcept = default;
	VulkanSwapChainImpl(const VulkanSwapChainImpl&) = delete;
	VulkanSwapChainImpl& operator=(VulkanSwapChainImpl&&) noexcept = default;
	VulkanSwapChainImpl& operator=(const VulkanSwapChainImpl&) = delete;

	~VulkanSwapChainImpl()
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release swap chain after parent device.");
		else
		{
			// Release the existing query pools.
			if (!m_timingQueryPools.empty())
				std::ranges::for_each(m_timingQueryPools, [device](auto& pool) { ::vkDestroyQueryPool(device->handle(), pool, nullptr); });

			// Release the image memory of the previously allocated images.
			std::ranges::for_each(m_presentImages, [device](const auto& image) { ::vkDestroyImage(device->handle(), std::as_const(*image).handle(), nullptr); });

			// Destroy the swap chain and interop device and resources.
			try
			{
				this->waitForInteropDevice();
				m_imageResources.clear();
				m_presentImages.clear();
				m_swapChain.Reset();
				m_d3dDevice.Reset();

				// Destroy state.
				::CloseHandle(m_fenceHandle);
				m_presentFences.clear();
				m_buffers = 0;
				m_renderArea = {};
				m_format = Format::None;
				m_currentImage = 0;
			}
			catch (...)
			{
				LITEFX_ERROR(VULKAN_LOG, "Unable to wait for interop device. Resources may not be properly released.");
			}
		}
	}

public:
	void initialize(const VulkanDevice& device, Format format, const Size2d& renderArea, UInt32 buffers, bool vsync)
	{
		if (format == Format::Other || format == Format::None) [[unlikely]]
			throw InvalidArgumentException("format", "The provided surface format it not a valid value.");

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(device.adapter().handle(), device.surface().handle());
		Format selectedFormat{ Format::None };

		if (auto match = std::ranges::find_if(surfaceFormats, [format](Format surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else [[unlikely]]
			throw InvalidArgumentException("format", "The requested format is not supported by this device.");

		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.adapter().handle(), device.surface().handle(), &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount == 0 ? buffers : deviceCaps.maxImageCount);

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Fix the render area, if required.
		Size2d extent(
			std::max(1u, std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width)), 
			std::max(1u, std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height)));

		[[unlikely]] if (extent.height() != static_cast<UInt32>(renderArea.height()) || extent.width() != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", extent.height(), extent.width(), static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		// Start initializing a mini D3D environment that hosts the swap chain.
		LITEFX_TRACE(VULKAN_LOG, "Initializing Direct3D interop for flip-model swap chain support.");
		
		// Create a D3D12 factory.
		ComPtr<IDXGIFactory7> factory;
		UInt32 tearingSupport = 0;
		m_vsync = vsync;
#ifndef NDEBUG
		D3D::raiseIfFailed(::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)), "Unable to crate D3D12 factory for interop.");
#else
		D3D::raiseIfFailed(::CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "Unable to crate D3D12 factory for interop.");
#endif // NDEBUG
		
		if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupport, sizeof(tearingSupport)))) [[unlikely]]
			LITEFX_WARNING(VULKAN_LOG, "Unable to check support for variable refresh rates. Feature will be disabled.");

		// Query the DXGI adapter.
		ComPtr<IDXGIAdapter1> adapter;
		auto adapterId = device.adapter().uniqueId();
		D3D::raiseIfFailed(factory->EnumAdapterByLuid(std::bit_cast<LUID>(adapterId), IID_PPV_ARGS(&adapter)), "Unable to query adapter \"{0:#x}\".", adapterId);

		// Create a D3D device.
		D3D::raiseIfFailed(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_d3dDevice)), "Unable to create D3D device.");

#ifndef NDEBUG
		// Try to query an info queue to forward log messages.
		ComPtr<ID3D12InfoQueue> infoQueue;

		if (FAILED(m_d3dDevice.As(&infoQueue)))
			LITEFX_WARNING(VULKAN_LOG, "Unable to query info queue on D3D interop device. Debugger support will be disabled disabled.");
		else
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, FALSE);

			// Suppress individual messages by their ID
			std::array<D3D12_MESSAGE_ID, 2> suppressIds { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE };
			std::array<D3D12_MESSAGE_SEVERITY, 1> severities { D3D12_MESSAGE_SEVERITY_INFO };	// Somehow it is required to deny info-level messages. Otherwise strange pointer issues are occurring.

			D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
			infoQueueFilter.DenyList.NumIDs = static_cast<UINT>(suppressIds.size());
			infoQueueFilter.DenyList.pIDList = suppressIds.data();
			infoQueueFilter.DenyList.NumSeverities = static_cast<UINT>(severities.size());
			infoQueueFilter.DenyList.pSeverityList = severities.data();

			D3D::raiseIfFailed(infoQueue->PushStorageFilter(&infoQueueFilter), "Unable to push message filter to info queue of D3D interop device.");
		}
#endif // NDEBUG

		// Create a command queue.
		D3D12_COMMAND_QUEUE_DESC presentQueueDesc {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			//.NodeMask = m_d3dDevice->GetNodeCount() <= 1 ? 0 : pdidProps.deviceNodeMask
		};
		D3D::raiseIfFailed(m_d3dDevice->CreateCommandQueue(&presentQueueDesc, IID_PPV_ARGS(&m_presentQueue)), "Unable to create present queue.");

		// Create the swap chain instance.
		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4}, VSync: {5} }}...", static_cast<const void*>(std::addressof(device)), images, extent.width(), extent.height(), selectedFormat, vsync);

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
			.Width = static_cast<UInt32>(extent.width()),
			.Height = static_cast<UInt32>(extent.height()),
			.Format = DX12::getFormat(selectedFormat),
			.Stereo = FALSE,
			.SampleDesc = { 1, 0 },
			.BufferUsage = DXGI_USAGE_BACK_BUFFER, // DXGI_USAGE_RENDER_TARGET_OUTPUT
			.BufferCount = images,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = (m_supportsTearing = static_cast<bool>(tearingSupport)) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : (UInt32)0
		};

		ComPtr<IDXGISwapChain1> swapChain;
		auto hwnd = device.surface().windowHandle();
		D3D::raiseIfFailed(factory->CreateSwapChainForHwnd(m_presentQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain), "Unable to create interop swap chain.");
		D3D::raiseIfFailed(swapChain.As(&m_swapChain), "The interop swap chain does not implement the IDXGISwapChain4 interface.");

		// Initialize swap chain images.
		this->createImages(device, selectedFormat, extent, images);

		// Disable Alt+Enter shortcut for fullscreen-toggle.
		if (FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))) [[unlikely]]
			LITEFX_WARNING(VULKAN_LOG, "Unable disable keyboard control sequence for full-screen switching.");

		// Initialize the present fences array.
		m_presentFences.resize(images, 0ul);

		// Create fences for synchronization.
		D3D::raiseIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_presentationFence)), "Unable to create presentation synchronization fence for swap chain.");
		D3D::raiseIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_workloadFence)), "Unable to create interop synchronization fence for swap chain.");
		D3D::raiseIfFailed(m_d3dDevice->CreateSharedHandle(m_workloadFence.Get(), nullptr, GENERIC_ALL, L"", &m_fenceHandle), "Unable to create shared handle for swap chain interop synchronization fence.");

		// Import the fence handle to signal it from Vulkan workloads.
		VkImportSemaphoreWin32HandleInfoKHR fenceImportInfo = {
			.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
			.semaphore = device.defaultQueue(QueueType::Graphics).timelineSemaphore(),
			.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT,
			.handle = m_fenceHandle
		};
		
		raiseIfFailed(importSemaphoreWin32HandleKHR(device.handle(), &fenceImportInfo), "Unable to import interop synchronization fence for swap chain.");

		// Allocate command lists.
		m_presentCommandAllocators.clear();
		m_presentCommandLists.clear();
		m_presentCommandAllocators.resize(images);
		m_presentCommandLists.resize(images);

		for (UInt32 i{ 0 }; i < images; ++i)
		{
			D3D::raiseIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_presentCommandAllocators[i])), "Unable to create command allocator for present queue commands.");
			D3D::raiseIfFailed(m_d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_presentCommandLists[i])), "Unable to create command list for present queue commands.");
		}
	}

	void reset(Format format, const Size2d& renderArea, UInt32 buffers, bool vsync)
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot reset swap chain on a released device instance.");

		// Release the image memory of the previously allocated images.
		std::ranges::for_each(m_presentImages, [device](const auto& image) { ::vkDestroyImage(device->handle(), std::as_const(*image).handle(), nullptr); });

		// Query the swap chain surface format.
		auto surfaceFormats = this->getSurfaceFormats(device->adapter().handle(), device->surface().handle());
		Format selectedFormat{ Format::None };

		if (auto match = std::ranges::find_if(surfaceFormats, [format](Format surfaceFormat) { return surfaceFormat == format; }); match != surfaceFormats.end()) [[likely]]
			selectedFormat = *match;
		else [[unlikely]]
			throw InvalidArgumentException("format", "The requested format is not supported by this device.");

		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		// Get the number of images in the swap chain.
		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->adapter().handle(), device->surface().handle(), &deviceCaps);
		UInt32 images = std::clamp(buffers, deviceCaps.minImageCount, deviceCaps.maxImageCount == 0 ? buffers : deviceCaps.maxImageCount);

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Fix the render area, if required.
		Size2d extent(
			std::max(1u, std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width)), 
			std::max(1u, std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height)));

		[[unlikely]] if (extent.height() != static_cast<UInt32>(renderArea.height()) || extent.width() != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", extent.height(), extent.width(), static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		// Reset the swap chain instance.
		LITEFX_TRACE(VULKAN_LOG, "Resetting swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4}, VSync: {5} }}...", static_cast<const void*>(device.get()), images, extent.width(), extent.height(), selectedFormat, vsync);

		// Wait for both devices to be idle.
		this->waitForInteropDevice();
		m_presentImages.clear();
		m_imageResources.clear();
		D3D::raiseIfFailed(m_swapChain->ResizeBuffers(buffers, static_cast<UInt32>(extent.width()), static_cast<UInt32>(extent.height()), DX12::getFormat(format), m_supportsTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0), "Unable to resize interop swap chain back buffers.");

		// Initialize swap chain images.
		this->createImages(*device, selectedFormat, extent, images);

		// Initialize the query pools.
		if (m_timingQueryPools.size() != images)
			this->resetQueryPools(m_timingEvents);

		// Reset the present fences array.
		m_presentFences.resize(images, 0ul);

		// Resize and re-allocate command lists.
		m_presentCommandAllocators.clear();
		m_presentCommandLists.clear();
		m_presentCommandAllocators.resize(images);
		m_presentCommandLists.resize(images);

		for (UInt32 i{ 0 }; i < images; ++i)
		{
			D3D::raiseIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_presentCommandAllocators[i])), "Unable to create command allocator for present queue commands.");
			D3D::raiseIfFailed(m_d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_presentCommandLists[i])), "Unable to create command list for present queue commands.");
		}

		// Store vsync flag.
		m_vsync = vsync;
	}

	void createImages(const VulkanDevice& device, Format format, const Size2d& renderArea, UInt32 buffers)
	{
		// NOTE: We maintain two sets of images: the swap chain back buffers and separate image resources that are shared and written to by the Vulkan renderer. During present
		//       the `m_workloadFence` is waited upon before copying the shared images into the swap chain back buffers. While it is possible to share and write the back buffers
		//       directly, they are not synchronized (even waiting for the workload fence before presenting is not enough). This causes back buffers to be written whilst they
		//       presented, resulting in artifacts or flickering.

		// Acquire the swap chain images.
		m_presentImages.resize(buffers);
		m_imageResources.resize(buffers);
		std::ranges::generate(m_presentImages, [&, i = 0]() mutable {
			// Acquire a image resource for the back buffer and create a shared handle for it.
			ComPtr<ID3D12Resource> resource;
			HANDLE resourceHandle = nullptr;
			const int image = i++;
			//D3D::raiseIfFailed(m_swapChain->GetBuffer(image, IID_PPV_ARGS(&resource)), "Unable to acquire image resource from swap chain back buffer {0}.", image);
			
			// Create a image resource.
			D3D12_RESOURCE_DESC imageDesc = {
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Width = renderArea.width(),
				.Height = static_cast<UInt32>(renderArea.height()),
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DX12::getFormat(format),
				.SampleDesc = { 1, 0 },
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE
			};

			auto heapInfo = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			
			D3D::raiseIfFailed(m_d3dDevice->CreateCommittedResource(&heapInfo, D3D12_HEAP_FLAG_SHARED, &imageDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)), "Unable to create image resource to interop back buffer.");
			D3D::raiseIfFailed(m_d3dDevice->CreateSharedHandle(resource.Get(), nullptr, GENERIC_ALL, nullptr, &resourceHandle), "Unable to create shared handle for interop back buffer.");

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
				.extent = { std::max<UInt32>(1, static_cast<UInt32>(renderArea.width())), std::max<UInt32>(1, static_cast<UInt32>(renderArea.height())), 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			// Create the image.
			VkImage backBuffer{};
			raiseIfFailed(::vkCreateImage(device.handle(), &imageInfo, nullptr, &backBuffer), "Unable to create swap-chain image.");

			// Get the memory requirements.
			VkMemoryRequirements memoryRequirements{};
			VkPhysicalDeviceMemoryProperties memoryProperties{};
			::vkGetImageMemoryRequirements(device.handle(), backBuffer, &memoryRequirements);
			::vkGetPhysicalDeviceMemoryProperties(device.adapter().handle(), &memoryProperties);

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

			VkDeviceMemory imageMemory{};
			raiseIfFailed(::vkAllocateMemory(device.handle(), &allocationInfo, nullptr, &imageMemory), "Unable to allocate memory for imported interop swap chain buffer.");
			raiseIfFailed(::vkBindImageMemory(device.handle(), backBuffer, imageMemory, 0), "Unable to bind back-buffer.");

			// Return the image instance.
			m_imageResources[image].device = device.handle();
			m_imageResources[image].memory = imageMemory;
			m_imageResources[image].handle = resourceHandle;
			m_imageResources[image].image = std::move(resource);

			return VulkanImage::create(backBuffer, Size3d{imageInfo.extent.width, imageInfo.extent.height, imageInfo.extent.depth}, format, ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination);
		});

		// Store state variables.
		m_renderArea = renderArea;
		m_format = format;
		m_buffers = buffers;
		m_currentImage = 0;
	}

	void resetQueryPools(const Array<SharedPtr<const TimingEvent>>& timingEvents)
	{
		// No events - no pools.
		if (timingEvents.empty())
			return;

		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot reset query pools on a released device instance.");

		// Release the existing query pools.
		if (!m_timingQueryPools.empty())
			std::ranges::for_each(m_timingQueryPools, [device](auto& pool) { ::vkDestroyQueryPool(device->handle(), pool, nullptr); });

		// Resize the query pools array and allocate a pool for each back buffer.
		m_timingQueryPools.resize(m_buffers);
		std::ranges::generate(m_timingQueryPools, [&timingEvents, device]() {
			VkQueryPoolCreateInfo poolInfo {
				.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
				.queryType = VkQueryType::VK_QUERY_TYPE_TIMESTAMP,
				.queryCount = static_cast<UInt32>(timingEvents.size())
			};

			VkQueryPool pool{};
			raiseIfFailed(::vkCreateQueryPool(device->handle(), &poolInfo, nullptr, &pool), "Unable to allocate timestamp query pool.");
			::vkResetQueryPool(device->handle(), pool, 0, static_cast<UInt32>(timingEvents.size()));

			return pool;
		});

		// Store the event and resize the time stamp collection.
		m_timingEvents = timingEvents;
		m_timestamps.resize(timingEvents.size());
	}

	UInt32 swapBackBuffer()
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot swap back buffers on a released device instance.");

		// Get the current back buffer index.
		m_currentImage = m_swapChain->GetCurrentBackBufferIndex();

		// Wait for all workloads on this image to finish in order to be able to re-use the associated command buffers.
		device->defaultQueue(QueueType::Graphics).waitFor(m_presentFences[m_currentImage]);

		// Wait for the last presentation on the current image to finish, so that we can re-use the command buffers associated with it.
		if (m_presentationFence->GetCompletedValue() < m_presentFences[m_currentImage])
		{
			HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
			HRESULT hr = m_presentationFence->SetEventOnCompletion(m_presentFences[m_currentImage], eventHandle);

			if (SUCCEEDED(hr))
				::WaitForSingleObject(eventHandle, INFINITE);

			::CloseHandle(eventHandle);
			raiseIfFailed(hr, "Unable to register presentation fence completion event.");
		}

		// Query the timing events.
		if (m_supportsTiming && !m_timingEvents.empty()) [[likely]]
		{
			auto result = ::vkGetQueryPoolResults(device->handle(), m_timingQueryPools[m_currentImage], 0, static_cast<UInt32>(m_timestamps.size()), m_timestamps.size() * sizeof(UInt64), m_timestamps.data(), sizeof(UInt64), VK_QUERY_RESULT_64_BIT);

			if (result != VK_NOT_READY)	// Initial frames do not yet contain query results.
				raiseIfFailed(result, "Unable to query timing events.");

			// Reset the query pool.
			::vkResetQueryPool(device->handle(), m_timingQueryPools[m_currentImage], 0, static_cast<UInt32>(m_timestamps.size()));
		}

		// Return the new back buffer index.
		return m_currentImage;
	}

	void present(UInt64 fence)
	{
		// Wait for all commands to finish on the default graphics queue. We assume that this is the last queue that receives (synchronized) workloads, as it is expected to
		// handle presentation by convention. Note that this performs a GPU-side wait for the fence and does not block.
		m_presentQueue->Wait(m_workloadFence.Get(), m_presentFences[m_currentImage] = fence);

		// Copy shared images to back buffers. See `createImages` for details on why we do this.
		ComPtr<ID3D12Resource> resource;
		D3D::raiseIfFailed(m_swapChain->GetBuffer(m_currentImage, IID_PPV_ARGS(&resource)), "Unable to acquire image resource from swap chain back buffer {0}.", m_currentImage);
		
		auto& allocator = m_presentCommandAllocators[m_currentImage];
		auto& commandList = m_presentCommandLists[m_currentImage];
		D3D::raiseIfFailed(allocator->Reset(), "Unable to reset command allocator before presentation.");
		D3D::raiseIfFailed(commandList->Reset(allocator.Get(), nullptr), "Unable to reset command list before presentation.");

		// Transition into copy destination state and copy the resource.
		D3D12_TEXTURE_BARRIER barrier = {
			.SyncBefore = D3D12_BARRIER_SYNC_NONE,
			.SyncAfter = D3D12_BARRIER_SYNC_COPY,
			.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
			.AccessAfter = D3D12_BARRIER_ACCESS_COPY_DEST,
			.LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_DEST,
			.pResource = resource.Get(),
			.Subresources = {.NumMipLevels = 1, .NumArraySlices = 1, .NumPlanes = 1 }
		};

		D3D12_BARRIER_GROUP barrierGroup = {
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &barrier
		};

		commandList->Barrier(1, &barrierGroup);
		commandList->CopyResource(resource.Get(), m_imageResources[m_currentImage].image.Get());

		// Transition into present state and close the command list.
		barrier = {
			.SyncBefore = D3D12_BARRIER_SYNC_COPY,
			.SyncAfter = D3D12_BARRIER_SYNC_NONE,
			.AccessBefore = D3D12_BARRIER_ACCESS_COPY_DEST,
			.AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS,
			.LayoutBefore = D3D12_BARRIER_LAYOUT_COPY_DEST,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT,
			.pResource = resource.Get(),
			.Subresources = { .NumMipLevels = 1, .NumArraySlices = 1, .NumPlanes = 1 }
		};

		commandList->Barrier(1, &barrierGroup);

		D3D::raiseIfFailed(commandList->Close(), "Unable to close command list for presentation.");
		
		// Submit the command buffer.
		auto commandBuffer = static_cast<ID3D12CommandList*>(commandList.Get());
		m_presentQueue->ExecuteCommandLists(1u, &commandBuffer);

		// Do the presentation.
		if (m_vsync)
			D3D::raiseIfFailed(m_swapChain->Present(1, 0), "Unable to queue present event on swap chain.");
		else
			D3D::raiseIfFailed(m_swapChain->Present(0, m_supportsTearing ? DXGI_PRESENT_ALLOW_TEARING : 0), "Unable to queue present event on swap chain.");

		D3D::raiseIfFailed(m_presentQueue->Signal(m_presentationFence.Get(), m_presentFences[m_currentImage]), "Unable to signal presentation fence.");
	}
	
	const VkQueryPool& currentTimestampQueryPool()
	{
		return m_timingQueryPools[m_currentImage];
	}

public:
	Array<Format> getSurfaceFormats(const VkPhysicalDevice adapter, const VkSurfaceKHR surface) const
	{
		uint32_t formats{};
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		return availableFormats 
			| std::views::transform([](const VkSurfaceFormatKHR& format) { return Vk::getFormat(format.format); })
			| std::ranges::to<Array<Format>>();
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, Format format) const
	{
		uint32_t formats{};
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
		D3D::raiseIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Unable to create queue synchronization fence on interop device.");

		// Create a signal event.
		HANDLE eventHandle = ::CreateEvent(nullptr, false, false, nullptr);
		HRESULT hr = fence->SetEventOnCompletion(1, eventHandle);

		if (FAILED(hr))
		{
			::CloseHandle(eventHandle);
			D3D::raiseIfFailed(hr, "Unable to register queue synchronization fence completion event.");
		}

		// Signal the event value on the graphics queue.
		hr = m_presentQueue->Signal(fence.Get(), 1);

		if (FAILED(hr))
		{
			::CloseHandle(eventHandle);
			raiseIfFailed(hr, "Unable to wait for queue synchronization fence.");
		}

		// Wait for the fence signal.
		if (fence->GetCompletedValue() < 1)
			::WaitForSingleObject(eventHandle, INFINITE);

		::CloseHandle(eventHandle);
	}
};
#endif // !defined(USE_VULKAN_INTEROP_SWAP_CHAIN)

#undef USE_VULKAN_INTEROP_SWAP_CHAIN

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSwapChain::VulkanSwapChain(const VulkanDevice& device, Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync) :
	m_impl(device)
{
	m_impl->initialize(device, surfaceFormat, renderArea, buffers, enableVsync);
}

VulkanSwapChain::~VulkanSwapChain() noexcept = default;

const VkQueryPool& VulkanSwapChain::timestampQueryPool() const noexcept
{
	return m_impl->currentTimestampQueryPool();
}

const Array<SharedPtr<const TimingEvent>>& VulkanSwapChain::timingEvents() const
{
	return m_impl->m_timingEvents;
}

SharedPtr<const TimingEvent> VulkanSwapChain::timingEvent(UInt32 queryId) const
{
	if (queryId >= m_impl->m_timingEvents.size())
		throw ArgumentOutOfRangeException("queryId", std::make_pair(0uz, m_impl->m_timingEvents.size()), static_cast<size_t>(queryId), "No timing event has been registered for query ID {0}.", queryId);

	return m_impl->m_timingEvents[queryId];
}

UInt64 VulkanSwapChain::readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const
{
	if (!m_impl->m_supportsTiming) [[unlikely]]
		return 0;

	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	if (auto match = std::find(m_impl->m_timingEvents.begin(), m_impl->m_timingEvents.end(), timingEvent); match != m_impl->m_timingEvents.end()) [[likely]]
		return m_impl->m_timestamps[std::distance(m_impl->m_timingEvents.begin(), match)];

	throw InvalidArgumentException("timingEvent", "The timing event is not registered on the swap chain.");
}

UInt32 VulkanSwapChain::resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const
{
	if (!m_impl->m_supportsTiming) [[unlikely]]
		return 0;

	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	if (auto match = std::find(m_impl->m_timingEvents.begin(), m_impl->m_timingEvents.end(), timingEvent); match != m_impl->m_timingEvents.end()) [[likely]]
		return static_cast<UInt32>(std::distance(m_impl->m_timingEvents.begin(), match));

	throw InvalidArgumentException("timingEvent", "The timing event is not registered on the swap chain.");
}

const IGraphicsDevice& VulkanSwapChain::device() const
{
	auto device = m_impl->m_device.lock();

	if (device == nullptr)
		throw RuntimeException("Unable to obtain device instance. The device has already been released.");

	return *device;
}

Format VulkanSwapChain::surfaceFormat() const noexcept
{
	return m_impl->m_format;
}

UInt32 VulkanSwapChain::buffers() const noexcept
{
	return m_impl->m_buffers;
}

const Size2d& VulkanSwapChain::renderArea() const noexcept
{
	return m_impl->m_renderArea;
}

bool VulkanSwapChain::verticalSynchronization() const noexcept
{
	return m_impl->m_vsync;
}

IVulkanImage* VulkanSwapChain::image(UInt32 backBuffer) const
{
	if (backBuffer >= m_impl->m_presentImages.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("backBuffer", std::make_pair(0uz, m_impl->m_presentImages.size()), static_cast<size_t>(backBuffer), "The back buffer must be a valid index.");

	return m_impl->m_presentImages[backBuffer].get();
}

const IVulkanImage& VulkanSwapChain::image() const noexcept
{
	return *m_impl->m_presentImages[m_impl->m_currentImage];
}

const Array<SharedPtr<IVulkanImage>>& VulkanSwapChain::images() const noexcept
{
	return m_impl->m_presentImages;
}

void VulkanSwapChain::present(UInt64 fence) const 
{
	m_impl->present(fence);
}

Enumerable<Format> VulkanSwapChain::getSurfaceFormats() const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot query surface formats from a released device instance.");

	return m_impl->getSurfaceFormats(device->adapter().handle(), device->surface().handle());
}

void VulkanSwapChain::addTimingEvent(SharedPtr<const TimingEvent> timingEvent)
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	if (!m_impl->m_supportsTiming)
		return;

	LITEFX_DEBUG(VULKAN_LOG, "Registering timing event: \"{0}\".", timingEvent->name());
	
	auto events = m_impl->m_timingEvents;
	events.push_back(timingEvent);
	m_impl->resetQueryPools(events);
}

void VulkanSwapChain::reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync)
{
	m_impl->reset(surfaceFormat, renderArea, buffers, enableVsync);
	this->reseted(this, { surfaceFormat, renderArea, buffers, enableVsync });
}

UInt32 VulkanSwapChain::swapBackBuffer() const
{
	auto backBuffer = m_impl->swapBackBuffer();
	this->swapped(this, { });
	return backBuffer;
}