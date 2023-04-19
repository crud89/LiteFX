#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsAdapter::VulkanGraphicsAdapterImpl : public Implement<VulkanGraphicsAdapter> {
public:
    friend class VulkanGraphicsAdapter;

private:
    VkPhysicalDeviceLimits m_limits;

public:
    VulkanGraphicsAdapterImpl(VulkanGraphicsAdapter* parent) : 
        base(parent) 
    {
        m_limits = this->getProperties().limits;
    }

public:
    VkPhysicalDeviceProperties getProperties() const noexcept
    {
        VkPhysicalDeviceProperties properties;
        ::vkGetPhysicalDeviceProperties(m_parent->handle(), &properties);

        return properties;
    }

    VkPhysicalDeviceIDProperties getIdProperties() const noexcept
    {
        // After obtaining VkPhysicalDevice of your choice:
        VkPhysicalDeviceIDProperties deviceIdProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
        VkPhysicalDeviceProperties2 deviceProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        deviceProperties.pNext = &deviceIdProperties;
        ::vkGetPhysicalDeviceProperties2(m_parent->handle(), &deviceProperties);

        return deviceIdProperties;
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

String VulkanGraphicsAdapter::name() const noexcept
{
    auto properties = m_impl->getProperties();
    return String(properties.deviceName);
}

UInt64 VulkanGraphicsAdapter::uniqueId() const noexcept
{
    auto properties = m_impl->getIdProperties();
    return *reinterpret_cast<UInt64*>(&properties.deviceLUID);
}

UInt32 VulkanGraphicsAdapter::vendorId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.vendorID;
}

UInt32 VulkanGraphicsAdapter::deviceId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.deviceID;
}

GraphicsAdapterType VulkanGraphicsAdapter::type() const noexcept
{
    auto properties = m_impl->getProperties();
    
    switch (properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return GraphicsAdapterType::CPU;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return GraphicsAdapterType::GPU;
    default:
        return GraphicsAdapterType::Other;
    }
}

UInt32 VulkanGraphicsAdapter::driverVersion() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.driverVersion;
}

UInt32 VulkanGraphicsAdapter::apiVersion() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.apiVersion;
}

VkPhysicalDeviceLimits VulkanGraphicsAdapter::limits() const noexcept
{
    return m_impl->m_limits;
}

UInt64 VulkanGraphicsAdapter::dedicatedMemory() const noexcept
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    ::vkGetPhysicalDeviceMemoryProperties(this->handle(), &memoryProperties);

    auto memoryHeaps = memoryProperties.memoryHeaps;
    auto heaps = Array<VkMemoryHeap>(memoryHeaps, memoryHeaps + memoryProperties.memoryHeapCount);

    size_t heapSize = 0;

    for (const auto& heap : heaps)
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            heapSize += heap.size;

    return heapSize;
}

bool VulkanGraphicsAdapter::validateDeviceExtensions(Span<const String> extensions) const noexcept
{
    auto availableExtensions = this->getAvailableDeviceExtensions();

    return std::ranges::all_of(extensions, [&availableExtensions](const auto& extension) {
        auto match = std::ranges::find_if(availableExtensions, [&extension](const auto& str) {
            return std::equal(str.begin(), str.end(), extension.begin(), extension.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });

        if (match == availableExtensions.end())
            LITEFX_ERROR(VULKAN_LOG, "Extension {0} is not supported by this adapter.", extension);

        return match != availableExtensions.end();
    });
}

Array<String> VulkanGraphicsAdapter::getAvailableDeviceExtensions() const noexcept
{
    UInt32 extensions = 0;
    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, availableExtensions.data());

    return availableExtensions |
        std::views::transform([](const VkExtensionProperties& extension) { return String(extension.extensionName); }) |
        std::ranges::to<Array<String>>();
}

bool VulkanGraphicsAdapter::validateDeviceLayers(Span<const String> layers) const noexcept
{
    auto availableLayers = this->deviceValidationLayers();

    return std::ranges::all_of(layers, [&availableLayers](const auto& layer) {
        auto match = std::ranges::find_if(availableLayers, [&layer](const auto& str) {
            return std::equal(str.begin(), str.end(), layer.begin(), layer.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });

        if (match == availableLayers.end())
            LITEFX_ERROR(VULKAN_LOG, "Validation layer {0} is not supported by this adapter.", layer);

        return match != availableLayers.end();
    });
}

Array<String> VulkanGraphicsAdapter::deviceValidationLayers() const noexcept
{
    UInt32 layers = 0;
    ::vkEnumerateDeviceLayerProperties(this->handle(), &layers, nullptr);

    Array<VkLayerProperties> availableLayers(layers);
    ::vkEnumerateDeviceLayerProperties(this->handle(), &layers, availableLayers.data());

    return availableLayers |
        std::views::transform([](const VkLayerProperties& layer) { return String(layer.layerName); }) |
        std::ranges::to<Array<String>>();
}