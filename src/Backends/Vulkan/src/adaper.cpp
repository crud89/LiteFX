#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl {
private:
    VkPhysicalDevice m_adapter;
    Array<SharedPtr<VulkanQueue>> m_queues;

public:
    VulkanGraphicsAdapterImpl(const VkPhysicalDevice adapter) : m_adapter(adapter) 
    {
        if (adapter == nullptr)
            throw std::invalid_argument("The argument `adapter` must be initialized.");

        this->loadQueues(adapter);
    }

public:
    SharedPtr<VulkanQueue> findQueue(const QueueType& type, Optional<uint32_t>& queueId) const noexcept
    {
        for (size_t q(0); q < m_queues.size(); ++q)
        {
            if ((m_queues[q]->getType() & type) == type)
            {
                queueId = static_cast<uint32_t>(q);
                return m_queues[q];
            }
        }

        return nullptr;
    }

    SharedPtr<VulkanQueue> findQueue(const QueueType& type, Optional<uint32_t>& queueId, const VulkanSurface* surface) const
    {
        if (surface == nullptr)
            throw std::invalid_argument("The argument `surface` is not initialized.");

        for (size_t q(0); q < m_queues.size(); ++q)
        {
            if ((m_queues[q]->getType() & type) == type)
            {
                VkBool32 canPresent = VK_FALSE;
                ::vkGetPhysicalDeviceSurfaceSupportKHR(m_adapter, q, surface->handle(), &canPresent);

                if (canPresent)
                {
                    queueId = static_cast<uint32_t>(q);
                    return m_queues[q];
                }
            }
        }

        return nullptr;
    }

    UniquePtr<VulkanDevice> createDevice(const VulkanSurface* surface)
    {
        if (surface == nullptr)
            throw std::invalid_argument("The provided surface is not initialized or not a valid Vulkan surface.");

        // Create a graphics device.
        Optional<uint32_t> queueId;
        auto queue = this->findQueue(QueueType::Graphics, queueId, surface);

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

        // NOTE: For legacy support we should also set the device validation layers here.

        // Create the device.
        VkDevice device;

        if (::vkCreateDevice(m_adapter, &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("Unable to create Vulkan device.");

        return makeUnique<VulkanDevice>(device);
    }

public:
    VkPhysicalDeviceProperties getProperties() const noexcept
    {
        VkPhysicalDeviceProperties properties;
        ::vkGetPhysicalDeviceProperties(m_adapter, &properties);

        return properties;
    }

    VkPhysicalDeviceFeatures getFeatures() const noexcept
    {
        VkPhysicalDeviceFeatures features;
        ::vkGetPhysicalDeviceFeatures(m_adapter, &features);

        return features;
    }

private:
    void loadQueues(const VkPhysicalDevice adapter) noexcept
    {
        // Find an available graphics queue.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        Array<SharedPtr<VulkanQueue>> queues(queueFamilies);

        ::vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilies, familyProperties.data());
        std::generate(queues.begin(), queues.end(), [&familyProperties, i = 0] () mutable {
            QueueType type = QueueType::None;
            auto& familyProperty = familyProperties[i++];

            if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
                type |= QueueType::Compute;
            if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                type |= QueueType::Graphics;
            if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
                type |= QueueType::Transfer;

            return makeShared<VulkanQueue>(type);
        });

        m_queues = std::move(queues);
    }
};

VulkanGraphicsAdapter::VulkanGraphicsAdapter(const VkPhysicalDevice adapter) :
	IResource(adapter), m_impl(makePimpl<VulkanGraphicsAdapterImpl>(adapter))
{
    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapter` must be initialized.");
}

VulkanGraphicsAdapter::~VulkanGraphicsAdapter() noexcept = default;


String VulkanGraphicsAdapter::getName() const noexcept
{
    auto properties = m_impl->getProperties();
    return String(properties.deviceName);
}

uint32_t VulkanGraphicsAdapter::getVendorId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.vendorID;
}

uint32_t VulkanGraphicsAdapter::getDeviceId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.deviceID;
}

GraphicsAdapterType VulkanGraphicsAdapter::getType() const noexcept
{
    auto properties = m_impl->getProperties();
    
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
    auto properties = m_impl->getProperties();
    return properties.driverVersion;
}

uint32_t VulkanGraphicsAdapter::getApiVersion() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.apiVersion;
}

UniquePtr<IGraphicsDevice> VulkanGraphicsAdapter::createDevice(const ISurface* surface) const
{
    return m_impl->createDevice(dynamic_cast<const VulkanSurface*>(surface));
}

SharedPtr<ICommandQueue> VulkanGraphicsAdapter::findQueue(const QueueType& queueType) const
{
    Optional<uint32_t> queueId;
    return m_impl->findQueue(queueType, queueId);
}