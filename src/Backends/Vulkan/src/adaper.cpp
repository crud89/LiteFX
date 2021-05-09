#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl : public Implement<VulkanGraphicsAdapter> {
public:
    friend class VulkanGraphicsAdapter;

public:
    VulkanGraphicsAdapterImpl(VulkanGraphicsAdapter* parent) : base(parent) 
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

uint32_t VulkanGraphicsAdapter::getDedicatedMemory() const noexcept
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