#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_platform.hpp"
#include "vulkan_builders.hpp"
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanRuntimeObject {
		LITEFX_IMPLEMENTATION(VulkanRuntimeObjectImpl);

	public:
		VulkanRuntimeObject(const VulkanDevice* device);
		VulkanRuntimeObject(VulkanRuntimeObject&&) = delete;
		VulkanRuntimeObject(const VulkanRuntimeObject&) = delete;
		virtual ~VulkanRuntimeObject() noexcept;

	public:
		virtual const VulkanDevice* getDevice() const noexcept;
	};

	class LITEFX_VULKAN_API VulkanBuffer : public Buffer, public IResource<VkBuffer> {
	public:
		VulkanBuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const UInt32& elementSize, const UInt32& binding) : 
			Buffer(type, elements, elementSize, binding), IResource(buffer) { }
		VulkanBuffer(VulkanBuffer&&) = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		virtual ~VulkanBuffer() noexcept = default;
	};

	class LITEFX_VULKAN_API VulkanBufferPool : public virtual VulkanRuntimeObject, public IBufferPool, public IResource<VkDescriptorPool> {
		LITEFX_IMPLEMENTATION(VulkanBufferPoolImpl);

	public:
		VulkanBufferPool(const VulkanBufferSet& bufferSet, const BufferUsage& usage);
		VulkanBufferPool(VulkanBufferPool&&) = delete;
		VulkanBufferPool(const VulkanBufferPool&) = delete;
		virtual ~VulkanBufferPool() noexcept;

	public:
		virtual IBuffer* getBuffer(const UInt32& binding) const noexcept override;
		virtual const BufferUsage& getUsage() const noexcept override;

	public:
		virtual const VkDescriptorSet getDescriptorSet() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBufferLayout : public virtual VulkanRuntimeObject, public BufferLayout {
		LITEFX_IMPLEMENTATION(VulkanBufferLayoutImpl);
		LITEFX_BUILDER(VulkanBufferLayoutBuilder);

	public:
		VulkanBufferLayout(const VulkanBufferSet& bufferSet, const BufferType& type, const size_t& elementSize, const UInt32& binding = 0);
		VulkanBufferLayout(VulkanBufferLayout&&) = delete;
		VulkanBufferLayout(const VulkanBufferLayout&) = delete;
		virtual ~VulkanBufferLayout() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBufferSet : public virtual VulkanRuntimeObject, public BufferSet, public IResource<VkDescriptorSetLayout> {
		LITEFX_IMPLEMENTATION(VulkanBufferSetImpl);
		LITEFX_BUILDER(VulkanBufferSetBuilder);

	public:
		VulkanBufferSet(const VulkanInputAssembler& inputAssembler, const BufferSetType& type, const UInt32& id = 0);
		VulkanBufferSet(VulkanBufferSet&&) = delete;
		VulkanBufferSet(const VulkanBufferSet&) = delete;
		virtual ~VulkanBufferSet() noexcept;

	public:
		virtual UniquePtr<IBufferPool> createBufferPool(const BufferUsage& usage) const noexcept override;

	public:
		virtual void create();
		virtual const Array<VkDescriptorPoolSize> getPoolSizes() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanCommandBuffer : public ICommandBuffer, public IResource<VkCommandBuffer> {
		LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);

	public:
		VulkanCommandBuffer(const VulkanQueue* queue);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
		virtual ~VulkanCommandBuffer() noexcept;

	public:
		virtual const ICommandQueue* getQueue() const noexcept override;

	public:
		virtual void begin() const override;
		virtual void end() const override;
		virtual void submit(const bool& waitForQueue = false) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPass : public virtual VulkanRuntimeObject, public IRenderPass, public IResource<VkRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
		LITEFX_BUILDER(VulkanRenderPassBuilder);

	public:
		VulkanRenderPass(const VulkanRenderPipeline& pipeline);
		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		virtual ~VulkanRenderPass() noexcept;

	public:
		virtual const ICommandBuffer* getCommandBuffer() const noexcept override;

	public:
		virtual void addTarget(UniquePtr<IRenderTarget>&& target) override;
		virtual const Array<const IRenderTarget*> getTargets() const noexcept override;
		virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) override;
		virtual void begin() const override;
		virtual void end(const bool& present = false) override;
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const override;
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const override;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanInputAssembler : public virtual VulkanRuntimeObject, public InputAssembler {
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
	class LITEFX_VULKAN_API VulkanViewport : public virtual VulkanRuntimeObject, public Viewport {
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
	class LITEFX_VULKAN_API VulkanRasterizer : public virtual VulkanRuntimeObject, public Rasterizer {
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
	class LITEFX_VULKAN_API VulkanRenderPipelineLayout : public virtual VulkanRuntimeObject, public RenderPipelineLayout, public IResource<VkPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutImpl);
		LITEFX_BUILDER(VulkanRenderPipelineLayoutBuilder);

	public:
		VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline);
		explicit VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline, const VulkanBufferLayout& bufferLayout);
		VulkanRenderPipelineLayout(VulkanRenderPipelineLayout&&) noexcept = delete;
		VulkanRenderPipelineLayout(const VulkanRenderPipelineLayout&) noexcept = delete;
		virtual ~VulkanRenderPipelineLayout() noexcept;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipeline : public virtual VulkanRuntimeObject, public RenderPipeline, public IResource<VkPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
		LITEFX_BUILDER(VulkanRenderPipelineBuilder);

	public:
		VulkanRenderPipeline(const IGraphicsDevice* device);	// Adapter for builder interface.
		VulkanRenderPipeline(const VulkanDevice* device);
		explicit VulkanRenderPipeline(UniquePtr<IRenderPipelineLayout>&& layout, const VulkanDevice* device);
		VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;
		VulkanRenderPipeline(const VulkanRenderPipeline&) noexcept = delete;
		virtual ~VulkanRenderPipeline() noexcept;

	public:
		virtual void bind(const IBuffer* buffer) const override;
		virtual void bind(const IBufferPool* buffer) const override;

	public:
		virtual void create();
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderModule : public virtual VulkanRuntimeObject, public IShaderModule, public IResource<VkShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

	public:
		VulkanShaderModule(const VulkanDevice* device, const ShaderType& type, const String& fileName, const String& entryPoint = "main");
		virtual ~VulkanShaderModule() noexcept;

	public:
		virtual const String& getFileName() const noexcept override;
		virtual const String& getEntryPoint() const noexcept override;
		virtual const ShaderType& getType() const noexcept override;

	public:
		virtual VkPipelineShaderStageCreateInfo getShaderStageDefinition() const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderProgram : public virtual VulkanRuntimeObject, public IShaderProgram {
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
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanTexture : public virtual VulkanRuntimeObject, public ITexture, public IResource<VkImage> {
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
	class LITEFX_VULKAN_API VulkanSwapChain : public virtual VulkanRuntimeObject, public ISwapChain, public IResource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);

	public:
		VulkanSwapChain(const VulkanDevice* device, const Format& format = Format::B8G8R8A8_UNORM_SRGB);
		virtual ~VulkanSwapChain() noexcept;

	public:
		virtual const Size2d& getBufferSize() const noexcept override;
		virtual size_t getWidth() const noexcept override;
		virtual size_t getHeight() const noexcept override;
		virtual const Format& getFormat() const noexcept override;
		virtual Array<const ITexture*> getFrames() const noexcept override;
		virtual UInt32 swapBackBuffer() const override;
		virtual void reset() override;

	public:
		virtual VkSemaphore getSemaphore() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanQueue : public ICommandQueue, public IResource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl);
	
	public:
		VulkanQueue(const QueueType& type, const uint32_t id);
		virtual ~VulkanQueue() noexcept;

	public:
		virtual VkCommandPool getCommandPool() const noexcept;

	public:
		virtual UInt32 getId() const noexcept override;
		virtual QueueType getType() const noexcept override;
		virtual const IGraphicsDevice* getDevice() const noexcept override;

	public:
		virtual void bindDevice(const IGraphicsDevice* device) override;
		virtual void release() override;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDevice : public GraphicsDevice, public IResource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl);

	public:
		explicit VulkanDevice(const IRenderBackend* backend, const Format& format, const Array<String>& extensions = { });
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

	public:
		virtual size_t getBufferWidth() const noexcept override;
		virtual size_t getBufferHeight() const noexcept override;
		virtual void wait() override;
		virtual void resize(int width, int height) override;
		virtual UniquePtr<IBuffer> createBuffer(const IBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const override;
		virtual UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const UInt32& elementSize, const UInt32& elements, const UInt32& binding) const override;

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
		virtual const Array<String>& getEnabledValidationLayers() const noexcept;

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