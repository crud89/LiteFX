#include <litefx/backends/vulkan.hpp>
#include <atomic>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
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

public:
	VulkanSwapChainImpl(VulkanSwapChain* parent) : 
		base(parent) 
	{ 
	}
	
	~VulkanSwapChainImpl()
	{
		this->cleanup();
	}

public:
	VkSwapchainKHR initialize(const Format& format, const Size2d& renderArea, const UInt32& buffers)
	{
		if (format == Format::Other || format == Format::None)
			throw InvalidArgumentException("The provided surface format it not a valid value.");

		auto adapter = m_parent->getDevice()->adapter().handle();
		auto surface = m_parent->getDevice()->surface().handle();

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
		createInfo.imageFormat = ::getFormat(selectedFormat);
		createInfo.imageColorSpace = this->findColorSpace(adapter, surface, selectedFormat);
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = deviceCaps.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		createInfo.imageExtent.height = std::clamp(static_cast<UInt32>(renderArea.height()), deviceCaps.minImageExtent.height, deviceCaps.maxImageExtent.height);
		createInfo.imageExtent.width = std::clamp(static_cast<UInt32>(renderArea.width()), deviceCaps.minImageExtent.width, deviceCaps.maxImageExtent.width);

		// Set the present mode to VK_PRESENT_MODE_FIFO_KHR for now, which is always available.
		// TODO: Change present mode:
		// -VK_PRESENT_MODE_IMMEDIATE_KHR: to disable VSync (e.g. on variable refresh displays)
		// -VK_PRESENT_MODE_MAILBOX_KHR: to enable triple buffering
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

		LITEFX_TRACE(VULKAN_LOG, "Creating swap chain for device {0} {{ Images: {1}, Extent: {2}x{3} Px, Format: {4} }}...", fmt::ptr(m_parent->getDevice()), images, createInfo.imageExtent.width, createInfo.imageExtent.height, selectedFormat);

		// Log if something needed to be changed.
		[[unlikely]] if (selectedFormat != format)
			LITEFX_INFO(VULKAN_LOG, "The format {0} has been changed to the compatible format {1}.", format, selectedFormat);

		[[unlikely]] if (createInfo.imageExtent.height != static_cast<UInt32>(renderArea.height()) || createInfo.imageExtent.width != static_cast<UInt32>(renderArea.width()))
			LITEFX_INFO(VULKAN_LOG, "The render area has been adjusted to {0}x{1} Px (was {2}x{3} Px).", createInfo.imageExtent.height, createInfo.imageExtent.width, static_cast<UInt32>(renderArea.height()), static_cast<UInt32>(renderArea.width()));

		[[unlikely]] if (images != buffers)
			LITEFX_INFO(VULKAN_LOG, "The number of buffers has been adjusted from {0} to {1}.", buffers, images);

		// Create the swap chain instance.
		VkSwapchainKHR swapChain;
		raiseIfFailed<RuntimeException>(::vkCreateSwapchainKHR(m_parent->getDevice()->handle(), &createInfo, nullptr, &swapChain), "Swap chain could not be created.");

		// Create a semaphore for swapping images.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_swapSemaphores.resize(images);
		std::ranges::generate(m_swapSemaphores, [&]() mutable {
			VkSemaphore semaphore;
			raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_parent->getDevice()->handle(), &semaphoreInfo, nullptr, &semaphore), "Unable to create swap semaphore.");
			
			return semaphore;
		});

		// Store state variables.
		m_renderArea = Size2d(static_cast<size_t>(createInfo.imageExtent.width), static_cast<size_t>(createInfo.imageExtent.height));
		m_format = selectedFormat;
		m_buffers = images;
		m_currentImage = 0;

		return swapChain;
	}

	void cleanup()
	{
		// Destroy the swap chain itself.
		::vkDestroySwapchainKHR(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);

		// Destroy the image swap semaphores.
		std::ranges::for_each(m_swapSemaphores, [&](const auto& semaphore) { ::vkDestroySemaphore(m_parent->getDevice()->handle(), semaphore, nullptr); });

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
		raiseIfFailed<RuntimeException>(::vkAcquireNextImageKHR(m_parent->getDevice()->handle(), m_parent->handle(), UINT64_MAX, this->currentSemaphore(), VK_NULL_HANDLE, &nextImage), "Unable to swap front buffer.");

		return nextImage;
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

		return availableFormats | std::views::transform([](const VkSurfaceFormatKHR& format) { return ::getFormat(format.format); }) | ranges::to<Array<Format>>();
	}

	VkColorSpaceKHR findColorSpace(const VkPhysicalDevice adapter, const VkSurfaceKHR surface, const Format& format) const noexcept
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());

		if (auto match = std::ranges::find_if(availableFormats, [&format](const VkSurfaceFormatKHR& surfaceFormat) { return surfaceFormat.format == ::getFormat(format); }); match != availableFormats.end()) [[likely]]
			return match->colorSpace;

		return VK_COLOR_SPACE_MAX_ENUM_KHR;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSwapChain::VulkanSwapChain(const VulkanDevice& device, const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) :
	m_impl(makePimpl<VulkanSwapChainImpl>(this)), VulkanRuntimeObject<VulkanDevice>(device, &device), IResource(nullptr)
{
	this->handle() = m_impl->initialize(surfaceFormat, renderArea, buffers);
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

Array<Format> VulkanSwapChain::getSurfaceFormats() const noexcept
{
	return m_impl->getSurfaceFormats(this->getDevice()->adapter().handle(), this->getDevice()->surface().handle());
}

void VulkanSwapChain::reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers)
{
	// Cleanup and re-initialize.
	m_impl->cleanup();
	this->handle() = m_impl->initialize(surfaceFormat, renderArea, buffers);

	// Get the swap chain images and return them.
	UInt32 imageCount = m_impl->m_buffers;	// NOTE: No guarantee, that this equals buffers (since it may have been clamped)!
	Array<VkImage> imageChain(m_impl->m_buffers);
	::vkGetSwapchainImagesKHR(this->getDevice()->handle(), this->handle(), &m_impl->m_buffers, imageChain.data());

	m_impl->m_presentImages = imageChain |
		std::views::transform([this](const VkImage& image) { return makeUnique<_VMAImage>(*this->getDevice(), image, 1, m_impl->m_renderArea, m_impl->m_format); }) |
		ranges::to<Array<UniquePtr<IVulkanImage>>>();
}

UInt32 VulkanSwapChain::swapBackBuffer() const
{
	return m_impl->swapBackBuffer();
}