#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl : public Implement<VulkanGraphicsAdapter> {
public:
    friend class VulkanGraphicsAdapter;

private:
    Array<UniquePtr<VulkanQueue>> m_queues;

public:
    VulkanGraphicsAdapterImpl(VulkanGraphicsAdapter* parent) : base(parent) 
    {
        this->initialize();
    }

public:
    void initialize()
    {
        // Find an available graphics queue.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_parent->handle(), &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        Array<UniquePtr<VulkanQueue>> queues(queueFamilies);

        ::vkGetPhysicalDeviceQueueFamilyProperties(m_parent->handle(), &queueFamilies, familyProperties.data());
        std::generate(queues.begin(), queues.end(), [&familyProperties, i = 0]() mutable {
            QueueType type = QueueType::None;
            auto& familyProperty = familyProperties[i];

            if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
                type |= QueueType::Compute;
            if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                type |= QueueType::Graphics;
            if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
                type |= QueueType::Transfer;

            return makeUnique<VulkanQueue>(type, i++);
        });

        m_queues = std::move(queues);
    }

public:
    VulkanQueue* findQueue(const QueueType& type) const noexcept
    {
        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable { return (queue->getType() & type) == type; });
        return match == m_queues.end() ? nullptr : match->get();
    }

    VulkanQueue* findQueue(const QueueType& type, const VulkanSurface* surface) const
    {
        if (surface == nullptr)
            throw std::invalid_argument("The argument `surface` is not initialized.");

        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable {
            if ((queue->getType() & type) != type)
                return false;

            VkBool32 canPresent = VK_FALSE;
            ::vkGetPhysicalDeviceSurfaceSupportKHR(m_parent->handle(), queue->getId(), surface->handle(), &canPresent);

            if (!canPresent)
                return false;

            return true;
        });

        if (match == m_queues.end())
            return nullptr;

        return match->get();
    }

public:
    VkPhysicalDeviceProperties getProperties() const noexcept
    {
        VkPhysicalDeviceProperties properties;
        ::vkGetPhysicalDeviceProperties(m_parent->handle(), &properties);

        return properties;
    }

    VkPhysicalDeviceFeatures getFeatures() const noexcept
    {
        VkPhysicalDeviceFeatures features;
        ::vkGetPhysicalDeviceFeatures(m_parent->handle(), &features);

        return features;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanGraphicsAdapter::VulkanGraphicsAdapter(VkPhysicalDevice adapter) :
	IResource(adapter), m_impl(makePimpl<VulkanGraphicsAdapterImpl>(this))
{
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

ICommandQueue* VulkanGraphicsAdapter::findQueue(const QueueType& queueType) const
{
    return m_impl->findQueue(queueType);
}

ICommandQueue* VulkanGraphicsAdapter::findQueue(const QueueType& queueType, const ISurface* surface) const
{
    auto forSurface = dynamic_cast<const VulkanSurface*>(surface);

    if (forSurface == nullptr)
        throw std::invalid_argument("The provided surface is not a valid Vulkan surface.");

    return m_impl->findQueue(queueType, forSurface);
}