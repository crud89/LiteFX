#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

#include <bit>
#include <mutex>

#include "image.h"
#include "buffer.h"
#include "virtual_allocator.hpp"

using namespace LiteFX::Rendering::Backends;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasks{ nullptr };
PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirect{ nullptr };
PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCount{ nullptr };
PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizes{ nullptr };
PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure{ nullptr };
PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure{ nullptr };
PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructures{ nullptr };
PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructure{ nullptr };
PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresProperties{ nullptr };
PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelines{ nullptr };
PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandles{ nullptr };
PFN_vkCmdTraceRaysKHR vkCmdTraceRays{ nullptr };
PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress{ nullptr };
PFN_vkWriteResourceDescriptorsEXT vkWriteResourceDescriptors { nullptr };
PFN_vkWriteSamplerDescriptorsEXT vkWriteSamplerDescriptors { nullptr };
PFN_vkCmdBindResourceHeapEXT vkCmdBindResourceHeap { nullptr };
PFN_vkCmdBindSamplerHeapEXT vkCmdBindSamplerHeap { nullptr };
PFN_vkCmdPushDataEXT vkCmdPushData { nullptr };
extern PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName;
extern PFN_vkGetPhysicalDeviceDescriptorSizeEXT vkGetPhysicalDeviceDescriptorSize;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDevice::VulkanDeviceImpl {
public:
    friend class VulkanDevice;

private:
    class QueueFamily {
    private:
        Array<SharedPtr<VulkanQueue>> m_queues{};
        Array<std::mutex> m_queueMutexes{};
        Array<Float> m_queuePriorities{};
        UInt32 m_id, m_queueCount;
        QueueType m_type;

    public:
        QueueType type() const noexcept { return m_type; }
        UInt32 total() const noexcept { return m_queueCount; }
        UInt32 active() const noexcept { return static_cast<UInt32>(m_queues.size()); }
        UInt32 id() const noexcept { return m_id; }
        const Array<SharedPtr<VulkanQueue>>& queues() const noexcept { return m_queues; }
        std::mutex& mutex(UInt32 queueId) { return m_queueMutexes.at(queueId); }

    public:
        QueueFamily(UInt32 id, UInt32 queueCount, QueueType type) :
            m_id(id), m_queueCount(queueCount), m_type(type)
        {
            if (queueCount == 0) [[unlikely]]
                throw InvalidArgumentException("queueCount", "An empty queue family is invalid.");

            // Allocate a mutex shared by all queue instances that map to the same queue id on the family.
            m_queueMutexes = Array<std::mutex>(queueCount);

            // Initialize queue priorities. First queue always has the highest priority possible, as those become the default queues. After this we fill up the space evenly 
            // through all available priorities. Currently there are two priorities (`High` and `Normal`)
            auto chunkSize = (queueCount - 1) / 2;
            auto remainder = (queueCount - 1) % 2;

            auto high = std::views::repeat(static_cast<Float>(QueuePriority::High) / 100.0f) | std::views::take(chunkSize);
            auto low = std::views::repeat(static_cast<Float>(QueuePriority::Normal) / 100.0f) | std::views::take(chunkSize + remainder);

            m_queuePriorities.push_back(1.0f);

#ifdef __cpp_lib_containers_ranges
            m_queuePriorities.append_range(high);
            m_queuePriorities.append_range(low);
#else
            m_queuePriorities.insert(m_queuePriorities.end(), high.cbegin(), high.cend());
            m_queuePriorities.insert(m_queuePriorities.end(), low.cbegin(), low.cend());
#endif
        }
        QueueFamily(QueueFamily&& _other) noexcept = default;
        QueueFamily(const QueueFamily& _other) = delete;
        QueueFamily& operator=(QueueFamily&& _other) noexcept = default;
        QueueFamily& operator=(const QueueFamily & _other) = delete;
        ~QueueFamily() noexcept = default;

    public:
        SharedPtr<VulkanQueue> createQueue(const VulkanDevice& device, QueuePriority priority) {
            // First, list all queues with the requested priority.
            auto left = std::ranges::lower_bound(m_queuePriorities, static_cast<Float>(priority) / 100.0f, std::greater<float>{});
            auto right = std::ranges::upper_bound(m_queuePriorities, static_cast<Float>(priority) / 100.0f, std::greater<float>{});

            if (left == std::end(m_queuePriorities)) [[unlikely]]
            {
                QueuePriority nextPriority{ QueuePriority::Normal };

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
                std::views::transform([this](auto i) { return std::make_tuple(i, std::ranges::count_if(m_queues, [i](const auto& q) { return q->queueId() == static_cast<UInt32>(i); })); });
            auto [queueId, refCount] = *std::ranges::min_element(indices, {}, [](const auto& i) { return std::get<1>(i); });

            LITEFX_DEBUG(VULKAN_LOG, "Creating queue with id {0} of type {2} (referenced {1} times).", queueId, refCount, m_type);

            // Create a queue instance with the queue id.
            return m_queues.emplace_back(VulkanQueue::create(device, m_type, priority, m_id, static_cast<UInt32>(queueId)));
        }
    };

    DeviceState m_deviceState;

    Array<QueueFamily> m_families;
    SharedPtr<VulkanQueue> m_graphicsQueue{};
    SharedPtr<VulkanQueue> m_transferQueue{};
    SharedPtr<VulkanQueue> m_computeQueue{};

    UniquePtr<VulkanSwapChain> m_swapChain;
    Array<String> m_extensions;

    SharedPtr<const VulkanGraphicsAdapter> m_adapter;
    UniquePtr<VulkanSurface> m_surface;
    SharedPtr<VulkanGraphicsFactory> m_factory;

    VkPhysicalDeviceDescriptorHeapPropertiesEXT m_descriptorHeapProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT };
    SharedPtr<IVulkanBuffer> m_globalResourceHeap, m_globalSamplerHeap;
    UniquePtr<GlobalDescriptorHeaps> m_descriptorHeaps;
    VkDeviceSize m_resourceReservedSize{ 0 }, m_samplerReservedSize{ 0 };
    UInt32 m_globalResourceHeapSize{}, m_globalSamplerHeapSize{};
    UInt32 m_resourceDescriptorStride{}, m_samplerDescriptorStride{};

public:
    VulkanDeviceImpl(const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, const GraphicsDeviceFeatures& features, Span<String> extensions, UInt32 globalResourceHeapSize, UInt32 globalSamplerHeapSize) :
        m_adapter(adapter.shared_from_this()), m_surface(std::move(surface)), m_globalResourceHeapSize(globalResourceHeapSize), m_globalSamplerHeapSize(globalSamplerHeapSize)
    {
        if (m_surface == nullptr)
            throw ArgumentNotInitializedException("surface", "The surface must be initialized.");

        m_extensions.assign(std::begin(extensions), std::end(extensions));
        
        // Define mandatory extensions for provided features.
        this->defineMandatoryExtensions(features);

        // Load the queue families.
        this->loadQueueFamilies();
    }

private:
    void defineMandatoryExtensions(const GraphicsDeviceFeatures& features)
    {
        // NOTE: If an extension is not supported, update the graphics driver to the most recent one. You can lookup extension support for individual drivers here:
        // https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_KHR_present_wait (replace the extension name to adjust the filter).

        // Required to query image and buffer requirements.
        m_extensions.emplace_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

        // Required for improved descriptor management.
        m_extensions.emplace_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);

        if (features.DynamicDescriptors)
            m_extensions.emplace_back(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);

        // Improved compatibility between Vulkan and DirectX 12 backends
        m_extensions.emplace_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

        // Allow less strict image layout transitions.
        m_extensions.emplace_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
        
        // Required for mesh shading.
        if (features.MeshShaders)
            m_extensions.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

        if (features.RayTracing)
            m_extensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

        if (features.RayQueries)
            m_extensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

        if (features.RayTracing || features.RayQueries)
        {
            m_extensions.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            m_extensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            m_extensions.emplace_back(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
        }

        if (features.ConservativeRasterization)
            m_extensions.emplace_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);

#if defined(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN) && defined(LITEFX_BUILD_DIRECTX_12_BACKEND)
        // Interop swap chain requires external memory access.
        m_extensions.emplace_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_extensions.emplace_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        m_extensions.emplace_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

        // Required to synchronize Vulkan command execution with D3D presentation.
        m_extensions.emplace_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
        //m_extensions.emplace_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else // defined(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN) && defined(LITEFX_BUILD_DIRECTX_12_BACKEND)
        m_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif // defined(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN) && defined(LITEFX_BUILD_DIRECTX_12_BACKEND)

        auto availableExtensions = m_adapter->getAvailableDeviceExtensions();

        // Required for native budget info by VMA - if not availabe VMA emulates this behavior.
        if (auto match = std::ranges::find_if(availableExtensions, [](const String& extension) { return extension == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME; }); match != availableExtensions.end())
            m_extensions.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        // Query maintenance 5 extension as well as win32 external memory, so that we can let VMA support it.
        if (auto match = std::ranges::find_if(availableExtensions, [](const String& extension) { return extension == VK_KHR_MAINTENANCE_5_EXTENSION_NAME; }); match != availableExtensions.end())
            m_extensions.emplace_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    }

public:
    void loadQueueFamilies()
    {
        // Find an available command queues.
        uint32_t queueFamilies = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter->handle(), &queueFamilies, nullptr);

        Array<VkQueueFamilyProperties> familyProperties(queueFamilies);
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_adapter->handle(), &queueFamilies, familyProperties.data());

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

    template <typename T>
    static T* findExtension(VkStructureType structureType, void* deviceExtensionChain) {
        // NOTE: Only use this at the beginning of an extension chain!
        auto next = reinterpret_cast<VkBaseOutStructure*>(deviceExtensionChain); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

        while (next != nullptr)
        {
            if (next->sType == structureType)
                return reinterpret_cast<T*>(next); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

            next = reinterpret_cast<VkBaseOutStructure*>(next->pNext); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        }

        return nullptr;
    }

    template <typename T>
    static T* defineExtension(void*& chain, VkStructureType type, T& storage, bool enabled = true) {
        if (!enabled)
            return nullptr;

        if (auto existing = findExtension<T>(type, chain); existing != nullptr)
            return existing;

        storage = { .sType = type, .pNext = chain };
        chain = &storage;

        return &storage;
    }

    VkDevice initialize(const GraphicsDeviceFeatures& features, void* deviceExtensionObjects)
    {
        if (!m_adapter->validateDeviceExtensions(m_extensions))
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
                return VkDeviceQueueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = family.id(),
                    .queueCount = family.total(),
                    .pQueuePriorities = priorities.data()
                };
            }) | std::ranges::to<Array<VkDeviceQueueCreateInfo>>();

        // Enable requested features. Look up if they are already provided in the extension chain and overwrite them if necessary. We only enable features for compatibility 
        // reasons. Only exceptions are GraphicsDeviceFeatures, which are provided by different means.
        // Start enabling ray-tracing features.
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};

        if (auto pRayQueryFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, rayQueryFeatures, features.RayQueries))
            pRayQueryFeatures->rayQuery = true;

        VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures{};

        if (auto pRayTracingMaintenanceFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR, rayTracingMaintenanceFeatures, features.RayTracing || features.RayQueries)) {
            pRayTracingMaintenanceFeatures->rayTracingMaintenance1 = true;
            pRayTracingMaintenanceFeatures->rayTracingPipelineTraceRaysIndirect2 = true;
        }

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

        if (auto pAccelerationStructureFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, accelerationStructureFeatures, features.RayTracing || features.RayQueries)) {
            pAccelerationStructureFeatures->accelerationStructure = true;
        }

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};

        if (auto pRayTracingPipelineFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, rayTracingPipelineFeatures, features.RayTracing))
            pRayTracingPipelineFeatures->rayTracingPipeline = true;

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};

        if (auto pMeshShaderFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, meshShaderFeatures, features.MeshShaders)) {
            pMeshShaderFeatures->taskShader = true;
            pMeshShaderFeatures->meshShader = true;
        }

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features{};
        const bool maintenance5 = std::ranges::find_if(m_extensions, [](auto& ext) { return ext == VK_KHR_MAINTENANCE_5_EXTENSION_NAME; }) != m_extensions.end();

        if (auto pMaintenance5Features = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR, maintenance5Features, maintenance5))
            pMaintenance5Features->maintenance5 = true;

        VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeatures{};
        defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR, unifiedImageLayoutsFeatures)->unifiedImageLayouts = true;

        VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeatures{};
        defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT, descriptorHeapFeatures)->descriptorHeap = true;

        VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPointersFeatures{};

        if (auto pUntypedPointersFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR, untypedPointersFeatures, features.DynamicDescriptors))
            pUntypedPointersFeatures->shaderUntypedPointers = true;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        auto pDeviceFeatures = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, deviceFeatures);

        pDeviceFeatures->features.fullDrawIndexUint32 = true;
        pDeviceFeatures->features.independentBlend = true;
#ifndef LITEFX_BUILD_TESTS
        pDeviceFeatures->features.geometryShader = true;
        pDeviceFeatures->features.tessellationShader = true;
#endif // LITEFX_BUILD_TESTS
        pDeviceFeatures->features.dualSrcBlend = true;
        pDeviceFeatures->features.logicOp = true;
        pDeviceFeatures->features.drawIndirectFirstInstance = features.DrawIndirect;
        pDeviceFeatures->features.depthClamp = true;
        pDeviceFeatures->features.depthBounds = features.DepthBoundsTest;
        pDeviceFeatures->features.alphaToOne = true;
        pDeviceFeatures->features.multiViewport = true;
        pDeviceFeatures->features.samplerAnisotropy = true;
        pDeviceFeatures->features.shaderFloat64 = true;
        pDeviceFeatures->features.shaderInt64 = true;
        pDeviceFeatures->features.shaderInt16 = true;

        VkPhysicalDeviceVulkan13Features deviceFeatures13{};
        auto pDeviceFeatures13 = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, deviceFeatures13);

        pDeviceFeatures13->synchronization2 = true;
        pDeviceFeatures13->dynamicRendering = true;
        pDeviceFeatures13->maintenance4 = true;

        VkPhysicalDeviceVulkan12Features deviceFeatures12{};
        auto pDeviceFeatures12 = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, deviceFeatures12);

        pDeviceFeatures12->drawIndirectCount = features.DrawIndirect;
        pDeviceFeatures12->descriptorIndexing = true;
        pDeviceFeatures12->shaderUniformTexelBufferArrayDynamicIndexing = true;
        pDeviceFeatures12->shaderStorageTexelBufferArrayDynamicIndexing = true;
        pDeviceFeatures12->shaderUniformBufferArrayNonUniformIndexing = true;
        pDeviceFeatures12->shaderSampledImageArrayNonUniformIndexing = true;
        pDeviceFeatures12->shaderStorageBufferArrayNonUniformIndexing = true;
        pDeviceFeatures12->shaderStorageImageArrayNonUniformIndexing = true;
        pDeviceFeatures12->shaderUniformTexelBufferArrayNonUniformIndexing = true;
        pDeviceFeatures12->shaderStorageTexelBufferArrayNonUniformIndexing = true;
        pDeviceFeatures12->runtimeDescriptorArray = true;
        pDeviceFeatures12->separateDepthStencilLayouts = true;
        pDeviceFeatures12->hostQueryReset = true;
        pDeviceFeatures12->timelineSemaphore = true;
        pDeviceFeatures12->bufferDeviceAddress = true;
        pDeviceFeatures12->shaderOutputViewportIndex = features.ViewInstancing;
        pDeviceFeatures12->shaderOutputLayer = features.ViewInstancing;

        VkPhysicalDeviceVulkan11Features deviceFeatures11{};
        auto pDeviceFeatures11 = defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, deviceFeatures11);

        pDeviceFeatures11->shaderDrawParameters = true;

        if (features.ViewInstancing) {
            pDeviceFeatures11->multiview = true;
#ifndef LITEFX_BUILD_TESTS
            pDeviceFeatures11->multiviewGeometryShader = true;
            pDeviceFeatures11->multiviewTessellationShader = true;
#endif // LITEFX_BUILD_TESTS
        }

        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures{};
        defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT, depthClipEnableFeatures)->depthClipEnable = true;

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
        defineExtension(deviceExtensionObjects, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT, extendedDynamicStateFeatures)->extendedDynamicState = true;

        // Define the device itself.
        VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = deviceExtensionObjects,
            .queueCreateInfoCount = static_cast<UInt32>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<UInt32>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()
        };

        // Create the device.
        VkDevice device{};
        raiseIfFailed(::vkCreateDevice(m_adapter->handle(), &createInfo, nullptr, &device), "Unable to create Vulkan device.");

        // Load extension methods.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        if (features.MeshShaders)
        {
            if (vkCmdDrawMeshTasks == nullptr)
                vkCmdDrawMeshTasks = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(::vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT"));

            if (vkCmdDrawMeshTasksIndirect)
                vkCmdDrawMeshTasksIndirect = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(::vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectEXT"));

            if (vkCmdDrawMeshTasksIndirectCount)
                vkCmdDrawMeshTasksIndirectCount = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(::vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountEXT"));
        }

        if (features.RayTracing)
        {
            if (vkGetAccelerationStructureBuildSizes == nullptr)
                vkGetAccelerationStructureBuildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(::vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));

            if (vkCreateAccelerationStructure == nullptr)
                vkCreateAccelerationStructure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));

            if (vkDestroyAccelerationStructure == nullptr)
                vkDestroyAccelerationStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));

            if (vkCmdBuildAccelerationStructures == nullptr)
                vkCmdBuildAccelerationStructures = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(::vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));

            if (vkCmdCopyAccelerationStructure == nullptr)
                vkCmdCopyAccelerationStructure = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR"));

            if (vkCmdWriteAccelerationStructuresProperties == nullptr)
                vkCmdWriteAccelerationStructuresProperties = reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(::vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));

            if (vkCreateRayTracingPipelines == nullptr)
                vkCreateRayTracingPipelines = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(::vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));

            if (vkGetRayTracingShaderGroupHandles == nullptr)
                vkGetRayTracingShaderGroupHandles = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(::vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));

            if (vkCmdTraceRays == nullptr)
                vkCmdTraceRays = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(::vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
        }

        if (features.RayQueries)
        {
            if (vkGetAccelerationStructureBuildSizes == nullptr)
                vkGetAccelerationStructureBuildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(::vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));

            if (vkCreateAccelerationStructure == nullptr)
                vkCreateAccelerationStructure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));

            if (vkDestroyAccelerationStructure == nullptr)
                vkDestroyAccelerationStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));

            if (vkCmdBuildAccelerationStructures == nullptr)
                vkCmdBuildAccelerationStructures = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(::vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));

            if (vkCmdCopyAccelerationStructure == nullptr)
                vkCmdCopyAccelerationStructure = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(::vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR"));

            if (vkCmdWriteAccelerationStructuresProperties == nullptr)
                vkCmdWriteAccelerationStructuresProperties = reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(::vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
        }

        // Load required extension functions.
        if (vkGetAccelerationStructureDeviceAddress == nullptr)
            vkGetAccelerationStructureDeviceAddress = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(::vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));

        if (vkWriteResourceDescriptors == nullptr)
            vkWriteResourceDescriptors = reinterpret_cast<PFN_vkWriteResourceDescriptorsEXT>(::vkGetDeviceProcAddr(device, "vkWriteResourceDescriptorsEXT"));

        if (vkWriteSamplerDescriptors == nullptr)
            vkWriteSamplerDescriptors = reinterpret_cast<PFN_vkWriteSamplerDescriptorsEXT>(::vkGetDeviceProcAddr(device, "vkWriteSamplerDescriptorsEXT"));

        if (vkCmdBindResourceHeap == nullptr)
            vkCmdBindResourceHeap = reinterpret_cast<PFN_vkCmdBindResourceHeapEXT>(::vkGetDeviceProcAddr(device, "vkCmdBindResourceHeapEXT"));

        if (vkCmdBindSamplerHeap == nullptr)
            vkCmdBindSamplerHeap = reinterpret_cast<PFN_vkCmdBindSamplerHeapEXT>(::vkGetDeviceProcAddr(device, "vkCmdBindSamplerHeapEXT"));

        if (vkCmdPushData == nullptr)
            vkCmdPushData = reinterpret_cast<PFN_vkCmdPushDataEXT>(::vkGetDeviceProcAddr(device, "vkCmdPushDataEXT"));
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

        // Return the device instance.
        return device;
    }

    void initializeDefaultQueues(const VulkanDevice& device)
    {
        // Initialize default queues.
        m_graphicsQueue = this->createQueue(device, QueueType::Graphics, QueuePriority::Realtime, std::as_const(*m_surface).handle());
        m_transferQueue = this->createQueue(device, QueueType::Transfer, QueuePriority::Realtime);
        m_computeQueue = this->createQueue(device, QueueType::Compute, QueuePriority::Realtime);

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

    inline void initializeResourceHeaps(const GraphicsDeviceFeatures& features)
    {
        // Devices may only support the full requested descriptor heap size with narrower descriptor sizes. We log a warning if the requested heap size is not available.
        auto clampDescriptorSize = [](UInt32 requested, VkDeviceSize maxHeapSize, VkDeviceSize reservedSize, UInt32 stride, StringView heap) static -> UInt32 {
            const auto available = maxHeapSize > reservedSize ? (maxHeapSize - reservedSize) / stride : 0u;

            if (available == 0u) [[unlikely]]
                throw RuntimeException("The device's maximum {0} heap size of {1} bytes does not leave room for any descriptors besides the reserved range.", heap, maxHeapSize);

            if (static_cast<VkDeviceSize>(requested) <= available)
                return requested;

            LITEFX_WARNING(VULKAN_LOG, "The requested {0} heap capacity of {1} descriptors exceeds the device limit and has been reduced to {2} descriptors.", heap, requested, available);

            return static_cast<UInt32>(available);
        };

        // Obtain the descriptor heap properties.
        VkPhysicalDeviceProperties2 properties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &m_descriptorHeapProperties };
        ::vkGetPhysicalDeviceProperties2(m_adapter->handle(), &properties);
        
        // Compute the descriptor sizes.
        auto samplerDescriptorSize = align(m_descriptorHeapProperties.samplerDescriptorSize, m_descriptorHeapProperties.samplerDescriptorAlignment);
        auto resourceDescriptorSize = std::max(m_descriptorHeapProperties.bufferDescriptorSize, m_descriptorHeapProperties.imageDescriptorSize);

        // Acceleration structure descriptors are not part of the heap properties and must be queried and validated separately.
        if (features.RayTracing || features.RayQueries) 
        {
            if (vkGetPhysicalDeviceDescriptorSize == nullptr) [[unlikely]]
                throw RuntimeException("Unable to validate acceleration structure descriptor sizes, as \"vkGetPhysicalDeviceDescriptorSizeEXT\" is not available.");

            auto descriptorSize = vkGetPhysicalDeviceDescriptorSize(m_adapter->handle(), VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);

            // The default behavior of the compiler (without explicit strides) emits `OpConstantSizeOfEXT`, which resolves to `max(bufferDescriptorSize, imageDescriptorSize)`
            // during pipeline creation, fetching buffer and image descriptor sizes from the device. If acceleration structure descriptors exceed this limit, this breaks. In 
            // this case, the strides must be manually extended. However, this is very unlikely since the AS descriptors are essentially addresses in practice.
            if (descriptorSize > resourceDescriptorSize) [[unlikely]] {
                LITEFX_WARNING(VULKAN_LOG, "The acceleration structure descriptor size {0} exceeds the resource descriptor size {1}. All shaders must be compiled with an explicit stride for resource (image/buffers) descriptor stride of {0} bytes!", descriptorSize, resourceDescriptorSize);
                resourceDescriptorSize = descriptorSize;
            }
        }

        // Align the resource descriptor size to both, image and buffer descriptor alignments.
        if (auto aligned = align(resourceDescriptorSize, m_descriptorHeapProperties.bufferDescriptorAlignment); aligned != resourceDescriptorSize) [[unlikely]] 
        {
            LITEFX_WARNING(VULKAN_LOG, "The resource descriptor size {0} is not aligned to the buffer descriptor alignment {2} and has been raised to {1} bytes. Shaders using native descriptor heap access must be compiled with an explicit stride of {1} bytes.", resourceDescriptorSize, aligned, m_descriptorHeapProperties.bufferDescriptorAlignment);
            resourceDescriptorSize = aligned;
        }

        if (auto aligned = align(resourceDescriptorSize, m_descriptorHeapProperties.imageDescriptorAlignment); aligned != resourceDescriptorSize) [[unlikely]]
        {
            LITEFX_WARNING(VULKAN_LOG, "The resource descriptor size {0} is not aligned to the image descriptor alignment {2} and has been raised to {1} bytes. Shaders using native descriptor heap access must be compiled with an explicit stride of {1} bytes.", resourceDescriptorSize, aligned, m_descriptorHeapProperties.imageDescriptorAlignment);
            resourceDescriptorSize = aligned;
        }

        // Get the reserved heap sizes.
        m_resourceReservedSize = align(m_descriptorHeapProperties.minResourceHeapReservedRange, resourceDescriptorSize);
        m_samplerReservedSize  = align(m_descriptorHeapProperties.minSamplerHeapReservedRangeWithEmbedded, samplerDescriptorSize);

        // Compute the descriptor heap sizes.
        auto resourceDescriptors = clampDescriptorSize(m_globalResourceHeapSize, m_descriptorHeapProperties.maxResourceHeapSize, m_resourceReservedSize, static_cast<UInt32>(resourceDescriptorSize), "resource");
        auto samplerDescriptors  = clampDescriptorSize(m_globalSamplerHeapSize,  m_descriptorHeapProperties.maxSamplerHeapSize,  m_samplerReservedSize,  static_cast<UInt32>(samplerDescriptorSize),  "sampler" );
        auto resourceHeapSize = static_cast<VkDeviceSize>(resourceDescriptors) * resourceDescriptorSize + m_resourceReservedSize;
        auto samplerHeapSize  = static_cast<VkDeviceSize>(samplerDescriptors)  * samplerDescriptorSize  + m_samplerReservedSize;

        // Allocate the global descriptor heaps.
        m_descriptorHeaps = GlobalDescriptorHeaps::create<VulkanBackend>(resourceDescriptors, samplerDescriptors);
        m_globalResourceHeap = m_factory->createDescriptorHeap("Global Resource Heap", static_cast<size_t>(resourceHeapSize));
        m_globalSamplerHeap  = m_factory->createDescriptorHeap("Global Sampler Heap",  static_cast<size_t>(samplerHeapSize));

        // Store the descriptor sizes.
        m_resourceDescriptorStride = static_cast<UInt32>(resourceDescriptorSize);
        m_samplerDescriptorStride  = static_cast<UInt32>(samplerDescriptorSize);

        // Descriptor heaps must be aligned.
        assert(m_globalResourceHeap->virtualAddress() % m_descriptorHeapProperties.resourceHeapAlignment == 0);
        assert(m_globalSamplerHeap->virtualAddress()  % m_descriptorHeapProperties.samplerHeapAlignment  == 0);
    }

public:
    SharedPtr<VulkanQueue> createQueue(const VulkanDevice& device, QueueType type, QueuePriority priority, const VkSurfaceKHR& surface = VK_NULL_HANDLE)
    {
        // Find the queue that is most specialized for the provided queue type. Since the queues are ordered based on their type popcount (most specialized queues come first, as they have 
        // lower type flags set), we can simply pick the first one we find, that matches all the flags.
        auto match = std::ranges::find_if(m_families, [&](const auto& family) {
            VkBool32 result = LITEFX_FLAG_IS_SET(family.type(), type);

            if (surface != VK_NULL_HANDLE) [[unlikely]]
            {
                // Check if presenting to the surface is supported.
                VkBool32 canPresent = VK_FALSE;
                ::vkGetPhysicalDeviceSurfaceSupportKHR(m_adapter->handle(), family.id(), surface, &canPresent);

                result &= canPresent;
            }

            return result != VK_FALSE;
        });

        return match == m_families.end() ? nullptr : match->createQueue(device, priority);
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanDevice::VulkanDevice(const VulkanBackend& /*backend*/, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features, Span<String> extensions, UInt32 globalResourceHeapSize, UInt32 globalSamplerHeapSize) :
    Resource<VkDevice>(nullptr), m_impl(adapter, std::move(surface), features, extensions, globalResourceHeapSize, globalSamplerHeapSize)
{
    LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {0}, Adapter: {1}, Extensions: {2} }}...", static_cast<void*>(m_impl->m_surface.get()), adapter.deviceId(), Join(this->enabledExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Vendor: {0:#0x}", adapter.vendorId());
    LITEFX_DEBUG(VULKAN_LOG, "Driver Version: {0:#0x}", adapter.driverVersion());
    LITEFX_DEBUG(VULKAN_LOG, "API Version: {0:#0x}", adapter.apiVersion());
    LITEFX_DEBUG(VULKAN_LOG, "Dedicated Memory: {0} Bytes", adapter.dedicatedMemory());
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Resource Descriptor Heap Size: {0}", globalResourceHeapSize);
    LITEFX_DEBUG(VULKAN_LOG, "Sampler Descriptor Heap Size: {0}", globalSamplerHeapSize);
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");
    LITEFX_DEBUG(VULKAN_LOG, "Available extensions: {0}", Join(adapter.getAvailableDeviceExtensions(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "Validation layers: {0}", Join(adapter.deviceValidationLayers(), ", "));
    LITEFX_DEBUG(VULKAN_LOG, "--------------------------------------------------------------------------");

    if (extensions.size() > 0)
        LITEFX_INFO(VULKAN_LOG, "Enabled validation layers: {0}", Join(extensions, ", "));
}

VulkanDevice::~VulkanDevice() noexcept = default;

SharedPtr<VulkanDevice> VulkanDevice::initialize(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync, GraphicsDeviceFeatures features, void* deviceExtensionObjects)
{
    this->handle() = m_impl->initialize(features, deviceExtensionObjects);

    // NOTE: The order of initialization here is important.
    m_impl->initializeDefaultQueues(*this);
    m_impl->m_swapChain = UniquePtr<VulkanSwapChain>(new VulkanSwapChain(*this, format, renderArea, backBuffers, enableVsync));
    m_impl->m_factory = VulkanGraphicsFactory::create(*this);
    m_impl->initializeResourceHeaps(features);

    return this->shared_from_this();
}

void VulkanDevice::release() noexcept 
{
    m_impl->m_deviceState.clear();
    m_impl->m_swapChain.reset();
    m_impl->m_transferQueue.reset();
    m_impl->m_computeQueue.reset();
    m_impl->m_graphicsQueue.reset();
    m_impl->m_families.clear();
    m_impl->m_surface.reset();
    m_impl->m_globalResourceHeap.reset();
    m_impl->m_globalSamplerHeap.reset();
    m_impl->m_factory.reset();

    // Destroy the device.
    ::vkDestroyDevice(this->handle(), nullptr);
}

Span<const String> VulkanDevice::enabledExtensions() const noexcept
{
    return m_impl->m_extensions;
}

void VulkanDevice::setDebugName([[maybe_unused]] VkObjectType type, [[maybe_unused]] UInt64 handle, [[maybe_unused]] StringView name) const
{
#ifndef NDEBUG
    if (vkSetDebugUtilsObjectName != nullptr)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = name.data() // NOLINT(bugprone-suspicious-stringview-data-usage)
        };

        if (vkSetDebugUtilsObjectName(this->handle(), &nameInfo) != VK_SUCCESS)
            LITEFX_WARNING(VULKAN_LOG, "Unable to set object name for object handle {0}.", static_cast<const void*>(&handle));
    }
#endif
}

std::mutex& VulkanDevice::queueMutex(UInt32 queueFamily, UInt32 queueId) const
{
    return m_impl->m_families.at(queueFamily).mutex(queueId);
}

Enumerable<UInt32> VulkanDevice::queueFamilyIndices(QueueType type) const
{
    return m_impl->m_families |
        std::views::filter([type](const auto& family) { return type == QueueType::None || LITEFX_FLAG_IS_SET(family.type(), type); }) |
        std::views::transform([](const auto& family) { return family.id(); }) |
        std::ranges::to<Enumerable<UInt32>>();
}

UInt32 VulkanDevice::descriptorSize(DescriptorType type) const
{
    switch (type)
    {
    case DescriptorType::AccelerationStructure:
    case DescriptorType::Buffer:
    case DescriptorType::ConstantBuffer:
    case DescriptorType::InputAttachment:
    case DescriptorType::RWBuffer:
    case DescriptorType::RWTexture:
    case DescriptorType::ByteAddressBuffer:
    case DescriptorType::RWByteAddressBuffer:
    case DescriptorType::StructuredBuffer:
    case DescriptorType::RWStructuredBuffer:
    case DescriptorType::Texture:
        return this->descriptorStride(DescriptorHeapType::Resource);
    case DescriptorType::Sampler:
        return this->descriptorStride(DescriptorHeapType::Sampler);
    default:
        throw InvalidArgumentException("type", "The provided descriptor type cannot be mapped to a descriptor heap.");
    }
}

UInt32 VulkanDevice::descriptorStride(DescriptorHeapType heapType) const
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return m_impl->m_resourceDescriptorStride;
    case DescriptorHeapType::Sampler:
        return m_impl->m_samplerDescriptorStride;
    default:
        throw InvalidArgumentException("heapType", "The descriptor heap type must be one of the following: {{ `Resource`, `Sampler` }}, but it was: `{0}`.", heapType);
    }
}

UInt32 VulkanDevice::maxPushDataSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_descriptorHeapProperties.maxPushDataSize);
}

const GlobalDescriptorHeaps& VulkanDevice::descriptorHeaps() const noexcept 
{
    return *m_impl->m_descriptorHeaps;
}

void VulkanDevice::copyDescriptors(const VulkanDescriptorSet& descriptorSet, DescriptorHeapType heapType, UInt32 targetIndex, UInt32 sourceIndex, UInt32 descriptors) const
{
    size_t stride = this->descriptorStride(heapType);

    this->writeGlobalDescriptorHeap(heapType, targetIndex, descriptorSet.descriptorBuffer(heapType).subspan(sourceIndex * stride, descriptors * stride));
}

void VulkanDevice::bindDescriptors(const VulkanCommandBuffer& commandBuffer, const VulkanPipelineState& pipeline, const VulkanDescriptorSet& descriptorSet, DescriptorHeapType heapType, UInt32 baseIndex) const
{
    auto pushOffset = pipeline.layout()->pushDataOffset(descriptorSet.layout().space(), heapType);

    if (!pushOffset.has_value()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "Unable to bind descriptor set at space {0}, as it is not part of the pipeline layout.", descriptorSet.layout().space());
        return;
    }
    else
    {
        const VkPushDataInfoEXT pushData{ .sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT, .offset = *pushOffset, .data = { .address = &baseIndex, .size = sizeof(UInt32) } };
        vkCmdPushData(commandBuffer.handle(), &pushData);
    }
}

DescriptorHeapAllocation VulkanDevice::allocateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, DescriptorHeapType heapType) const
{
    return m_impl->m_descriptorHeaps->allocate(heapType, static_cast<UInt32>(descriptorSet.descriptorBuffer(heapType).size() / this->descriptorStride(heapType)));
}

UniquePtr<VulkanDescriptorRange> VulkanDevice::allocateGlobalDescriptors(DescriptorHeapType heapType, UInt32 descriptors) const
{
    return UniquePtr<VulkanDescriptorRange>(new VulkanDescriptorRange(*this, m_impl->m_descriptorHeaps->allocate(heapType, descriptors)));
}

void VulkanDevice::writeGlobalDescriptorHeap(DescriptorHeapType heapType, UInt32 targetIndex, Span<const Byte> descriptors) const
{
    const size_t stride = this->descriptorStride(heapType);
    auto& heap = heapType == DescriptorHeapType::Sampler ? m_impl->m_globalSamplerHeap : m_impl->m_globalResourceHeap;

    heap->write(descriptors.data(), descriptors.size(), static_cast<size_t>(targetIndex) * stride);
}

void VulkanDevice::releaseGlobalDescriptors(DescriptorHeapAllocation&& allocation) const
{
    m_impl->m_descriptorHeaps->release(std::move(allocation)); // NOLINT(performance-move-const-arg)
}

void VulkanDevice::bindGlobalDescriptorHeaps(const VulkanCommandBuffer& commandBuffer) const
{
    VkBindHeapInfoEXT resourceHeap {
        .sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT,
        .heapRange = { .address = m_impl->m_globalResourceHeap->virtualAddress(), .size = m_impl->m_globalResourceHeap->size() },
        .reservedRangeOffset = static_cast<VkDeviceSize>(m_impl->m_descriptorHeaps->capacity(DescriptorHeapType::Resource)) * this->descriptorStride(DescriptorHeapType::Resource),
        .reservedRangeSize = m_impl->m_resourceReservedSize 
    };

    VkBindHeapInfoEXT samplerHeap {
        .sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT,
        .heapRange = { .address = m_impl->m_globalSamplerHeap->virtualAddress(), .size = m_impl->m_globalSamplerHeap->size() },
        .reservedRangeOffset = static_cast<VkDeviceSize>(m_impl->m_descriptorHeaps->capacity(DescriptorHeapType::Sampler)) * this->descriptorStride(DescriptorHeapType::Sampler),
        .reservedRangeSize = m_impl->m_samplerReservedSize 
    };

    vkCmdBindResourceHeap(commandBuffer.handle(), &resourceHeap);
    vkCmdBindSamplerHeap(commandBuffer.handle(), &samplerHeap);
}

VulkanSwapChain& VulkanDevice::swapChain() noexcept
{
    return *m_impl->m_swapChain;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
VulkanRenderPassBuilder VulkanDevice::buildRenderPass(UInt32 commandBuffers) const
{
    return VulkanRenderPassBuilder(*this, commandBuffers);
}

VulkanRenderPassBuilder VulkanDevice::buildRenderPass(const String& name, UInt32 commandBuffers) const
{
    return VulkanRenderPassBuilder(*this, commandBuffers, name);
}

VulkanRenderPipelineBuilder VulkanDevice::buildRenderPipeline(const VulkanRenderPass& renderPass, const String& name) const
{
    return VulkanRenderPipelineBuilder(renderPass, name);
}

VulkanComputePipelineBuilder VulkanDevice::buildComputePipeline(const String& name) const
{
    return VulkanComputePipelineBuilder(*this, name);
}

VulkanRayTracingPipelineBuilder VulkanDevice::buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const
{
    return this->buildRayTracingPipeline("", std::move(shaderRecords));
}

VulkanRayTracingPipelineBuilder VulkanDevice::buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const
{
    return VulkanRayTracingPipelineBuilder(*this, std::move(shaderRecords), name);
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
    return *m_impl->m_adapter;
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

SharedPtr<const VulkanQueue> VulkanDevice::createQueue(QueueType type, QueuePriority priority)
{
    return m_impl->createQueue(*this, type, priority);
}

UniquePtr<VulkanBarrier> VulkanDevice::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const
{
    return makeUnique<VulkanBarrier>(syncBefore, syncAfter);
}

SharedPtr<VulkanFrameBuffer> VulkanDevice::makeFrameBuffer(StringView name, const Size2d& renderArea) const
{
    return VulkanFrameBuffer::create(*this, renderArea, name);
}

SharedPtr<VulkanFrameBuffer> VulkanDevice::makeFrameBuffer(StringView name, const Size2d& renderArea, VulkanFrameBuffer::allocation_callback_type allocationCallback) const
{
    return VulkanFrameBuffer::create(*this, renderArea, allocationCallback, name);
}

MultiSamplingLevel VulkanDevice::maximumMultiSamplingLevel(Format format) const
{
    auto limits = m_impl->m_adapter->limits();
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
    constexpr double NANOSECONDS_PER_SECOND = 1000000.0;

    return NANOSECONDS_PER_SECOND / static_cast<double>(this->adapter().limits().timestampPeriod);
}

void VulkanDevice::wait() const
{
    // Lock all queues in order so that no more work can be is submitted by any thread, before the wait finishes.
    Array<std::unique_lock<std::mutex>> locks{};

    for (auto& family : m_impl->m_families)
        for (UInt32 queueId{ 0u }; queueId < family.total(); ++queueId)
            locks.emplace_back(family.mutex(queueId));

    // Wait for the device to finish all work.
    raiseIfFailed(::vkDeviceWaitIdle(this->handle()), "Unable to wait for the device.");
}

void VulkanDevice::computeAccelerationStructureSizes(const VulkanBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const
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

    // Get the pre-build info and align the buffer sizes.
    const auto alignment = this->adapter().limits().minUniformBufferOffsetAlignment;
    ::vkGetAccelerationStructureBuildSizes(this->handle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &inputs, sizes.data(), &prebuildInfo);
    bufferSize = (prebuildInfo.accelerationStructureSize + alignment - 1) & ~(alignment - 1);

    if (forUpdate)
        scratchSize = (prebuildInfo.updateScratchSize + alignment - 1) & ~(alignment - 1);
    else
        scratchSize = (prebuildInfo.buildScratchSize + alignment - 1) & ~(alignment - 1);
}

void VulkanDevice::computeAccelerationStructureSizes(const VulkanTopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const
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

    // Get the pre-build info and align the buffer sizes.
    const auto alignment = this->adapter().limits().minUniformBufferOffsetAlignment;
    ::vkGetAccelerationStructureBuildSizes(this->handle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &inputs, &instanceCount, &prebuildInfo);
    bufferSize = (prebuildInfo.accelerationStructureSize + alignment - 1) & ~(alignment - 1);

    if (forUpdate)
        scratchSize = (prebuildInfo.updateScratchSize + alignment - 1) & ~(alignment - 1);
    else
        scratchSize = (prebuildInfo.buildScratchSize + alignment - 1) & ~(alignment - 1);
}