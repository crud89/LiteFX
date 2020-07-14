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

#include <vulkan/vulkan.h>

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr char VULKAN_LOG[] = "Backend::Vulkan";

    // Conversion helpers.
    /// <summary>
    /// 
    /// </summary>
    Format LITEFX_VULKAN_API getFormat(const VkFormat& format);

    /// <summary>
    /// 
    /// </summary>
    VkFormat LITEFX_VULKAN_API getFormat(const Format& format);

    /// <summary>
    /// 
    /// </summary>
    //BufferFormat LITEFX_VULKAN_API getFormat(const VkFormat& format);

    /// <summary>
    /// 
    /// </summary>
    VkFormat LITEFX_VULKAN_API getFormat(const BufferFormat& format);

    /// <summary>
    /// 
    /// </summary>
    PolygonMode LITEFX_VULKAN_API getPolygonMode(const VkPolygonMode& mode);

    /// <summary>
    /// 
    /// </summary>
    VkPolygonMode LITEFX_VULKAN_API getPolygonMode(const PolygonMode& mode);

    /// <summary>
    /// 
    /// </summary>
    CullMode LITEFX_VULKAN_API getCullMode(const VkCullModeFlags& mode);

    /// <summary>
    /// 
    /// </summary>
    VkCullModeFlags LITEFX_VULKAN_API getCullMode(const CullMode& mode);

    /// <summary>
    /// 
    /// </summary>
    PrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const VkPrimitiveTopology& topology);

    /// <summary>
    /// 
    /// </summary>
    VkPrimitiveTopology LITEFX_VULKAN_API getPrimitiveTopology(const PrimitiveTopology& topology);

    /// <summary>
    /// 
    /// </summary>
    ShaderType LITEFX_VULKAN_API getShaderStage(const VkShaderStageFlagBits& shaderType);

    /// <summary>
    /// 
    /// </summary>
    VkShaderStageFlagBits LITEFX_VULKAN_API getShaderStage(const ShaderType& shaderType);

    /// <summary>
    /// 
    /// </summary>
    MultiSamplingLevel LITEFX_VULKAN_API getSamples(const VkSampleCountFlagBits& samples);

    /// <summary>
    /// 
    /// </summary>
    VkSampleCountFlagBits LITEFX_VULKAN_API getSamples(const MultiSamplingLevel& samples);

    // Forward declarations.
    class VulkanTexture;
    class VulkanSwapChain;
    class VulkanQueue;
    class VulkanDevice;
    class VulkanGraphicsAdapter;
    class VulkanBackend;
    class VulkanRenderPipeline;
    class VulkanRenderPipelineLayout;
    class VulkanRenderPass;
    class VulkanRasterizer;
    class VulkanViewport;
    class VulkanInputAssembler;
    class VulkanShaderModule;
    class VulkanShaderProgram;
    class VulkanCommandBuffer;
    class VulkanBufferSet;
    class VulkanBufferLayout;
    class VulkanBuffer;

}