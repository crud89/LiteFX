#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSwapChain::VulkanSwapChainImpl : public Implement<VulkanSwapChain> {
public:
	friend class VulkanSwapChain;

private:
	Array<UniquePtr<ITexture>> m_frames;
	const VulkanDevice* m_device;
	Size2d m_size { };
	Format m_format{ Format::None };
	VkSemaphore m_swapSemaphore;

public:
	VulkanSwapChainImpl(VulkanSwapChain* parent, const VulkanDevice* device) :
		base(parent), m_device(device) { }
	
	~VulkanSwapChainImpl()
	{
		this->cleanup();
	}

private:
	void cleanup()
	{
		// Clear the frame buffers.
		m_frames.clear();

		// Destroy the swap chain itself.
		::vkDestroySwapchainKHR(m_device->handle(), m_parent->handle(), nullptr);

		// Destroy the image swap semaphore.
		::vkDestroySemaphore(m_device->handle(), m_swapSemaphore, nullptr);
	}

private:
	void loadImages(const VkSwapchainKHR& swapChain, const Format& format, const VkExtent2D& extent)
	{
		uint32_t images;
		::vkGetSwapchainImagesKHR(m_device->handle(), swapChain, &images, nullptr);

		Array<VkImage> imageChain(images);
		Array<UniquePtr<ITexture>> textures(images);

		::vkGetSwapchainImagesKHR(m_device->handle(), swapChain, &images, imageChain.data());
		std::generate(textures.begin(), textures.end(), [&, i = 0]() mutable { 
			return m_device->makeTexture2d(imageChain[i++], format, Size2d(static_cast<size_t>(extent.width), static_cast<size_t>(extent.height)));
		});

		m_frames = std::move(textures);
		m_size = Size2d(static_cast<size_t>(extent.width), static_cast<size_t>(extent.height));
		m_format = format;
	}

private:
	VkSurfaceKHR getSurface() const noexcept
	{
		auto surface = dynamic_cast<const VulkanSurface*>(m_device->getBackend()->getSurface());
		return surface ? surface->handle() : nullptr;
	}

	VkPhysicalDevice getAdapter() const noexcept
	{
		auto adapter = dynamic_cast<const VulkanGraphicsAdapter*>(m_device->getBackend()->getAdapter());
		return adapter ? adapter->handle() : nullptr;
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, const Format& format) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		auto match = std::find_if(availableFormats.begin(), availableFormats.end(), [format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == ::getFormat(format); });

		if (match == availableFormats.end())
			return VK_COLOR_SPACE_MAX_ENUM_KHR;

		return match->colorSpace;
	}

public:
	VkSwapchainKHR initialize(const Format& format)
	{
		if (format == Format::Other || format == Format::None)
			throw std::invalid_argument("The provided surface format it not a valid value.");

		auto adapter = this->getAdapter();
		auto surface = this->getSurface();

		if (adapter == nullptr)
			throw std::runtime_error("The adapter is not a valid Vulkan adapter.");

		if (surface == nullptr)
			throw std::runtime_error("The surface is not a valid Vulkan surface.");

		// Query swap chain format.
		auto surfaceFormats = m_device->getSurfaceFormats();
		auto selectedFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [format](const Format& surfaceFormat) { return surfaceFormat == format; });

		if (selectedFormat == surfaceFormats.end())
			throw std::invalid_argument("The requested format is not supported by this device.");

		VkSurfaceCapabilitiesKHR deviceCaps;
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &deviceCaps);

		// Get the number of images in the swap chain.
		uint32_t images = deviceCaps.minImageCount + 1;

		if (deviceCaps.maxImageCount > 0 && images > deviceCaps.maxImageCount)
			images = deviceCaps.maxImageCount;

		// Create a swap chain.
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = images;
		createInfo.imageFormat = ::getFormat(*selectedFormat);
		createInfo.imageColorSpace = this->findColorSpace(adapter, surface, *selectedFormat);
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = deviceCaps.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// TODO: For maximum platform compatibility we should check if `deviceCaps.currentExtent.width` equals UINT32_MAX and select another resolution in this case.
		createInfo.imageExtent = deviceCaps.currentExtent;

		// Set the present mode to VK_PRESENT_MODE_FIFO_KHR for now, which is always available.
		// TODO: Change present mode:
		// -VK_PRESENT_MODE_IMMEDIATE_KHR: to disable VSync
		// -VK_PRESENT_MODE_MAILBOX_KHR: to enable triple buffering
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px }}...", fmt::ptr(m_device), images, createInfo.imageExtent.width, createInfo.imageExtent.height);

		// Create the swap chain instance.
		VkSwapchainKHR swapChain;

		if (::vkCreateSwapchainKHR(m_device->handle(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
			throw std::runtime_error("Swap chain could not be created.");

		this->loadImages(swapChain, format, deviceCaps.currentExtent);

		// Create a semaphore for swapping images.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (::vkCreateSemaphore(m_device->handle(), &semaphoreInfo, nullptr, &m_swapSemaphore) != VK_SUCCESS)
			throw std::runtime_error("Unable to create swap semaphore.");

		return swapChain;
	}

	void reset()
	{
		// Cleanup and re-initialize.
		this->cleanup();
		m_parent->handle() = this->initialize(m_format);
	}

	UInt32 swapBackBuffer() const
	{
		UInt32 imageIndex;

		if (::vkAcquireNextImageKHR(m_device->handle(), m_parent->handle(), UINT64_MAX, m_swapSemaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS)
			throw std::runtime_error("Unable to swap front buffer.");

		return imageIndex;
	}

public:
	Array<const ITexture*> getFrames() const noexcept
	{
		Array<const ITexture*> frames(m_frames.size());
		std::generate(std::begin(frames), std::end(frames), [&, i = 0]() mutable { return m_frames[i++].get(); });

		return frames;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSwapChain::VulkanSwapChain(const VulkanDevice* device, const Format& format) :
	m_impl(makePimpl<VulkanSwapChainImpl>(this, device)), IResource(nullptr)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");

	this->handle() = m_impl->initialize(format);
}

VulkanSwapChain::~VulkanSwapChain() noexcept = default;

const IGraphicsDevice* VulkanSwapChain::getDevice() const noexcept
{
	return m_impl->m_device;
}

const Size2d& VulkanSwapChain::getBufferSize() const noexcept
{
	return m_impl->m_size;
}

size_t VulkanSwapChain::getWidth() const noexcept
{
	return m_impl->m_size.width();
}

size_t VulkanSwapChain::getHeight() const noexcept
{
	return m_impl->m_size.height();
}

const Format& VulkanSwapChain::getFormat() const noexcept
{
	return m_impl->m_format;
}

Array<const ITexture*> VulkanSwapChain::getFrames() const noexcept
{
	return m_impl->getFrames();
}

UInt32 VulkanSwapChain::swapBackBuffer() const
{
	return m_impl->swapBackBuffer();
}

void VulkanSwapChain::reset()
{
	m_impl->reset();
}

VkSemaphore VulkanSwapChain::getSemaphore() const noexcept
{
	return m_impl->m_swapSemaphore;
}