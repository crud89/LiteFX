#pragma once

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

#include <litefx/config.h>
#include <litefx/rendering.hpp>
#include <vulkan/vulkan.h>

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr char VULKAN_LOG[] = "Backend::Vulkan";

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
    class VulkanFrameBuffer;
    class VulkanRenderPass;
    class VulkanInputAttachmentMapping;
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

#if defined(BUILD_DEFINE_BUILDERS)
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
    class VulkanRenderPassBuilder;
    class VulkanBarrierBuilder;
#endif // defined(BUILD_DEFINE_BUILDERS)

    /// <summary>
    /// Contains conversion helpers for Vulkan.
    /// </summary>
    namespace Vk
    {
        /// <summary>
        /// 
        /// </summary>
        constexpr inline Format LITEFX_VULKAN_API getFormat(const VkFormat& format);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkFormat LITEFX_VULKAN_API getFormat(Format format);

        /// <summary>
        /// 
        /// </summary>
        //constexpr inline BufferFormat LITEFX_VULKAN_API getFormat(const VkFormat& format);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkFormat LITEFX_VULKAN_API getFormat(BufferFormat format);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline PolygonMode LITEFX_VULKAN_API getPolygonMode(const VkPolygonMode& mode);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkPolygonMode LITEFX_VULKAN_API getPolygonMode(PolygonMode mode);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline CullMode LITEFX_VULKAN_API getCullMode(const VkCullModeFlags& mode);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkCullModeFlags LITEFX_VULKAN_API getCullMode(CullMode mode);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline PrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const VkPrimitiveTopology& topology);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkPrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const PrimitiveTopology& topology);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline ShaderStage LITEFX_VULKAN_API getShaderStage(const VkShaderStageFlagBits& shaderType);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkShaderStageFlagBits LITEFX_VULKAN_API getShaderStage(ShaderStage shaderType);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline MultiSamplingLevel LITEFX_VULKAN_API getSamples(const VkSampleCountFlagBits& samples);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkImageType LITEFX_VULKAN_API getImageType(ImageDimensions dimension);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkImageViewType LITEFX_VULKAN_API getImageViewType(ImageDimensions dimension, const UInt32& layers = 1);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkSampleCountFlagBits LITEFX_VULKAN_API getSamples(MultiSamplingLevel samples);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkCompareOp LITEFX_VULKAN_API getCompareOp(CompareOperation compareOp);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkStencilOp LITEFX_VULKAN_API getStencilOp(StencilOperation stencilOp);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkBlendFactor LITEFX_VULKAN_API getBlendFactor(BlendFactor blendFactor);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkBlendOp LITEFX_VULKAN_API getBlendOperation(BlendOperation blendOperation);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkPipelineStageFlags LITEFX_VULKAN_API getPipelineStage(PipelineStage pipelineStage);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkAccessFlags LITEFX_VULKAN_API getResourceAccess(ResourceAccess resourceAccess);

        /// <summary>
        /// 
        /// </summary>
        constexpr inline VkImageLayout LITEFX_VULKAN_API getImageLayout(ImageLayout imageLayout);
    }

    /// <summary>
    /// Represents a Vulkan <see cref="IGraphicsAdapter" />.
    /// </summary>
    class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public Resource<VkPhysicalDevice> {
        LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl);

    public:
        /// <summary>
        /// Initializes a graphics adapter instance with a physical device.
        /// </summary>
        /// <param name="adapter">The physical device to initialize the instance with.</param>
        explicit VulkanGraphicsAdapter(VkPhysicalDevice adapter);
        VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;
        VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) = delete;
        virtual ~VulkanGraphicsAdapter() noexcept;

    public:
        /// <inheritdoc />
        virtual String name() const noexcept override;

        /// <inheritdoc />
        virtual UInt64 uniqueId() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 vendorId() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 deviceId() const noexcept override;

        /// <inheritdoc />
        virtual GraphicsAdapterType type() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 driverVersion() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 apiVersion() const noexcept override;

        /// <inheritdoc />
        virtual UInt64 dedicatedMemory() const noexcept override;

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
        bool validateDeviceExtensions(Span<const String> extensions) const noexcept;

        /// <summary>
        /// Returns a list of available extensions.
        /// </summary>
        /// <returns>A list of available extensions.</returns>
        /// <seealso cref="validateDeviceExtensions" />
        Enumerable<String> getAvailableDeviceExtensions() const noexcept;

        /// <summary>
        /// Returns <c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.
        /// </summary>
        /// <returns><c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.</returns>
        /// <seealso cref="getDeviceValidationLayers" />
        bool validateDeviceLayers(const Span<const String> validationLayers) const noexcept;

        /// <summary>
        /// Returns a list of available validation layers.
        /// </summary>
        /// <returns>A list of available validation layers.</returns>
        /// <seealso cref="validateDeviceLayers" />
        Enumerable<String> deviceValidationLayers() const noexcept;
    };

    /// <summary>
    /// Represents a Vulkan <see cref="ISurface" />.
    /// </summary>
    class LITEFX_VULKAN_API VulkanSurface : public ISurface, public Resource<VkSurfaceKHR> {
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

        VulkanSurface(const VulkanSurface&) = delete;
        VulkanSurface(VulkanSurface&&) = delete;
        virtual ~VulkanSurface() noexcept;

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
        const HWND windowHandle() const noexcept;
#endif // VK_USE_PLATFORM_WIN32_KHR
    };

    DEFINE_EXCEPTION(VulkanPlatformException, std::runtime_error);

    /// <summary>
    /// 
    /// </summary>
    /// <typeparam name="TException"></typeparam>
    /// <typeparam name="...TArgs"></typeparam>
    /// <param name="result"></param>
    /// <param name="message"></param>
    /// <param name="...args"></param>
    template <typename TException, typename ...TArgs>
    inline void raiseIfFailed(VkResult result, StringView message, TArgs&&... args) {
        if (result == VK_SUCCESS) [[likely]]
            return;

        throw TException(VulkanPlatformException("Result: {0}", result), fmt::format(fmt::runtime(message), std::forward<TArgs>(args)...));
    }
}