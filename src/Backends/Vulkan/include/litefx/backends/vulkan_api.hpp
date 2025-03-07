#pragma once

#include <litefx/config.h>

#if !defined (LITEFX_VULKAN_API)
#  if defined(LiteFX_Backends_Vulkan_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_VULKAN_API __declspec(dllexport)
#  elif (defined(LiteFX_Backends_Vulkan_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_VULKAN_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Backends_Vulkan_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_VULKAN_API __declspec(dllimport)
#  endif
#endif 

#ifndef LITEFX_VULKAN_API
#  define LITEFX_VULKAN_API
#endif

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

    /// <summary>
    /// Contains conversion helpers for Vulkan.
    /// </summary>
    namespace Vk
    {
        /// <summary>
        /// 
        /// </summary>
        Format LITEFX_VULKAN_API getFormat(const VkFormat& format);

        /// <summary>
        /// 
        /// </summary>
        VkFormat LITEFX_VULKAN_API getFormat(Format format);

        /// <summary>
        /// 
        /// </summary>
        //BufferFormat LITEFX_VULKAN_API getFormat(const VkFormat& format);

        /// <summary>
        /// 
        /// </summary>
        VkFormat LITEFX_VULKAN_API getFormat(BufferFormat format);

        /// <summary>
        /// 
        /// </summary>
        PolygonMode LITEFX_VULKAN_API getPolygonMode(const VkPolygonMode& mode);

        /// <summary>
        /// 
        /// </summary>
        VkPolygonMode LITEFX_VULKAN_API getPolygonMode(PolygonMode mode);

        /// <summary>
        /// 
        /// </summary>
        CullMode LITEFX_VULKAN_API getCullMode(const VkCullModeFlags& mode);

        /// <summary>
        /// 
        /// </summary>
        VkCullModeFlags LITEFX_VULKAN_API getCullMode(CullMode mode);

        /// <summary>
        /// 
        /// </summary>
        PrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const VkPrimitiveTopology& topology);

        /// <summary>
        /// 
        /// </summary>
        VkPrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(PrimitiveTopology topology);

        /// <summary>
        /// 
        /// </summary>
        ShaderStage LITEFX_VULKAN_API getShaderStage(const VkShaderStageFlagBits& shaderType);

        /// <summary>
        /// 
        /// </summary>
        VkShaderStageFlagBits LITEFX_VULKAN_API getShaderStage(ShaderStage shaderType);

        /// <summary>
        /// 
        /// </summary>
        MultiSamplingLevel LITEFX_VULKAN_API getSamples(const VkSampleCountFlagBits& samples);

        /// <summary>
        /// 
        /// </summary>
        VkImageType LITEFX_VULKAN_API getImageType(ImageDimensions dimension);

        /// <summary>
        /// 
        /// </summary>
        VkImageViewType LITEFX_VULKAN_API getImageViewType(ImageDimensions dimension, UInt32 layers = 1);

        /// <summary>
        /// 
        /// </summary>
        VkSampleCountFlagBits LITEFX_VULKAN_API getSamples(MultiSamplingLevel samples);

        /// <summary>
        /// 
        /// </summary>
        VkCompareOp LITEFX_VULKAN_API getCompareOp(CompareOperation compareOp);

        /// <summary>
        /// 
        /// </summary>
        VkStencilOp LITEFX_VULKAN_API getStencilOp(StencilOperation stencilOp);

        /// <summary>
        /// 
        /// </summary>
        VkBlendFactor LITEFX_VULKAN_API getBlendFactor(BlendFactor blendFactor);

        /// <summary>
        /// 
        /// </summary>
        VkBlendOp LITEFX_VULKAN_API getBlendOperation(BlendOperation blendOperation);

        /// <summary>
        /// 
        /// </summary>
        VkPipelineStageFlags2 LITEFX_VULKAN_API getPipelineStage(PipelineStage pipelineStage);

        /// <summary>
        /// 
        /// </summary>
        VkAccessFlags2 LITEFX_VULKAN_API getResourceAccess(ResourceAccess resourceAccess);

        /// <summary>
        /// 
        /// </summary>
        VkImageLayout LITEFX_VULKAN_API getImageLayout(ImageLayout imageLayout);

        /// <summary>
        /// Returns the address of a dispatchable handle.
        /// </summary>
        /// <typeparam name="THandle">The type of the handle.</typeparam>
        /// <param name="handle">The handle to convert.</param>
        /// <returns>The address of the handle.</returns>
        template <typename THandle>
        constexpr UInt64 handleAddress(const THandle handle) noexcept {
            return reinterpret_cast<std::uintptr_t>(handle); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        }

        /// <summary>
        /// Returns the address of a non-dispatchable handle.
        /// </summary>
        /// <param name="handle">The handle to convert.</param>
        /// <returns>The address of the handle.</returns>
        template <>
        constexpr UInt64 handleAddress(const UInt64 handle) noexcept {
            return handle;
        }
    }

    /// <summary>
    /// Represents a Vulkan <see cref="IGraphicsAdapter" />.
    /// </summary>
    class LITEFX_VULKAN_API VulkanGraphicsAdapter final : public IGraphicsAdapter, public Resource<VkPhysicalDevice> {
        LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl);
        friend struct SharedObject::Allocator<VulkanGraphicsAdapter>;

    private:
        /// <summary>
        /// Initializes a graphics adapter instance with a physical device.
        /// </summary>
        /// <param name="adapter">The physical device to initialize the instance with.</param>
        explicit VulkanGraphicsAdapter(VkPhysicalDevice adapter);

    private:
        /// <inheritdoc />
        VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) noexcept = delete;

        /// <inheritdoc />
        VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;

        /// <inheritdoc />
        VulkanGraphicsAdapter& operator=(VulkanGraphicsAdapter&&) noexcept = delete;

        /// <inheritdoc />
        VulkanGraphicsAdapter& operator=(const VulkanGraphicsAdapter&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanGraphicsAdapter() noexcept override;

    public:
        /// <summary>
        /// Creates a graphics adapter instance with a physical device.
        /// </summary>
        /// <param name="adapter">The physical device to initialize the instance with.</param>
        /// <returns>A shared pointer to the newly created graphics adapter instance.</returns>
        static inline auto create(VkPhysicalDevice adapter) {
            return SharedObject::create<VulkanGraphicsAdapter>(adapter);
        }

    public:
        /// <inheritdoc />
        String name() const override;

        /// <inheritdoc />
        UInt64 uniqueId() const noexcept override;

        /// <inheritdoc />
        UInt32 vendorId() const noexcept override;

        /// <inheritdoc />
        UInt32 deviceId() const noexcept override;

        /// <inheritdoc />
        GraphicsAdapterType type() const noexcept override;

        /// <inheritdoc />
        UInt64 driverVersion() const noexcept override;

        /// <inheritdoc />
        UInt32 apiVersion() const noexcept override;

        /// <inheritdoc />
        UInt64 dedicatedMemory() const noexcept override;

    public:
        /// <summary>
        /// Returns the limits of the physical device.
        /// </summary>
        /// <returns>The limits of the physical device.</returns>
        VkPhysicalDeviceLimits limits() const noexcept;

        /// <summary>
        /// Returns <c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.
        /// </summary>
        /// <returns><c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.</returns>
        /// <seealso cref="getAvailableDeviceExtensions" />
        bool validateDeviceExtensions(Span<const String> extensions) const;

        /// <summary>
        /// Returns a list of available extensions.
        /// </summary>
        /// <returns>A list of available extensions.</returns>
        /// <seealso cref="validateDeviceExtensions" />
        Enumerable<String> getAvailableDeviceExtensions() const;

        /// <summary>
        /// Returns <c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.
        /// </summary>
        /// <returns><c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.</returns>
        /// <seealso cref="getDeviceValidationLayers" />
        bool validateDeviceLayers(const Span<const String> validationLayers) const;

        /// <summary>
        /// Returns a list of available validation layers.
        /// </summary>
        /// <returns>A list of available validation layers.</returns>
        /// <seealso cref="validateDeviceLayers" />
        Enumerable<String> deviceValidationLayers() const;
    };

    /// <summary>
    /// Represents a Vulkan <see cref="ISurface" />.
    /// </summary>
    class LITEFX_VULKAN_API VulkanSurface final : public ISurface, public Resource<VkSurfaceKHR> {
        LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

    public:
#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// <summary>
        /// Initializes the surface from a surface and instance handle.
        /// </summary>
        /// <param name="surface">The handle of the Vulkan surface.</param>
        /// <param name="instance">The handle of the parent instance.</param>
        /// <param name="hwnd">The handle of the surface window.</param>
        VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& instance, const HWND hwnd);
#else
        /// <summary>
        /// Initializes the surface from a surface and instance handle.
        /// </summary>
        /// <param name="surface">The handle of the Vulkan surface.</param>
        /// <param name="instance">The handle of the parent instance.</param>
        VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& instance);
#endif // VK_USE_PLATFORM_WIN32_KHR


        /// <inheritdoc />
        VulkanSurface(VulkanSurface&&) noexcept;

        /// <inheritdoc />
        VulkanSurface(const VulkanSurface&) = delete;

        /// <inheritdoc />
        VulkanSurface& operator=(VulkanSurface&&) noexcept;

        /// <inheritdoc />
        VulkanSurface& operator=(const VulkanSurface&) = delete;

        /// <inheritdoc />
        ~VulkanSurface() noexcept override;

    public:
        /// <summary>
        /// Returns the handle of the backend, the surface has been created from.
        /// </summary>
        /// <returns>The handle of the backend, the surface has been created from.</returns>
        const VkInstance& instance() const noexcept;

#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// <summary>
        /// Returns the window handle of the surface.
        /// </summary>
        /// <returns>The window handle of the surface.</returns>
        /// <seealso cref="createSurface" />
        HWND windowHandle() const noexcept;
#endif // VK_USE_PLATFORM_WIN32_KHR
    };

    /// <summary>
    /// An exception that is thrown, if a requested Vulkan operation could not be executed.
    /// </summary>
    class LITEFX_VULKAN_API VulkanPlatformException : public RuntimeException {
    private:
        VkResult m_code;

    public:
        /// <summary>
        /// Initializes a new exception.
        /// </summary>
        /// <param name="result">The error code returned by the operation.</param>
        explicit VulkanPlatformException(VkResult result) :
            RuntimeException("Operation returned {0}.", result), m_code(result) { }

        /// <summary>
        /// Initializes a new exception.
        /// </summary>
        /// <param name="result">The error code returned by the operation.</param>
        /// <param name="message">The error message.</param>
        explicit VulkanPlatformException(VkResult result, StringView message) :
            RuntimeException("{1} Operation returned {0}.", result, message), m_code(result) { }

        /// <summary>
        /// Initializes a new exception.
        /// </summary>
        /// <param name="format">The format string for the error message.</param>
        /// <param name="result">The error code returned by the operation.</param>
        /// <param name="args">The arguments passed to the error message format string.</param>
        template <typename ...TArgs>
        explicit VulkanPlatformException(VkResult result, StringView format, TArgs&&... args) :
            VulkanPlatformException(result, std::format(format, std::forward<TArgs>(args)...)) { }

        /// <inheritdoc />
        VulkanPlatformException(VulkanPlatformException&&) noexcept = default;

        /// <inheritdoc />
        VulkanPlatformException(const VulkanPlatformException&) = default;

        /// <inheritdoc />
        VulkanPlatformException& operator=(VulkanPlatformException&&) noexcept = default;

        /// <inheritdoc />
        VulkanPlatformException& operator=(const VulkanPlatformException&) = default;

        /// <inheritdoc />
        ~VulkanPlatformException() noexcept override = default;

    public:
        /// <summary>
        /// Returns the error code.
        /// </summary>
        /// <returns>The code of the error.</returns>
        VkResult code() const noexcept {
            return m_code;
        }
    };

    /// <summary>
    /// Raises a <see cref="VulkanPlatformException" />, if <paramref name="result" /> does not equal `VK_SUCCESS`.
    /// </summary>
    /// <param name="hr">The error code returned by the operation.</param>
    /// <param name="message">The format string for the error message.</param>
    /// <param name="args">The arguments passed to the error message format string.</param>
    template <typename ...TArgs>
    static inline void raiseIfFailed(VkResult result, StringView message, TArgs&&... args) {
        if (result == VK_SUCCESS) [[likely]]
            return;
        else
            throw VulkanPlatformException(result, message, std::forward<TArgs>(args)...);
    }
}