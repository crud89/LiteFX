#include <litefx/backends/vulkan.hpp>
#include <optional>

using namespace LiteFX::Rendering::Backends;

VulkanGraphicsAdapter::VulkanGraphicsAdapter(const VkPhysicalDevice adapter) :
	GraphicsAdapter(), IResource(adapter)
{
}

VkPhysicalDeviceProperties VulkanGraphicsAdapter::getProperties() const
{
    VkPhysicalDeviceProperties properties;
    ::vkGetPhysicalDeviceProperties(this->handle(), &properties);

    return properties;
}

VkPhysicalDeviceFeatures VulkanGraphicsAdapter::getFeatures() const
{
    VkPhysicalDeviceFeatures features;
    ::vkGetPhysicalDeviceFeatures(this->handle(), &features);
    
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

UniquePtr<GraphicsDevice> VulkanGraphicsAdapter::createDevice() const
{
    // Find an available graphics queue.
    uint32_t queueFamilies = 0;
    ::vkGetPhysicalDeviceQueueFamilyProperties(this->handle(), &queueFamilies, nullptr);

    Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
    ::vkGetPhysicalDeviceQueueFamilyProperties(this->handle(), &queueFamilies, familyProperties.data());
    
    std::optional<uint32_t> graphicsQueue, presentQueue;
    
    for (size_t i(0); i < familyProperties.size(); ++i)
    {
        if (familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsQueue = static_cast<uint32_t>(i);

        VkBool32 canPresent = false;
        //::vkGetPhysicalDeviceSurfaceSupportKHR(this->handle(), i, surface, &canPresent);

        if (canPresent)
            presentQueue = static_cast<uint32_t>(i);

        if (presentQueue.has_value() && graphicsQueue.has_value())
            break;
    }

    if (!graphicsQueue.has_value())
        throw std::runtime_error("No graphics queue could be found.");

    // Define a graphics queue for the device.
    const float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueue.value();
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

    if (::vkCreateDevice(this->handle(), &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Unable to create Vulkan device.");

    return makeUnique<VulkanDevice>(device);
}