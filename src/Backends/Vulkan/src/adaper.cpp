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
    String m_name;
    UInt64 m_luid;
    UInt32 m_vendorId;
    UInt32 m_deviceId;
    UInt32 m_driverVersion;
    UInt32 m_apiVersion;
    GraphicsAdapterType m_type;
    UInt64 m_deviceLocalMemory{ 0 };

public:
    VulkanGraphicsAdapterImpl(VulkanGraphicsAdapter* parent) : 
        base(parent) 
    {
        // Cache device properties.
        VkPhysicalDeviceIDProperties deviceIdProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
        VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        deviceProperties.pNext = &deviceIdProperties;
        ::vkGetPhysicalDeviceProperties2(m_parent->handle(), &deviceProperties);

        const auto& properties = deviceProperties.properties;
        m_limits = properties.limits;
        m_name = String(properties.deviceName);
        m_vendorId = properties.vendorID;
        m_deviceId = properties.deviceID;
        m_driverVersion = properties.driverVersion;
        m_apiVersion = properties.apiVersion;

        // Get the LUID.
        m_luid = *reinterpret_cast<UInt64*>(&deviceIdProperties.deviceLUID);

        // Get device type.
        switch (properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            m_type = GraphicsAdapterType::CPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            m_type = GraphicsAdapterType::GPU;
            break;
        default:
            m_type = GraphicsAdapterType::Other;
            break;
        }

        // Get available dedicated (device-local) memory.
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        ::vkGetPhysicalDeviceMemoryProperties(m_parent->handle(), &memoryProperties);

        auto memoryHeaps = memoryProperties.memoryHeaps;
        auto heaps = Span<VkMemoryHeap>(memoryHeaps, memoryHeaps + memoryProperties.memoryHeapCount);

        for (const auto& heap : heaps)
            if ((heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                m_deviceLocalMemory += heap.size;
    }

public:

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
    return m_impl->m_name;
}

UInt64 VulkanGraphicsAdapter::uniqueId() const noexcept
{
    return m_impl->m_luid;
}

UInt32 VulkanGraphicsAdapter::vendorId() const noexcept
{
    return m_impl->m_vendorId;
}

UInt32 VulkanGraphicsAdapter::deviceId() const noexcept
{
    return m_impl->m_deviceId;
}

GraphicsAdapterType VulkanGraphicsAdapter::type() const noexcept
{
    return m_impl->m_type;
}

UInt32 VulkanGraphicsAdapter::driverVersion() const noexcept
{
    return m_impl->m_driverVersion;
}

UInt32 VulkanGraphicsAdapter::apiVersion() const noexcept
{
    return m_impl->m_apiVersion;
}

VkPhysicalDeviceLimits VulkanGraphicsAdapter::limits() const noexcept
{
    return m_impl->m_limits;
}

UInt64 VulkanGraphicsAdapter::dedicatedMemory() const noexcept
{
    return m_impl->m_deviceLocalMemory;
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

Enumerable<String> VulkanGraphicsAdapter::getAvailableDeviceExtensions() const noexcept
{
    UInt32 extensions = 0;
    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    ::vkEnumerateDeviceExtensionProperties(this->handle(), nullptr, &extensions, availableExtensions.data());

    return availableExtensions | std::views::transform([](const VkExtensionProperties& extension) { return String(extension.extensionName); });
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

Enumerable<String> VulkanGraphicsAdapter::deviceValidationLayers() const noexcept
{
    UInt32 layers = 0;
    ::vkEnumerateDeviceLayerProperties(this->handle(), &layers, nullptr);

    Array<VkLayerProperties> availableLayers(layers);
    ::vkEnumerateDeviceLayerProperties(this->handle(), &layers, availableLayers.data());

    return availableLayers | std::views::transform([](const VkLayerProperties& layer) { return String(layer.layerName); });
}