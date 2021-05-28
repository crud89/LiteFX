#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl : public Implement<VulkanGraphicsAdapter> {
public:
    friend class VulkanGraphicsAdapter;

public:
    VulkanGraphicsAdapterImpl(VulkanGraphicsAdapter* parent) : 
        base(parent) 
    {
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
	Resource<VkPhysicalDevice>(adapter), m_impl(makePimpl<VulkanGraphicsAdapterImpl>(this))
{
}

VulkanGraphicsAdapter::~VulkanGraphicsAdapter() noexcept = default;

String VulkanGraphicsAdapter::getName() const noexcept
{
    auto properties = m_impl->getProperties();
    return String(properties.deviceName);
}

UInt32 VulkanGraphicsAdapter::getVendorId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.vendorID;
}

UInt32 VulkanGraphicsAdapter::getDeviceId() const noexcept
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

UInt32 VulkanGraphicsAdapter::getDriverVersion() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.driverVersion;
}

UInt32 VulkanGraphicsAdapter::getApiVersion() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.apiVersion;
}

UInt64 VulkanGraphicsAdapter::getDedicatedMemory() const noexcept
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    ::vkGetPhysicalDeviceMemoryProperties(this->handle(), &memoryProperties);

    auto memoryHeaps = memoryProperties.memoryHeaps;
    auto heaps = Array<VkMemoryHeap>(memoryHeaps, memoryHeaps + memoryProperties.memoryHeapCount);

    for (const auto& heap : heaps)
        if (LITEFX_FLAG_IS_SET(heap.flags, VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
            return heap.size;

    return 0;
}