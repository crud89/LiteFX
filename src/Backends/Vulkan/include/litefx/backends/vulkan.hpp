#pragma once

#include <litefx/core_types.hpp>
#include <litefx/rendering.hpp>

#if (defined _WIN32 || defined WINCE)
#  define VK_USE_PLATFORM_WIN32_KHR
#else 
#  pragma message ("Vulkan: No supported surface platform detected.")
#endif

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

	// Conversion helpers.
	Format LITEFX_VULKAN_API getFormat(const VkFormat& format);
	VkFormat LITEFX_VULKAN_API getFormat(const Format& format);

	// Forward declarations.
	class VulkanSwapChain;
	class VulkanSurface;
	class VulkanQueue;
	class VulkanDevice;
	class VulkanGraphicsAdapter;
	class VulkanBackend;

	class LITEFX_VULKAN_API VulkanSurface : public ISurface, public IResource<VkSurfaceKHR> {
		LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

	public:
		VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent = nullptr) noexcept;
		virtual ~VulkanSurface() noexcept;
	};

	class LITEFX_VULKAN_API VulkanQueue : public ICommandQueue, public IResource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl)
	
	public:
		VulkanQueue(const QueueType& type, const uint32_t id) noexcept;
		virtual ~VulkanQueue() noexcept;

	public:
		virtual uint32_t getId() const noexcept;

	public:
		virtual void initDeviceQueue(const VulkanDevice* device);

	public:
		virtual QueueType getType() const noexcept override;
	};

	class LITEFX_VULKAN_API VulkanDevice : public IGraphicsDevice, public IResource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl)

	public:
		VulkanDevice(const VkDevice device, SharedPtr<VulkanQueue> queue, const Array<String>& extensions = { });
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;
	};

	class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public IResource<VkPhysicalDevice> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl)

	public:
		VulkanGraphicsAdapter(const VkPhysicalDevice adapter);
		VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;
		VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) = delete;
		virtual ~VulkanGraphicsAdapter() noexcept;

	public:
		virtual String getName() const noexcept override;
		virtual uint32_t getVendorId() const noexcept override;
		virtual uint32_t getDeviceId() const noexcept override;
		virtual GraphicsAdapterType getType() const noexcept override;
		virtual uint32_t getDriverVersion() const noexcept override;
		virtual uint32_t getApiVersion() const noexcept override;

	public:
		virtual UniquePtr<IGraphicsDevice> createDevice(const ISurface* surface, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Array<String>& extensions = { }) const override;
		virtual SharedPtr<ICommandQueue> findQueue(const QueueType& queueType) const override;

	public:
		virtual bool validateDeviceExtensions(const Array<String>& extensions) const noexcept;
		virtual Array<String> getAvailableDeviceExtensions() const noexcept;
	};

	class LITEFX_VULKAN_API VulkanBackend : public RenderBackend, public IResource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);

	public:
		explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend();

	public:
		virtual Array<UniquePtr<IGraphicsAdapter>> getAdapters() const override;
		virtual UniquePtr<IGraphicsAdapter> getAdapter(Optional<uint32_t> adapterId = std::nullopt) const override;

	public:
		static bool validateExtensions(const Array<String>& extensions) noexcept;
		static Array<String> getAvailableExtensions() noexcept;
		static bool validateLayers(const Array<String>& validationLayers) noexcept;
		static Array<String> getValidationLayers() noexcept;

		// Platform specific code.
	public:
#ifdef VK_USE_PLATFORM_WIN32_KHR
		virtual UniquePtr<ISurface> createSurfaceWin32(HWND hwnd) const override;
#endif
	};

}