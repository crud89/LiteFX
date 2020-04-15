#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

AppBuilder& VulkanBackendBuilder::go()
{
    auto adapter = this->instance()->getAdapter();
    auto surface = this->instance()->getSurface();

    if (adapter == nullptr)
        throw std::runtime_error("No adapter has been defined to use for this backend.");

    if (surface == nullptr)
        throw std::runtime_error("No surface has been defined to use for this backend.");

    Logger::get(VULKAN_LOG).info("Creating Vulkan rendering backend for adapter {0} ({1}).", 
        adapter->getName(), adapter->getDeviceId());

    Logger::get(VULKAN_LOG).debug("--------------------------------------------------------------------------");
    Logger::get(VULKAN_LOG).debug("Vendor: {0:#0x}", adapter->getVendorId());
    Logger::get(VULKAN_LOG).debug("Driver Version: {0:#0x}", adapter->getDriverVersion());
    Logger::get(VULKAN_LOG).debug("API Version: {0:#0x}", adapter->getApiVersion());
    Logger::get(VULKAN_LOG).debug("--------------------------------------------------------------------------");
    Logger::get(VULKAN_LOG).debug("Available extensions: {0}", Join(VulkanBackend::getAvailableExtensions(), ", "));
    Logger::get(VULKAN_LOG).debug("Validation layers: {0}", Join(VulkanBackend::getValidationLayers(), ", "));
    Logger::get(VULKAN_LOG).debug("--------------------------------------------------------------------------");

    return builder_type::go();
}

VulkanBackendBuilder& VulkanBackendBuilder::withSurface(UniquePtr<ISurface>&& surface)
{
    Logger::get(VULKAN_LOG).trace("Settings surface...");
    this->instance()->use(std::move(surface));
    return *this;
}

VulkanBackendBuilder& VulkanBackendBuilder::withSurface(VulkanSurface::surface_callback callback)
{
    return this->withSurface(std::move(VulkanSurface::createSurface(*this->instance(), callback)));
}

VulkanBackendBuilder& VulkanBackendBuilder::withAdapter(const UInt32& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);
    
    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapterId` is invalid.");

    Logger::get(VULKAN_LOG).trace("Using adapter id: {0}...", adapterId);
    this->instance()->use(adapter);
    return *this;
}

VulkanBackendBuilder& VulkanBackendBuilder::withAdapterOrDefault(const Optional<UInt32>& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);
    
    if (adapter == nullptr)
        adapter = this->instance()->findAdapter(std::nullopt);

    Logger::get(VULKAN_LOG).trace("Using adapter id: {0}...", adapter->getDeviceId());
    this->instance()->use(adapter);
    return *this;
}