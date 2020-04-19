#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl {
private:
	UniquePtr<VulkanSwapChain> m_swapChain;
	Array<String> m_extensions;

public:
	VulkanDeviceImpl(const Array<String>& extensions = { }) noexcept :
		m_extensions(extensions) { this->defineMandatoryExtensions(); }

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
	VkDevice initialize(const VulkanDevice& parent, const Format& format, const VulkanQueue* deviceQueue)
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
		queueCreateInfo.queueFamilyIndex = deviceQueue->getId();
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

VulkanDevice::VulkanDevice(const IGraphicsAdapter* adapter, const ISurface* surface, ICommandQueue* deviceQueue, const Format& format, const Array<String>& extensions) :
	VulkanDevice(adapter, surface, extensions)
{
	this->create(format, deviceQueue);
}

VulkanDevice::VulkanDevice(const IGraphicsAdapter* adapter, const ISurface* surface, const Array<String>& extensions) :
	IResource(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(extensions)), GraphicsDevice(adapter, surface)
{
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

void VulkanDevice::create(const Format& format, ICommandQueue* queue)
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating device on surface {0} (adapterId: {1}, format: {2}, extensions: {3})...", fmt::ptr(this->getSurface()), this->getAdapter()->getDeviceId(), format, Join(this->getExtensions(), ", "));

	auto deviceQueue = dynamic_cast<VulkanQueue*>(queue);

	if (deviceQueue == nullptr)
		throw std::invalid_argument("The argument `deviceQueue` is not a valid Vulkan queue.");

	auto& h = this->handle();
	
	if (h != nullptr)
		throw std::runtime_error("The device can only be created once.");

	this->handle() = m_impl->initialize(*this, format, deviceQueue);

	m_impl->createSwapChain(*this, format);
	deviceQueue->initDeviceQueue(this);
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

VkImageView VulkanDevice::vkCreateImageView(const VkImage& image, const Format& format) const
{
	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;

	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = ::getFormat(format);

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView view;

	if (::vkCreateImageView(this->handle(), &createInfo, nullptr, &view) != VK_SUCCESS) 
		throw std::runtime_error("Unable to create image view!");

	return view;
}

UniquePtr<IShaderModule> VulkanDevice::loadShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint) const
{
	return makeUnique<VulkanShaderModule>(this, type, fileName, entryPoint);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDeviceBuilder::VulkanDeviceBuilderImpl {
private:
	QueueType m_queueType;
	Format m_format;

public:
	VulkanDeviceBuilderImpl() : m_queueType(QueueType::Graphics), m_format(Format::B8G8R8A8_UNORM_SRGB) { }

public:
	void setQueue(const QueueType& queueType)
	{
		m_queueType = queueType;
	}

	const QueueType& getQueue()
	{
		return m_queueType;
	}

	void setFormat(const Format& format)
	{
		m_format = format;
	}

	const Format& getFormat()
	{
		return m_format;
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanDeviceBuilder::VulkanDeviceBuilder(UniquePtr<VulkanDevice>&& instance) noexcept :
	GraphicsDeviceBuilder(std::move(instance)), m_impl(makePimpl<VulkanDeviceBuilderImpl>())
{
}

VulkanDeviceBuilder::~VulkanDeviceBuilder() noexcept = default;

UniquePtr<VulkanDevice> VulkanDeviceBuilder::go()
{
	auto surface = this->instance()->getSurface();
	auto queue = this->instance()->getAdapter()->findQueue(m_impl->getQueue(), surface);

	if (queue == nullptr)
		throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

	this->instance()->create(m_impl->getFormat(), queue);

	return GraphicsDeviceBuilder::go();
}

VulkanDeviceBuilder& VulkanDeviceBuilder::withFormat(const Format & format)
{
	m_impl->setFormat(format);
	return *this;
}

VulkanDeviceBuilder& VulkanDeviceBuilder::withQueue(const QueueType & queueType)
{
	m_impl->setQueue(queueType);
	return *this;
}