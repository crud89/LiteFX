#pragma once

#include <litefx/config.h>
#include <litefx/backends/vulkan_export.h>

#if (defined _WIN32 || defined WINCE)
#  define VK_USE_PLATFORM_WIN32_KHR
#else 
#  pragma message ("Vulkan: No supported surface platform detected.")
#endif

#include <litefx/rendering.hpp>
#include <vulkan/vulkan.h>
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr StringView VULKAN_LOG = "Backend::Vulkan"sv;

	// Forward declarations.
    class VulkanVertexBufferLayout;
    class VulkanIndexBufferLayout;
    class VulkanDescriptorLayout;
    class VulkanDescriptorSetLayout;
    class VulkanDescriptorSet;
    class VulkanPushConstantsRange;
    class VulkanPushConstantsLayout;
    class VulkanPipelineLayout;
    class VulkanShaderModule;
    class VulkanShaderProgram;
    class VulkanCommandBuffer;
    class VulkanInputAssembler;
    class VulkanRasterizer; 
    class VulkanPipelineState;
    class VulkanRenderPipeline;
    class VulkanComputePipeline;
    class VulkanRayTracingPipeline;
    class VulkanFrameBuffer;
    class VulkanRenderPass;
    class VulkanSwapChain;
    class VulkanQueue;
    class VulkanGraphicsFactory;
    class VulkanDevice;
    class VulkanBackend;

    // Interface declarations.
    class IVulkanBuffer;
    class IVulkanVertexBuffer;
    class IVulkanIndexBuffer;
    class IVulkanImage;
    class IVulkanSampler;
    class IVulkanAccelerationStructure;
    class VulkanBottomLevelAccelerationStructure;
    class VulkanTopLevelAccelerationStructure;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    // Builder declarations.
    class VulkanVertexBufferLayoutBuilder;
    class VulkanDescriptorSetLayoutBuilder;
    class VulkanPushConstantsLayoutBuilder;
    class VulkanPipelineLayoutBuilder;
    class VulkanShaderProgramBuilder;
    class VulkanInputAssemblerBuilder;
    class VulkanRasterizerBuilder;
    class VulkanRenderPipelineBuilder;
    class VulkanComputePipelineBuilder;
    class VulkanRayTracingPipelineBuilder;
    class VulkanRenderPassBuilder;
    class VulkanBarrierBuilder;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)

    /// @brief Contains conversion helpers for Vulkan.
    namespace Vk
    {
        Format LITEFX_VULKAN_API getFormat(const VkFormat& format);

        VkFormat LITEFX_VULKAN_API getFormat(Format format);

        //BufferFormat LITEFX_VULKAN_API getFormat(const VkFormat& format);

        VkFormat LITEFX_VULKAN_API getFormat(BufferFormat format);

        PolygonMode LITEFX_VULKAN_API getPolygonMode(const VkPolygonMode& mode);

        VkPolygonMode LITEFX_VULKAN_API getPolygonMode(PolygonMode mode);

        CullMode LITEFX_VULKAN_API getCullMode(const VkCullModeFlags& mode);

        VkCullModeFlags LITEFX_VULKAN_API getCullMode(CullMode mode);

        PrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const VkPrimitiveTopology& topology);

        VkPrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(PrimitiveTopology topology);

        ShaderStage LITEFX_VULKAN_API getShaderStage(const VkShaderStageFlagBits& shaderType);

        VkShaderStageFlagBits LITEFX_VULKAN_API getShaderStage(ShaderStage shaderType);

        MultiSamplingLevel LITEFX_VULKAN_API getSamples(const VkSampleCountFlagBits& samples);

        VkImageType LITEFX_VULKAN_API getImageType(ImageDimensions dimension);

        VkImageViewType LITEFX_VULKAN_API getImageViewType(ImageDimensions dimension, UInt32 layers = 1);

        VkSampleCountFlagBits LITEFX_VULKAN_API getSamples(MultiSamplingLevel samples);

        VkCompareOp LITEFX_VULKAN_API getCompareOp(CompareOperation compareOp);

        VkStencilOp LITEFX_VULKAN_API getStencilOp(StencilOperation stencilOp);

        VkBlendFactor LITEFX_VULKAN_API getBlendFactor(BlendFactor blendFactor);

        VkBlendOp LITEFX_VULKAN_API getBlendOperation(BlendOperation blendOperation);

        VkPipelineStageFlags2 LITEFX_VULKAN_API getPipelineStage(PipelineStage pipelineStage);

        VkAccessFlags2 LITEFX_VULKAN_API getResourceAccess(ResourceAccess resourceAccess);

        VkImageLayout LITEFX_VULKAN_API getImageLayout(ImageLayout imageLayout);

        /// @brief Returns the address of a dispatchable handle.
        ///
        /// @tparam THandle The type of the handle.
        /// @param handle The handle to convert.
        /// @return The address of the handle.
        template <typename THandle>
        constexpr UInt64 handleAddress(const THandle handle) noexcept {
            return reinterpret_cast<std::uintptr_t>(handle); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        }

        /// @brief Returns the address of a non-dispatchable handle.
        ///
        /// @param handle The handle to convert.
        /// @return The address of the handle.
        template <>
        constexpr UInt64 handleAddress(const UInt64 handle) noexcept {
            return handle;
        }
    }

    /// @brief Represents a Vulkan @ref IGraphicsAdapter.
    class LITEFX_VULKAN_API VulkanGraphicsAdapter final : public IGraphicsAdapter, public Resource<VkPhysicalDevice> {
        LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl);
        friend struct SharedObject::Allocator<VulkanGraphicsAdapter>;

    private:
        /// @brief Initializes a graphics adapter instance with a physical device.
        ///
        /// @param adapter The physical device to initialize the instance with.
        explicit VulkanGraphicsAdapter(VkPhysicalDevice adapter);

    private:
        /// @copydoc IGraphicsAdapter::IGraphicsAdapter(IGraphicsAdapter&&)
        VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) noexcept = delete;

        /// @copydoc IGraphicsAdapter::IGraphicsAdapter(const IGraphicsAdapter&)
        VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;

        /// @copydoc IGraphicsAdapter::operator=(IGraphicsAdapter&&)
        VulkanGraphicsAdapter& operator=(VulkanGraphicsAdapter&&) noexcept = delete;

        /// @copydoc IGraphicsAdapter::operator=(const IGraphicsAdapter&)
        VulkanGraphicsAdapter& operator=(const VulkanGraphicsAdapter&) = delete;

    public:
        /// @copydoc IGraphicsAdapter::~IGraphicsAdapter
        ~VulkanGraphicsAdapter() noexcept override;

    public:
        /// @brief Creates a graphics adapter instance with a physical device.
        ///
        /// @param adapter The physical device to initialize the instance with.
        /// @return A shared pointer to the newly created graphics adapter instance.
        static inline auto create(VkPhysicalDevice adapter) {
            return SharedObject::create<VulkanGraphicsAdapter>(adapter);
        }

    public:
        /// @copydoc IGraphicsAdapter::name
        String name() const override;

        /// @copydoc IGraphicsAdapter::uniqueId
        UInt64 uniqueId() const noexcept override;

        /// @copydoc IGraphicsAdapter::vendorId
        UInt32 vendorId() const noexcept override;

        /// @copydoc IGraphicsAdapter::deviceId
        UInt32 deviceId() const noexcept override;

        /// @copydoc IGraphicsAdapter::type
        GraphicsAdapterType type() const noexcept override;

        /// @copydoc IGraphicsAdapter::driverVersion
        UInt64 driverVersion() const noexcept override;

        /// @copydoc IGraphicsAdapter::apiVersion
        UInt32 apiVersion() const noexcept override;

        /// @copydoc IGraphicsAdapter::dedicatedMemory
        UInt64 dedicatedMemory() const noexcept override;

    public:
        /// @brief Returns the limits of the physical device.
        ///
        /// @return The limits of the physical device.
        VkPhysicalDeviceLimits limits() const noexcept;

        /// @brief Returns `true`, if all elements of @p are contained by the a list of available extensions.
        ///
        /// @return `true`, if all elements of @p are contained by the a list of available extensions.
        /// @see getAvailableDeviceExtensions
        bool validateDeviceExtensions(Span<const String> extensions) const;

        /// @brief Returns a list of available extensions.
        ///
        /// @return A list of available extensions.
        /// @see validateDeviceExtensions
        Enumerable<String> getAvailableDeviceExtensions() const;

        /// @brief Returns `true`, if all elements of @p are contained by the a list of available validation layers.
        ///
        /// @return `true`, if all elements of @p are contained by the a list of available validation layers.
        /// @see getDeviceValidationLayers
        bool validateDeviceLayers(const Span<const String> validationLayers) const;

        /// @brief Returns a list of available validation layers.
        ///
        /// @return A list of available validation layers.
        /// @see validateDeviceLayers
        Enumerable<String> deviceValidationLayers() const;
    };

    /// @brief Represents a Vulkan @ref ISurface.
    class LITEFX_VULKAN_API VulkanSurface final : public ISurface, public Resource<VkSurfaceKHR> {
        LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

    public:
#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// @brief Initializes the surface from a surface and instance handle.
        ///
        /// @param surface The handle of the Vulkan surface.
        /// @param instance The handle of the parent instance.
        /// @param hwnd The handle of the surface window.
        VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& instance, const HWND hwnd);
#else
        /// @brief Initializes the surface from a surface and instance handle.
        ///
        /// @param surface The handle of the Vulkan surface.
        /// @param instance The handle of the parent instance.
        VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& instance);
#endif // VK_USE_PLATFORM_WIN32_KHR


        /// @copydoc ISurface::ISurface(ISurface&&)
        VulkanSurface(VulkanSurface&&) noexcept;

        /// @copydoc ISurface::ISurface(const ISurface&)
        VulkanSurface(const VulkanSurface&) = delete;

        /// @copydoc ISurface::operator=(ISurface&&)
        VulkanSurface& operator=(VulkanSurface&&) noexcept;

        /// @copydoc ISurface::operator=(const ISurface&)
        VulkanSurface& operator=(const VulkanSurface&) = delete;

        /// @copydoc ISurface::~ISurface
        ~VulkanSurface() noexcept override;

    public:
        /// @brief Returns the handle of the backend, the surface has been created from.
        ///
        /// @return The handle of the backend, the surface has been created from.
        const VkInstance& instance() const noexcept;

#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// @brief Returns the window handle of the surface.
        ///
        /// @return The window handle of the surface.
        /// @see createSurface
        HWND windowHandle() const noexcept;
#endif // VK_USE_PLATFORM_WIN32_KHR
    };

    /// @brief An exception that is thrown, if a requested Vulkan operation could not be executed.
    class LITEFX_VULKAN_API VulkanPlatformException : public RuntimeException {
    private:
        VkResult m_code;

    public:
        /// @brief Initializes a new exception.
        ///
        /// @param result The error code returned by the operation.
        explicit VulkanPlatformException(VkResult result) :
            RuntimeException("Operation returned {0}.", result), m_code(result) { }

        /// @brief Initializes a new exception.
        ///
        /// @param result The error code returned by the operation.
        /// @param message The error message.
        explicit VulkanPlatformException(VkResult result, StringView message) :
            RuntimeException("{1} Operation returned {0}.", result, message), m_code(result) { }

        /// @brief Initializes a new exception.
        ///
        /// @param format The format string for the error message.
        /// @param result The error code returned by the operation.
        /// @param args The arguments passed to the error message format string.
        template <typename ...TArgs>
        explicit VulkanPlatformException(VkResult result, StringView format, TArgs&&... args) :
            VulkanPlatformException(result, std::format(format, std::forward<TArgs>(args)...)) { }

        /// @copydoc RuntimeException::RuntimeException(RuntimeException&&)
        VulkanPlatformException(VulkanPlatformException&&) noexcept = default;

        /// @copydoc RuntimeException::RuntimeException(const RuntimeException&)
        VulkanPlatformException(const VulkanPlatformException&) = default;

        /// @copydoc RuntimeException::operator=(RuntimeException&&)
        VulkanPlatformException& operator=(VulkanPlatformException&&) noexcept = default;

        /// @copydoc RuntimeException::operator=(const RuntimeException&)
        VulkanPlatformException& operator=(const VulkanPlatformException&) = default;

        /// @copydoc RuntimeException::~RuntimeException
        ~VulkanPlatformException() noexcept override = default;

    public:
        /// @brief Returns the error code.
        ///
        /// @return The code of the error.
        VkResult code() const noexcept {
            return m_code;
        }
    };

    /// @brief Raises a @ref VulkanPlatformException, if @p result does not equal `VK_SUCCESS`.
    ///
    /// @param hr The error code returned by the operation.
    /// @param message The format string for the error message.
    /// @param args The arguments passed to the error message format string.
    template <typename ...TArgs>
    static inline void raiseIfFailed(VkResult result, StringView message, TArgs&&... args) {
        if (result == VK_SUCCESS) [[likely]]
            return;
        else
            throw VulkanPlatformException(result, message, std::forward<TArgs>(args)...);
    }
}