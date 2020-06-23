#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_platform.hpp"
#include "vulkan_builders.hpp"
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanCommandBuffer : public ICommandBuffer, public IResource<VkCommandBuffer> {
		LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);

	public:
		VulkanCommandBuffer(const VulkanDevice* device);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
		virtual ~VulkanCommandBuffer() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPass : public IRenderPass, public IResource<VkRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
		LITEFX_BUILDER(VulkanRenderPassBuilder);

	public:
		VulkanRenderPass(const VulkanRenderPipeline& pipeline);
		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		virtual ~VulkanRenderPass() noexcept;

	public:
		virtual void addTarget(UniquePtr<IRenderTarget>&& target) override;
		virtual const Array<const IRenderTarget*> getTargets() const noexcept override;
		virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) override;
		virtual void begin() const override;
		virtual void end(const bool& present = false) override;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanInputAssembler : public InputAssembler {
		LITEFX_BUILDER(VulkanInputAssemblerBuilder);

	public:
		VulkanInputAssembler(const VulkanRenderPipelineLayout& layout) noexcept;
		VulkanInputAssembler(VulkanInputAssembler&&) noexcept = delete;
		VulkanInputAssembler(const VulkanInputAssembler&) noexcept = delete;
		virtual ~VulkanInputAssembler() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanViewport : public Viewport {
		LITEFX_BUILDER(VulkanViewportBuilder);

	public:
		VulkanViewport(const VulkanRenderPipelineLayout& layout) noexcept;
		VulkanViewport(VulkanViewport&&) noexcept = delete;
		VulkanViewport(const VulkanViewport&) noexcept = delete;
		virtual ~VulkanViewport() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRasterizer : public Rasterizer {
		LITEFX_BUILDER(VulkanRasterizerBuilder);

	public:
		VulkanRasterizer(const VulkanRenderPipelineLayout& layout) noexcept;
		VulkanRasterizer(VulkanRasterizer&&) noexcept = delete;
		VulkanRasterizer(const VulkanRasterizer&) noexcept = delete;
		virtual ~VulkanRasterizer() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineLayout : public RenderPipelineLayout, public IResource<VkPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutImpl);
		LITEFX_BUILDER(VulkanRenderPipelineLayoutBuilder);

	public:
		VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline);
		explicit VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline, const BufferLayout& bufferLayout);
		VulkanRenderPipelineLayout(VulkanRenderPipelineLayout&&) noexcept = delete;
		VulkanRenderPipelineLayout(const VulkanRenderPipelineLayout&) noexcept = delete;
		virtual ~VulkanRenderPipelineLayout() noexcept;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipeline : public RenderPipeline, public IResource<VkPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
		LITEFX_BUILDER(VulkanRenderPipelineBuilder);

	public:
		VulkanRenderPipeline(const IGraphicsDevice* device);
		explicit VulkanRenderPipeline(UniquePtr<IRenderPipelineLayout>&& layout, const IGraphicsDevice* device);
		VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;
		VulkanRenderPipeline(const VulkanRenderPipeline&) noexcept = delete;
		virtual ~VulkanRenderPipeline() noexcept;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderModule : public IShaderModule, public IResource<VkShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

	public:
		VulkanShaderModule(const VulkanDevice* device, const ShaderType& type, const String& fileName, const String& entryPoint = "main");
		virtual ~VulkanShaderModule() noexcept;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept override;
		virtual const String& getFileName() const noexcept override;
		virtual const String& getEntryPoint() const noexcept override;
		virtual const ShaderType& getType() const noexcept override;

	public:
		virtual VkPipelineShaderStageCreateInfo getShaderStageDefinition() const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderProgram : public IShaderProgram {
		LITEFX_IMPLEMENTATION(VulkanShaderProgramImpl);
		LITEFX_BUILDER(VulkanShaderProgramBuilder);

	public:
		VulkanShaderProgram(const VulkanRenderPipeline& pipeline);
		VulkanShaderProgram(VulkanShaderProgram&&) noexcept = delete;
		VulkanShaderProgram(const VulkanShaderProgram&) noexcept = delete;
		virtual ~VulkanShaderProgram() noexcept;

	public:
		virtual Array<const IShaderModule*> getModules() const noexcept override;
		virtual void use(UniquePtr<IShaderModule>&& module) override;
		virtual UniquePtr<IShaderModule> remove(const IShaderModule* module) override;
		virtual const VulkanDevice* getDevice() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanTexture : public ITexture, public IResource<VkImage> {
		LITEFX_IMPLEMENTATION(VulkanTextureImpl);

	public:
		//VulkanTexture() noexcept = default;
		VulkanTexture(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size);
		virtual ~VulkanTexture() noexcept;

	public:
		virtual Size2d getSize() const noexcept override;
		virtual Format getFormat() const noexcept override;

	public:
		virtual VkImageView getView() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanSwapChain : public ISwapChain, public IResource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);

	public:
		VulkanSwapChain(const VulkanDevice* device, const Format& format = Format::B8G8R8A8_UNORM_SRGB);
		virtual ~VulkanSwapChain() noexcept;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept override;
		virtual const Size2d& getBufferSize() const noexcept override;
		virtual size_t getWidth() const noexcept override;
		virtual size_t getHeight() const noexcept override;
		virtual const Format& getFormat() const noexcept override;
		virtual Array<const ITexture*> getFrames() const noexcept override;
		virtual UInt32 swapFrontBuffer() const override;

	public:
		virtual VkSemaphore getSemaphore() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanQueue : public ICommandQueue, public IResource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl);
	
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
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl);

	public:
		explicit VulkanDevice(const IGraphicsAdapter* adapter, const ISurface* surface, const Format& format, const Array<String>& extensions = { });
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

	public:
		virtual size_t getBufferWidth() const noexcept override;
		virtual size_t getBufferHeight() const noexcept override;
		virtual void wait() override;
		virtual void resize(int width, int height) override;

	public:
		virtual const Array<String>& getExtensions() const noexcept;
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;

	public:
		//virtual UniquePtr<ITexture> createTexture2d(const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const override;
		virtual UniquePtr<ITexture> makeTexture2d(VkImage image, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main") const override;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const override;

	public:
		virtual VkCommandPool getCommandPool() const noexcept;
		virtual VkImageView vkCreateImageView(const VkImage& image, const Format& format) const;

	public:
		virtual bool validateDeviceExtensions(const Array<String>& extensions) const noexcept;
		virtual Array<String> getAvailableDeviceExtensions() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public IResource<VkPhysicalDevice> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl);

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
		virtual ICommandQueue* findQueue(const QueueType& queueType) const override;
		virtual ICommandQueue* findQueue(const QueueType& queueType, const ISurface* forSurface) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackend : public RenderBackend, public IResource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);
		LITEFX_BUILDER(VulkanBackendBuilder);

	public:
		explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend();

	public:
		virtual Array<const IGraphicsAdapter*> listAdapters() const override;
		virtual const IGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const override;
		virtual const ISurface* getSurface() const noexcept override;
		virtual const IGraphicsAdapter* getAdapter() const noexcept override;

	public:
		virtual void use(const IGraphicsAdapter* adapter) override;
		virtual void use(UniquePtr<ISurface>&& surface) override;

	public:
		static bool validateExtensions(const Array<String>& extensions) noexcept;
		static Array<String> getAvailableExtensions() noexcept;
		static bool validateLayers(const Array<String>& validationLayers) noexcept;
		static Array<String> getValidationLayers() noexcept;
	};

}