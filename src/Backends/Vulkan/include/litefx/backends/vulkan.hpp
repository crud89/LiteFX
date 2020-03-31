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

			class LITEFX_VULKAN_API VulkanDevice :
				public GraphicsAdapter
			{
			public:
				VulkanDevice(VkPhysicalDevice device);
				VulkanDevice(const VulkanDevice&) = delete;
				VulkanDevice(VulkanDevice&&) = delete;
				virtual ~VulkanDevice() = default;

			public:
				virtual String getName() const override;
				virtual uint32_t getVendorId() const override;
				virtual uint32_t getDeviceId() const override;
				virtual GraphicsAdapterType getType() const override;
				virtual uint32_t getDriverVersion() const override;
				virtual uint32_t getApiVersion() const override;

			public:
				virtual VkPhysicalDeviceProperties getProperties() const;
				virtual VkPhysicalDeviceFeatures getFeatures() const;
			};

			class LITEFX_VULKAN_API VulkanBackend :
				public RenderBackend
			{
			private:
				VkInstance m_instance;

			public:
				explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
				VulkanBackend(const VulkanBackend&) = delete;
				VulkanBackend(VulkanBackend&&) = delete;
				virtual ~VulkanBackend();

			public:
				virtual Array<UniquePtr<GraphicsAdapter>> getDevices() const override;
				virtual void useDevice(const GraphicsAdapter* device) override;

			protected:
				virtual void initialize(const Array<String>& extensions, const Array<String>& validationLayers);
				virtual void release();

			public:
				static bool validateExtensions(const Array<String>& extensions);
				static Array<String> getAvailableExtensions();
				static bool validateLayers(const Array<String>& validationLayers);
				static Array<String> getValidationLayers();
			};
		}
	}
}