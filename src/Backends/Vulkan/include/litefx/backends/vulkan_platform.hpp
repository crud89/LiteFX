#pragma once

#include <litefx/rendering.hpp>

#if (defined _WIN32 || defined WINCE)
#  define VK_USE_PLATFORM_WIN32_KHR
#else 
#  pragma message ("Vulkan: No supported surface platform detected.")
#endif

#include <vulkan/vulkan.h>

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class VulkanBackend;

	class LITEFX_VULKAN_API VulkanSurface : public ISurface, public IResource<VkSurfaceKHR> {
		LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

	protected:
		VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent = nullptr) noexcept;

	public:
		virtual ~VulkanSurface() noexcept;

	public:
		static UniquePtr<ISurface> createSurface(const VulkanBackend& backend, std::function<VkSurfaceKHR(const VkInstance&)> predicate);

#ifdef VK_USE_PLATFORM_WIN32_KHR
		static UniquePtr<ISurface> createSurface(const VkInstance& instance, const HWND& hwnd);
#endif
	};

}