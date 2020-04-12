#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class VulkanBackend;

	class LITEFX_VULKAN_API VulkanSurface : public ISurface, public IResource<VkSurfaceKHR> {
		LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

	public:
		typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

	protected:
		VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent = nullptr) noexcept;

	public:
		virtual ~VulkanSurface() noexcept;

	public:
		static UniquePtr<ISurface> createSurface(const VulkanBackend& backend, surface_callback predicate);

#ifdef VK_USE_PLATFORM_WIN32_KHR
		static UniquePtr<ISurface> createSurface(const VkInstance& instance, const HWND& hwnd);
#endif
	};

}