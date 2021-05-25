#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanDescriptorSet : public virtual VulkanRuntimeObject<VulkanDescriptorSetLayout>, public IDescriptorSet<IVulkanConstantBuffer, IVulkanTexture, IVulkanSampler, IVulkanImage, IVulkanBuffer>, public IResource<VkDescriptorPool> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetImpl);

	public:
		explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& bufferSet);
		VulkanDescriptorSet(VulkanDescriptorSet&&) = delete;
		VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
		virtual ~VulkanDescriptorSet() noexcept;

	public:
		/// <inheritdoc />
		virtual UniquePtr<IVulkanConstantBuffer> makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1) const noexcept override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanTexture> makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const noexcept override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanSampler> makeSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const noexcept override;

		/// <inheritdoc />
		virtual void update(const IVulkanConstantBuffer& buffer) const override;
		
		/// <inheritdoc />
		virtual void update(const IVulkanTexture& texture) const override;
		
		/// <inheritdoc />
		virtual void update(const IVulkanSampler& sampler) const override;
				
		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IVulkanImage& image) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<VulkanDescriptorSetLayoutBuilder, VulkanDescriptorSetLayout, VulkanRenderPipelineLayoutBuilder> {
	public:
		using DescriptorSetLayoutBuilder<VulkanDescriptorSetLayoutBuilder, VulkanDescriptorSetLayout, VulkanRenderPipelineLayoutBuilder>::DescriptorSetLayoutBuilder;

	public:
		virtual VulkanRenderPipelineLayoutBuilder& go() override;

	public:
		virtual VulkanDescriptorSetLayoutBuilder& addDescriptor(UniquePtr<IDescriptorLayout>&& layout) override;
		virtual VulkanDescriptorSetLayoutBuilder& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize) override;
	};

	/// <summary>
	/// Implements a Vulkan vertex buffer layout.
	/// </summary>
	class LITEFX_VULKAN_API VulkanVertexBufferLayout : public virtual VulkanRuntimeObject<VulkanInputAssembler>, public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferLayoutImpl);
		LITEFX_BUILDER(VulkanVertexBufferLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new vertex buffer layout.
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the vertex buffer layout is initialized for.</param>
		/// <param name="vertexSize">The size of a single vertex.</param>
		/// <param name="binding">The binding point of the vertex buffers using this layout.</param>
		explicit VulkanVertexBufferLayout(const VulkanInputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding = 0);
		VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) = delete;
		VulkanVertexBufferLayout(const VulkanVertexBufferLayout&) = delete;
		virtual ~VulkanVertexBufferLayout() noexcept;

		// IVertexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual Array<const BufferAttribute*> attributes() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanVertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder>::VertexBufferLayoutBuilder;

	public:
		virtual VulkanVertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

		/// <summary>
		/// 
		/// </summary>
		/// <reamrks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </reamrks>
		virtual VulkanVertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
		virtual VulkanVertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	};

	/// <summary>
	/// Implements a Vulkan index buffer layout.
	/// </summary>
	class LITEFX_VULKAN_API VulkanIndexBufferLayout : public virtual VulkanRuntimeObject<VulkanInputAssembler>, public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new index buffer layout
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the index buffer layout is initialized for.</param>
		/// <param name="type">The type of the indices within the index buffer.</param>
		explicit VulkanIndexBufferLayout(const VulkanInputAssembler& inputAssembler, const IndexType& type);
		VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) = delete;
		VulkanIndexBufferLayout(const VulkanIndexBufferLayout&) = delete;
		virtual ~VulkanIndexBufferLayout() noexcept;

		// IIndexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual const IndexType& indexType() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
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

		// IDescriptorLayout interface.
	public:
		virtual const DescriptorType& descriptorType() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
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
		/// <summary>
		/// The size of each descriptor pool.
		/// </summary>
		/// <remarks>
		/// Descriptors are allocated from descriptor pools in Vulkan. Each descriptor pool has a number of descriptor sets it can hand out. Before allocating a new descriptor set
		/// the layout tries to find an unused descriptor set, that it can hand out. If there are no free descriptor sets, the layout tries to allocate a new one. This is only possible
		/// if the descriptor pool is not yet full, in which case a new pool needs to be created. All created pools are cached and destroyed, if the layout itself gets destroyed, 
		/// causing all descriptor sets allocated from the layout to be invalidated. 
		/// 
		/// In general, if the number of required descriptor sets can be pre-calculated, it should be used as a pool size. Otherwise there is a trade-off to be made, based on the 
		/// frequency of which new descriptor sets are required. A small pool size is more memory efficient, but can have a significant runtime cost, as long as new allocations happen
		/// and no descriptor sets can be reused. A large pool size on the other hand is faster, whilst it may leave a large chunk of descriptor sets unallocated. Keep in mind, that the 
		/// layout might not be the only active layout, hence a large portion of descriptor sets might end up not being used.
		/// </remarks>
		/// <returns></returns>
		/// <seealso cref="pools" />
		/// <seealso cref="IDescriptorSetLayout::allocate" />
		virtual const UInt32& poolSize() const noexcept = 0;

		/// <summary>
		/// Returns the number of active descriptor pools.
		/// </summary>
		/// <returns>The number of active descriptor pools.</returns>
		/// <seealso cref="poolSize" />
		/// <seealso cref="IDescriptorSetLayout::allocate" />
		virtual size_t pools() const noexcept = 0;

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
	class LITEFX_VULKAN_API VulkanRenderPipelineLayout : public virtual VulkanRuntimeObject<VulkanRenderPipeline>, public IRenderPipelineLayout, public IResource<VkPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutImpl);
		LITEFX_BUILDER(VulkanRenderPipelineLayoutBuilder);

	public:
		explicit VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline);
		VulkanRenderPipelineLayout(VulkanRenderPipelineLayout&&) noexcept = delete;
		VulkanRenderPipelineLayout(const VulkanRenderPipelineLayout&) noexcept = delete;
		virtual ~VulkanRenderPipelineLayout() noexcept;

	public:
		//virtual void initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts) override;

		// IRenderPipelineLayout
	public:
		virtual const IShaderProgram* getProgram() const noexcept override;
		virtual Array<const IDescriptorSetLayout*> getDescriptorSetLayouts() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanRenderPipelineLayout, VulkanRenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutBuilderImpl);

	public:
		VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineBuilder& parent, UniquePtr<VulkanRenderPipelineLayout>&& instance);
		VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineLayoutBuilder&&) = delete;
		VulkanRenderPipelineLayoutBuilder(const VulkanRenderPipelineLayoutBuilder&) = delete;
		virtual ~VulkanRenderPipelineLayoutBuilder() noexcept;

	public:
		virtual VulkanShaderProgramBuilder shaderProgram();
		virtual VulkanDescriptorSetLayoutBuilder addDescriptorSet(const UInt32& id, const ShaderStage& stages);

	public:
		virtual VulkanRenderPipelineBuilder& go() override;

	public:
		virtual void use(UniquePtr<IShaderProgram>&& program) override;
		virtual void use(UniquePtr<IDescriptorSetLayout>&& layout) override;
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
	class LITEFX_VULKAN_API VulkanShaderProgramBuilder : public ShaderProgramBuilder<VulkanShaderProgramBuilder, VulkanShaderProgram, VulkanRenderPipelineLayoutBuilder> {
	public:
		using ShaderProgramBuilder<VulkanShaderProgramBuilder, VulkanShaderProgram, VulkanRenderPipelineLayoutBuilder>::ShaderProgramBuilder;

	public:
		virtual VulkanShaderProgramBuilder& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual VulkanShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	};

	/// <summary>
	/// Records commands for a <see cref="VulkanCommandQueue" />
	/// </summary>
	class LITEFX_VULKAN_API VulkanCommandBuffer : public virtual VulkanRuntimeObject<VulkanQueue>, public ICommandBuffer, public IResource<VkCommandBuffer> {
		LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);

	public:
		/// <summary>
		/// Initializes the command buffer from a command queue.
		/// </summary>
		/// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
		/// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
		explicit VulkanCommandBuffer(const VulkanQueue& queue, const bool& begin = false);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
		virtual ~VulkanCommandBuffer() noexcept;

		// Vulkan command buffer interface.
	public:
		/// <summary>
		/// Submits the command buffer.
		/// </summary>
		/// <param name="waitForSemaphores">The semaphores to wait for on each pipeline stage. There must be a semaphore for each entry in the <see cref="waitForStages" /> array.</param>
		/// <param name="waitForStages">The pipeline stages of the current render pass to wait for before submitting the command buffer.</param>
		/// <param name="signalSemaphores">The semaphores to signal, when the command buffer is executed.</param>
		/// <param name="waitForQueue"><c>true</c> to wait for the command queue to be idle.</param>
		virtual void submit(const Array<VkSemaphore>& waitForSemaphores, const Array<VkPipelineStageFlags>& waitForStages, const Array<VkSemaphore>& signalSemaphores = { }, const bool& waitForQueue = false) const;

		// ICommandBuffer interface.
	public:
		/// <inheritdoc />
		virtual void wait() const override;

		/// <inheritdoc />
		virtual void begin() const override;

		/// <inheritdoc />
		virtual void end(const bool& submit = true, const bool& wait = false) const override;

		/// <inheritdoc />
		virtual void submit(const bool& wait = false) const override;
	};

	/// <summary>
	/// Implements the Vulkan input assembler state.
	/// </summary>
	/// <seealso cref="VulkanInputAssemblerBuilder" />
	class LITEFX_VULKAN_API VulkanInputAssembler : public virtual VulkanRuntimeObject<VulkanDevice>, public IInputAssembler<VulkanVertexBufferLayout, VulkanIndexBufferLayout> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerImpl);
		LITEFX_BUILDER(VulkanInputAssemblerBuilder);

	public:
		/// <summary>
		/// Initializes a new Vulkan input assembler state.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit VulkanInputAssembler(const VulkanDevice& device, Array<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology = PrimitiveTopology::TriangleList);
		VulkanInputAssembler(VulkanInputAssembler&&) noexcept = delete;
		VulkanInputAssembler(const VulkanInputAssembler&) noexcept = delete;
		virtual ~VulkanInputAssembler() noexcept;

	private:
		explicit VulkanInputAssembler(const VulkanDevice& device) noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const VulkanVertexBufferLayout*> vertexBufferLayouts() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanVertexBufferLayout& vertexBufferLayout(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const VulkanIndexBufferLayout& indexBufferLayout() const override;

		/// <inheritdoc />
		virtual const PrimitiveTopology& topology() const noexcept override;
	};

	/// <summary>
	/// Builds a <see cref="VulkanInputAssembler" />.
	/// </summary>
	/// <seealso cref="VulkanInputAssembler" />
	class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerBuilderImpl);

		// TODO: Create overload that can create input assemblers from a device directly.

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="parent"></param>
		/// <returns></returns>
		explicit VulkanInputAssemblerBuilder(VulkanRenderPipelineBuilder& parent) noexcept;
		VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) noexcept = delete;
		VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
		virtual ~VulkanInputAssemblerBuilder() noexcept;

	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		virtual VulkanVertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		virtual VulkanInputAssemblerBuilder& withIndexType(const IndexType& type);

		// IInputAssemblerBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanInputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanVertexBufferLayout>&& layout) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanIndexBufferLayout>&& layout) override;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& go() override;
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
	class LITEFX_VULKAN_API VulkanRasterizerBuilder : public RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineBuilder> {
	public:
		using RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineBuilder>::RasterizerBuilder;

	public:
		virtual VulkanRasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) override;
		virtual VulkanRasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) override;
		virtual VulkanRasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) override;
		virtual VulkanRasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) override;
		virtual VulkanRasterizerBuilder& enableDepthBias(const bool& enable = false) override;
		virtual VulkanRasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) override;
		virtual VulkanRasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) override;
		virtual VulkanRasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipeline : public virtual VulkanRuntimeObject<VulkanRenderPass>, public IRenderPipeline<VulkanRenderPipelineLayout, VulkanInputAssembler, IVulkanVertexBuffer, IVulkanIndexBuffer>, public IResource<VkPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
		LITEFX_BUILDER(VulkanRenderPipelineBuilder);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="renderPass"></param>
		/// <param name="id"></param>
		/// <param name="name"></param>
		explicit VulkanRenderPipeline(const VulkanRenderPass& renderPass, const UInt32& id, const String& name = "");
		VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;
		VulkanRenderPipeline(const VulkanRenderPipeline&) noexcept = delete;
		virtual ~VulkanRenderPipeline() noexcept;

	public:
		/// <inheritdoc />
		virtual const String& name() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& id() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanRenderPipelineLayout& layout() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<VulkanInputAssembler> inputAssembler() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<IRasterizer> rasterizer() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IViewport*> viewports() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IScissor*> scissors() const noexcept override;

		/// <inheritdoc />
		virtual const IDescriptorSet* descriptorSet(const UInt32& descriptorSet) const override;

	public:
		/// <inheritdoc />
		virtual void bind(const VulkanVertexBuffer& buffer) const override;

		/// <inheritdoc />
		virtual void bind(const VulkanIndexBuffer& buffer) const override;

		/// <inheritdoc />
		virtual void bind(IDescriptorSet* descriptorSet) const override;

		/// <inheritdoc />
		virtual void use() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineBuilder : public RenderPipelineBuilder<VulkanRenderPipelineBuilder, VulkanRenderPipeline, VulkanRenderPassBuilder> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineBuilderImpl);

	public:
		VulkanRenderPipelineBuilder(VulkanRenderPassBuilder& parent, UniquePtr<VulkanRenderPipeline>&& instance);
		VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) = delete;
		VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) = delete;
		virtual ~VulkanRenderPipelineBuilder() noexcept;

	public:
		virtual VulkanRenderPipelineLayoutBuilder layout();
		virtual VulkanRasterizerBuilder rasterizer();
		virtual VulkanInputAssemblerBuilder inputAssembler();
		virtual VulkanRenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);
		virtual VulkanRenderPipelineBuilder& withInputAssembler(SharedPtr<VulkanInputAssembler> inputAssembler);
		virtual VulkanRenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);
		virtual VulkanRenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);

	public:
		virtual VulkanRenderPassBuilder& go() override;
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual void use(SharedPtr<IRasterizer> rasterizer) override;
		virtual void use(SharedPtr<VulkanInputAssembler> inputAssembler) override;
		virtual void use(SharedPtr<IViewport> viewport) override;
		virtual void use(SharedPtr<IScissor> scissor) override;
	};

	/// <summary>
	/// Implements a Vulkan frame buffer.
	/// </summary>
	class LITEFX_VULKAN_API VulkanFrameBuffer : public virtual VulkanRuntimeObject<VulkanRenderPass>, public IFrameBuffer<VulkanCommandBuffer, IVulkanImage>, public IResource<VkFramebuffer> {
		LITEFX_IMPLEMENTATION(VulkanFrameBufferImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan frame buffer.
		/// </summary>
		/// <param name="renderPass">The parent render pass of the frame buffer.</param>
		/// <param name="bufferIndex">The index of the frame buffer within the parent render pass.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		VulkanFrameBuffer(const VulkanRenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea);
		VulkanFrameBuffer(const VulkanFrameBuffer&) noexcept = delete;
		VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = delete;
		virtual ~VulkanFrameBuffer() noexcept;

		// Vulkan frame buffer interface.
	public:
		/// <summary>
		/// Returns a reference of the semaphore, that can be used to signal, that the frame buffer is finished.
		/// </summary>
		/// <returns>A reference of the semaphore, that can be used to signal, that the frame buffer is finished.</returns>
		virtual const VkSemaphore& semaphore() const noexcept;

		// IFrameBuffer interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& bufferIndex() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& size() const noexcept override;

		/// <inheritdoc />
		virtual size_t getWidth() const noexcept override;

		/// <inheritdoc />
		virtual size_t getHeight() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanCommandBuffer& commandBuffer() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IVulkanImage*> images() const noexcept override;

		/// <inheritdoc />
		virtual const IVulkanImage& image(const UInt32& location) const override;

	public:
		/// <inheritdoc />
		virtual void resize(const Size2d& renderArea) override;
	};

	/// <summary>
	/// Implements a Vulkan render pass.
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPass : public virtual VulkanRuntimeObject<VulkanDevice>, public IRenderPass<VulkanRenderPipeline, VulkanFrameBuffer, VulkanInputAttachmentMapping>, public IResource<VkRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
		LITEFX_BUILDER(VulkanRenderPassBuilder);

	public:
		/// <summary>
		/// Creates and initializes a new Vulkan render pass instance.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="renderTargets"></param>
		/// <param name="inputAttachments"></param>
		explicit VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<VulkanInputAttachmentMapping> inputAttachments = { });
		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		virtual ~VulkanRenderPass() noexcept;

	private:
		/// <summary>
		/// Creates an uninitialized Vulkan render pass instance.
		/// </summary>
		/// <remarks>
		/// This constructor is called by the <see cref="VulkanRenderPassBuilder" /> in order to create a render pass instance without initializing it. The instance 
		/// is only initialized after calling <see cref="VulkanRenderPassBuilder::go" />.
		/// </remarks>
		/// <param name="device">The parent device of the render pass.</param>
		explicit VulkanRenderPass(const VulkanDevice& device) noexcept;

		// IInputAttachmentMappingSource interface.
	public:
		/// <inheritdoc />
		virtual const VulkanFrameBuffer& frameBuffer(const UInt32& buffer) const override;

		// IRenderPass interface.
	public:
		/// <inheritdoc />
		virtual const VulkanFrameBuffer& activeFrameBuffer() const override;

		/// <inheritdoc />
		virtual Array<const VulkanFrameBuffer*> frameBuffers() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanRenderPipeline& pipeline(const UInt32& id) const override;

		/// <inheritdoc />
		virtual Array<const VulkanRenderPipeline*> pipelines() const noexcept override;

		/// <inheritdoc />
		virtual Span<const RenderTarget> renderTargets() const noexcept override;

		/// <inheritdoc />
		virtual bool hasPresentTarget() const noexcept override;

		/// <inheritdoc />
		virtual Span<const VulkanInputAttachmentMapping> inputAttachments() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void begin(const UInt32& buffer) override;
		
		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual void resizeFrameBuffers(const Size2d& renderArea) override;
	};

	/// <summary>
	/// Implements the Vulkan <see cref="RenderPassBuilder" />.
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPassBuilder : public RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassBuilderImpl)

	public:
		explicit VulkanRenderPassBuilder(const VulkanDevice& device) noexcept;
		VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) noexcept = delete;
		VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
		virtual ~VulkanRenderPassBuilder() noexcept;

	public:
		virtual VulkanRenderPipelineBuilder addPipeline(const UInt32& id, const String& name = "");

	public:
		virtual void use(RenderTarget&& target) override;
		virtual void use(VulkanInputAttachmentMapping&& inputAttachment) override;
		virtual void use(UniquePtr<VulkanRenderPipeline>&& pipeline) override;

	public:
		virtual VulkanRenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual VulkanRenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual VulkanRenderPassBuilder& inputAttachment(const UInt32& location, const RenderTarget& renderTarget, const VulkanRenderPass& renderPass) override;

	public:
		virtual UniquePtr<VulkanRenderPass> go() override;
	};

	/// <summary>
	/// Implements a <see cref="IInputAttachmentMapping" />.
	/// </summary>
	class LITEFX_VULKAN_API VulkanInputAttachmentMapping : public IInputAttachmentMapping<VulkanRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanInputAttachmentMappingImpl);

	public:
		/// <summary>
		/// Creates a new Vulkan input attachment mapping.
		/// </summary>
		/// <param name="renderPass">The render pass to fetch the input attachment from.</param>
		/// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
		/// <param name="location">The location to bind the input attachment to.</param>
		VulkanInputAttachmentMapping(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location);
		VulkanInputAttachmentMapping(const VulkanInputAttachmentMapping&) noexcept;
		VulkanInputAttachmentMapping(VulkanInputAttachmentMapping&&) noexcept;
		virtual ~VulkanInputAttachmentMapping() noexcept;

	public:
		virtual const VulkanRenderPass& inputAttachmentSource() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& location() const noexcept override;
	};

	/// <summary>
	/// Implements a Vulkan swap chain.
	/// </summary>
	class LITEFX_VULKAN_API VulkanSwapChain : public virtual VulkanRuntimeObject<VulkanDevice>, public ISwapChain<IVulkanImage>, public IResource<VkSwapchainKHR> {
		LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan swap chain.
		/// </summary>
		/// <param name="device">The device that owns the swap chain.</param>
		/// <param name="format">The initial surface format.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="buffers">The initial number of buffers.</param>
		explicit VulkanSwapChain(const VulkanDevice& device, const Format& surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, const UInt32& buffers = 3);
		VulkanSwapChain(const VulkanSwapChain&) = delete;
		VulkanSwapChain(VulkanSwapChain&&) = delete;
		virtual ~VulkanSwapChain() noexcept;

		// Vulkan Swap Chain interface.
	public:
		/// <summary>
		/// Returns a reference of the current swap semaphore, a command queue can wait on for presenting.
		/// </summary>
		/// <returns>A reference of the current swap semaphore, a command queue can wait on for presenting.</returns>
		virtual const VkSemaphore& semaphore() const noexcept;

		// ISwapChain interface.
	public:
		/// <inheritdoc />
		virtual const Format& surfaceFormat() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& buffers() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& renderArea() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IVulkanImage*> images() const noexcept override;

	public:
		/// <inheritdoc />
		virtual Array<Format> getSurfaceFormats() const noexcept override;

		/// <inheritdoc />
		virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) override;

		/// <inheritdoc />
		[[nodiscard]] virtual UInt32 swapBackBuffer() const override;
	};

	/// <summary>
	/// Implements a Vulkan command queue.
	/// </summary>
	class LITEFX_VULKAN_API VulkanQueue : public virtual VulkanRuntimeObject<VulkanDevice>, public ICommandQueue<VulkanCommandBuffer>, public IResource<VkQueue> {
		LITEFX_IMPLEMENTATION(VulkanQueueImpl);
	
	public:
		/// <summary>
		/// Initializes the Vulkan command queue.
		/// </summary>
		/// <param name="device">The device, commands get send to.</param>
		/// <param name="type">The type of the command queue.</param>
		/// <param name="priority">The priority, of which commands are issued on the device.</param>
		/// <param name="familyId">The ID of the queue family.</param>
		/// <param name="queueId">The ID of the queue.</param>
		explicit VulkanQueue(const VulkanDevice& device, const QueueType& type, const QueuePriority& priority, const UInt32& familyId, const UInt32& queueId);
		VulkanQueue(const VulkanQueue&) = delete;
		VulkanQueue(VulkanQueue&&) = delete;
		virtual ~VulkanQueue() noexcept;

		// VulkanQueue interface.
	public:
		/// <summary>
		/// Returns a reference of the command pool that is used to allocate commands.
		/// </summary>
		/// <remarks>
		/// Note that the command pool does only exist, if the queue is bound on a device.
		/// </remarks>
		/// <seealso cref="isBound" />
		/// <seealso cref="bind" />
		/// <seealso cref="release" />
		/// <returns>A reference of the command pool that is used to allocate commands</returns>
		virtual const VkCommandPool& commandPool() const noexcept;

		/// <summary>
		/// Returns the queue family ID.
		/// </summary>
		/// <returns>The queue family ID.</returns>
		virtual const UInt32& familyId() const noexcept;

		/// <summary>
		/// Returns the queue ID.
		/// </summary>
		/// <returns>The queue ID.</returns>
		virtual const UInt32& queueId() const noexcept;

		// ICommandQueue interface.
	public:
		/// <inheritdoc />
		virtual bool isBound() const noexcept override;

		/// <inheritdoc />
		virtual const QueuePriority& priority() const noexcept override;

		/// <inheritdoc />
		virtual const QueueType& type() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void bind() override;

		/// <inheritdoc />
		virtual void release() override;

		/// <inheritdoc />
		virtual UniquePtr<VulkanCommandBuffer> createCommandBuffer(const bool& beginRecording = false) const override;
	};

	/// <summary>
	/// A graphics factory that produces objects for a <see cref="VulkanDevice" />.
	/// </summary>
	/// <remarks>
	/// Internally this factory implementation is based on <a href="https://gpuopen.com/vulkan-memory-allocator/" target="_blank">Vulkan Memory Allocator</a>.
	/// </remarks>
	class LITEFX_VULKAN_API VulkanGraphicsFactory : public IGraphicsFactory<VulkanDescriptorLayout, IVulkanImage, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanConstantBuffer, IVulkanBuffer, IVulkanTexture, IVulkanSampler> {
		LITEFX_IMPLEMENTATION(VulkanGraphicsFactoryImpl);

	public:
		/// <summary>
		/// Creates a new graphics factory.
		/// </summary>
		/// <param name="device">The device the factory should produce objects for.</param>
		explicit VulkanGraphicsFactory(const VulkanDevice& device);
		VulkanGraphicsFactory(const VulkanGraphicsFactory&) = delete;
		VulkanGraphicsFactory(VulkanGraphicsFactory&&) = delete;
		virtual ~VulkanGraphicsFactory() noexcept;

	public:
		/// <inheritdoc />
		virtual UniquePtr<IVulkanImage> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanVertexBuffer> createVertexBuffer(const VulkanVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanIndexBuffer> createIndexBuffer(const VulkanIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanConstantBuffer> createConstantBuffer(const VulkanDescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanTexture> createTexture(const VulkanDescriptorLayout& layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IVulkanSampler> createSampler(const VulkanDescriptorLayout& layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
	};

	/// <summary>
	/// Implements a Vulkan graphics device.
	/// </summary>
	class LITEFX_VULKAN_API VulkanDevice : public IGraphicsDevice<VulkanGraphicsFactory, VulkanSurface, VulkanGraphicsAdapter, VulkanSwapChain, VulkanQueue, VulkanRenderPass>, public IResource<VkDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceImpl);

	public:
		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="extensions">The required extensions the device gets initialized with.</param>
		explicit VulkanDevice(const VulkanGraphicsAdapter& adapter, const VulkanSurface& surface, Span<String> extensions = { });

		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="format">The initial surface format, device uses for drawing.</param>
		/// <param name="frameBufferSize">The initial size of the frame buffers.</param>
		/// <param name="frameBuffers">The initial number of frame buffers.</param>
		/// <param name="extensions">The required extensions the device gets initialized with.</param>
		explicit VulkanDevice(const VulkanGraphicsAdapter& adapter, const VulkanSurface& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, Span<String> extensions = { });

		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		virtual ~VulkanDevice() noexcept;

		// Vulkan Device interface.
	public:
		/// <summary>
		/// Returns the array that stores the extensions that were used to initialize the device.
		/// </summary>
		/// <returns>A reference to the array that stores the extensions that were used to initialize the device.</returns>
		virtual Span<const String> enabledExtensions() const noexcept;

		/// <summary>
		/// Returns a reference of the swap chain.
		/// </summary>
		/// <returns>A reference of the swap chain.</returns>
		virtual VulkanSwapChain& swapChain() noexcept;

	public:
		/// <summary>
		/// Returns a builder for a <see cref="VulkanRenderPass" />.
		/// </summary>
		/// <returns>An instance of a builder that is used to create a new render pass.</returns>
		/// <seealso cref="IGraphicsDevice::build" />
		VulkanRenderPassBuilder buildRenderPass() const;

		// IGraphicsDevice interface.
	public:
		/// <inheritdoc />
		virtual const VulkanSwapChain& swapChain() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanSurface& surface() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanGraphicsAdapter& adapter() const noexcept override;

		/// <inheritdoc />
		virtual const VulkanGraphicsFactory& factory() const noexcept override;
		
		/// <inheritdoc />
		virtual const VulkanQueue& graphicsQueue() const noexcept override;
		
		/// <inheritdoc />
		virtual const VulkanQueue& transferQueue() const noexcept override;
		
		/// <inheritdoc />
		virtual const VulkanQueue& bufferQueue() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void wait() const override;
	};

	/// <summary>
	/// Defines a rendering backend that creates a Vulkan device.
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackend : public IRenderBackend<VulkanDevice>, public IResource<VkInstance> {
		LITEFX_IMPLEMENTATION(VulkanBackendImpl);
		LITEFX_BUILDER(VulkanBackendBuilder);

	public:
		/// <summary>
		/// Initializes a new vulkan rendering backend.
		/// </summary>
		/// <param name="app">An instance of the app that owns the backend.</param>
		/// <param name="extensions">A set of extensions to enable on the graphics device.</param>
		/// <param name="validationLayers">A set of validation layers to enable on the rendering backend.</param>
		explicit VulkanBackend(const App& app, const Span<String> extensions = { }, const Span<String> validationLayers = { });
		VulkanBackend(const VulkanBackend&) noexcept = delete;
		VulkanBackend(VulkanBackend&&) noexcept = delete;
		virtual ~VulkanBackend() noexcept;

		// Vulkan Backend interface.
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
		virtual Span<const String> getEnabledValidationLayers() const noexcept;

		/// <summary>
		/// Creates a surface using the <paramref name="predicate" /> callback.
		/// </summary>
		/// <param name="predicate">A callback that gets called with the backend instance handle and creates the surface instance</param>
		/// <returns>The instance of the created surface.</returns>
		/// <seealso cref="surface_callback" />
		UniquePtr<VulkanSurface> createSurface(surface_callback predicate) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
		/// <summary>
		/// Creates a surface on a window handle.
		/// </summary>
		/// <param name="hwnd">The window handle on which the surface should be created.</param>
		/// <returns>The instance of the created surface.</returns>
		UniquePtr<VulkanSurface> createSurface(const HWND& hwnd) const;
#endif

	public:
		/// <summary>
		/// Returns <c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.
		/// </summary>
		/// <returns><c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.</returns>
		/// <seealso cref="getAvailableExtensions" />
		static bool validateExtensions(Span<const String> extensions) noexcept;

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
		static bool validateLayers(const Span<const String> validationLayers) noexcept;

		/// <summary>
		/// Returns a list of available validation layers.
		/// </summary>
		/// <returns>A list of available validation layers.</returns>
		static Array<String> getValidationLayers() noexcept;

		// IBackend interface.
	public:
		/// <inheritdoc />
		virtual BackendType getType() const noexcept override;

		// IRenderBackend interface.
	public:
		/// <inheritdoc />
		virtual Array<const VulkanGraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const VulkanGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackendBuilder : public Builder<VulkanBackendBuilder, VulkanBackend, AppBuilder> {
	public:
		using builder_type::Builder;

	public:
		virtual AppBuilder& go() override;
	};
}