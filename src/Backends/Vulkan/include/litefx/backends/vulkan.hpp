#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_builders.hpp"
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanDescriptorSet : public virtual VulkanRuntimeObject<VulkanDescriptorSetLayout>, public IDescriptorSet, public IResource<VkDescriptorPool> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetImpl);

	public:
		explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& bufferSet);
		VulkanDescriptorSet(VulkanDescriptorSet&&) = delete;
		VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
		virtual ~VulkanDescriptorSet() noexcept;

	public:
		// TODO: Find a way to abstract this.
		virtual const VkDescriptorSet swapBuffer();

	public:
		virtual UniquePtr<IConstantBuffer> makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1) const noexcept override;
		virtual UniquePtr<ITexture> makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const noexcept override;
		virtual UniquePtr<ISampler> makeSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const noexcept override;

		/// <inheritdoc />
		virtual void update(const IConstantBuffer* buffer) const override;
		
		/// <inheritdoc />
		virtual void update(const ITexture* texture) const override;
		
		/// <inheritdoc />
		virtual void update(const ISampler* sampler) const override;
		
		/// <inheritdoc />
		virtual void updateAll(const IConstantBuffer* buffer) const override;
		
		/// <inheritdoc />
		virtual void updateAll(const ITexture* texture) const override;
		
		/// <inheritdoc />
		virtual void updateAll(const ISampler* sampler) const override;
		
		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IRenderPass* renderPass, const UInt32& attachmentId) const override;
		
		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IImage* image) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanVertexBufferLayout : public virtual VulkanRuntimeObject<VulkanInputAssembler>, public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferLayoutImpl);
		LITEFX_BUILDER(VulkanVertexBufferLayoutBuilder);

	public:
		explicit VulkanVertexBufferLayout(const VulkanInputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding = 0);
		VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) = delete;
		VulkanVertexBufferLayout(const VulkanVertexBufferLayout&) = delete;
		virtual ~VulkanVertexBufferLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;
	
	public:
		virtual Array<const BufferAttribute*> getAttributes() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanIndexBufferLayout : public virtual VulkanRuntimeObject<VulkanInputAssembler>, public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferLayoutImpl);

	public:
		explicit VulkanIndexBufferLayout(const VulkanInputAssembler& inputAssembler, const IndexType& type);
		VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) = delete;
		VulkanIndexBufferLayout(const VulkanIndexBufferLayout&) = delete;
		virtual ~VulkanIndexBufferLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;

	public:
		virtual const IndexType& getIndexType() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDescriptorLayout : public virtual VulkanRuntimeObject<VulkanDescriptorSetLayout>, public IDescriptorLayout {
		LITEFX_IMPLEMENTATION(VulkanDescriptorLayoutImpl);

	public:
		explicit VulkanDescriptorLayout(const VulkanDescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize);
		VulkanDescriptorLayout(VulkanDescriptorLayout&&) = delete;
		VulkanDescriptorLayout(const VulkanDescriptorLayout&) = delete;
		virtual ~VulkanDescriptorLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;

	public:
		virtual DescriptorType getDescriptorType() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDescriptorSetLayout : public virtual VulkanRuntimeObject<VulkanRenderPipelineLayout>, public IDescriptorSetLayout, public IResource<VkDescriptorSetLayout> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetLayoutImpl);
		LITEFX_BUILDER(VulkanDescriptorSetLayoutBuilder);

	public:
		explicit VulkanDescriptorSetLayout(const VulkanRenderPipelineLayout& pipelineLayout, const UInt32& id, const ShaderStage& stages);
		VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&&) = delete;
		VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
		virtual ~VulkanDescriptorSetLayout() noexcept;

	public:
		virtual Array<const IDescriptorLayout*> getLayouts() const noexcept override;
		virtual const IDescriptorLayout* getLayout(const UInt32& binding) const noexcept override;
		virtual const UInt32& getSetId() const noexcept override;
		virtual const ShaderStage& getShaderStages() const noexcept override;
		virtual UniquePtr<IDescriptorSet> createBufferPool() const noexcept override;

	public:
		virtual UInt32 uniforms() const noexcept override;
		virtual UInt32 storages() const noexcept override;
		virtual UInt32 images() const noexcept override;
		virtual UInt32 samplers() const noexcept override;
		virtual UInt32 inputAttachments() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanCommandBuffer : public virtual VulkanRuntimeObject<VulkanQueue>, public ICommandBuffer, public IResource<VkCommandBuffer> {
		LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);

	public:
		explicit VulkanCommandBuffer(const VulkanQueue& queue);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
		virtual ~VulkanCommandBuffer() noexcept;

	public:
		virtual void begin() const override;
		virtual void end() const override;
		virtual void submit(const bool& waitForQueue = false) const override;
		virtual void submit(const Array<VkSemaphore>& waitForSemaphores, const Array<VkPipelineStageFlags>& waitForStages, const Array<VkSemaphore>& signalSemaphores = { }, const bool& waitForQueue = false) const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanInputAssembler : public virtual VulkanRuntimeObject<VulkanRenderPipeline>, public InputAssembler {
		LITEFX_BUILDER(VulkanInputAssemblerBuilder);

	public:
		explicit VulkanInputAssembler(const VulkanRenderPipeline& pipeline) noexcept;
		VulkanInputAssembler(VulkanInputAssembler&&) noexcept = delete;
		VulkanInputAssembler(const VulkanInputAssembler&) noexcept = delete;
		virtual ~VulkanInputAssembler() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRasterizer : public virtual VulkanRuntimeObject<VulkanRenderPipeline>, public Rasterizer {
		LITEFX_BUILDER(VulkanRasterizerBuilder);

	public:
		explicit VulkanRasterizer(const VulkanRenderPipeline& pipeline) noexcept;
		VulkanRasterizer(VulkanRasterizer&&) noexcept = delete;
		VulkanRasterizer(const VulkanRasterizer&) noexcept = delete;
		virtual ~VulkanRasterizer() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineLayout : public virtual VulkanRuntimeObject<VulkanRenderPipeline>, public IRenderPipelineLayout, public IResource<VkPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutImpl);
		LITEFX_BUILDER(VulkanRenderPipelineLayoutBuilder);

	public:
		explicit VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline);
		VulkanRenderPipelineLayout(VulkanRenderPipelineLayout&&) noexcept = delete;
		VulkanRenderPipelineLayout(const VulkanRenderPipelineLayout&) noexcept = delete;
		virtual ~VulkanRenderPipelineLayout() noexcept;

		// IRequiresInitialization
	public:
		virtual bool isInitialized() const noexcept override;

		// IRenderPipelineLayout
	public:
		virtual void initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts) override;

	public:
		virtual const IShaderProgram* getProgram() const noexcept override;
		virtual Array<const IDescriptorSetLayout*> getDescriptorSetLayouts() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPass : public virtual VulkanRuntimeObject<VulkanDevice>, public IRenderPass, public IResource<VkRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
		LITEFX_BUILDER(VulkanRenderPassBuilder);

	public:
		explicit VulkanRenderPass(const IGraphicsDevice& device);	// Adapter for builder interface.
		explicit VulkanRenderPass(const VulkanDevice& device);
		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		virtual ~VulkanRenderPass() noexcept;

	public:
		virtual const VulkanCommandBuffer* getVkCommandBuffer() const noexcept;

	public:
		virtual const ICommandBuffer* getCommandBuffer() const noexcept override;
		virtual const UInt32 getCurrentBackBuffer() const override;
		virtual void addTarget(UniquePtr<IRenderTarget>&& target) override;
		virtual const Array<const IRenderTarget*> getTargets() const noexcept override;
		virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) override;
		virtual Array<const IRenderPipeline*> getPipelines() const noexcept override;
		virtual const IRenderPipeline* getPipeline(const UInt32& id) const noexcept override;
		virtual void addPipeline(UniquePtr<IRenderPipeline>&& pipeline) override;
		virtual void removePipeline(const UInt32& id) override;
		virtual void setDependency(const IRenderPass* renderPass = nullptr) override;
		virtual const IRenderPass* getDependency() const noexcept override;
		virtual void begin() const override;
		virtual void end(const bool& present = false) override;
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const override;
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const override;
		virtual const IImage* getAttachment(const UInt32& attachmentId) const override;
		virtual void resetFramebuffer() override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipeline : public virtual VulkanRuntimeObject<VulkanRenderPass>, public IRenderPipeline, public IResource<VkPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
		LITEFX_BUILDER(VulkanRenderPipelineBuilder);

	public:
		explicit VulkanRenderPipeline(const VulkanRenderPass& renderPass, const UInt32& id, const String& name = "");
		VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;
		VulkanRenderPipeline(const VulkanRenderPipeline&) noexcept = delete;
		virtual ~VulkanRenderPipeline() noexcept;

		// IRequiresInitialization
	public:
		virtual bool isInitialized() const noexcept override;

		// IRenderPipeline
	public:
		/// <inheritdoc />
		virtual const String& name() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& id() const noexcept override;

	public:
		virtual void initialize(UniquePtr<IRenderPipelineLayout>&& layout, SharedPtr<IInputAssembler> inputAssembler, SharedPtr<IRasterizer> rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors) override;

	public:
		virtual const IRenderPipelineLayout* getLayout() const noexcept override;
		virtual SharedPtr<IInputAssembler> getInputAssembler() const noexcept override;
		virtual SharedPtr<IRasterizer> getRasterizer() const noexcept override;
		virtual Array<const IViewport*> getViewports() const noexcept override;
		virtual Array<const IScissor*> getScissors() const noexcept override;

	public:
		virtual UniquePtr<IVertexBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const override;
		virtual UniquePtr<IIndexBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const override;
		virtual UniquePtr<IDescriptorSet> makeBufferPool(const UInt32& bufferSet) const override;
		virtual void bind(const IVertexBuffer* buffer) const override;
		virtual void bind(const IIndexBuffer* buffer) const override;
		virtual void bind(IDescriptorSet* buffer) const override;
		virtual void use() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderModule : public virtual VulkanRuntimeObject<VulkanDevice>, public IShaderModule, public IResource<VkShaderModule> {
		LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

	public:
		explicit VulkanShaderModule(const VulkanDevice& device, const ShaderStage& type, const String& fileName, const String& entryPoint = "main");
		VulkanShaderModule(const VulkanShaderModule&) noexcept = delete;
		VulkanShaderModule(VulkanShaderModule&&) noexcept = delete;
		virtual ~VulkanShaderModule() noexcept;

	public:
		virtual const String& getFileName() const noexcept override;
		virtual const String& getEntryPoint() const noexcept override;
		virtual const ShaderStage& getType() const noexcept override;

	public:
		virtual VkPipelineShaderStageCreateInfo getShaderStageDefinition() const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderProgram : public virtual VulkanRuntimeObject<VulkanRenderPipelineLayout>, public IShaderProgram {
		LITEFX_IMPLEMENTATION(VulkanShaderProgramImpl);
		LITEFX_BUILDER(VulkanShaderProgramBuilder);

	public:
		explicit VulkanShaderProgram(const VulkanRenderPipelineLayout& pipelineLayout);
		VulkanShaderProgram(VulkanShaderProgram&&) noexcept = delete;
		VulkanShaderProgram(const VulkanShaderProgram&) noexcept = delete;
		virtual ~VulkanShaderProgram() noexcept;

	public:
		virtual Array<const IShaderModule*> getModules() const noexcept override;
		virtual void use(UniquePtr<IShaderModule>&& module) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanSampler : public VulkanRuntimeObject<VulkanDevice>, public virtual Sampler, public IResource<VkSampler> {
		LITEFX_IMPLEMENTATION(VulkanSamplerImpl);

	public:
		explicit VulkanSampler(
			const VulkanDevice& device,
			const IDescriptorLayout* layout,
			const FilterMode& magFilter = FilterMode::Nearest, 
			const FilterMode& minFilter = FilterMode::Nearest, 
			const BorderMode& borderU = BorderMode::Repeat, 
			const BorderMode& borderV = BorderMode::Repeat, 
			const BorderMode& borderW = BorderMode::Repeat, 
			const MipMapMode& mipMapMode = MipMapMode::Nearest, 
			const Float& mipMapBias = 0.f, 
			const Float& maxLod = std::numeric_limits<Float>::max(), 
			const Float& minLod = 0.f, 
			const Float& anisotropy = 0.f);
		VulkanSampler(VulkanSampler&&) = delete;
		VulkanSampler(const VulkanSampler&) = delete;
		virtual ~VulkanSampler() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanSwapChain : public virtual VulkanRuntimeObject<VulkanDevice>, public ISwapChain, public IResource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);

	public:
		explicit VulkanSwapChain(const VulkanDevice& device, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Format& format = Format::B8G8R8A8_SRGB);
		VulkanSwapChain(const VulkanSwapChain&) = delete;
		VulkanSwapChain(VulkanSwapChain&&) = delete;
		virtual ~VulkanSwapChain() noexcept;

	public:
		virtual const Size2d& getBufferSize() const noexcept override;
		virtual size_t getWidth() const noexcept override;
		virtual size_t getHeight() const noexcept override;
		virtual const Format& getFormat() const noexcept override;
		virtual UInt32 swapBackBuffer() const override;
		virtual void reset(const Size2d& frameBufferSize, const UInt32& frameBuffers) override;
		virtual UInt32 getBuffers() const noexcept override;

	public:
		virtual Array<const IImage*> getFrames() const noexcept;
		virtual const VkSemaphore& getCurrentSemaphore() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanQueue : public virtual VulkanRuntimeObject<VulkanDevice>, public ICommandQueue, public IResource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl);
	
	public:
		explicit VulkanQueue(const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId);
		VulkanQueue(const VulkanQueue&) = delete;
		VulkanQueue(VulkanQueue&&) = delete;
		virtual ~VulkanQueue() noexcept;

	public:
		virtual VkCommandPool getCommandPool() const noexcept;
		virtual UInt32 getFamilyId() const noexcept;
		virtual UInt32 getQueueId() const noexcept;

	public:
		virtual bool isBound() const noexcept override;
		virtual QueuePriority getPriority() const noexcept override;
		virtual QueueType getType() const noexcept override;

	public:
		virtual void bind() override;
		virtual void release() override;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanGraphicsAdapter : public IGraphicsAdapter, public IResource<VkPhysicalDevice> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsAdapterImpl);

	public:
		explicit VulkanGraphicsAdapter(VkPhysicalDevice adapter);
		VulkanGraphicsAdapter(const VulkanGraphicsAdapter&) = delete;
		VulkanGraphicsAdapter(VulkanGraphicsAdapter&&) = delete;
		virtual ~VulkanGraphicsAdapter() noexcept;

	public:
		virtual String getName() const noexcept override;
		virtual UInt32 getVendorId() const noexcept override;
		virtual UInt32 getDeviceId() const noexcept override;
		virtual GraphicsAdapterType getType() const noexcept override;
		virtual UInt32 getDriverVersion() const noexcept override;
		virtual UInt32 getApiVersion() const noexcept override;
		virtual UInt64 getDedicatedMemory() const noexcept override;
	};

	/// <summary>
	/// Represents a Vulkan surface.
	/// </summary>
	class LITEFX_VULKAN_API VulkanSurface : public ISurface, public IResource<VkSurfaceKHR> {
		LITEFX_IMPLEMENTATION(VulkanSurfaceImpl)

	public:
		/// <summary>
		/// Initializes the surface from a surface and instance handle.
		/// </summary>
		/// <param name="surface">The handle of the Vulkan surface.</param>
		/// <param name="instance">The handle of the parent instance.</param>
		VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& instance);
		VulkanSurface(const VulkanSurface&) = delete;
		VulkanSurface(VulkanSurface&&) = delete;
		virtual ~VulkanSurface() noexcept;

	public:
		/// <summary>
		/// Returns the handle of the backend, the surface has been created from.
		/// </summary>
		/// <returns>The handle of the backend, the surface has been created from.</returns>
		const VkInstance& instance() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDevice : public IGraphicsDevice, public IResource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl);

	public:
		explicit VulkanDevice(const VulkanGraphicsAdapter& adapter, const VulkanSurface& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Array<String>& extensions = { });
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

	public:
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;
		virtual size_t getBufferWidth() const noexcept override;
		virtual size_t getBufferHeight() const noexcept override;
		virtual const ICommandQueue* graphicsQueue() const noexcept override;
		virtual const ICommandQueue* transferQueue() const noexcept override;
		virtual const ICommandQueue* bufferQueue() const noexcept override;
		virtual void wait() override;
		virtual void resize(int width, int height) override;
		virtual UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements = 1) const override;
		virtual UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements = 1) const override;
		virtual UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const override;
		virtual UniquePtr<IConstantBuffer> createConstantBuffer(const IDescriptorLayout* layout, const BufferUsage& usage, const UInt32& elements) const override;
		virtual UniquePtr<IImage> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<IImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<ITexture> createTexture(const IDescriptorLayout* layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<ISampler> createSampler(const IDescriptorLayout* layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") const override;

	public:
		virtual const Array<String>& getExtensions() const noexcept;
		virtual bool validateDeviceExtensions(const Array<String>& extensions) const noexcept;
		virtual Array<String> getAvailableDeviceExtensions() const noexcept; 
		virtual Array<UniquePtr<IImage>> createSwapChainImages(const ISwapChain* swapChain) const;

	public:
		VulkanRenderPassBuilder buildRenderPass() const;
	};

	/// <summary>
	/// Defines a rendering backend that creates a Vulkan device.
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackend : public IRenderBackend<VulkanGraphicsAdapter, VulkanSurface, VulkanDevice>, public IResource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);
		LITEFX_BUILDER(VulkanBackendBuilder);

	public:
		/// <summary>
		/// Initializes a new vulkan rendering backend.
		/// </summary>
		/// <param name="app">An instance of the app that owns the backend.</param>
		/// <param name="extensions">A set of extensions to enable on the graphics device.</param>
		/// <param name="validationLayers">A set of validation layers to enable on the rendering backend.</param>
		explicit VulkanBackend(const App& app, const Array<String>& extensions = { }, const Array<String>& validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend() noexcept;

		// IBackend
	public:
		virtual BackendType getType() const noexcept override;

		// IRenderBackend
	public:
		/// <inheritdoc />
		virtual Array<const VulkanGraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const VulkanGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const override;

		// VulkanBackend
	public:
		/// <summary>
		/// A callback that creates a surface from a Vulkan instance.
		/// </summary>
		typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

	public:
		/// <summary>
		/// Returns the validation layers that are enabled on the backend.
		/// </summary>
		/// <returns>An array of validation layers that are enabled on the backend.</returns>
		virtual const Array<String> getEnabledValidationLayers() const noexcept;

		/// <summary>
		/// Creates a surface using the <paramref name="predicate" /> callback.
		/// </summary>
		/// <param name="predicate">A callback that gets called with the backend instance handle and creates the surface instance</param>
		/// <returns>The instance of the created surface.</returns>
		/// <seealso cref="surface_callback" />
		UniquePtr<VulkanSurface> createSurface(surface_callback predicate);

#ifdef VK_USE_PLATFORM_WIN32_KHR
		/// <summary>
		/// Creates a surface on a window handle.
		/// </summary>
		/// <param name="hwnd">The window handle on which the surface should be created.</param>
		/// <returns>The instance of the created surface.</returns>
		UniquePtr<VulkanSurface> createSurface(const HWND& hwnd);
#endif

	public:
		/// <summary>
		/// Returns <c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.
		/// </summary>
		/// <returns><c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.</returns>
		/// <seealso cref="getAvailableExtensions" />
		static bool validateExtensions(const Array<String>& extensions) noexcept;

		/// <summary>
		/// Returns a list of available extensions.
		/// </summary>
		/// <returns>A list of available extensions.</returns>
		static Array<String> getAvailableExtensions() noexcept;

		/// <summary>
		/// Returns <c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.
		/// </summary>
		/// <returns><c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.</returns>
		/// <seealso cref="getValidationLayers" />
		static bool validateLayers(const Array<String>& validationLayers) noexcept;

		/// <summary>
		/// Returns a list of available validation layers.
		/// </summary>
		/// <returns>A list of available validation layers.</returns>
		static Array<String> getValidationLayers() noexcept;
	};

}