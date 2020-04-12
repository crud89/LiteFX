#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_platform.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	// Conversion helpers.
	/// <summary>
	/// 
	/// </summary>
	Format LITEFX_VULKAN_API getFormat(const VkFormat& format);

	/// <summary>
	/// 
	/// </summary>
	VkFormat LITEFX_VULKAN_API getFormat(const Format& format);

	// Forward declarations.
	class VulkanTexture;
	class VulkanSwapChain;
	class VulkanQueue;
	class VulkanDevice;
	class VulkanGraphicsAdapter;
	class VulkanBackend;

	// Class definitions.
	
	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderModule : public IShaderModule, public IResource<VkShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl)

	public:
		VulkanShaderModule(const VulkanDevice* device, const ShaderType& type, const String& fileName, const String& entryPoint = "main");
		virtual ~VulkanShaderModule() noexcept;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept override;
		virtual const String& getFileName() const noexcept override;
		virtual const String& getEntryPoint() const noexcept override;
		virtual const ShaderType& getType() const noexcept override;

	public:
		virtual VkPipelineShaderStageCreateInfo getShaderStageDefinition() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanTexture : public ITexture, public IResource<VkImage> {
		LITEFX_IMPLEMENTATION(VulkanTextureImpl)

	public:
		//VulkanTexture() noexcept = default;
		VulkanTexture(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size);
		virtual ~VulkanTexture() noexcept;

	public:
		virtual Size2d getSize() const noexcept override;
		virtual Format getFormat() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanSwapChain : public ISwapChain, public IResource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl)

	public:
		VulkanSwapChain(const VulkanDevice* device, const Format& format = Format::B8G8R8A8_UNORM_SRGB);
		virtual ~VulkanSwapChain() noexcept;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
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

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDevice : public GraphicsDevice, public IResource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl)

	public:
		VulkanDevice(const VulkanGraphicsAdapter* adapter, const VulkanSurface* surface, VulkanQueue* deviceQueue, const Format& format, const Array<String>& extensions = { });
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

	public:
		virtual const Array<String>& getExtensions() const noexcept;
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;

	public:
		//virtual UniquePtr<ITexture> createTexture2d(const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const override;
		virtual UniquePtr<ITexture> makeTexture2d(VkImage image, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main") const override;

	public:
		virtual VkImageView vkCreateImageView(const VkImage& image, const Format& format) const;

	public:
		virtual bool validateDeviceExtensions(const Array<String>& extensions) const noexcept;
		virtual Array<String> getAvailableDeviceExtensions() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public IResource<VkPhysicalDevice> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl)

	public:
		VulkanGraphicsAdapter(VkPhysicalDevice adapter);
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
		virtual const ICommandQueue* findQueue(const QueueType& queueType) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackend : public RenderBackend, public IResource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);

	public:
		explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend();

	public:
		virtual Array<const IGraphicsAdapter*> getAdapters() const override;
		virtual const IGraphicsAdapter* getAdapter(Optional<uint32_t> adapterId = std::nullopt) const override;

	public:
		static bool validateExtensions(const Array<String>& extensions) noexcept;
		static Array<String> getAvailableExtensions() noexcept;
		static bool validateLayers(const Array<String>& validationLayers) noexcept;
		static Array<String> getValidationLayers() noexcept;
	};

}