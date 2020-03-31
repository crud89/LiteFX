#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanGraphicsAdapter::VulkanGraphicsAdapter(VkPhysicalDevice adapter) :
	GraphicsAdapter(adapter)
{
}

VkPhysicalDeviceProperties VulkanGraphicsAdapter::getProperties() const
{
    VkPhysicalDeviceProperties properties;
    ::vkGetPhysicalDeviceProperties(this->getHandle<VkPhysicalDevice>(), &properties);

    return properties;
}

VkPhysicalDeviceFeatures VulkanGraphicsAdapter::getFeatures() const
{
    VkPhysicalDeviceFeatures features;
    ::vkGetPhysicalDeviceFeatures(this->getHandle<VkPhysicalDevice>(), &features);
    
    return features;
}

String VulkanGraphicsAdapter::getName() const
{
    auto properties = this->getProperties();
    return String(properties.deviceName);
}

uint32_t VulkanGraphicsAdapter::getVendorId() const
{
    auto properties = this->getProperties();
    return properties.vendorID;
}

uint32_t VulkanGraphicsAdapter::getDeviceId() const
{
    auto properties = this->getProperties();
    return properties.deviceID;
}

GraphicsAdapterType VulkanGraphicsAdapter::getType() const
{
    auto properties = this->getProperties();
    
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

uint32_t VulkanGraphicsAdapter::getDriverVersion() const
{
    auto properties = this->getProperties();
    return properties.driverVersion;
}

uint32_t VulkanGraphicsAdapter::getApiVersion() const
{
    auto properties = this->getProperties();
    return properties.apiVersion;
}