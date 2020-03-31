#include <litefx/backends/vulkan.hpp>

#include <stdexcept>
#include <algorithm>

using namespace LiteFX::Rendering::Backends;

VulkanBackend::VulkanBackend(const App& app, const Array<String>& extensions) :
    RenderBackend(app), m_instance(nullptr)
{
	this->initialize(extensions);
}

VulkanBackend::~VulkanBackend()
{
    this->release();
}

void VulkanBackend::initialize(const Array<String>& extensions)
{
    // Check, if already initialized.
    if (m_instance != nullptr)
        throw std::runtime_error("The backend is already initialized. Call `release` and try again.");

    // Parse the extensions.
    std::vector<const char*> requiredExtensions;

    for each (auto & extension in extensions)
        requiredExtensions.push_back(extension.data());

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
    createInfo.enabledLayerCount = 0;

    auto result = ::vkCreateInstance(&createInfo, nullptr, &m_instance);
}

void VulkanBackend::release()
{
    ::vkDestroyInstance(m_instance, nullptr);
    m_instance = nullptr;
}

bool VulkanBackend::validateExtensions(const Array<String>& extensions)
{
    auto availableExtensions = this->getAvailableExtensions();
    
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

Array<String> VulkanBackend::getAvailableExtensions()
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