#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl {
private:
	UniquePtr<VulkanSwapChain> m_swapChain;
	VulkanQueue* m_queue;
	Array<String> m_extensions;

public:
	VulkanDeviceImpl(VulkanQueue* queue, const Array<String>& extensions = { }) noexcept :
		m_queue(queue), m_extensions(extensions) { this->defineMandatoryExtensions(); }

private:
	void defineMandatoryExtensions() noexcept
	{
		m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

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

public:
	bool validateDeviceExtensions(const VulkanDevice& parent, const Array<String>& extensions) const noexcept
	{
		auto availableExtensions = this->getAvailableDeviceExtensions(parent);

		return std::all_of(extensions.begin(), extensions.end(), [&availableExtensions](const String& extension) {
			return std::find_if(availableExtensions.begin(), availableExtensions.end(), [&extension](String& str) {
				return std::equal(str.begin(), str.end(), extension.begin(), extension.end(), [](char a, char b) {
					return std::tolower(a) == std::tolower(b);
					});
				}) != availableExtensions.end();
			});
	}

	Array<String> getAvailableDeviceExtensions(const VulkanDevice& parent) const noexcept
	{
		auto adapter = this->getAdapter(parent);

		uint32_t extensions = 0;
		::vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extensions, nullptr);

		Array<VkExtensionProperties> availableExtensions(extensions);
		Array<String> extensionNames(extensions);

		::vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extensions, availableExtensions.data());
		std::generate(extensionNames.begin(), extensionNames.end(), [&availableExtensions, i = 0]() mutable { return availableExtensions[i++].extensionName; });

		return extensionNames;
	}

public:
	VkDevice initialize(const VulkanDevice& parent, const Format& format)
	{
		auto adapter = this->getAdapter(parent);

		if (adapter == nullptr)
			throw std::invalid_argument("The argument `adapter` must be initialized.");

		if (!this->validateDeviceExtensions(parent, m_extensions))
			throw std::runtime_error("Some required device extensions are not supported by the system.");

		// Parse the extensions.
		// NOTE: For legacy support we should also set the device validation layers here.
		std::vector<const char*> requiredExtensions(m_extensions.size());
		std::generate(requiredExtensions.begin(), requiredExtensions.end(), [this, i = 0]() mutable { return m_extensions[i++].data(); });

		// Define a graphics queue for the device.
		const float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_queue->getId();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// Define the device features.
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Define the device itself.
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// Create the device.
		VkDevice device;

		if (::vkCreateDevice(adapter, &createInfo, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("Unable to create Vulkan device.");

		return device;
	}

	void createSwapChain(const VulkanDevice& parent, const Format& format)
	{
		m_swapChain = makeUnique<VulkanSwapChain>(&parent, format);
	}

	void initDeviceQueue(const VulkanDevice& parent)
	{
		m_queue->initDeviceQueue(&parent);
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

		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		Array<Format> surfaceFormats(formats);

		::vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formats, availableFormats.data());
		std::generate(surfaceFormats.begin(), surfaceFormats.end(), [&availableFormats, i = 0]() mutable { return getFormat(availableFormats[i++].format); });

		return surfaceFormats;
	}

	const Array<String>& getExtensions() const noexcept 
	{
		return m_extensions;
	}

	const VulkanSwapChain* getSwapChain() const noexcept
	{
		return m_swapChain.get();
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const VulkanGraphicsAdapter* adapter, const VulkanSurface* surface, VulkanQueue* deviceQueue, const Format& format, const Array<String>& extensions) :
	IResource(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(deviceQueue, extensions)), GraphicsDevice(adapter, surface)
{
	if (deviceQueue == nullptr)
		throw std::invalid_argument("The argument `deviceQueue` must be initialized.");
	
	this->handle() = m_impl->initialize(*this, format);

	m_impl->createSwapChain(*this, format);
	m_impl->initDeviceQueue(*this);
}

VulkanDevice::~VulkanDevice() noexcept
{
	m_impl.destroy();

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

bool VulkanDevice::validateDeviceExtensions(const Array<String>& extensions) const noexcept
{
	return m_impl->validateDeviceExtensions(*this, extensions);
}

Array<String> VulkanDevice::getAvailableDeviceExtensions() const noexcept
{
	return m_impl->getAvailableDeviceExtensions(*this);
}

// ------------------------------------------------------------------------------------------------
// Factory.
// ------------------------------------------------------------------------------------------------

//UniquePtr<ITexture> VulkanDevice::createTexture2d(const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const
//{
//  VkImage image;
//  // TODO: ...
//	return makeUnique<VulkanTexture>(this, image, format, size);
//}

UniquePtr<ITexture> VulkanDevice::makeTexture2d(VkImage image, const Format& format, const Size2d& size) const
{
	return makeUnique<VulkanTexture>(this, image, format, size);
}