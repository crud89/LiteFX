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
	Array<UniquePtr<VulkanQueue>> m_queues;
	VulkanQueue* m_graphicsQueue;
	VulkanQueue* m_transferQueue;
	//VulkanQueue* m_bufferQueue;

	VkCommandPool m_commandPool;
	UniquePtr<VulkanSwapChain> m_swapChain;
	Array<String> m_extensions;
	VmaAllocator m_allocator{ nullptr };

public:
	VulkanDeviceImpl(VulkanDevice* parent, const Array<String>& extensions = { }) :
		base(parent), m_extensions(extensions)
	{
		this->defineMandatoryExtensions();
		this->loadQueues();
	}

	~VulkanDeviceImpl()
	{
		m_graphicsQueue->release();
		m_transferQueue->release();
		//m_bufferQueue->release();

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
	void loadQueues()
	{
		auto adapter = this->getAdapter();

		if (adapter == nullptr)
			throw std::invalid_argument("The argument `adapter` must be initialized.");

		// Find an available command queues.
		uint32_t queueFamilies = 0;
		::vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilies, nullptr);

		Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
		Array<UniquePtr<VulkanQueue>> queues(queueFamilies);

		::vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilies, familyProperties.data());
		std::generate(queues.begin(), queues.end(), [this, &familyProperties, i = 0]() mutable {
			QueueType type = QueueType::None;
			auto& familyProperty = familyProperties[i];

			if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				type |= QueueType::Compute;
			if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				type |= QueueType::Graphics;
			if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
				type |= QueueType::Transfer;

			return makeUnique<VulkanQueue>(m_parent, type, i++);
		});

		m_queues = std::move(queues);
	}

	VkDevice initialize(const Format& format)
	{
		auto adapter = this->getAdapter();
		auto instance = this->getInstance();
		auto surface = this->getSurface();

		if (adapter == nullptr)
			throw std::invalid_argument("The parent adapter must be initialized.");

		if (instance == nullptr)
			throw std::invalid_argument("The parent backend must be initialized.");

		if (surface == nullptr)
			throw std::invalid_argument("The parent surface must be initialized.");

		if (!this->validateDeviceExtensions(m_extensions))
			throw std::runtime_error("Some required device extensions are not supported by the system.");

		// Parse the extensions.
		// NOTE: For legacy support we should also set the device validation layers here.
		std::vector<const char*> requiredExtensions(m_extensions.size());
		std::generate(requiredExtensions.begin(), requiredExtensions.end(), [this, i = 0]() mutable { return m_extensions[i++].data(); });

		// Create graphics and transfer queue.
		m_graphicsQueue = this->findQueue(QueueType::Graphics, surface);
		m_transferQueue = this->findQueue(QueueType::Transfer);

		if (m_graphicsQueue == nullptr)
			throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

		if (m_transferQueue == nullptr)
		{
			LITEFX_WARNING(VULKAN_LOG, "Unable to find dedicated transfer queue - falling back to graphics queue for transfer.");
			m_transferQueue = m_graphicsQueue;
		}

		// Define a graphics queue for the device.
		const float graphicsQueuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfos[2] = {};
		queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[0].queueFamilyIndex = m_graphicsQueue->getId();
		queueCreateInfos[0].queueCount = 1;
		queueCreateInfos[0].pQueuePriorities = &graphicsQueuePriority;

		// Define a device-device transfer queue for the device.
		const float transferQueuePriority = 0.9f;
		queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[1].queueFamilyIndex = m_transferQueue->getId();
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

	VulkanQueue* findQueue(const QueueType& type) const noexcept
	{
		decltype(m_queues)::const_iterator match;

		// If a transfer queue is requested, look up for a queue that is explicitly *NOT* a graphics queue (since each queue implicitly supports transfer).
		match = type == QueueType::Transfer ?
			std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable { return !queue->isBound() && (queue->getType() & QueueType::Graphics) == QueueType::None && (queue->getType() & type) == type; }) :
			std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable { return !queue->isBound() && (queue->getType() & type) == type; });

		return match == m_queues.end() ? nullptr : match->get();
	}

	VulkanQueue* findQueue(const QueueType& type, const VkSurfaceKHR& surface) const
	{
		if (surface == nullptr)
			throw std::invalid_argument("The argument `surface` is not initialized.");

		auto adapter = this->getAdapter();

		if (adapter == nullptr)
			throw std::invalid_argument("The argument `adapter` must be initialized.");

		auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable {
			if (queue->isBound() || (queue->getType() & type) != type)
				return false;

			VkBool32 canPresent = VK_FALSE;
			::vkGetPhysicalDeviceSurfaceSupportKHR(adapter, queue->getId(), surface, &canPresent);

			if (!canPresent)
				return false;

			return true;
		});

		return match == m_queues.end() ? nullptr : match->get();
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const IRenderBackend* backend, const Format& format, const Array<String>& extensions) :
	IResource(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(this, extensions)), GraphicsDevice(backend)
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating device on backend {0} {{ Surface: {1}, Adapter: {2}, Format: {3}, Extensions: {4} }}...", fmt::ptr(backend), fmt::ptr(backend->getSurface()), backend->getAdapter()->getDeviceId(), format, Join(this->getExtensions(), ", "));
	
	this->handle() = m_impl->initialize(format);

	m_impl->createSwapChain(format);
	m_impl->m_graphicsQueue->bind();
	m_impl->m_transferQueue->bind();
}

VulkanDevice::~VulkanDevice() noexcept
{
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

const ICommandQueue* VulkanDevice::graphicsQueue() const noexcept
{
	return m_impl->m_graphicsQueue;
}

const ICommandQueue* VulkanDevice::transferQueue() const noexcept
{
	return m_impl->m_transferQueue;
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
		Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

		if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
			queues.push_back(m_impl->m_transferQueue->getId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();
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
		Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

		if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
			queues.push_back(m_impl->m_transferQueue->getId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();
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
		Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

		if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
			queues.push_back(m_impl->m_transferQueue->getId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();
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
		Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

		if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
			queues.push_back(m_impl->m_transferQueue->getId());

		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
		bufferInfo.pQueueFamilyIndices = queues.data();
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

UniquePtr<IImage> VulkanDevice::createImage(const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

	if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
		queues.push_back(m_impl->m_transferQueue->getId());

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
	imageInfo.pQueueFamilyIndices = queues.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return _VMAImage::allocate(this, 1, size, format, m_impl->m_allocator, imageInfo, allocInfo);
}

UniquePtr<IImage> VulkanDevice::createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = (::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	UInt32 queues[] = { m_impl->m_graphicsQueue->getId() };
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 1;
	imageInfo.pQueueFamilyIndices = queues;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return _VMAImage::allocate(this, 1, size, format, m_impl->m_allocator, imageInfo, allocInfo);
}

UniquePtr<ITexture> VulkanDevice::createTexture(const IDescriptorLayout* layout, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	if (layout == nullptr)
		throw std::invalid_argument("The texture descriptor layout must be initialized.");

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = size.width();
	imageInfo.extent.height = size.height();
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = ::getFormat(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = ::getSamples(samples);
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | (::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_SAMPLED_BIT);

	Array<UInt32> queues{ m_impl->m_graphicsQueue->getId() };

	if (m_impl->m_transferQueue != m_impl->m_graphicsQueue)
		queues.push_back(m_impl->m_transferQueue->getId());

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = static_cast<UInt32>(queues.size());
	imageInfo.pQueueFamilyIndices = queues.data();

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	return _VMATexture::allocate(this, layout, 1, size, format, levels, samples, m_impl->m_allocator, imageInfo, allocInfo);
}

UniquePtr<ISampler> VulkanDevice::createSampler(const IDescriptorLayout* layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	return makeUnique<VulkanSampler>(this, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
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
	std::generate(images.begin(), images.end(), [&, i = 0]() mutable { return makeUnique<_VMAImage>(this, imageChain[i++], 1, size, format); });

	return images;
}

UniquePtr<IShaderModule> VulkanDevice::loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint) const
{
	return makeUnique<VulkanShaderModule>(this, type, fileName, entryPoint);
}

VulkanRenderPassBuilder VulkanDevice::buildRenderPass() const
{
	return this->build<VulkanRenderPass>();
}