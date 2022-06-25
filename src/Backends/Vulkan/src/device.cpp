#include <litefx/backends/vulkan.hpp>

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
		QueueFamily(const QueueFamily& _other) = delete;
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
	VulkanQueue* m_computeQueue;

	VkCommandPool m_commandPool;
	UniquePtr<VulkanSwapChain> m_swapChain;
	Array<String> m_extensions;

	const VulkanGraphicsAdapter& m_adapter;
	UniquePtr<VulkanSurface> m_surface;
	UniquePtr<VulkanGraphicsFactory> m_factory;

public:
	VulkanDeviceImpl(VulkanDevice* parent, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Span<String> extensions) :
		base(parent), m_adapter(adapter), m_surface(std::move(surface))
	{
		if (m_surface == nullptr)
			throw ArgumentNotInitializedException("The surface must be initialized.");

		m_extensions.assign(std::begin(extensions), std::end(extensions));

		this->defineMandatoryExtensions();
		this->loadQueueFamilies();
	}

	~VulkanDeviceImpl()
	{
		// This will also cause all queue instances to be automatically released (graphicsQueue, transferQueue, bufferQueue).
		m_families.clear();

		// Destroy the surface.
		m_surface = nullptr;
	}

private:
	void defineMandatoryExtensions() noexcept
	{
		m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

public:
	void loadQueueFamilies()
	{
		// Find an available command queues.
		uint32_t queueFamilies = 0;
		::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, nullptr);

		Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
		::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, familyProperties.data());

		m_families = familyProperties | 
			std::views::transform([i = 0](const VkQueueFamilyProperties& familyProperty) mutable {
				QueueType type = QueueType::None;

				if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
					type |= QueueType::Compute;
				if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					type |= QueueType::Graphics;
				if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
					type |= QueueType::Transfer;

				return QueueFamily(i++, familyProperty.queueCount, type);
			}) |
			ranges::to<Array<QueueFamily>>();
	}

	VkDevice initialize()
	{
		if (!m_adapter.validateDeviceExtensions(m_extensions))
			throw InvalidArgumentException("Some required device extensions are not supported by the system.");

		auto const requiredExtensions = m_extensions | std::views::transform([](const auto& extension) { return extension.c_str(); }) | ranges::to<Array<const char*>>();

		// Create graphics and transfer queue.
		m_graphicsQueue = this->createQueue(QueueType::Graphics, QueuePriority::Realtime, std::as_const(*m_surface).handle());
		m_transferQueue = this->createQueue(QueueType::Transfer, QueuePriority::Normal);
		m_bufferQueue = this->createQueue(QueueType::Transfer, QueuePriority::Normal);
		m_computeQueue = this->createQueue(QueueType::Compute, QueuePriority::Normal);

		if (m_graphicsQueue == nullptr)
			throw RuntimeException("Unable to find a fitting command queue to present the specified surface.");

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

		if (m_computeQueue == nullptr)
		{
			// NOTE: Default compute queue can be a fallback, too.
			LITEFX_WARNING(VULKAN_LOG, "Unable to find dedicated compute queue for host-device transfer. Using graphics queue instead.");
			m_computeQueue = m_graphicsQueue;
		}

		// Define used queue families.
		Array<Array<Float>> queuePriorities;
		auto const queueCreateInfos = m_families |
			std::views::filter([](const QueueFamily& family) { return family.active() > 0; }) |
			std::views::transform([&queuePriorities](const QueueFamily& family) {
				auto const priorities = family.queues() | 
					std::views::transform([](const UniquePtr<VulkanQueue>& queue) { return static_cast<Float>(queue->priority()) / 100.f; }) |
					ranges::to<Array<Float>>();
				queuePriorities.push_back(priorities);

				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = family.id();
				queueCreateInfo.queueCount = family.active();
				queueCreateInfo.pQueuePriorities = queuePriorities.back().data();

				return queueCreateInfo;
			}) | ranges::to<Array<VkDeviceQueueCreateInfo>>();

		// Define the device features.
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkPhysicalDeviceVulkan12Features deviceFeatures12 = { 
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.timelineSemaphore = true 
		};

		// Define the device itself.
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &deviceFeatures12;
		createInfo.queueCreateInfoCount = static_cast<UInt32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// Create the device.
		// NOTE: This can time-out under very mysterious circumstances, in which case the event log shows a TDR error. Unfortunately, the only way I found
		//       to fix this is rebooting the entire system.
		VkDevice device;
		raiseIfFailed<RuntimeException>(::vkCreateDevice(m_adapter.handle(), &createInfo, nullptr, &device), "Unable to create Vulkan device.");

		return device;
	}

	void createFactory()
	{
		m_factory = makeUnique<VulkanGraphicsFactory>(*m_parent);
	}

	void createSwapChain(const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers)
	{
		m_swapChain = makeUnique<VulkanSwapChain>(*m_parent, format, frameBufferSize, frameBuffers);
	}

	void createQueues()
	{
		m_graphicsQueue->bind();
		m_transferQueue->bind();
		m_bufferQueue->bind();
		m_computeQueue->bind();
	}

public:
	VulkanQueue* createQueue(const QueueType& type, const QueuePriority& priority)
	{
		// If a transfer queue is requested, look up only dedicated transfer queues. If none is available, fallbacks need to be handled manually. Every queue implicitly handles transfer.
		auto match = type == QueueType::Transfer ?
			std::ranges::find_if(m_families, [](const auto& family) { return family.type() == QueueType::Transfer; }) :
			std::ranges::find_if(m_families, [&type](const auto& family) { return LITEFX_FLAG_IS_SET(family.type(), type); });

		return match == m_families.end() ? nullptr : match->createQueue(*m_parent, priority);
	}

	VulkanQueue* createQueue(const QueueType& type, const QueuePriority& priority, const VkSurfaceKHR& surface)
	{
		if (auto match = std::ranges::find_if(m_families, [&](const auto& family) {
				if (!LITEFX_FLAG_IS_SET(family.type(), type))
					return false;

				VkBool32 canPresent = VK_FALSE;
				::vkGetPhysicalDeviceSurfaceSupportKHR(m_adapter.handle(), family.id(), surface, &canPresent);

				return static_cast<bool>(canPresent);
			}); match != m_families.end()) [[likely]]
			return match->createQueue(*m_parent, priority);
		
		return nullptr;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Span<String> extensions) :
	VulkanDevice(backend, adapter, std::move(surface), Format::B8G8R8A8_SRGB, { 800, 600 }, 3, extensions)
{
}

VulkanDevice::VulkanDevice(const VulkanBackend& /*backend*/, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, Span<String> extensions) :
	Resource<VkDevice>(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(this, adapter, std::move(surface), extensions))
{
	LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {0}, Adapter: {1}, Extensions: {2} }}...", fmt::ptr(reinterpret_cast<const void*>(m_impl->m_surface.get())), adapter.getDeviceId(), Join(this->enabledExtensions(), ", "));
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(VULKAN_LOG, "Vendor: {0:#0x}", adapter.getVendorId());
	LITEFX_DEBUG(VULKAN_LOG, "Driver Version: {0:#0x}", adapter.getDriverVersion());
	LITEFX_DEBUG(VULKAN_LOG, "API Version: {0:#0x}", adapter.getApiVersion());
	LITEFX_DEBUG(VULKAN_LOG, "Dedicated Memory: {0} Bytes", adapter.getDedicatedMemory());
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
	LITEFX_DEBUG(VULKAN_LOG, "Available extensions: {0}", Join(adapter.getAvailableDeviceExtensions(), ", "));
	LITEFX_DEBUG(VULKAN_LOG, "Validation layers: {0}", Join(adapter.getDeviceValidationLayers(), ", "));
	LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

	if (extensions.size() > 0)
		LITEFX_INFO(VULKAN_LOG, "Enabled validation layers: {0}", Join(extensions, ", "));

	this->handle() = m_impl->initialize();
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

Span<const String> VulkanDevice::enabledExtensions() const noexcept
{
	return m_impl->m_extensions;
}

VulkanSwapChain& VulkanDevice::swapChain() noexcept
{
	return *m_impl->m_swapChain;
}

VulkanRenderPassBuilder VulkanDevice::buildRenderPass(const MultiSamplingLevel& samples, const UInt32& commandBuffers) const
{
	return VulkanRenderPassBuilder(*this, commandBuffers, samples);
}

VulkanComputePipelineBuilder VulkanDevice::buildComputePipeline() const
{
	return VulkanComputePipelineBuilder(*this);
}

const VulkanSwapChain& VulkanDevice::swapChain() const noexcept
{
	return *m_impl->m_swapChain;
}

const VulkanSurface& VulkanDevice::surface() const noexcept
{
	return *m_impl->m_surface;
}

const VulkanGraphicsAdapter& VulkanDevice::adapter() const noexcept
{
	return m_impl->m_adapter;
}

const VulkanGraphicsFactory& VulkanDevice::factory() const noexcept
{
	return *m_impl->m_factory;
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

const VulkanQueue& VulkanDevice::computeQueue() const noexcept
{
	return *m_impl->m_computeQueue;
}

MultiSamplingLevel VulkanDevice::maximumMultiSamplingLevel(const Format& format) const noexcept
{
	auto limits = m_impl->m_adapter.getLimits();
	VkSampleCountFlags sampleCounts = limits.framebufferColorSampleCounts;

	if (::hasDepth(format) && ::hasStencil(format))
		sampleCounts = limits.framebufferDepthSampleCounts & limits.framebufferStencilSampleCounts;
	else if (::hasDepth(format))
		sampleCounts = limits.framebufferDepthSampleCounts;
	else if (::hasStencil(format))
		sampleCounts = limits.framebufferStencilSampleCounts;

	if (sampleCounts & VK_SAMPLE_COUNT_64_BIT)
		return MultiSamplingLevel::x64;
	else if (sampleCounts & VK_SAMPLE_COUNT_32_BIT)
		return MultiSamplingLevel::x32;
	else if (sampleCounts & VK_SAMPLE_COUNT_16_BIT)
		return MultiSamplingLevel::x16;
	else if (sampleCounts & VK_SAMPLE_COUNT_8_BIT)
		return MultiSamplingLevel::x8;
	else if (sampleCounts & VK_SAMPLE_COUNT_4_BIT)
		return MultiSamplingLevel::x4;
	else if (sampleCounts & VK_SAMPLE_COUNT_2_BIT)
		return MultiSamplingLevel::x2;
	else
		return MultiSamplingLevel::x1;
}

void VulkanDevice::wait() const
{
	raiseIfFailed<RuntimeException>(::vkDeviceWaitIdle(this->handle()), "Unable to wait for the device.");
}