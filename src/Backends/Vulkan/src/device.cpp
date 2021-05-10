#include <litefx/backends/vulkan.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl : public Implement<VulkanDevice> {
public:
	friend class VulkanDevice;

private:
	class QueueFamily {
	private:
		Array<UniquePtr<VulkanQueue>> m_queues;
		UInt32 m_id, m_queueCount;
		QueueType m_type;

	public:
		const QueueType& type() const noexcept { return m_type; }
		const UInt32& total() const noexcept { return m_queueCount; }
		const UInt32 active() const noexcept { return static_cast<UInt32>(m_queues.size()); }
		const UInt32& id() const noexcept { return m_id; }
		const Array<UniquePtr<VulkanQueue>>& queues() const noexcept { return m_queues; }

	public:
		QueueFamily(const UInt32& id, const UInt32& queueCount, const QueueType& type) :
			m_id(id), m_queueCount(queueCount), m_type(type) { 
		}
		QueueFamily(const QueueFamily&) = delete;
		QueueFamily(QueueFamily&& _other) {
			m_queues = std::move(_other.m_queues);
			m_id = std::move(_other.m_id);
			m_queueCount = std::move(_other.m_queueCount);
			m_type = std::move(_other.m_type);
		}
		~QueueFamily() noexcept {
			m_queues.clear();
		}

	public:
		VulkanQueue* createQueue(const VulkanDevice& device, const QueuePriority& priority) {
			if (this->active() >= this->total())
				throw RuntimeException("Unable to create another queue for family {0}, since all {1} queues are already created.", m_id, m_queueCount);

			auto queue = makeUnique<VulkanQueue>(device, m_type, priority, m_id, this->active());
			auto queuePointer = queue.get();
			m_queues.push_back(std::move(queue));
			return queuePointer;
		}
	};

	Array<QueueFamily> m_families;
	VulkanQueue* m_graphicsQueue;
	VulkanQueue* m_transferQueue;
	VulkanQueue* m_bufferQueue;

	VkCommandPool m_commandPool;
	UniquePtr<VulkanSwapChain> m_swapChain;
	Array<String> m_extensions;

	const VulkanGraphicsAdapter& m_adapter;
	const VulkanSurface& m_surface;
	UniquePtr<VulkanGraphicsFactory> m_factory;

public:
	VulkanDeviceImpl(VulkanDevice* parent, const VulkanGraphicsAdapter& adapter, const VulkanSurface& surface, const Array<String>& extensions = { }) :
		base(parent), m_extensions(extensions), m_adapter(adapter), m_surface(surface)
	{
		this->defineMandatoryExtensions();
		this->loadQueueFamilies();
	}

	~VulkanDeviceImpl()
	{
		// This will also cause all queue instances to be automatically released (graphicsQueue, transferQueue, bufferQueue).
		m_families.clear();
	}

private:
	void defineMandatoryExtensions() noexcept
	{
		m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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
		uint32_t extensions = 0;
		::vkEnumerateDeviceExtensionProperties(m_adapter.handle(), nullptr, &extensions, nullptr);

		Array<VkExtensionProperties> availableExtensions(extensions);
		Array<String> extensionNames(extensions);

		::vkEnumerateDeviceExtensionProperties(m_adapter.handle(), nullptr, &extensions, availableExtensions.data());
		std::generate(extensionNames.begin(), extensionNames.end(), [&availableExtensions, i = 0]() mutable { return availableExtensions[i++].extensionName; });

		return extensionNames;
	}

public:
	void loadQueueFamilies()
	{
		// Find an available command queues.
		uint32_t queueFamilies = 0;
		::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, nullptr);
		Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
		::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, familyProperties.data());

		std::for_each(familyProperties.begin(), familyProperties.end(), [this, i = 0](const auto& familyProperty) mutable {
			QueueType type = QueueType::None;

			if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				type |= QueueType::Compute;
			if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				type |= QueueType::Graphics;
			if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
				type |= QueueType::Transfer;
			
			m_families.push_back(QueueFamily(i++, familyProperty.queueCount, type));
		});
	}

	VkDevice initialize(const Format& format)
	{
		if (!this->validateDeviceExtensions(m_extensions))
			throw std::runtime_error("Some required device extensions are not supported by the system.");

		// Parse the extensions.
		// NOTE: For legacy support we should also set the device validation layers here.
		std::vector<const char*> requiredExtensions(m_extensions.size());
		std::generate(requiredExtensions.begin(), requiredExtensions.end(), [this, i = 0]() mutable { return m_extensions[i++].data(); });

		// Create graphics and transfer queue.
		m_graphicsQueue = this->createQueue(QueueType::Graphics, QueuePriority::Realtime, m_surface.handle());
		m_transferQueue = this->createQueue(QueueType::Transfer, QueuePriority::Normal);
		m_bufferQueue = this->createQueue(QueueType::Transfer, QueuePriority::Normal);

		if (m_graphicsQueue == nullptr)
			throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

		if (m_transferQueue == nullptr)
		{
			LITEFX_WARNING(VULKAN_LOG, "Unable to find dedicated transfer queue for device-device transfer. Using graphics queue instead.");
			m_transferQueue = m_graphicsQueue;
		}

		if (m_bufferQueue == nullptr)
		{
			// NOTE: Default transfer queue can be a fallback, too.
			LITEFX_WARNING(VULKAN_LOG, "Unable to find dedicated transfer queue for host-device transfer. Using default transfer queue instead.");
			m_bufferQueue = m_transferQueue;
		}

		// Define used queue families.
		Array<VkDeviceQueueCreateInfo> queueCreateInfos;

		for (const auto& family : m_families)
		{
			// Ignore, if no queues have been created.
			if (family.active() == 0) 
				continue;
			
			Array<float> priorities;
			std::for_each(family.queues().begin(), family.queues().end(), [&priorities](const auto& queue) { priorities.push_back(static_cast<Float>(queue->priority()) / 100.f); });

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family.id();
			queueCreateInfo.queueCount = family.active();
			queueCreateInfo.pQueuePriorities = priorities.data();
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Define the device features.
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Define the device itself.
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<UInt32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// Create the device.
		VkDevice device;

		if (::vkCreateDevice(m_adapter.handle(), &createInfo, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("Unable to create Vulkan device.");

		return device;
	}

	void createFactory()
	{
		m_factory = makeUnique<VulkanGraphicsFactory>(*m_parent);
	}

	void createSwapChain(const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		m_swapChain = makeUnique<VulkanSwapChain>(*m_parent, frameBufferSize, frameBuffers, format);
	}

	void createQueues()
	{
		m_graphicsQueue->bind();
		m_transferQueue->bind();
		m_bufferQueue->bind();
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
		m_swapChain->reset(Size2d(width, height), m_swapChain->getBuffers());
	}

public:
	Array<Format> getSurfaceFormats() const
	{
		uint32_t formats;
		::vkGetPhysicalDeviceSurfaceFormatsKHR(m_adapter.handle(), m_surface.handle(), &formats, nullptr);

		Array<VkSurfaceFormatKHR> availableFormats(formats);
		Array<Format> surfaceFormats(formats);

		::vkGetPhysicalDeviceSurfaceFormatsKHR(m_adapter.handle(), m_surface.handle(), &formats, availableFormats.data());
		std::generate(surfaceFormats.begin(), surfaceFormats.end(), [&availableFormats, i = 0]() mutable { return getFormat(availableFormats[i++].format); });

		return surfaceFormats;
	}

	VulkanQueue* createQueue(const QueueType& type, const QueuePriority& priority)
	{
		// If a transfer queue is requested, look up only dedicated transfer queues. If none is available, fallbacks need to be handled manually. Every queue implicitly handles transfer.
		auto match = type == QueueType::Transfer ?
			std::find_if(m_families.begin(), m_families.end(), [&](const QueueFamily& family) mutable { return family.type() == QueueType::Transfer; }) :
			std::find_if(m_families.begin(), m_families.end(), [&](const QueueFamily& family) mutable { return LITEFX_FLAG_IS_SET(family.type(), type); });

		return match == m_families.end() ? nullptr : match->createQueue(*m_parent, priority);
	}

	VulkanQueue* createQueue(const QueueType& type, const QueuePriority& priority, const VkSurfaceKHR& surface)
	{
		auto match = std::find_if(m_families.begin(), m_families.end(), [&](const QueueFamily& family) mutable {
			if (!LITEFX_FLAG_IS_SET(family.type(), type))
				return false;

			VkBool32 canPresent = VK_FALSE;
			::vkGetPhysicalDeviceSurfaceSupportKHR(m_adapter.handle(), family.id(), surface, &canPresent);

			return static_cast<bool>(canPresent);
		});

		return match == m_families.end() ? nullptr : match->createQueue(*m_parent, priority);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const VulkanGraphicsAdapter& adapter, const VulkanSurface& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Array<String>& extensions) :
	IResource(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(this, adapter, surface, extensions))
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {1}, Adapter: {2}, Format: {3}, Extensions: {4} }}...", fmt::ptr(&surface), adapter.getDeviceId(), format, Join(this->getExtensions(), ", "));
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(VULKAN_LOG, "Vendor: {0:#0x}", adapter.getVendorId());
	LITEFX_DEBUG(VULKAN_LOG, "Driver Version: {0:#0x}", adapter.getDriverVersion());
	LITEFX_DEBUG(VULKAN_LOG, "API Version: {0:#0x}", adapter.getApiVersion());
	LITEFX_DEBUG(VULKAN_LOG, "Dedicated Memory: {0} Bytes", adapter.getDedicatedMemory());
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
	
	this->handle() = m_impl->initialize(format);
	m_impl->createQueues();
	m_impl->createFactory();
	m_impl->createSwapChain(format, frameBufferSize, frameBuffers);
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

const VulkanQueue& VulkanDevice::graphicsQueue() const noexcept
{
	return *m_impl->m_graphicsQueue;
}

const VulkanQueue& VulkanDevice::transferQueue() const noexcept
{
	return *m_impl->m_transferQueue;
}

const VulkanQueue& VulkanDevice::bufferQueue() const noexcept
{
	return *m_impl->m_bufferQueue;
}

const Array<String>& VulkanDevice::getExtensions() const noexcept
{
	return m_impl->m_extensions;
}

const VulkanSurface& VulkanDevice::surface() const noexcept
{
	return m_impl->m_surface;
}

const VulkanGraphicsAdapter& VulkanDevice::adapter() const noexcept
{
	return m_impl->m_adapter;
}

const VulkanGraphicsFactory& VulkanDevice::factory() const noexcept
{
	return *m_impl->m_factory;
}

Array<Format> VulkanDevice::getSurfaceFormats() const
{
	return m_impl->getSurfaceFormats();
}

const VulkanSwapChain& VulkanDevice::swapChain() const noexcept
{
	return *m_impl->m_swapChain;
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
	std::generate(images.begin(), images.end(), [&, i = 0]() mutable { return makeUnique<_VMAImage>(*this, imageChain[i++], 1, size, format); });

	return images;
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

VulkanRenderPassBuilder VulkanDevice::buildRenderPass() const
{
	return this->build<VulkanRenderPass, VulkanDevice>();
}