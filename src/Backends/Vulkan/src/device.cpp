#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include <bit>

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
PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSize{ nullptr };
PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffset{ nullptr };
PFN_vkGetDescriptorEXT vkGetDescriptor{ nullptr };
PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffers{ nullptr };
PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsets{ nullptr };
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
        Array<Float> m_queuePriorities{};
        UInt32 m_id, m_queueCount;
        QueueType m_type;

    public:
        QueueType type() const noexcept { return m_type; }
        UInt32 total() const noexcept { return m_queueCount; }
        UInt32 active() const noexcept { return static_cast<UInt32>(m_queues.size()); }
        UInt32 id() const noexcept { return m_id; }
        const Array<SharedPtr<VulkanQueue>>& queues() const noexcept { return m_queues; }

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

#ifndef NDEBUG
    PFN_vkDebugMarkerSetObjectNameEXT debugMarkerSetObjectName = nullptr;
#endif

    VkPhysicalDeviceDescriptorBufferPropertiesEXT m_descriptorBufferProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT };
    SharedPtr<IVulkanBuffer> m_globalDescriptorHeap;
    VirtualAllocator m_globalDescriptorHeapAllocator;
    mutable std::mutex m_bufferBindMutex;

public:
    VulkanDeviceImpl(const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, const GraphicsDeviceFeatures& features, Span<String> extensions, size_t globalDescriptorHeapSize) :
        m_adapter(adapter.shared_from_this()), m_surface(std::move(surface)),
        m_globalDescriptorHeapAllocator(VirtualAllocator::create<VulkanBackend>(globalDescriptorHeapSize))
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
        m_extensions.emplace_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

        // Improved compatibility between Vulkan and DirectX 12 backends
        m_extensions.emplace_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
        
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

        if (features.DynamicDescriptors)
            m_extensions.emplace_back(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);

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

#ifndef NDEBUG
        // Required to set debug names.
        if (auto match = std::ranges::find_if(availableExtensions, [](const String& extension) { return extension == VK_EXT_DEBUG_MARKER_EXTENSION_NAME; }); match != availableExtensions.end())
            m_extensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif

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

    VkDevice initialize(const GraphicsDeviceFeatures& features)
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

        // Enable requested features.
        // NOTE: We keep track of the last feature set with this pointer, as it is against the standard to include features in the pNext chain without requiring the extension.
        void* lastFeature = nullptr;

        // Enable ray-tracing features.
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
            .rayQuery = features.RayQueries
        };

        if (features.RayQueries)
            lastFeature = &rayQueryFeatures;

        VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR,
            .pNext = lastFeature,
            .rayTracingMaintenance1 = features.RayTracing || features.RayQueries,
            .rayTracingPipelineTraceRaysIndirect2 = features.RayQueries
        };

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .pNext = &rayTracingMaintenanceFeatures,
            .accelerationStructure = features.RayTracing || features.RayQueries,
            .descriptorBindingAccelerationStructureUpdateAfterBind = features.RayTracing || features.RayQueries
        };
      
        if (features.RayQueries || features.RayTracing)
            lastFeature = &accelerationStructureFeatures;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = lastFeature,
            .rayTracingPipeline = features.RayTracing
        };

        if (features.RayTracing)
            lastFeature = &rayTracingPipelineFeatures;

        // Enable task and mesh shaders.
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = lastFeature,
            .taskShader = features.MeshShaders,
            .meshShader = features.MeshShaders
        };

        if (features.MeshShaders)
            lastFeature = &meshShaderFeatures;

        VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutableDescriptorTypeFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
            .pNext = lastFeature,
            .mutableDescriptorType = features.DynamicDescriptors
        };

        if (features.DynamicDescriptors)
            lastFeature = &mutableDescriptorTypeFeatures;

        // Enable maintenance 5 extension, if supported.
        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
            .pNext = lastFeature,
            .maintenance5 = true
        };

        if (std::ranges::find_if(m_extensions, [](auto& ext) { return ext == VK_KHR_MAINTENANCE_5_EXTENSION_NAME; }) != m_extensions.end())
            lastFeature = &maintenance5Features;

        // Allow geometry and tessellation shader stages.
        // NOTE: ... except when building tests, as they are not supported by SwiftShader.
        VkPhysicalDeviceFeatures2 deviceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = lastFeature,
            .features = {
                .fullDrawIndexUint32 = true,
                .independentBlend = true,
#ifndef LITEFX_BUILD_TESTS
                .geometryShader = true,
                .tessellationShader = true,
#endif // LITEFX_BUILD_TESTS
                .dualSrcBlend = true,
                .logicOp = true,
                .drawIndirectFirstInstance = features.DrawIndirect,
                .depthClamp = true,
                .depthBounds = features.DepthBoundsTest,
                .alphaToOne = true,
                .multiViewport = true,
                .samplerAnisotropy = true,
                .shaderFloat64 = true,
                .shaderInt64 = true,
                .shaderInt16 = true
            }
        };

        // Enable synchronization overhaul.
        VkPhysicalDeviceVulkan13Features deviceFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &deviceFeatures,
            .synchronization2 = true,
            .dynamicRendering = true,
            .maintenance4 = true
        };

        // Enable various descriptor related features, as well as timeline semaphores and other little QoL improvements.
        // NOTE: Input attachment features are disabled, as they are not supported by Intel ARC drivers and SwiftShader and since we're now using dynamic rendering anyway, this became (even) less important.
        VkPhysicalDeviceVulkan12Features deviceFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &deviceFeatures13,
            .drawIndirectCount = features.DrawIndirect,
            .descriptorIndexing = true,
            //.shaderInputAttachmentArrayDynamicIndexing = true,
            .shaderUniformTexelBufferArrayDynamicIndexing = true,
            .shaderStorageTexelBufferArrayDynamicIndexing = true,
            .shaderUniformBufferArrayNonUniformIndexing = true,
            .shaderSampledImageArrayNonUniformIndexing = true,
            .shaderStorageBufferArrayNonUniformIndexing = true,
            .shaderStorageImageArrayNonUniformIndexing = true,
            //.shaderInputAttachmentArrayNonUniformIndexing = true,
            .shaderUniformTexelBufferArrayNonUniformIndexing = true,
            .shaderStorageTexelBufferArrayNonUniformIndexing = true,
            //.descriptorBindingUniformBufferUpdateAfterBind = true, // Not supported on NVidia Pascal and earlier, as well as SwiftShader.
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

        // Enable shader draw parameters, if we use indirect draw.
        VkPhysicalDeviceVulkan11Features deviceFeatures11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &deviceFeatures12,
            .shaderDrawParameters = true
        };
        
        // Enable explicit depth clip toggle.
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT,
            .pNext = &deviceFeatures11,
            .depthClipEnable = true
        };

        // Enable extended dynamic state.
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = &depthClipEnableFeatures,
            .extendedDynamicState = true
        };

        // Enable descriptor buffer.
        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = &extendedDynamicStateFeatures,
            .descriptorBuffer = true
        };

        // Define the device itself.
        VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &descriptorBufferFeatures,
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
#ifndef NDEBUG
        debugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(::vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
#endif

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
        if (vkGetDescriptorSetLayoutSize == nullptr)
            vkGetDescriptorSetLayoutSize = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(::vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSizeEXT"));

        if (vkGetDescriptorSetLayoutBindingOffset == nullptr)
            vkGetDescriptorSetLayoutBindingOffset = reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(::vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutBindingOffsetEXT"));

        if (vkGetDescriptor == nullptr)
            vkGetDescriptor = reinterpret_cast<PFN_vkGetDescriptorEXT>(::vkGetDeviceProcAddr(device, "vkGetDescriptorEXT"));

        if (vkCmdBindDescriptorBuffers == nullptr)
            vkCmdBindDescriptorBuffers = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(::vkGetDeviceProcAddr(device, "vkCmdBindDescriptorBuffersEXT"));

        if (vkCmdSetDescriptorBufferOffsets == nullptr)
            vkCmdSetDescriptorBufferOffsets = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(::vkGetDeviceProcAddr(device, "vkCmdSetDescriptorBufferOffsetsEXT"));

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

    inline void initializeResourceHeaps() noexcept
    {
        // NOTE: Check the notes in `VulkanGraphicsFactory::createDescriptorHeap` for an explanation why we only use one descriptor heap in the Vulkan backend!

        // Initialize the global descriptor heaps. First, we need to request the physical device limits that we need later to bind descriptors within the descriptor buffer.
        VkPhysicalDeviceProperties2 deviceProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &m_descriptorBufferProperties
        };

        ::vkGetPhysicalDeviceProperties2(m_adapter->handle(), &deviceProperties);

        // Reduce the descriptor buffer size and warn about it, if the device properties exhibit a lower limit. Creation should then fail in factory, if 
        // validation layers are enabled. Otherwise a more severe error could be encountered (probably VK_DEVICE_LOST).
        auto alignedGlobalDescriptorHeapSize = align(static_cast<size_t>(m_globalDescriptorHeapAllocator.size()), static_cast<size_t>(m_descriptorBufferProperties.descriptorBufferOffsetAlignment));

        if (static_cast<size_t>(m_descriptorBufferProperties.descriptorBufferAddressSpaceSize) < alignedGlobalDescriptorHeapSize)
            LITEFX_WARNING(VULKAN_LOG, "Requested descriptor heap size was aligned to {0} bytes, but the device only supports descriptor buffers up to {1} bytes.", alignedGlobalDescriptorHeapSize, m_descriptorBufferProperties.descriptorBufferAddressSpaceSize);

        // Create the descriptor buffers for both heaps.
        m_globalDescriptorHeap = m_factory->createDescriptorHeap("Global Descriptor Heap", alignedGlobalDescriptorHeapSize);
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

VulkanDevice::VulkanDevice(const VulkanBackend& /*backend*/, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features, Span<String> extensions, size_t globalDescriptorHeapSize) :
    Resource<VkDevice>(nullptr), m_impl(adapter, std::move(surface), features, extensions, globalDescriptorHeapSize)
{
    LITEFX_DEBUG(VULKAN_LOG, "Creating Vulkan device {{ Surface: {0}, Adapter: {1}, Extensions: {2} }}...", static_cast<void*>(m_impl->m_surface.get()), adapter.deviceId(), Join(this->enabledExtensions(), ", "));
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
}

VulkanDevice::~VulkanDevice() noexcept = default;

SharedPtr<VulkanDevice> VulkanDevice::initialize(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync, GraphicsDeviceFeatures features)
{
    this->handle() = m_impl->initialize(features);

    // NOTE: The order of initialization here is important.
    m_impl->initializeDefaultQueues(*this);
    m_impl->m_swapChain = UniquePtr<VulkanSwapChain>(new VulkanSwapChain(*this, format, renderArea, backBuffers, enableVsync));
    m_impl->m_factory = VulkanGraphicsFactory::create(*this);
    m_impl->initializeResourceHeaps();

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
    m_impl->m_globalDescriptorHeap.reset();
    m_impl->m_factory.reset();

    // Destroy the device.
    ::vkDestroyDevice(this->handle(), nullptr);
}

Span<const String> VulkanDevice::enabledExtensions() const noexcept
{
    return m_impl->m_extensions;
}

void VulkanDevice::setDebugName([[maybe_unused]] VkDebugReportObjectTypeEXT type, [[maybe_unused]] UInt64 handle, [[maybe_unused]] StringView name) const noexcept
{
#ifndef NDEBUG
    if (m_impl->debugMarkerSetObjectName != nullptr)
    {
        VkDebugMarkerObjectNameInfoEXT nameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .object = handle,
            .pObjectName = name.data() // NOLINT(bugprone-suspicious-stringview-data-usage)
        };

        if (m_impl->debugMarkerSetObjectName(this->handle(), &nameInfo) != VK_SUCCESS)
            LITEFX_WARNING(VULKAN_LOG, "Unable to set object name for object handle {0}.", static_cast<const void*>(&handle));
    }
#endif
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
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.accelerationStructureDescriptorSize);
    case DescriptorType::Buffer:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.uniformTexelBufferDescriptorSize);
    case DescriptorType::ConstantBuffer:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.uniformBufferDescriptorSize);
    case DescriptorType::InputAttachment:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.inputAttachmentDescriptorSize);
    case DescriptorType::RWBuffer:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.storageTexelBufferDescriptorSize);
    case DescriptorType::RWTexture:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.storageImageDescriptorSize);
    case DescriptorType::Sampler:
    case DescriptorType::SamplerDescriptorHeap:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.samplerDescriptorSize);
    case DescriptorType::ByteAddressBuffer:
    case DescriptorType::RWByteAddressBuffer:
    case DescriptorType::StructuredBuffer:
    case DescriptorType::RWStructuredBuffer:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.storageBufferDescriptorSize);
    case DescriptorType::Texture:
        return static_cast<UInt32>(m_impl->m_descriptorBufferProperties.sampledImageDescriptorSize);
    case DescriptorType::ResourceDescriptorHeap:
        // We need to return the largest descriptor from all supported ones. Supported descriptor types are listed in `VulkanDescriptorSetLayoutImpl::initialize`.
        // See: https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceDescriptorBufferPropertiesEXT.html#_description
        return static_cast<UInt32>(std::max({
                m_impl->m_descriptorBufferProperties.uniformBufferDescriptorSize,       // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                m_impl->m_descriptorBufferProperties.uniformTexelBufferDescriptorSize,  // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                m_impl->m_descriptorBufferProperties.storageTexelBufferDescriptorSize,  // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                m_impl->m_descriptorBufferProperties.sampledImageDescriptorSize,        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                m_impl->m_descriptorBufferProperties.storageImageDescriptorSize,        // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                m_impl->m_descriptorBufferProperties.storageBufferDescriptorSize        // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }));
    default:
        throw InvalidArgumentException("type", "The provided descriptor type cannot be mapped to a descriptor heap.");
    }
}

VirtualAllocator::Allocation VulkanDevice::allocateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, DescriptorHeapType /*heapType*/) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);
    return m_impl->m_globalDescriptorHeapAllocator.allocate(
        static_cast<UInt64>(descriptorSet.descriptorBuffer().size()), 
        static_cast<UInt32>(m_impl->m_descriptorBufferProperties.descriptorBufferOffsetAlignment), 
        AllocationStrategy::OptimizeTime);
}

void VulkanDevice::releaseGlobalDescriptors(const VulkanDescriptorSet& descriptorSet) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_bufferBindMutex);
    m_impl->m_globalDescriptorHeapAllocator.free(descriptorSet.globalHeapAllocation(DescriptorHeapType::Resource)); // NOTE: Heap type does not matter in Vulkan.
}

void VulkanDevice::updateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const
{
    // Bind the descriptor to the appropriate type. Note that static samplers aren't bound, so effectively this call is invalid. However we simply treat it as a no-op.
    auto descriptorLayout = descriptorSet.layout().descriptor(binding);

    // Compute the descriptor offset and binding range.
    auto firstDescriptor = descriptorSet.layout().getDescriptorOffset(binding, offset);
    auto descriptorBuffer = descriptorSet.descriptorBuffer();
    auto descriptorOffset = std::next(descriptorBuffer.data(), firstDescriptor);
    auto descriptorSize = this->descriptorSize(descriptorLayout.descriptorType());
    auto mappedRange = static_cast<size_t>(descriptors) * descriptorSize;

    // NOTE: We actually only need to check for a static sampler here, but in case we need to change this later, we'll keep it this way.
    if (descriptorLayout.descriptorType() == DescriptorType::Sampler && descriptorLayout.staticSampler() == nullptr)
        m_impl->m_globalDescriptorHeap->write(descriptorOffset, mappedRange, 
            static_cast<size_t>(descriptorSet.globalHeapAllocation(DescriptorHeapType::Sampler).Offset) + firstDescriptor);
    else if (descriptorLayout.descriptorType() != DescriptorType::Sampler)
        m_impl->m_globalDescriptorHeap->write(descriptorOffset, mappedRange, 
            static_cast<size_t>(descriptorSet.globalHeapAllocation(DescriptorHeapType::Resource).Offset) + firstDescriptor);
}

void VulkanDevice::bindDescriptorSet(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const noexcept
{
    // Copy the descriptors to the global heaps and set the root table parameters.
    if (descriptorSet.layout().bindsResources() || descriptorSet.layout().bindsSamplers()) // Discard empty sets.
    {
        const UInt32 bufferIndex{ 0u }; // See `bindGlobalDescriptorHeaps` below - the only heap is bound at index 0 there.
        const auto bufferOffset = static_cast<VkDeviceSize>(descriptorSet.globalHeapAllocation(DescriptorHeapType::Resource).Offset); // NOTE: Heap type does not matter in Vulkan.

        // Set the descriptor buffer offsets for the descriptor sets.
        vkCmdSetDescriptorBufferOffsets(commandBuffer.handle(), pipeline.pipelineType(), pipeline.layout()->handle(), descriptorSet.layout().space(), 1u, &bufferIndex, &bufferOffset);
    }
}

void VulkanDevice::bindGlobalDescriptorHeaps(const VulkanCommandBuffer& commandBuffer) const noexcept
{
    // Create the descriptor buffer binding infos.
    // NOTE: The order is important here! If we change this, `bindDescriptorSet` must be updated as well.
    auto descriptorHeaps = std::array {
        VkDescriptorBufferBindingInfoEXT { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT, .address = m_impl->m_globalDescriptorHeap->virtualAddress(), .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT },
    };

    // Bind the descriptor buffers.
    vkCmdBindDescriptorBuffers(commandBuffer.handle(), static_cast<UInt32>(descriptorHeaps.size()), descriptorHeaps.data());
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

MultiSamplingLevel VulkanDevice::maximumMultiSamplingLevel(Format format) const noexcept
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