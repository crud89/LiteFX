#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include <bit>

using namespace LiteFX::Rendering::Backends;

extern PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasks { nullptr };
extern PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizes { nullptr };
extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure { nullptr };
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure { nullptr };
extern PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructures { nullptr };

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl : public Implement<VulkanDevice> {
public:
    friend class VulkanDevice;

private:
    class QueueFamily {
    private:
        Array<UniquePtr<VulkanQueue>> m_queues;
        Array<Float> m_queuePriorities;
        UInt32 m_id, m_queueCount;
        QueueType m_type;

    public:
        QueueType type() const noexcept { return m_type; }
        UInt32 total() const noexcept { return m_queueCount; }
        UInt32 active() const noexcept { return static_cast<UInt32>(m_queues.size()); }
        UInt32 id() const noexcept { return m_id; }
        const Array<UniquePtr<VulkanQueue>>& queues() const noexcept { return m_queues; }

    public:
        QueueFamily(UInt32 id, UInt32 queueCount, QueueType type) :
            m_id(id), m_queueCount(queueCount), m_type(type) 
        {
            if (queueCount == 0) [[unlikely]]
                throw InvalidArgumentException("queueCount", "An empty queue family is invalid.");

            // Initialize queue priorities. First queue always has the highest priority possible, as those become the default queues. After this we fill up the space evenly 
            // through all available priorities. Currently there are two priorities (`High` and `Normal`)
            auto chunkSize = (queueCount - 1) / 2;
            auto remainder = (queueCount - 1) % 2;

            auto high = std::views::repeat(static_cast<Float>(QueuePriority::High) / 100.0f)   | std::views::take(chunkSize);
            auto low  = std::views::repeat(static_cast<Float>(QueuePriority::Normal) / 100.0f) | std::views::take(chunkSize + remainder);
            
            m_queuePriorities.push_back(1.0f);

#ifdef __cpp_lib_containers_ranges
            m_queuePriorities.append_range(high);
            m_queuePriorities.append_range(low);
#else
            m_queuePriorities.insert(m_queuePriorities.end(), high.cbegin(), high.cend());
            m_queuePriorities.insert(m_queuePriorities.end(), low.cbegin(), low.cend());
#endif
        }
        QueueFamily(const QueueFamily& _other) = delete;
        QueueFamily(QueueFamily&& _other) noexcept {
            m_queues = std::move(_other.m_queues);
            m_id = std::move(_other.m_id);
            m_queueCount = std::move(_other.m_queueCount);
            m_type = std::move(_other.m_type);
            m_queuePriorities = std::move(_other.m_queuePriorities);
        }
        ~QueueFamily() noexcept {
            m_queues.clear();
        }

    public:
        VulkanQueue* createQueue(const VulkanDevice& device, QueuePriority priority) {
            // First, list all queues with the requested priority.
            auto left  = std::ranges::lower_bound(m_queuePriorities, static_cast<Float>(priority) / 100.0f, std::greater<float>{});
            auto right = std::ranges::upper_bound(m_queuePriorities, static_cast<Float>(priority) / 100.0f, std::greater<float>{});

            if (left == std::end(m_queuePriorities)) [[unlikely]]
            {
                QueuePriority nextPriority;
                
                switch (priority)
                {
                using enum QueuePriority;
                case Normal: nextPriority = QueuePriority::High; break;
                case High: nextPriority = QueuePriority::Realtime; break; // This will find the default queue
                case Realtime: [[unlikely]] // Should never actually happen, as there should be at least one queue in the family.
                default: throw RuntimeException("No queue found in this priority. Check if the queue family contains any queues.");
                }

                LITEFX_WARNING(VULKAN_LOG, "No queues found at priority {0}. Attempting higher priority {1}.", priority, nextPriority);
                return this->createQueue(device, nextPriority);
            }

            // List the queue indices for the matched priorities and how often they are used.
            auto indices = std::views::iota(std::ranges::distance(std::cbegin(m_queuePriorities), left), std::ranges::distance(std::cbegin(m_queuePriorities), right)) |
                std::views::transform([this](auto i) { return std::make_tuple(i, std::ranges::count_if(m_queues, [i](const auto& q) { return q->queueId() == i; })); });
            auto [queueId, refCount] = *std::ranges::min_element(indices, {}, [](const auto& i) { return std::get<1>(i); });

            LITEFX_DEBUG(VULKAN_LOG, "Creating queue with id {0} of type {2} (referenced {1} times).", queueId, refCount, m_type);

            // Create a queue instance with the queue id.
            auto queue = makeUnique<VulkanQueue>(device, m_type, priority, m_id, static_cast<UInt32>(queueId));
            auto queuePointer = queue.get();
            m_queues.push_back(std::move(queue));
            return queuePointer;
        }
    };

    DeviceState m_deviceState;

    Array<QueueFamily> m_families;
    VulkanQueue* m_graphicsQueue;
    VulkanQueue* m_transferQueue;
    VulkanQueue* m_computeQueue;

    UniquePtr<VulkanSwapChain> m_swapChain;
    Array<String> m_extensions;

    const VulkanGraphicsAdapter& m_adapter;
    UniquePtr<VulkanSurface> m_surface;
    UniquePtr<VulkanGraphicsFactory> m_factory;

#ifndef NDEBUG
    PFN_vkDebugMarkerSetObjectNameEXT debugMarkerSetObjectName = nullptr;
#endif

public:
    VulkanDeviceImpl(VulkanDevice* parent, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Span<String> extensions) :
        base(parent), m_adapter(adapter), m_surface(std::move(surface))
    {
        if (m_surface == nullptr)
            throw ArgumentNotInitializedException("surface", "The surface must be initialized.");

        m_extensions.assign(std::begin(extensions), std::end(extensions));

        this->defineMandatoryExtensions();
        this->loadQueueFamilies();
    }

    ~VulkanDeviceImpl()
    {
        // Clear the device state.
        m_deviceState.clear();

        // This will also cause all queue instances to be automatically released (graphicsQueue, transferQueue, bufferQueue).
        m_families.clear();

        // Release the swap chain.
        m_swapChain = nullptr;

        // Destroy the surface.
        m_surface = nullptr;
    }

private:
    void defineMandatoryExtensions() noexcept
    {
        // NOTE: If an extension is not supported, update the graphics driver to the most recent one. You can lookup extension support for individual drivers here:
        // https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_KHR_present_wait (replace the extension name to adjust the filter).

        // Required to query image and buffer requirements.
        m_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

#ifdef LITEFX_BUILD_MESH_SHADER_SUPPORT
        // Required for mesh shading.
        m_extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
#endif

#ifdef LITEFX_BUILD_RAY_TRACING_SUPPORT
        m_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        m_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
#endif

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
        // Interop swap chain requires external memory access.
        m_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        m_extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

        // Required to synchronize Vulkan command execution with D3D presentation.
        m_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
        //m_extensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else
        m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

#ifndef NDEBUG
        auto availableExtensions = m_adapter.getAvailableDeviceExtensions();

        // Required to set debug names.
        if (auto match = std::ranges::find_if(availableExtensions, [](const String& extension) { return extension == VK_EXT_DEBUG_MARKER_EXTENSION_NAME; }); match != availableExtensions.end())
            m_extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif
    }

public:
    void loadQueueFamilies()
    {
        // Find an available command queues.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter.handle(), &queueFamilies, familyProperties.data());

        auto families = familyProperties | 
            std::views::transform([i = 0](const VkQueueFamilyProperties& familyProperty) mutable -> Tuple<int, UInt32, QueueType> {
                QueueType type = QueueType::None;

                if (familyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    type |= QueueType::Compute;
                if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    type |= QueueType::Graphics;
                if (familyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    type |= QueueType::Transfer;
                if (familyProperty.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                    type |= QueueType::VideoDecode;

#ifdef VK_ENABLE_BETA_EXTENSIONS
                if (familyProperty.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                    type |= QueueType::VideoEncode;
#endif // VK_ENABLE_BETA_EXTENSIONS

                return { i++, familyProperty.queueCount, type };
            }) | std::ranges::to<std::vector>();

        // Sort by flag popcount, so that the most specialized queues are always first.
        std::ranges::sort(families, std::less<>{}, [](const auto& family) -> int { return std::popcount(std::to_underlying(std::get<2>(family))); });

        // Create all the queues.
        m_families = families |
            std::views::transform([](const auto& family) { return QueueFamily(std::get<0>(family), std::get<1>(family), std::get<2>(family)); }) |
            std::ranges::to<Array<QueueFamily>>();
    }

    VkDevice initialize()
    {
        if (!m_adapter.validateDeviceExtensions(m_extensions))
            throw InvalidArgumentException("extensions", "Some required device extensions are not supported by the system.");

        auto const requiredExtensions = m_extensions | std::views::transform([](const auto& extension) { return extension.c_str(); }) | std::ranges::to<Array<const char*>>();
        
        // Creating queues in Vulkan is a bit odd in that we need to specify how many queues we want before actually allocating them. Since this is different to other APIs,
        // where we can create queues, as we need them, we allocate as many as possible and only hand them out if required. This will cause most of the queues to idle, in
        // which case it should not matter, that we created them in the first place. On NVidia it seems that all queues from one family are virtualized into a single queue
        // anyway, which is similar to D3D12. On AMD it appears that there is only one queue per family in the first place. Regarding queue priority, we assign the highest 
        // priority to the first queue in each family (as they are used for default queues).
        auto maxQueueCount = std::ranges::max(m_families | std::views::transform([](auto& family) { return family.total(); }));
        Array<Float> priorities(maxQueueCount);
        std::ranges::generate(priorities, [i = 0]() mutable { return static_cast<Float>(i++ == 0 ? QueuePriority::High : QueuePriority::Normal) / 100.0f; });

        auto const queueCreateInfos = m_families |
            std::views::transform([&priorities](auto& family) {
                return VkDeviceQueueCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = family.id(),
                    .queueCount = family.total(),
                    .pQueuePriorities = priorities.data()
                };
            }) | std::ranges::to<Array<VkDeviceQueueCreateInfo>>();

        // Enable raytracing features.
#if   defined(LITEFX_BUILD_RAY_TRACING_SUPPORT)
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .accelerationStructure = true,
            .descriptorBindingAccelerationStructureUpdateAfterBind = true
        };
#endif

        // Enable task and mesh shaders.
#if   defined(LITEFX_BUILD_MESH_SHADER_SUPPORT)
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
#if   defined(LITEFX_BUILD_RAY_TRACING_SUPPORT)
            .pNext = &accelerationStructureFeatures,
#endif
            .taskShader = true,
            .meshShader = true
        };
#endif

        // Allow geometry and tessellation shader stages.
        VkPhysicalDeviceFeatures2 deviceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
#if   defined(LITEFX_BUILD_MESH_SHADER_SUPPORT)
            .pNext = &meshShaderFeatures,
#elif defined(LITEFX_BUILD_RAY_TRACING_SUPPORT)
            .pNext = &accelerationStructureFeatures,
#endif
            .features = {
                .geometryShader = true,
                .tessellationShader = true,
                .samplerAnisotropy = true
            }
        };

        // Enable synchronization overhaul.
        VkPhysicalDeviceVulkan13Features deviceFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &deviceFeatures,
            .synchronization2 = true,
            .maintenance4 = true
        };

        // Enable various descriptor related features, as well as timelime semaphores and other little QoL improvements.
        VkPhysicalDeviceVulkan12Features deviceFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &deviceFeatures13,
            .descriptorIndexing = true,
            .shaderInputAttachmentArrayDynamicIndexing = true,
            .shaderUniformTexelBufferArrayDynamicIndexing = true,
            .shaderStorageTexelBufferArrayDynamicIndexing = true,
            .shaderUniformBufferArrayNonUniformIndexing = true,
            .shaderSampledImageArrayNonUniformIndexing = true,
            .shaderStorageBufferArrayNonUniformIndexing = true,
            .shaderStorageImageArrayNonUniformIndexing = true,
            .shaderInputAttachmentArrayNonUniformIndexing = true,
            .shaderUniformTexelBufferArrayNonUniformIndexing = true,
            .shaderStorageTexelBufferArrayNonUniformIndexing = true,
            //.descriptorBindingUniformBufferUpdateAfterBind = true,	// Causes problems on some NVidia cards.
            .descriptorBindingSampledImageUpdateAfterBind = true,
            .descriptorBindingStorageImageUpdateAfterBind = true,
            .descriptorBindingStorageBufferUpdateAfterBind = true,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = true,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = true,
            .descriptorBindingUpdateUnusedWhilePending = true,
            .descriptorBindingPartiallyBound = true,
            .descriptorBindingVariableDescriptorCount = true,
            .runtimeDescriptorArray = true,
            .separateDepthStencilLayouts = true,
            .hostQueryReset = true,
            .timelineSemaphore = true,
            .bufferDeviceAddress = true
        };

        // Enable extended dynamic state.
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = &deviceFeatures12,
            .extendedDynamicState = true
        };

        // Define the device itself.
        VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &extendedDynamicStateFeatures,
            .queueCreateInfoCount = static_cast<UInt32>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()
        };

        // Create the device.
        // NOTE: This can time-out under very mysterious circumstances, in which case the event log shows a TDR error. Unfortunately, the only way I found
        //       to fix this is rebooting the entire system.
        VkDevice device;
        raiseIfFailed(::vkCreateDevice(m_adapter.handle(), &createInfo, nullptr, &device), "Unable to create Vulkan device.");

        // Load extension methods.
#ifndef NDEBUG
        debugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(::vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
#endif

#ifdef LITEFX_BUILD_MESH_SHADER_SUPPORT
        if (vkCmdDrawMeshTasks == nullptr)
            vkCmdDrawMeshTasks = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(::vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT"));
#endif

#ifdef LITEFX_BUILD_RAY_TRACING_SUPPORT
        if (vkGetAccelerationStructureBuildSizes == nullptr)
            vkGetAccelerationStructureBuildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(::vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));

        if (vkCreateAccelerationStructure == nullptr)
            vkCreateAccelerationStructure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));

        if (vkDestroyAccelerationStructure == nullptr)
            vkDestroyAccelerationStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));

        if (vkCmdBuildAccelerationStructures == nullptr)
            vkCmdBuildAccelerationStructures = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(::vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
#endif

        return device;
    }

    void initializeDefaultQueues() 
    {
        // Initialize default queues.
        m_graphicsQueue = this->createQueue(QueueType::Graphics, QueuePriority::Realtime, std::as_const(*m_surface).handle());
        m_transferQueue = this->createQueue(QueueType::Transfer, QueuePriority::Realtime);
        m_computeQueue  = this->createQueue(QueueType::Compute,  QueuePriority::Realtime);

        if (m_graphicsQueue == nullptr)
            throw RuntimeException("Unable to find a fitting command queue to present the specified surface.");

        if (m_transferQueue == nullptr)
        {
            LITEFX_INFO(VULKAN_LOG, "Unable to find dedicated transfer queue for device-device transfer. Using default graphics queue instead.");
            m_transferQueue = m_graphicsQueue;
        }

        if (m_computeQueue == nullptr)
        {
            LITEFX_INFO(VULKAN_LOG, "Unable to find dedicated compute queue. Using default graphics queue instead.");
            m_computeQueue = m_graphicsQueue;
        }
    }

    void createFactory()
    {
        m_factory = makeUnique<VulkanGraphicsFactory>(*m_parent);
    }

    void createSwapChain(Format format, const Size2d& frameBufferSize, UInt32 frameBuffers)
    {
        m_swapChain = makeUnique<VulkanSwapChain>(*m_parent, format, frameBufferSize, frameBuffers);
    }

public:
    VulkanQueue* createQueue(QueueType type, QueuePriority priority, const VkSurfaceKHR& surface = VK_NULL_HANDLE)
    {
        // Find the queue that is most specialized for the provided queue type. Since the queues are ordered based on their type popcount (most specialized queues come first, as they have 
        // lower type flags set), we can simply pick the first one we find, that matches all the flags.
        auto match = std::ranges::find_if(m_families, [&](const auto& family) { 
            VkBool32 result = LITEFX_FLAG_IS_SET(family.type(), type);

            if (surface != VK_NULL_HANDLE) [[unlikely]]
            {
                // Check if presenting to the surface is supported.
                VkBool32 canPresent = VK_FALSE;
                ::vkGetPhysicalDeviceSurfaceSupportKHR(m_adapter.handle(), family.id(), surface, &canPresent);

                result &= canPresent;
            }

            return result != VK_FALSE;
        });

        return match == m_families.end() ? nullptr : match->createQueue(*m_parent, priority);
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Span<String> extensions) :
    VulkanDevice(backend, adapter, std::move(surface), Format::B8G8R8A8_SRGB, { 800, 600 }, 3, extensions)
{
}

VulkanDevice::VulkanDevice(const VulkanBackend& /*backend*/, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Format format, const Size2d& frameBufferSize, UInt32 frameBuffers, Span<String> extensions) :
    Resource<VkDevice>(nullptr), m_impl(makePimpl<VulkanDeviceImpl>(this, adapter, std::move(surface), extensions))
{
    LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {0}, Adapter: {1}, Extensions: {2} }}...", fmt::ptr(reinterpret_cast<const void*>(m_impl->m_surface.get())), adapter.deviceId(), Join(this->enabledExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Vendor: {0:#0x}", adapter.vendorId());
    LITEFX_DEBUG(VULKAN_LOG, "Driver Version: {0:#0x}", adapter.driverVersion());
    LITEFX_DEBUG(VULKAN_LOG, "API Version: {0:#0x}", adapter.apiVersion());
    LITEFX_DEBUG(VULKAN_LOG, "Dedicated Memory: {0} Bytes", adapter.dedicatedMemory());
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Available extensions: {0}", Join(adapter.getAvailableDeviceExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "Validation layers: {0}", Join(adapter.deviceValidationLayers(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

    if (extensions.size() > 0)
        LITEFX_INFO(VULKAN_LOG, "Enabled validation layers: {0}", Join(extensions, ", "));

    this->handle() = m_impl->initialize();
    m_impl->initializeDefaultQueues();
    m_impl->createFactory();
    m_impl->createSwapChain(format, frameBufferSize, frameBuffers);
}

VulkanDevice::~VulkanDevice() noexcept
{
    // Destroy the implementation.
    m_impl.destroy();

    // Destroy the device.
    ::vkDestroyDevice(this->handle(), nullptr);
}

Span<const String> VulkanDevice::enabledExtensions() const noexcept
{
    return m_impl->m_extensions;
}

void VulkanDevice::setDebugName(UInt64 handle, VkDebugReportObjectTypeEXT type, StringView name) const noexcept
{
#ifndef NDEBUG
    if (m_impl->debugMarkerSetObjectName != nullptr)
    {
        VkDebugMarkerObjectNameInfoEXT nameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .object = handle,
            .pObjectName = name.data()
        };
        
        if (m_impl->debugMarkerSetObjectName(this->handle(), &nameInfo) != VK_SUCCESS)
            LITEFX_WARNING(VULKAN_LOG, "Unable to set object name for object handle {0}.", fmt::ptr(reinterpret_cast<void*>(handle)));
    }
#endif
}

Enumerable<UInt32> VulkanDevice::queueFamilyIndices(QueueType type) const noexcept
{
    return m_impl->m_families | 
        std::views::filter([type](const auto& family) { return type == QueueType::None || LITEFX_FLAG_IS_SET(family.type(), type); }) |
        std::views::transform([](const auto& family) { return family.id(); }) | 
        std::ranges::to<Enumerable<UInt32>>();
}

VulkanSwapChain& VulkanDevice::swapChain() noexcept
{
    return *m_impl->m_swapChain;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
VulkanRenderPassBuilder VulkanDevice::buildRenderPass(MultiSamplingLevel samples, UInt32 commandBuffers) const
{
    return VulkanRenderPassBuilder(*this, commandBuffers, samples);
}

VulkanRenderPassBuilder VulkanDevice::buildRenderPass(const String& name, MultiSamplingLevel samples, UInt32 commandBuffers) const
{
    return VulkanRenderPassBuilder(*this, commandBuffers, samples, name);
}

VulkanRenderPipelineBuilder VulkanDevice::buildRenderPipeline(const VulkanRenderPass& renderPass, const String& name) const
{
    return VulkanRenderPipelineBuilder(renderPass, name);
}

VulkanComputePipelineBuilder VulkanDevice::buildComputePipeline(const String& name) const
{
    return VulkanComputePipelineBuilder(*this, name);
}

VulkanPipelineLayoutBuilder VulkanDevice::buildPipelineLayout() const
{
    return VulkanPipelineLayoutBuilder(*this);
}

VulkanInputAssemblerBuilder VulkanDevice::buildInputAssembler() const
{
    return VulkanInputAssemblerBuilder();
}

VulkanRasterizerBuilder VulkanDevice::buildRasterizer() const
{
    return VulkanRasterizerBuilder();
}

VulkanShaderProgramBuilder VulkanDevice::buildShaderProgram() const
{
    return VulkanShaderProgramBuilder(*this);
}

VulkanBarrierBuilder VulkanDevice::buildBarrier() const
{
    return VulkanBarrierBuilder();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)

DeviceState& VulkanDevice::state() const noexcept
{
    return m_impl->m_deviceState;
}

const VulkanSwapChain& VulkanDevice::swapChain() const noexcept
{
    return *m_impl->m_swapChain;
}

const VulkanSurface& VulkanDevice::surface() const noexcept
{
    return *m_impl->m_surface;
}

const VulkanGraphicsAdapter& VulkanDevice::adapter() const noexcept
{
    return m_impl->m_adapter;
}

const VulkanGraphicsFactory& VulkanDevice::factory() const noexcept
{
    return *m_impl->m_factory;
}

const VulkanQueue& VulkanDevice::defaultQueue(QueueType type) const
{
    // If the type contains the graphics flag, always return the graphics queue.
    if (LITEFX_FLAG_IS_SET(type, QueueType::Graphics))
        return *m_impl->m_graphicsQueue;
    else if (LITEFX_FLAG_IS_SET(type, QueueType::Compute))
        return *m_impl->m_computeQueue;
    else if (LITEFX_FLAG_IS_SET(type, QueueType::Transfer))
        return *m_impl->m_transferQueue;
    else
        throw InvalidArgumentException("type", "No default queue for the provided queue type has was found.");
}

const VulkanQueue* VulkanDevice::createQueue(QueueType type, QueuePriority priority) noexcept 
{
    return m_impl->createQueue(type, priority);
}

UniquePtr<VulkanBarrier> VulkanDevice::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept
{
    return makeUnique<VulkanBarrier>(syncBefore, syncAfter);
}

MultiSamplingLevel VulkanDevice::maximumMultiSamplingLevel(Format format) const noexcept
{
    auto limits = m_impl->m_adapter.limits();
    VkSampleCountFlags sampleCounts = limits.framebufferColorSampleCounts;

    if (::hasDepth(format) && ::hasStencil(format))
        sampleCounts = limits.framebufferDepthSampleCounts & limits.framebufferStencilSampleCounts;
    else if (::hasDepth(format))
        sampleCounts = limits.framebufferDepthSampleCounts;
    else if (::hasStencil(format))
        sampleCounts = limits.framebufferStencilSampleCounts;

    if (sampleCounts & VK_SAMPLE_COUNT_64_BIT)
        return MultiSamplingLevel::x64;
    else if (sampleCounts & VK_SAMPLE_COUNT_32_BIT)
        return MultiSamplingLevel::x32;
    else if (sampleCounts & VK_SAMPLE_COUNT_16_BIT)
        return MultiSamplingLevel::x16;
    else if (sampleCounts & VK_SAMPLE_COUNT_8_BIT)
        return MultiSamplingLevel::x8;
    else if (sampleCounts & VK_SAMPLE_COUNT_4_BIT)
        return MultiSamplingLevel::x4;
    else if (sampleCounts & VK_SAMPLE_COUNT_2_BIT)
        return MultiSamplingLevel::x2;
    else
        return MultiSamplingLevel::x1;
}

double VulkanDevice::ticksPerMillisecond() const noexcept
{
    return 1000000.0 / static_cast<double>(this->adapter().limits().timestampPeriod);
}

void VulkanDevice::wait() const
{
    raiseIfFailed(::vkDeviceWaitIdle(this->handle()), "Unable to wait for the device.");
}

#if defined(LITEFX_BUILD_RAY_TRACING_SUPPORT)
void VulkanDevice::computeAccelerationStructureSizes(const VulkanBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize) const
{
    auto buildInfo = blas.buildInfo();
    auto descriptions = buildInfo | std::views::values | std::ranges::to<Array<VkAccelerationStructureGeometryKHR>>();
    auto sizes = buildInfo | std::views::keys | std::ranges::to<Array<UInt32>>();

    VkAccelerationStructureBuildSizesInfoKHR prebuildInfo = { 
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR 
    };

    VkAccelerationStructureBuildGeometryInfoKHR inputs = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = std::bit_cast<VkBuildAccelerationStructureFlagsKHR>(blas.flags()),
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = static_cast<UInt32>(descriptions.size()),
        .pGeometries = descriptions.data()
    };

    // Get the prebuild info and align the buffer sizes.
    const auto alignment = this->adapter().limits().minUniformBufferOffsetAlignment;
    ::vkGetAccelerationStructureBuildSizes(this->handle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &inputs, sizes.data(), &prebuildInfo);
    bufferSize = (prebuildInfo.accelerationStructureSize + alignment - 1) & ~(alignment - 1);
    scratchSize = (prebuildInfo.buildScratchSize + alignment - 1) & ~(alignment - 1);
}

void VulkanDevice::computeAccelerationStructureSizes(const VulkanTopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize) const
{
    auto instances = tlas.buildInfo();
    auto instanceCount = static_cast<UInt32>(instances.size());

    VkAccelerationStructureGeometryInstancesDataKHR instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
        .arrayOfPointers = false,
        .data = {
            .hostAddress = instances.data()
        }
    };

    VkAccelerationStructureGeometryKHR geometryInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry = {
            .instances = instanceInfo
        }
    };

    VkAccelerationStructureBuildSizesInfoKHR prebuildInfo = { 
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR 
    };

    VkAccelerationStructureBuildGeometryInfoKHR inputs = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = std::bit_cast<VkBuildAccelerationStructureFlagsKHR>(tlas.flags()),
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = 1u,
        .pGeometries = &geometryInfo
    };

    // Get the prebuild info and align the buffer sizes.
    const auto alignment = this->adapter().limits().minUniformBufferOffsetAlignment;
    ::vkGetAccelerationStructureBuildSizes(this->handle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &inputs, &instanceCount, &prebuildInfo);
    bufferSize = (prebuildInfo.accelerationStructureSize + alignment - 1) & ~(alignment - 1);
    scratchSize = (prebuildInfo.buildScratchSize + alignment - 1) & ~(alignment - 1);
}
#endif // defined(LITEFX_BUILD_RAY_TRACING_SUPPORT)