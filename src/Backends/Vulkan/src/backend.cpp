#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// Exported extensions (we should probably find a better solution for this).
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
PFN_vkCreateDebugUtilsMessengerEXT   vkCreateDebugUtilsMessenger;
PFN_vkDestroyDebugUtilsMessengerEXT  vkDestroyDebugUtilsMessenger;
PFN_vkQueueBeginDebugUtilsLabelEXT   vkQueueBeginDebugUtilsLabel;
PFN_vkQueueEndDebugUtilsLabelEXT     vkQueueEndDebugUtilsLabel;
PFN_vkQueueInsertDebugUtilsLabelEXT  vkQueueInsertDebugUtilsLabel;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBackend::VulkanBackendImpl {
public:
    friend class VulkanBackend;

private:
    Array<UniquePtr<VulkanGraphicsAdapter>> m_adapters{ };
    Dictionary<String, UniquePtr<VulkanDevice>> m_devices;
    Array<String> m_extensions;
    Array<String> m_layers;
    const App& m_app;

public:
    VulkanBackendImpl(const App& app, Span<String> extensions, Span<String> validationLayers) :
        m_app(app)
    {
        m_extensions.assign(std::begin(extensions), std::end(extensions));
        m_layers.assign(std::begin(validationLayers), std::end(validationLayers));

        this->defineMandatoryExtensions();
    }

    void defineMandatoryExtensions()
    {
        m_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
        // Interop swap chain requires external memory access.
        m_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) || !defined(NDEBUG)
        // Debugging extension should be guaranteed to be available.
        m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) || !defined(NDEBUG)
    }

#ifndef NDEBUG
private:
    VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_debugBreaker{ VK_NULL_HANDLE };
    VkInstance m_instance{ nullptr };

private:    
    static VKAPI_ATTR VkBool32 VKAPI_CALL onDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, [[maybe_unused]] void* userData)
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

        // Write to debug output.
        OutputDebugString(callbackData->pMessage);

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL onDebugBreak([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, [[maybe_unused]] void* userData)
    {
        // Ignore layer loader errors.
        constexpr UInt32 LAYER_LOAD_ERROR_MESSAGE_ID { 0x79DE34D4 };

        if (callbackData->messageIdNumber == LAYER_LOAD_ERROR_MESSAGE_ID)
            return VK_FALSE;

        __debugbreak();
        return VK_FALSE;
    }
#endif

private:
    inline void release()
    {
#ifndef NDEBUG
        if (m_debugMessenger != VK_NULL_HANDLE && vkDestroyDebugUtilsMessenger != nullptr)
            vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);

        if (m_debugBreaker != VK_NULL_HANDLE && vkDestroyDebugUtilsMessenger != nullptr)
            vkDestroyDebugUtilsMessenger(m_instance, m_debugBreaker, nullptr);
#endif
    }

public:
    VkInstance initialize()
    {
        // Check if all extensions are available.
        if (!VulkanBackend::validateInstanceExtensions(m_extensions))
            throw InvalidArgumentException("extensions", "Some required Vulkan extensions are not supported by the system.");

        auto requiredExtensions = m_extensions | std::views::transform([](const auto& extension) { return extension.c_str(); }) | std::ranges::to<Array<const char*>>();

        // Check if all extensions are available.
        if (!VulkanBackend::validateInstanceLayers(m_layers))
            throw InvalidArgumentException("validationLayers", "Some required Vulkan layers are not supported by the system.");

        auto enabledLayers = m_layers | std::views::transform([](const auto& layer) { return layer.c_str(); }) | std::ranges::to<Array<const char*>>();

        // Get the app instance.
        auto appName = String(m_app.name());

        // Define Vulkan app.
        VkApplicationInfo appInfo = {};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(m_app.version().major(), m_app.version().minor(), m_app.version().patch());
        appInfo.pEngineName = LITEFX_ENGINE_ID;
        appInfo.engineVersion = VK_MAKE_VERSION(LITEFX_MAJOR, LITEFX_MINOR, LITEFX_REV);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // Create Vulkan instance.
        VkInstanceCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = static_cast<UInt32>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();

#ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugMessageCallbackInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &onDebugMessage
        };

        VkDebugUtilsMessengerCreateInfoEXT debugBreakCallbackInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = &debugMessageCallbackInfo,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = &onDebugBreak
        };

        createInfo.pNext = &debugBreakCallbackInfo;
#endif
        VkInstance instance{};
        raiseIfFailed(::vkCreateInstance(&createInfo, nullptr, &instance), "Unable to create Vulkan instance.");

#ifndef NDEBUG
        vkCreateDebugUtilsMessenger     = std::bit_cast<PFN_vkCreateDebugUtilsMessengerEXT>(::vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessenger    = std::bit_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(::vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        vkQueueBeginDebugUtilsLabel     = std::bit_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(::vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
        vkQueueEndDebugUtilsLabel       = std::bit_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(::vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
        vkQueueInsertDebugUtilsLabel    = std::bit_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(::vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));

        if (vkCreateDebugUtilsMessenger == nullptr)
            LITEFX_WARNING(VULKAN_LOG, "The debug messenger factory \"vkCreateDebugUtilsMessengerEXT\" could not be loaded. Debug utilities will not be enabled.");
        else if (vkDestroyDebugUtilsMessenger == nullptr)
            LITEFX_WARNING(VULKAN_LOG, "The debug messenger factory \"vkDestroyDebugUtilsMessengerEXT\" could not be loaded. Debug utilities will not be enabled.");
        else
        {
            auto result = vkCreateDebugUtilsMessenger(instance, &debugMessageCallbackInfo, nullptr, &m_debugMessenger);

            if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
                LITEFX_WARNING(VULKAN_LOG, "The extension \"{0}\" is not present. Debug utilities will not be enabled.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            else
                raiseIfFailed(result, "Unable to initialize debug message callback.");

            debugBreakCallbackInfo.pNext = nullptr; // Reset pNext to comply with validation requirements.
            raiseIfFailed(vkCreateDebugUtilsMessenger(instance, &debugBreakCallbackInfo, nullptr, &m_debugBreaker), "Unable to initialize debug break callback.");

            // Remember the instance so we can destroy the debug messenger.
            m_instance = instance;
        }
#endif

        // Return the instance.
        return instance;
    }

    void loadAdapters(const VulkanBackend& backend)
    {
        uint32_t adapters = 0;
        ::vkEnumeratePhysicalDevices(backend.handle(), &adapters, nullptr);

        Array<VkPhysicalDevice> handles(adapters);
        ::vkEnumeratePhysicalDevices(backend.handle(), &adapters, handles.data());

        m_adapters = handles | 
            std::views::transform([](const auto& handle) { return makeUnique<VulkanGraphicsAdapter>(handle); }) |
            std::ranges::to<Array<UniquePtr<VulkanGraphicsAdapter>>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBackend::VulkanBackend(const App& app, Span<String> extensions, Span<String> validationLayers) :
    Resource<VkInstance>(nullptr), m_impl(app, extensions, validationLayers)
{
    this->handle() = m_impl->initialize();
    m_impl->loadAdapters(*this);

    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Available extensions: {0}", Join(this->getAvailableInstanceExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "Validation layers: {0}", Join(this->getInstanceValidationLayers(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

    if (this->getEnabledValidationLayers().size() > 0)
        LITEFX_INFO(VULKAN_LOG, "Enabled validation layers: {0}", Join(this->getEnabledValidationLayers(), ", "));
}

VulkanBackend::VulkanBackend(VulkanBackend&&) noexcept = default;
VulkanBackend& VulkanBackend::operator=(VulkanBackend&&) noexcept = default;
VulkanBackend::~VulkanBackend() noexcept 
{
    m_impl->release();
    ::vkDestroyInstance(this->handle(), nullptr);
}

BackendType VulkanBackend::type() const noexcept
{
    return BackendType::Rendering;
}

StringView VulkanBackend::name() const noexcept
{
    return "Vulkan"sv;
}

void VulkanBackend::activate()
{
    this->state() = BackendState::Active;
}

void VulkanBackend::deactivate()
{
    this->state() = BackendState::Inactive;
}

Enumerable<const VulkanGraphicsAdapter*> VulkanBackend::listAdapters() const
{
    return m_impl->m_adapters | std::views::transform([](const UniquePtr<VulkanGraphicsAdapter>& adapter) { return adapter.get(); });
}

const VulkanGraphicsAdapter* VulkanBackend::findAdapter(const Optional<UInt64>& adapterId) const
{
    if (auto match = std::ranges::find_if(m_impl->m_adapters, [&adapterId](const auto& adapter) { return !adapterId.has_value() || adapter->uniqueId() == adapterId; }); match != m_impl->m_adapters.end()) [[likely]]
        return match->get();

    return nullptr;
}

void VulkanBackend::registerDevice(String name, UniquePtr<VulkanDevice>&& device)
{
    if (m_impl->m_devices.contains(name))
        throw InvalidArgumentException("name", "The backend already contains a device with the name \"{0}\".", name);

#ifndef NDEBUG
    device->setDebugName(std::as_const(*device).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, name);
#endif

    m_impl->m_devices.insert(std::make_pair(name, std::move(device)));
}

void VulkanBackend::releaseDevice(const String& name)
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return;

    auto device = m_impl->m_devices[name].get();
    device->wait();

    m_impl->m_devices.erase(name);
}

VulkanDevice* VulkanBackend::device(const String& name) noexcept
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return nullptr;

    return m_impl->m_devices[name].get();
}

const VulkanDevice* VulkanBackend::device(const String& name) const noexcept
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return nullptr;

    return m_impl->m_devices[name].get();
}

Span<const String> VulkanBackend::getEnabledValidationLayers() const noexcept
{
    return m_impl->m_layers;
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

    VkSurfaceKHR surface{};
    raiseIfFailed(::vkCreateWin32SurfaceKHR(this->handle(), &createInfo, nullptr, &surface), "Unable to create vulkan surface for provided window.");

    return makeUnique<VulkanSurface>(surface, this->handle(), hwnd);
}

#else

UniquePtr<VulkanSurface> VulkanBackend::createSurface(surface_callback predicate) const
{
    auto surface = predicate(this->handle());
    return makeUnique<VulkanSurface>(surface, this->handle());
}

#endif

// ------------------------------------------------------------------------------------------------
// Static interface.
// ------------------------------------------------------------------------------------------------

bool VulkanBackend::validateInstanceExtensions(Span<const String> extensions)
{
    auto availableExtensions = VulkanBackend::getAvailableInstanceExtensions();

    return std::ranges::all_of(extensions, [&availableExtensions](const auto& extension) {
        auto match = std::ranges::find_if(availableExtensions, [&extension](const auto& str) {
            return std::equal(str.begin(), str.end(), extension.begin(), extension.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });

        if (match == availableExtensions.end())
            LITEFX_ERROR(VULKAN_LOG, "Extension {0} is not supported by this instance.", extension);

        return match != availableExtensions.end();
    });
}

Enumerable<String> VulkanBackend::getAvailableInstanceExtensions()
{
    UInt32 extensions = 0;
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, nullptr);

    Array<VkExtensionProperties> availableExtensions(extensions);
    ::vkEnumerateInstanceExtensionProperties(nullptr, &extensions, availableExtensions.data());

    return availableExtensions | std::views::transform([](const VkExtensionProperties& extension) { return String(extension.extensionName); }); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

bool VulkanBackend::validateInstanceLayers(Span<const String> layers)
{
    auto availableLayers = VulkanBackend::getInstanceValidationLayers();

    return std::ranges::all_of(layers, [&availableLayers](const auto& layer) {
        auto match = std::ranges::find_if(availableLayers, [&layer](const auto& str) {
            return std::equal(str.begin(), str.end(), layer.begin(), layer.end(), [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });

        if (match == availableLayers.end())
            LITEFX_ERROR(VULKAN_LOG, "Validation layer {0} is not supported by this instance.", layer);

        return match != availableLayers.end();
    });
}

Enumerable<String> VulkanBackend::getInstanceValidationLayers()
{
    UInt32 layers = 0;
    ::vkEnumerateInstanceLayerProperties(&layers, nullptr);

    Array<VkLayerProperties> availableLayers(layers);
    ::vkEnumerateInstanceLayerProperties(&layers, availableLayers.data());

    return availableLayers | std::views::transform([](const VkLayerProperties& layer) { return String(layer.layerName); }); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}