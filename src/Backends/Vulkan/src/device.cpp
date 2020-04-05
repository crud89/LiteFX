#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanDevice::VulkanDeviceImpl {
private:
	UniquePtr<VulkanSwapChain> m_swapChain;
	SharedPtr<VulkanQueue> m_queue;
	Array<String> m_extensions;

public:
	VulkanDeviceImpl(SharedPtr<VulkanQueue> queue, const Array<String>& extensions = { }) noexcept :
		m_queue(queue), m_extensions(extensions) { }

private:
	VkSurfaceKHR getSurface(const VulkanDevice& parent) const noexcept
	{
		auto surface = dynamic_cast<const VulkanSurface*>(parent.getSurface());
		return surface ? surface->handle() : nullptr;
	}

	VkPhysicalDevice getAdapter(const VulkanDevice& parent) const noexcept
	{
		auto adapter = dynamic_cast<const VulkanGraphicsAdapter*>(parent.getAdapter());
		return adapter ? adapter->handle() : nullptr;
	}
	
	Array<Format> getSurfaceFormats(const VkPhysicalDevice adapter, const VkSurfaceKHR surface) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		Array<Format> surfaceFormats(formats);

		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());
		std::generate(surfaceFormats.begin(), surfaceFormats.end(), [&availableFormats, i = 0]() mutable { return getFormat(availableFormats[i++].format); });

		return surfaceFormats;
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, const Format& format) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		auto match = std::find_if(availableFormats.begin(), availableFormats.end(), [format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == getFormat(format); });

		if (match == availableFormats.end())
			return VK_COLOR_SPACE_MAX_ENUM_KHR;

		return match->colorSpace;
	}

public:
	Array<Format> getSurfaceFormats(const VulkanDevice& parent) const
	{
		auto adapter = this->getAdapter(parent);
		auto surface = this->getSurface(parent);

		if (adapter == nullptr)
			throw std::runtime_error("The adapter is not a valid Vulkan adapter.");

		if (surface == nullptr)
			throw std::runtime_error("The surface is not a valid Vulkan surface.");

		return this->getSurfaceFormats(adapter, surface);
	}

	const Array<String>& getExtensions() const noexcept 
	{
		return m_extensions;
	}

	const VulkanSwapChain* getSwapChain() const noexcept
	{
		return m_swapChain.get();
	}

	void createSwapChain(const VulkanDevice& parent, const Format& format)
	{
		if (format == Format::Other || format == Format::None)
			throw std::invalid_argument("The provided surface format it not a valid value.");

		auto adapter = this->getAdapter(parent);
		auto surface = this->getSurface(parent);

		if (adapter == nullptr)
			throw std::runtime_error("The adapter is not a valid Vulkan adapter.");

		if (surface == nullptr)
			throw std::runtime_error("The surface is not a valid Vulkan surface.");

		// Query swap chain format.
		auto surfaceFormats = this->getSurfaceFormats(adapter, surface);
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
		createInfo.imageFormat = getFormat(*selectedFormat);
		createInfo.imageColorSpace = findColorSpace(adapter, surface, *selectedFormat);
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

		// Create the swap chain instance.
		VkSwapchainKHR swapChain;

		if (::vkCreateSwapchainKHR(parent.handle(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
			throw std::runtime_error("Swap chain could not be created.");

		m_swapChain = makeUnique<VulkanSwapChain>(swapChain, &parent);
	}
};

VulkanDevice::VulkanDevice(const VulkanGraphicsAdapter* adapter, const VulkanSurface* surface, const VkDevice device, SharedPtr<VulkanQueue> queue, const Format& format, const Array<String>& extensions) :
	IResource(device), m_impl(makePimpl<VulkanDeviceImpl>(queue, extensions)), GraphicsDevice(adapter, surface)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");
	
	if (queue == nullptr)
		throw std::invalid_argument("The argument `queue` must be initialized.");

	queue->initDeviceQueue(this);
	m_impl->createSwapChain(*this, format);
}

VulkanDevice::~VulkanDevice() noexcept
{
	::vkDestroySwapchainKHR(this->handle(), m_impl->getSwapChain()->handle(), nullptr);
	::vkDestroyDevice(this->handle(), nullptr);
}

const Array<String>& VulkanDevice::getExtensions() const noexcept
{
	return m_impl->getExtensions();
}

Array<Format> VulkanDevice::getSurfaceFormats() const
{
	return m_impl->getSurfaceFormats(*this);
}

const ISwapChain* VulkanDevice::getSwapChain() const noexcept
{
	return m_impl->getSwapChain();
}