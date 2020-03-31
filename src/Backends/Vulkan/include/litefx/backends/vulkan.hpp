#pragma once

#include <litefx/core_types.hpp>
#include <litefx/rendering.hpp>

#include <vulkan/vulkan.h>

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

namespace LiteFX {
	namespace Rendering {
		namespace Backends {

			using namespace LiteFX::Rendering;

			class LITEFX_VULKAN_API VulkanBackend :
				public RenderBackend
			{
			private:
				VkInstance m_instance;

			public:
				explicit VulkanBackend(const App& app, const Array<String>& extensions);
				VulkanBackend(const VulkanBackend&) = delete;
				VulkanBackend(VulkanBackend&&) = delete;
				virtual ~VulkanBackend();

			protected:
				virtual void initialize(const Array<String>& extensions);
				virtual void release();

			public:
				virtual bool validateExtensions(const Array<String>& extensions) const;
				virtual Array<String> getAvailableExtensions() const;
				virtual bool validateLayers(const Array<String>& validationLayers) const;
				virtual Array<String> getValidationLayers() const;
			};
		}
	}
}