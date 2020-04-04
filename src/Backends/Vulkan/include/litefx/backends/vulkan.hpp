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

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanDevice : public IGraphicsDevice, public IResource<VkDevice> {
	public:
		VulkanDevice(const VkDevice device);
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice();
	};

	class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public IResource<VkPhysicalDevice> {
	public:
		VulkanGraphicsAdapter(const VkPhysicalDevice adapter);
		VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;
		VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) = delete;
		virtual ~VulkanGraphicsAdapter() = default;

	public:
		virtual String getName() const noexcept override;
		virtual uint32_t getVendorId() const noexcept override;
		virtual uint32_t getDeviceId() const noexcept override;
		virtual GraphicsAdapterType getType() const noexcept override;
		virtual uint32_t getDriverVersion() const noexcept override;
		virtual uint32_t getApiVersion() const noexcept override;

	public:
		virtual UniquePtr<IGraphicsDevice> createDevice() const override;

	public:
		virtual VkPhysicalDeviceProperties getProperties() const noexcept;
		virtual VkPhysicalDeviceFeatures getFeatures() const noexcept;
	};

	class LITEFX_VULKAN_API VulkanBackend : public RenderBackend, public IResource<VkInstance> {
	public:
		explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend();

	public:
		virtual Array<UniquePtr<IGraphicsAdapter>> getAdapters() const override;
		virtual UniquePtr<IGraphicsAdapter> getAdapter(Optional<uint32_t> adapterId = std::nullopt) const override;
		virtual UniquePtr<ICommandQueue> createQueue(const QueueType& queueType) const override;
		//virtual UniquePtr<ISurface> createSurface() const override;
		//virtual void useAdapter(const GraphicsAdapter* adapter) const override;

	protected:
		virtual void initialize(const Array<String>& extensions, const Array<String>& validationLayers);
		virtual void release();

	public:
		static bool validateExtensions(const Array<String>& extensions) noexcept;
		static Array<String> getAvailableExtensions() noexcept;
		static bool validateLayers(const Array<String>& validationLayers) noexcept;
		static Array<String> getValidationLayers() noexcept;
	};

}