#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

// Include Vulkan Memory Allocator.
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl : public Implement<VulkanDevice> {
public:
	friend class VulkanDevice;

private:
	const VulkanQueue* m_graphicsQueue{ nullptr };
	VkCommandPool m_commandPool;
	UniquePtr<VulkanSwapChain> m_swapChain;
	Array<String> m_extensions;
	VmaAllocator m_allocator{ nullptr };

public:
	VulkanDeviceImpl(VulkanDevice* parent, const Array<String>& extensions = { }) :
		base(parent), m_extensions(extensions) 
	{
		this->defineMandatoryExtensions();
	}

	~VulkanDeviceImpl()
	{
		if (m_allocator != nullptr)
			::vmaDestroyAllocator(m_allocator);
	}

private:
	void defineMandatoryExtensions() noexcept
	{
		m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

private:
	VkSurfaceKHR getSurface() const noexcept
	{
		auto surface = dynamic_cast<const VulkanSurface*>(m_parent->getBackend()->getSurface());
		return surface ? surface->handle() : nullptr;
	}

	VkPhysicalDevice getAdapter() const noexcept
	{
		auto adapter = dynamic_cast<const VulkanGraphicsAdapter*>(m_parent->getBackend()->getAdapter());
		return adapter ? adapter->handle() : nullptr;
	}

	VkInstance getInstance() const noexcept
	{
		auto backend = dynamic_cast<const VulkanBackend*>(m_parent->getBackend());
		return backend ? backend->handle() : nullptr;
	}

public:
	bool validateDeviceExtensions(const Array<String>& extensions) const noexcept
	{
		auto availableExtensions = this->getAvailableDeviceExtensions();

		return std::all_of(extensions.begin(), extensions.end(), [&availableExtensions](const String& extension) {
			return std::find_if(availableExtensions.begin(), availableExtensions.end(), [&extension](String& str) {
				return std::equal(str.begin(), str.end(), extension.begin(), extension.end(), [](char a, char b) {
					return std::tolower(a) == std::tolower(b);
					});
				}) != availableExtensions.end();
			});
	}

	Array<String> getAvailableDeviceExtensions() const noexcept
	{
		auto adapter = this->getAdapter();

		uint32_t extensions = 0;
		::vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extensions, nullptr);

		Array<VkExtensionProperties> availableExtensions(extensions);
		Array<String> extensionNames(extensions);

		::vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extensions, availableExtensions.data());
		std::generate(extensionNames.begin(), extensionNames.end(), [&availableExtensions, i = 0]() mutable { return availableExtensions[i++].extensionName; });

		return extensionNames;
	}

public:
	VkDevice initialize(const Format& format, const VulkanQueue* deviceQueue)
	{
		auto adapter = this->getAdapter();

		if (adapter == nullptr)
			throw std::invalid_argument("The argument `adapter` must be initialized.");

		if (!this->validateDeviceExtensions(m_extensions))
			throw std::runtime_error("Some required device extensions are not supported by the system.");

		auto instance = this->getInstance();

		if (instance == nullptr)
			throw std::invalid_argument("The parent backend is not initialized.");

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

		// Create an buffer allocator.
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = adapter;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;

		if (::vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
			throw std::runtime_error("Unable to create Vulkan memory allocator.");

		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = deviceQueue->getId();
		//poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.flags = 0;

		if (::vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
			throw std::runtime_error("Unable to create command pool.");

		// Store the queue.
		m_graphicsQueue = deviceQueue;

		return device;
	}

	void createSwapChain(const Format& format)
	{
		m_swapChain = makeUnique<VulkanSwapChain>(m_parent, format);
	}

	void wait()
	{
		if (::vkDeviceWaitIdle(m_parent->handle()) != VK_SUCCESS)
			throw std::runtime_error("Unable to wait for the device.");
	}

	void resize(int width, int height)
	{
		// Wait for the device to be idle.
		this->wait();

		// Reset the swap chain.
		m_swapChain->reset();
	}

public:
	Array<Format> getSurfaceFormats() const
	{
		auto adapter = this->getAdapter();
		auto surface = this->getSurface();

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
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const IRenderBackend* backend, const Format& format, const Array<String>& extensions) :
	IResource(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(this, extensions)), GraphicsDevice(backend)
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating device on backend {0} (surface: {1}, adapter: {2}, format: {3}, extensions: {4})...", fmt::ptr(backend), fmt::ptr(backend->getSurface()), backend->getAdapter()->getDeviceId(), format, Join(this->getExtensions(), ", "));
	
	auto queue = dynamic_cast<VulkanQueue*>(backend->getAdapter()->findQueue(QueueType::Graphics, backend->getSurface()));

	if (queue == nullptr)
		throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

	auto& h = this->handle();

	if (h != nullptr)
		throw std::runtime_error("The device can only be created once.");

	this->handle() = m_impl->initialize(format, queue);

	m_impl->createSwapChain(format);
	queue->initDeviceQueue(this);
	this->setQueue(queue);
}

VulkanDevice::~VulkanDevice() noexcept
{
	auto commandPool = m_impl->m_commandPool;
	m_impl.destroy();

	::vkDestroyCommandPool(this->handle(), commandPool, nullptr);
	::vkDestroyDevice(this->handle(), nullptr);
}

size_t VulkanDevice::getBufferWidth() const noexcept
{
	return m_impl->m_swapChain->getWidth();
}

size_t VulkanDevice::getBufferHeight() const noexcept
{
	return m_impl->m_swapChain->getHeight();
}

const Array<String>& VulkanDevice::getExtensions() const noexcept
{
	return m_impl->m_extensions;
}

Array<Format> VulkanDevice::getSurfaceFormats() const
{
	return m_impl->getSurfaceFormats();
}

const ISwapChain* VulkanDevice::getSwapChain() const noexcept
{
	return m_impl->m_swapChain.get();
}

bool VulkanDevice::validateDeviceExtensions(const Array<String>& extensions) const noexcept
{
	return m_impl->validateDeviceExtensions(extensions);
}

Array<String> VulkanDevice::getAvailableDeviceExtensions() const noexcept
{
	return m_impl->getAvailableDeviceExtensions();
}

VkCommandPool VulkanDevice::getCommandPool() const noexcept
{
	return m_impl->m_commandPool;
}

void VulkanDevice::wait()
{
	m_impl->wait();
}

void VulkanDevice::resize(int width, int height)
{
	m_impl->resize(width, height);
}

UniquePtr<IBuffer> VulkanDevice::createBuffer(const BufferType& type, const BufferUsage& usage, const BufferLayout* layout, const UInt32& elements) const
{
	if (layout == nullptr)
		throw std::invalid_argument("The buffer layout must be initialized.");

	auto bufferSize = layout->getElementSize() * elements;

	LITEFX_TRACE(VULKAN_LOG, "Creating buffer: {{ Type: {0}, Usage: {1}, Size: {2}}}...", type, usage, bufferSize);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = bufferSize;

	// Deduct the usage buffer bit based on the type.
	VkBufferUsageFlags usageFlags;

	switch (type)
	{
	case BufferType::Uniform: usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
	case BufferType::Storage: usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
	case BufferType::Vertex:  usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;  break;
	case BufferType::Index:   usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;   break;
	default: LITEFX_WARNING(VULKAN_LOG, "Invalid buffer type: {0}", type);      break;
	}

	// Staging buffers also receive the transfer source, resource buffers the transfer destination bits.
	if (usage == BufferUsage::Staging)
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	else if (usage == BufferUsage::Resource)
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	bufferInfo.usage = usageFlags;

	// Deduct the allocation usage from the buffer usage scenario.
	VmaAllocationCreateInfo allocInfo = {};
	
	switch (usage)
	{
	case BufferUsage::Staging:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;   break;
	case BufferUsage::Resource: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;   break;
	case BufferUsage::Dynamic:  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
	case BufferUsage::Readback: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}

	// Create a buffer using VMA.
	return _VMABuffer::makeBuffer(layout, m_impl->m_allocator, bufferInfo, allocInfo);
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

UniquePtr<ICommandBuffer> VulkanDevice::createCommandBuffer() const
{
	return makeUnique<VulkanCommandBuffer>(this);
}