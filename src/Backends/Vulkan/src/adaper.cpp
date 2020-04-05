#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl {
private:
    Array<SharedPtr<VulkanQueue>> m_queues;

public:
    VulkanGraphicsAdapterImpl() noexcept = default;

public:
    void loadQueues(const VulkanGraphicsAdapter& parent) noexcept
    {
        // Find an available graphics queue.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(parent.handle(), &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        Array<SharedPtr<VulkanQueue>> queues(queueFamilies);

        ::vkGetPhysicalDeviceQueueFamilyProperties(parent.handle(), &queueFamilies, familyProperties.data());
        std::generate(queues.begin(), queues.end(), [&familyProperties, i = 0]() mutable {
            QueueType type = QueueType::None;
            auto& familyProperty = familyProperties[i];

            if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
                type |= QueueType::Compute;
            if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                type |= QueueType::Graphics;
            if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
                type |= QueueType::Transfer;

            return makeShared<VulkanQueue>(type, i++);
        });

        m_queues = std::move(queues);
    }

public:
    SharedPtr<VulkanQueue> findQueue(const QueueType& type, Optional<uint32_t>& queueId) const noexcept
    {
        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const SharedPtr<VulkanQueue>& queue) mutable {
            if ((queue->getType() & type) != type)
                return false;

            queueId = queue->getId();
            return true;
        });

        if (match == m_queues.end())
            return nullptr;

        return *match;
    }

    SharedPtr<VulkanQueue> findQueue(const VulkanGraphicsAdapter& parent, const QueueType& type, Optional<uint32_t>& queueId, const VulkanSurface* surface) const
    {
        if (surface == nullptr)
            throw std::invalid_argument("The argument `surface` is not initialized.");

        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const SharedPtr<VulkanQueue>& queue) mutable {
            if ((queue->getType() & type) != type)
                return false;

            VkBool32 canPresent = VK_FALSE;
            ::vkGetPhysicalDeviceSurfaceSupportKHR(parent.handle(), queue->getId(), surface->handle(), &canPresent);

            if (!canPresent)
                return false;

            queueId = queue->getId();
            return true;
        });

        if (match == m_queues.end())
            return nullptr;

        return *match;
    }

    UniquePtr<VulkanDevice> createDevice(const VulkanGraphicsAdapter& parent, const VulkanSurface* surface, const Array<String>& ext = { })
    {
        if (surface == nullptr)
            throw std::invalid_argument("The provided surface is not initialized or not a valid Vulkan surface.");

        // Add mandatory extensions and check for availability.
        Array<String> extensions = ext;
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (!parent.validateDeviceExtensions(extensions))
            throw std::runtime_error("Some required device extensions are not supported by the system.");

        // Parse the extensions.
        // NOTE: For legacy support we should also set the device validation layers here.
        std::vector<const char*> requiredExtensions(extensions.size());
        std::generate(requiredExtensions.begin(), requiredExtensions.end(), [&extensions, i = 0]() mutable { return extensions[i++].data(); });

        // Create a graphics device.
        Optional<uint32_t> queueId;
        auto queue = this->findQueue(parent, QueueType::Graphics, queueId, surface);

        if (!queueId.has_value())
            throw std::runtime_error("The adapter does not provide a graphics device.");

        // Define a graphics queue for the device.
        const float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueId.value();
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

        if (::vkCreateDevice(parent.handle(), &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("Unable to create Vulkan device.");

        return makeUnique<VulkanDevice>(device, queue, extensions);
    }

public:
    VkPhysicalDeviceProperties getProperties(const VulkanGraphicsAdapter& parent) const noexcept
    {
        VkPhysicalDeviceProperties properties;
        ::vkGetPhysicalDeviceProperties(parent.handle(), &properties);

        return properties;
    }

    VkPhysicalDeviceFeatures getFeatures(const VulkanGraphicsAdapter& parent) const noexcept
    {
        VkPhysicalDeviceFeatures features;
        ::vkGetPhysicalDeviceFeatures(parent.handle(), &features);

        return features;
    }
};

VulkanGraphicsAdapter::VulkanGraphicsAdapter(const VkPhysicalDevice adapter) :
	IResource(adapter), m_impl(makePimpl<VulkanGraphicsAdapterImpl>())
{
    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapter` must be initialized.");

    m_impl->loadQueues(*this);
}

VulkanGraphicsAdapter::~VulkanGraphicsAdapter() noexcept = default;

String VulkanGraphicsAdapter::getName() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    return String(properties.deviceName);
}

uint32_t VulkanGraphicsAdapter::getVendorId() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    return properties.vendorID;
}

uint32_t VulkanGraphicsAdapter::getDeviceId() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    return properties.deviceID;
}

GraphicsAdapterType VulkanGraphicsAdapter::getType() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    
    switch (properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return GraphicsAdapterType::CPU;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return GraphicsAdapterType::CPU;
    default:
        return GraphicsAdapterType::Other;
    }
}

uint32_t VulkanGraphicsAdapter::getDriverVersion() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    return properties.driverVersion;
}

uint32_t VulkanGraphicsAdapter::getApiVersion() const noexcept
{
    auto properties = m_impl->getProperties(*this);
    return properties.apiVersion;
}

UniquePtr<IGraphicsDevice> VulkanGraphicsAdapter::createDevice(const ISurface* surface, const Array<String>& extensions) const
{
    return m_impl->createDevice(*this, dynamic_cast<const VulkanSurface*>(surface), extensions);
}

SharedPtr<ICommandQueue> VulkanGraphicsAdapter::findQueue(const QueueType& queueType) const
{
    Optional<uint32_t> queueId;
    return m_impl->findQueue(queueType, queueId);
}

bool VulkanGraphicsAdapter::validateDeviceExtensions(const Array<String>& extensions) const noexcept
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

Array<String> VulkanGraphicsAdapter::getAvailableDeviceExtensions() const noexcept
{
    uint32_t extensions = 0;
    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    Array<String> extensionNames(extensions);

    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, availableExtensions.data());
    std::generate(extensionNames.begin(), extensionNames.end(), [&availableExtensions, i = 0]() mutable { return availableExtensions[i++].extensionName; });

    return extensionNames;
}