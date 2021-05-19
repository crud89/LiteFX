#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBackend::VulkanBackendImpl : public Implement<VulkanBackend> {
public:
    friend class VulkanBackend;

private:
    Array<UniquePtr<VulkanGraphicsAdapter>> m_adapters{ };
    Array<String> m_extensions;
    Array<String> m_layers;
    const App& m_app;

public:
    VulkanBackendImpl(VulkanBackend* parent, const App& app, Span<String> extensions, Span<String> validationLayers) :
        base(parent), m_app(app)
    {
        m_extensions.assign(std::begin(extensions), std::end(extensions));
        m_layers.assign(std::begin(validationLayers), std::end(validationLayers));
    }

#ifndef NDEBUG
private:
    VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr };
    VkInstance m_instance{ nullptr };
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger{ nullptr };
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger{ nullptr };

private:    
    static VKAPI_ATTR VkBool32 VKAPI_CALL onDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) 
    {
        String t = "";

        switch (type)
        {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: t = "GENERAL"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: t = "VALIDATION"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: t = "PERFORMANCE"; break;
        default: break;
        }

        switch (severity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LITEFX_WARNING(VULKAN_LOG, "{1}: {0}", callbackData->pMessage, t); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: LITEFX_ERROR(VULKAN_LOG, "{1}: {0}", callbackData->pMessage, t); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: LITEFX_DEBUG(VULKAN_LOG, "{1}: {0}", callbackData->pMessage, t); break;
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: LITEFX_TRACE(VULKAN_LOG, "{1}: {0}", callbackData->pMessage, t); break;
        }

        return VK_FALSE;
    }

public:
    ~VulkanBackendImpl() 
    {
        if (m_debugMessenger != nullptr && vkDestroyDebugUtilsMessenger != nullptr)
            vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
    }
#endif

public:
    VkInstance initialize()
    {
        // Check if all extensions are available.
        if (!VulkanBackend::validateExtensions(m_extensions))
            throw InvalidArgumentException("Some required Vulkan extensions are not supported by the system.");

        auto requiredExtensions = m_extensions | std::views::transform([this](const auto& extension) { return extension.c_str(); }) | ranges::to<Array<const char*>>();

        // Check if all extensions are available.
        if (!VulkanBackend::validateLayers(m_layers))
            throw InvalidArgumentException("Some required Vulkan layers are not supported by the system.");

        auto enabledLayers = m_layers | std::views::transform([this](const auto& layer) { return layer.c_str(); }) | ranges::to<Array<const char*>>();

        // Get the app instance.
        auto appName = m_app.getName();

        // Define Vulkan app.
        VkApplicationInfo appInfo = {};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(m_app.getVersion().getMajor(), m_app.getVersion().getMinor(), m_app.getVersion().getPatch());
        appInfo.pEngineName = LITEFX_ENGINE_ID;
        appInfo.engineVersion = VK_MAKE_VERSION(LITEFX_MAJOR, LITEFX_MINOR, LITEFX_REV);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        // Create Vulkan instance.
        VkInstanceCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = static_cast<UInt32>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();

#ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugCallbackInfo = {};
        debugCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCallbackInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCallbackInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCallbackInfo.pfnUserCallback = onDebugMessage;

        createInfo.pNext = &debugCallbackInfo;
#endif
        VkInstance instance;
        raiseIfFailed<RuntimeException>(::vkCreateInstance(&createInfo, nullptr, &instance), "Unable to create Vulkan instance.");

#ifndef NDEBUG
        vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessenger == nullptr)
            LITEFX_WARNING(VULKAN_LOG, "The debug messenger factory \"vkCreateDebugUtilsMessengerEXT\" could not be loaded. Debug utilities will not be enabled.");
        else if (vkDestroyDebugUtilsMessenger == nullptr)
            LITEFX_WARNING(VULKAN_LOG, "The debug messenger factory \"vkDestroyDebugUtilsMessengerEXT\" could not be loaded. Debug utilities will not be enabled.");
        else
        {
            auto result = vkCreateDebugUtilsMessenger(instance, &debugCallbackInfo, nullptr, &m_debugMessenger);

            if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
                LITEFX_WARNING(VULKAN_LOG, "The extension \"{0}\" is not present. Debug utilities will not be enabled.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            else
                raiseIfFailed<RuntimeException>(result, "Unable to initialize debug callback.");

            // Remember the instance so we can destroy the debug messenger.
            m_instance = instance;
        }
#endif

        // Return the instance.
        return instance;
    }

    void loadAdapters() noexcept
    {
        uint32_t adapters = 0;
        ::vkEnumeratePhysicalDevices(m_parent->handle(), &adapters, nullptr);

        Array<VkPhysicalDevice> handles(adapters);
        ::vkEnumeratePhysicalDevices(m_parent->handle(), &adapters, handles.data());

        m_adapters = handles | 
            std::views::transform([this](const auto& handle) { return makeUnique<VulkanGraphicsAdapter>(handle); }) |
            ranges::to<Array<UniquePtr<VulkanGraphicsAdapter>>>();
    }

    const VulkanGraphicsAdapter* findAdapter(const Optional<uint32_t> adapterId) const noexcept
    {
        auto match = std::find_if(m_adapters.begin(), m_adapters.end(), [&adapterId](const UniquePtr<VulkanGraphicsAdapter>& adapter) { return !adapterId.has_value() || adapter->getDeviceId() == adapterId; });

        if (match != m_adapters.end())
            return match->get();
        
        return nullptr;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBackend::VulkanBackend(const App& app, Span<String> extensions, Span<String> validationLayers) :
    m_impl(makePimpl<VulkanBackendImpl>(this, app, extensions, validationLayers)), IResource(nullptr)
{
    this->handle() = m_impl->initialize();
    m_impl->loadAdapters();
}

VulkanBackend::~VulkanBackend() noexcept 
{
    m_impl.destroy();
    ::vkDestroyInstance(this->handle(), nullptr);
}

BackendType VulkanBackend::getType() const noexcept
{
    return BackendType::Rendering;
}

Array<const VulkanGraphicsAdapter*> VulkanBackend::listAdapters() const
{
    return m_impl->m_adapters | std::views::transform([](const UniquePtr<VulkanGraphicsAdapter>& adapter) { return adapter.get(); }) | ranges::to<Array<const VulkanGraphicsAdapter*>>();
}

const VulkanGraphicsAdapter* VulkanBackend::findAdapter(const Optional<uint32_t>& adapterId) const
{
    if (auto match = std::ranges::find_if(m_impl->m_adapters, [&adapterId](const auto& adapter) { return !adapterId.has_value() || adapter->getDeviceId() == adapterId; }); match != m_impl->m_adapters.end()) [[likely]]
        return match->get();

    return nullptr;
}

Span<const String> VulkanBackend::getEnabledValidationLayers() const noexcept
{
    return m_impl->m_layers;
}

UniquePtr<VulkanSurface> VulkanBackend::createSurface(surface_callback predicate) const
{
    auto surface = predicate(this->handle());
    return makeUnique<VulkanSurface>(surface, this->handle());
}

// ------------------------------------------------------------------------------------------------
// Platform-specific implementation.
// ------------------------------------------------------------------------------------------------

#ifdef VK_USE_PLATFORM_WIN32_KHR

UniquePtr<VulkanSurface> VulkanBackend::createSurface(const HWND& hwnd) const
{
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = ::GetModuleHandle(nullptr);

    VkSurfaceKHR surface;
    raiseIfFailed<RuntimeException>(::vkCreateWin32SurfaceKHR(this->handle(), &createInfo, nullptr, &surface), "Unable to create vulkan surface for provided window.");

    return makeUnique<VulkanSurface>(surface, this->handle());
}

#endif

// ------------------------------------------------------------------------------------------------
// Static interface.
// ------------------------------------------------------------------------------------------------

bool VulkanBackend::validateExtensions(Span<const String> extensions) noexcept
{
    auto availableExtensions = VulkanBackend::getAvailableExtensions();

    return std::ranges::all_of(extensions, [&availableExtensions](const auto& extension) {
        auto match = std::ranges::find_if(availableExtensions, [&extension](const auto& str) {
            return std::equal(str.begin(), str.end(), extension.begin(), extension.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        }); 

        return match != availableExtensions.end();
    });
}

Array<String> VulkanBackend::getAvailableExtensions() noexcept
{
    UInt32 extensions = 0;
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, availableExtensions.data());

    return availableExtensions | 
        std::views::transform([](const VkExtensionProperties& extension) { return String(extension.extensionName); }) |
        ranges::to<Array<String>>();
}

bool VulkanBackend::validateLayers(Span<const String> layers) noexcept
{
    auto availableExtensions = VulkanBackend::getAvailableExtensions();

    return std::ranges::all_of(layers, [&availableExtensions](const auto& layer) {
        auto match = std::ranges::find_if(availableExtensions, [&layer](const auto& str) {
            return std::equal(str.begin(), str.end(), layer.begin(), layer.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });

        return match != availableExtensions.end();
    });
}

Array<String> VulkanBackend::getValidationLayers() noexcept
{
    UInt32 layers = 0;
    ::vkEnumerateInstanceLayerProperties(&layers, nullptr);

    Array<VkLayerProperties> availableLayers(layers);
    ::vkEnumerateInstanceLayerProperties(&layers, availableLayers.data());

    return availableLayers | 
        std::views::transform([](const VkLayerProperties& layer) { return String(layer.layerName); }) |
        ranges::to<Array<String>>();
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

AppBuilder& VulkanBackendBuilder::go()
{
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Available extensions: {0}", Join(VulkanBackend::getAvailableExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "Validation layers: {0}", Join(VulkanBackend::getValidationLayers(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

    if (this->instance()->getEnabledValidationLayers().size() > 0)
        LITEFX_INFO(VULKAN_LOG, "Enabled validation layers: {0}", Join(this->instance()->getEnabledValidationLayers(), ", "));

    return builder_type::go();
}