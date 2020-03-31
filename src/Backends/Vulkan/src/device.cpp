#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanDevice::VulkanDevice(VkPhysicalDevice device) :
	RenderDevice(device)
{
}

VkPhysicalDeviceProperties VulkanDevice::getProperties() const
{
    VkPhysicalDeviceProperties properties;
    ::vkGetPhysicalDeviceProperties(this->getHandle<VkPhysicalDevice>(), &properties);

    return properties;
}

VkPhysicalDeviceFeatures VulkanDevice::getFeatures() const
{
    VkPhysicalDeviceFeatures features;
    ::vkGetPhysicalDeviceFeatures(this->getHandle<VkPhysicalDevice>(), &features);
    
    return features;
}

String VulkanDevice::getName() const
{
    auto properties = this->getProperties();
    return String(properties.deviceName);
}

uint32_t VulkanDevice::getVendorId() const
{
    auto properties = this->getProperties();
    return properties.vendorID;
}

uint32_t VulkanDevice::getDeviceId() const
{
    auto properties = this->getProperties();
    return properties.deviceID;
}

RenderDeviceType VulkanDevice::getType() const
{
    auto properties = this->getProperties();
    
    switch (properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return RenderDeviceType::CPU;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return RenderDeviceType::CPU;
    default:
        return RenderDeviceType::Other;
    }
}

uint32_t VulkanDevice::getDriverVersion() const
{
    auto properties = this->getProperties();
    return properties.driverVersion;
}

uint32_t VulkanDevice::getApiVersion() const
{
    auto properties = this->getProperties();
    return properties.apiVersion;
}