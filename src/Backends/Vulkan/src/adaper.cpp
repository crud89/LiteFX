#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl {
private:
    Array<UniquePtr<VulkanQueue>> m_queues;

public:
    VulkanGraphicsAdapterImpl() noexcept = default;

public:
    void initialize(const VulkanGraphicsAdapter& parent)
    {
        // Find an available graphics queue.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(parent.handle(), &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        Array<UniquePtr<VulkanQueue>> queues(queueFamilies);

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

            return makeUnique<VulkanQueue>(type, i++);
        });

        m_queues = std::move(queues);
    }

public:
    UniquePtr<VulkanDevice> createDevice(const VulkanGraphicsAdapter& parent, const VulkanSurface* surface, const Format& format, const Array<String>& extensions = { })
    {
        if (surface == nullptr)
            throw std::invalid_argument("The provided surface is not initialized or not a valid Vulkan surface.");

        // Create a graphics device.
        auto queue = this->findQueue(parent, QueueType::Graphics, surface);

        if (queue == nullptr)
            throw std::runtime_error("The adapter does not provide a graphics device.");

        return makeUnique<VulkanDevice>(&parent, surface, queue, format, extensions);
    }

    VulkanQueue* findQueue(const QueueType& type) const noexcept
    {
        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable { return (queue->getType() & type) == type; });
        return match == m_queues.end() ? nullptr : match->get();
    }

    VulkanQueue* findQueue(const VulkanGraphicsAdapter& parent, const QueueType& type, const VulkanSurface* surface) const
    {
        if (surface == nullptr)
            throw std::invalid_argument("The argument `surface` is not initialized.");

        auto match = std::find_if(m_queues.begin(), m_queues.end(), [&](const UniquePtr<VulkanQueue>& queue) mutable {
            if ((queue->getType() & type) != type)
                return false;

            VkBool32 canPresent = VK_FALSE;
            ::vkGetPhysicalDeviceSurfaceSupportKHR(parent.handle(), queue->getId(), surface->handle(), &canPresent);

            if (!canPresent)
                return false;

            return true;
        });

        if (match == m_queues.end())
            return nullptr;

        return match->get();
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

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanGraphicsAdapter::VulkanGraphicsAdapter(VkPhysicalDevice adapter) :
	IResource(adapter), m_impl(makePimpl<VulkanGraphicsAdapterImpl>())
{
    m_impl->initialize(*this);
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

UniquePtr<IGraphicsDevice> VulkanGraphicsAdapter::createDevice(const ISurface* surface, const Format& format, const Array<String>& extensions) const
{
    return m_impl->createDevice(*this, dynamic_cast<const VulkanSurface*>(surface), format, extensions);
}

const ICommandQueue* VulkanGraphicsAdapter::findQueue(const QueueType& queueType) const
{
    return m_impl->findQueue(queueType);
}

const ICommandQueue* VulkanGraphicsAdapter::findQueue(const QueueType& queueType, const VulkanSurface* forSurface) const
{
    return m_impl->findQueue(*this, queueType, forSurface);
}