#include <litefx/backends/vulkan.hpp>

#include <stdexcept>
#include <algorithm>

using namespace LiteFX::Rendering::Backends;

VulkanBackend::VulkanBackend(const App& app, const Array<String>& extensions, const Array<String>& validationLayers) :
    RenderBackend(app), IResource(nullptr)
{
    this->initialize(extensions, validationLayers);
}

VulkanBackend::~VulkanBackend()
{
    this->release();
}

void VulkanBackend::initialize(const Array<String>& extensions, const Array<String>& validationLayers)
{
    // Check, if already initialized.
    if (this->handle() != nullptr)
        throw std::runtime_error("The backend is already initialized. Call `release` and try again.");

    // Parse the extensions.
    std::vector<const char*> requiredExtensions, enabledLayers;

    for each (auto & extension in extensions)
        requiredExtensions.push_back(extension.data());

    for each (auto & layer in validationLayers)
        enabledLayers.push_back(layer.data());

    // Check if all extensions are available.
    if (!this->validateExtensions(extensions))
        throw std::runtime_error("Some required Vulkan extensions are not supported by the system.");

    // Get the app instance.
    auto& app = this->getApp();
    auto appName = app.getName();

    // Define Vulkan app.
    VkApplicationInfo appInfo = {};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(app.getVersion().getMajor(), app.getVersion().getMinor(), app.getVersion().getPatch());
    appInfo.pEngineName = LITEFX_ENGINE_ID;
    appInfo.engineVersion = VK_MAKE_VERSION(LITEFX_MAJOR, LITEFX_MINOR, LITEFX_REV);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Create Vulkan instance.
    VkInstanceCreateInfo createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.size() == 0 ? nullptr : enabledLayers.data();

    auto result = ::vkCreateInstance(&createInfo, nullptr, &this->handle());
}

void VulkanBackend::release()
{
    ::vkDestroyInstance(this->handle(), nullptr);
    this->handle() = nullptr;
}

Array<UniquePtr<IGraphicsAdapter>> VulkanBackend::getAdapters() const
{
    if (this->handle() == nullptr)
        throw std::runtime_error("The backend is not initialized.");

    uint32_t adapters = 0;
    ::vkEnumeratePhysicalDevices(this->handle(), &adapters, nullptr);

    Array<VkPhysicalDevice> handles(adapters);
    Array<UniquePtr<IGraphicsAdapter>> results(adapters);

    ::vkEnumeratePhysicalDevices(this->handle(), &adapters, handles.data());
    std::generate(results.begin(), results.end(), [&handles, i = 0] () mutable { 
        return makeUnique<VulkanGraphicsAdapter>(handles[i++]); 
    });
    
    return results;
}

UniquePtr<IGraphicsAdapter> VulkanBackend::getAdapter(Optional<uint32_t> adapterId) const
{
    if (this->handle() == nullptr)
        throw std::runtime_error("The backend is not initialized.");

    uint32_t adapters = 0;
    ::vkEnumeratePhysicalDevices(this->handle(), &adapters, nullptr);

    Array<VkPhysicalDevice> handles(adapters);
    ::vkEnumeratePhysicalDevices(this->handle(), &adapters, handles.data());

    auto match = std::find_if(handles.begin(), handles.end(), [&adapterId](const VkPhysicalDevice& dvc) {
        return !adapterId.has_value() || makeUnique<VulkanGraphicsAdapter>(dvc)->getDeviceId() == adapterId;
    });

    if (match == handles.end())
        throw std::runtime_error("The requested graphics adapter could not be found.");

    return makeUnique<VulkanGraphicsAdapter>(*match);
}

UniquePtr<ICommandQueue> VulkanBackend::createQueue(const QueueType& queueType) const
{
    if (this->handle() == nullptr)
        throw std::runtime_error("The backend is not initialized.");

    if (LITEFX_FLAG_IS_SET(queueType, QueueType::Graphics));
    {
        throw;
    }

    throw;
}

//UniquePtr<ISurface> VulkanBackend::createSurface() const
//{
//    throw;
//}

// ------------------------------------------------------------------------------------------------
// Static interface.
// ------------------------------------------------------------------------------------------------

bool VulkanBackend::validateExtensions(const Array<String>& extensions) noexcept
{
    auto availableExtensions = VulkanBackend::getAvailableExtensions();

    for each (auto & extension in extensions)
    {
        auto match = std::find_if(availableExtensions.begin(), availableExtensions.end(), [&extension](String& str) {
            for (size_t i(0); i < str.size(); ++i)
                if (::tolower(str[i]) != ::tolower(extension[i]))
                    return false;

            return true;
        });

        if (match == availableExtensions.end())
            return false;
    }

    return true;
}

Array<String> VulkanBackend::getAvailableExtensions() noexcept
{
    uint32_t extensions = 0;
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, availableExtensions.data());

    Array<String> extensionNames;

    for each (auto & extension in availableExtensions)
        extensionNames.push_back(extension.extensionName);

    return extensionNames;
}

bool VulkanBackend::validateLayers(const Array<String>& validationLayers) noexcept
{
    auto layers = VulkanBackend::getValidationLayers();

    for each (auto & layer in validationLayers)
    {
        auto match = std::find_if(layers.begin(), layers.end(), [&layer](String& str) {
            for (size_t i(0); i < str.size(); ++i)
                if (::tolower(str[i]) != ::tolower(layer[i]))
                    return false;

            return true;
        });

        if (match == layers.end())
            return false;
    }

    return true;
}

Array<String> VulkanBackend::getValidationLayers() noexcept
{
    uint32_t layers = 0;
    ::vkEnumerateInstanceLayerProperties(&layers, nullptr);

    Array<VkLayerProperties> availableLayers(layers);
    ::vkEnumerateInstanceLayerProperties(&layers, availableLayers.data());

    Array<String> layerNames;

    for each (auto & layer in availableLayers)
        layerNames.push_back(layer.layerName);

    return layerNames;
}