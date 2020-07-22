#include <litefx/backends/vulkan.hpp>
#include "buffer.h"
#include "image.h"

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
	VkDevice initialize(const Format& format, const VulkanQueue* deviceQueue, const VulkanQueue* transferQueue)
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
		const float graphicsQueuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfos[2] = {};
		queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[0].queueFamilyIndex = deviceQueue->getId();
		queueCreateInfos[0].queueCount = 1;
		queueCreateInfos[0].pQueuePriorities = &graphicsQueuePriority;

		// Define a transfer queue for the device.
		const float transferQueuePriority = 0.9f;
		queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[1].queueFamilyIndex = transferQueue->getId();
		queueCreateInfos[1].queueCount = 1;
		queueCreateInfos[1].pQueuePriorities = &transferQueuePriority;

		// Define the device features.
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Define the device itself.
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos;
		createInfo.queueCreateInfoCount = 2;
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
	LITEFX_DEBUG(VULKAN_LOG, "Creating device on backend {0} {{ Surface: {1}, Adapter: {2}, Format: {3}, Extensions: {4} }}...", fmt::ptr(backend), fmt::ptr(backend->getSurface()), backend->getAdapter()->getDeviceId(), format, Join(this->getExtensions(), ", "));
	
	auto graphicsQueue = dynamic_cast<VulkanQueue*>(backend->getAdapter()->findQueue(QueueType::Graphics, backend->getSurface()));
	auto transferQueue = dynamic_cast<VulkanQueue*>(backend->getAdapter()->findQueue(QueueType::Transfer));

	if (graphicsQueue == nullptr)
		throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

	if (transferQueue == nullptr)
		throw std::runtime_error("Unable to find a dedicated transfer queue.");

	this->handle() = m_impl->initialize(format, graphicsQueue, transferQueue);

	m_impl->createSwapChain(format);

	graphicsQueue->bindDevice(this);
	this->setGraphicsQueue(graphicsQueue);
	transferQueue->bindDevice(this);
	this->setTransferQueue(transferQueue);
}

VulkanDevice::~VulkanDevice() noexcept
{
	// Release the command queues first.
	this->graphicsQueue()->release();
	this->transferQueue()->release();

	// Destroy the implementation.
	m_impl.destroy();

	// Destroy the device.
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

void VulkanDevice::wait()
{
	m_impl->wait();
}

void VulkanDevice::resize(int width, int height)
{
	m_impl->resize(width, height);
}

UniquePtr<IBuffer> VulkanDevice::createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements) const
{
	LITEFX_TRACE(VULKAN_LOG, "Creating buffer: {{ Type: {0}, Usage: {1}, Size: {2}, Elements: {3} }}...", type, usage, size, elements);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;

	VkBufferUsageFlags usageFlags = {};

	switch (type)
	{
	case BufferType::Vertex:  usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;  break;
	case BufferType::Index:   usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;   break;
	case BufferType::Uniform: usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
	case BufferType::Storage: usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
	}

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		UInt32 queues[2] = { this->getGraphicsQueue()->getId(), this->getTransferQueue()->getId() };
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queues;
	}

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
	return _VMABuffer::allocate(type, elements, size, m_impl->m_allocator, bufferInfo, allocInfo);
}

UniquePtr<IVertexBuffer> VulkanDevice::createVertexBuffer(const IVertexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	if (layout == nullptr)
		throw std::invalid_argument("The vertex buffer layout must be initialized.");

	LITEFX_TRACE(VULKAN_LOG, "Creating vertex buffer: {{ Usage: {1}, Size: {2}, Elements: {3} }}...", usage, layout->getElementSize() * elements, elements);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout->getElementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		UInt32 queues[2] = { this->getGraphicsQueue()->getId(), this->getTransferQueue()->getId() };
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queues;
	}

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
	return _VMAVertexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
}

UniquePtr<IIndexBuffer> VulkanDevice::createIndexBuffer(const IIndexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	if (layout == nullptr)
		throw std::invalid_argument("The index buffer layout must be initialized.");

	LITEFX_TRACE(VULKAN_LOG, "Creating index buffer: {{ Usage: {1}, Size: {2}, Elements: {3} }}...", usage, layout->getElementSize() * elements, elements);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout->getElementSize() * elements;

	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		UInt32 queues[2] = { this->getGraphicsQueue()->getId(), this->getTransferQueue()->getId() };
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queues;
	}

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
	return _VMAIndexBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
}

UniquePtr<IConstantBuffer> VulkanDevice::createConstantBuffer(const IDescriptorLayout* layout, const BufferUsage& usage, const UInt32& elements) const
{
	if (layout == nullptr)
		throw std::invalid_argument("The constant buffer descriptor layout must be initialized.");

	LITEFX_TRACE(VULKAN_LOG, "Creating constant buffer: {{ Usage: {1}, Size: {2}, Elements: {3} }}...", usage, layout->getElementSize() * elements, elements);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = layout->getElementSize() * elements;

	VkBufferUsageFlags usageFlags = {};

	switch (layout->getType())
	{
	case BufferType::Uniform: usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
	case BufferType::Storage: usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
	}

	switch (usage)
	{
	case BufferUsage::Staging: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  break;
	case BufferUsage::Resource: usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
	}

	bufferInfo.usage = usageFlags;

	// If the buffer is used as a static resource or staging buffer, it needs to be accessible concurrently by the graphics and transfer queues.
	if (usage != BufferUsage::Staging && usage != BufferUsage::Resource)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		UInt32 queues[2] = { this->getGraphicsQueue()->getId(), this->getTransferQueue()->getId() };
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queues;
	}

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
	return _VMAConstantBuffer::allocate(layout, elements, m_impl->m_allocator, bufferInfo, allocInfo);
}

Array<UniquePtr<IImage>> VulkanDevice::createSwapChainImages(const ISwapChain* sc) const
{
	auto swapChain = dynamic_cast<const VulkanSwapChain*>(sc);

	if (swapChain == nullptr)
		throw std::runtime_error("The swap chain is not a valid Vulkan swap chain.");

	uint32_t imageCount;
	::vkGetSwapchainImagesKHR(this->handle(), swapChain->handle(), &imageCount, nullptr);
	auto size = swapChain->getBufferSize();
	auto format = swapChain->getFormat();

	Array<VkImage> imageChain(imageCount);
	Array<UniquePtr<IImage>> images(imageCount);

	::vkGetSwapchainImagesKHR(this->handle(), swapChain->handle(), &imageCount, imageChain.data());
	std::generate(images.begin(), images.end(), [&, i = 0]() mutable { return makeUnique<_VMAImage>(this, imageChain[i++], 1, size.width() * size.height() * ::getSize(format), size, format); });

	return images;
}

UniquePtr<IShaderModule> VulkanDevice::loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint) const
{
	return makeUnique<VulkanShaderModule>(this, type, fileName, entryPoint);
}