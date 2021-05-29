#pragma once

#include <litefx/rendering.hpp>

#include "dx12_api.hpp"
#include "dx12_builders.hpp"
#include "dx12_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Records commands for a <see cref="DirectX12CommandQueue" />
	/// </summary>
	/// <seealso cref="DirectX12CommandQueue" />
	class LITEFX_DIRECTX12_API DirectX12CommandBuffer : public ICommandBuffer, public DirectX12RuntimeObject<DirectX12Queue>, public ComResource<ID3D12GraphicsCommandList4> {
		LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);

	public:
		/// <summary>
		/// Initializes the command buffer from a command queue.
		/// </summary>
		/// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
		/// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
		explicit DirectX12CommandBuffer(const DirectX12Queue& queue, const bool& begin = false);
		DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;
		DirectX12CommandBuffer(DirectX12CommandBuffer&&) = delete;
		virtual ~DirectX12CommandBuffer() noexcept;

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
	/// Implements a DirectX12 vertex buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12VertexBufferLayoutBuilder" />
	/// <seealso cref="DirectX12VertexBuffer" />
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout : public virtual DirectX12RuntimeObject<DirectX12InputAssembler>, public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
		LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new vertex buffer layout.
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the vertex buffer layout is initialized for.</param>
		/// <param name="vertexSize">The size of a single vertex.</param>
		/// <param name="binding">The binding point of the vertex buffers using this layout.</param>
		explicit DirectX12VertexBufferLayout(const DirectX12InputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding = 0);
		DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) = delete;
		DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = delete;
		virtual ~DirectX12VertexBufferLayout() noexcept;

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
	/// Builds a see <cref="DirectX12VertexBufferLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12VertexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder>::VertexBufferLayoutBuilder;

	public:
		/// <inheritdoc />
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

	public:
		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <reamrks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </reamrks>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);

		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <param name="location">The location, the attribute is bound to.</param>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	};

	/// <summary>
	/// Implements a DirectX12 index buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12IndexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout : public virtual DirectX12RuntimeObject<DirectX12InputAssembler>, public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new index buffer layout
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the index buffer layout is initialized for.</param>
		/// <param name="type">The type of the indices within the index buffer.</param>
		explicit DirectX12IndexBufferLayout(const DirectX12InputAssembler& inputAssembler, const IndexType& type);
		DirectX12IndexBufferLayout(DirectX12IndexBufferLayout&&) = delete;
		DirectX12IndexBufferLayout(const DirectX12IndexBufferLayout&) = delete;
		virtual ~DirectX12IndexBufferLayout() noexcept;

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
	class LITEFX_DIRECTX12_API DirectX12RenderPassBuilder : public RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass> {
	public:
		DirectX12RenderPassBuilder(UniquePtr<DirectX12RenderPass>&& instance);
		virtual ~DirectX12RenderPassBuilder() noexcept;

	public:
		virtual UniquePtr<DirectX12RenderPass> go() override;

	public:
		virtual DirectX12RenderPipelineBuilder addPipeline(const UInt32& id, const String& name = "");

	public:
		virtual void use(UniquePtr<IRenderPipeline>&& pipeline) override;
		virtual void use(UniquePtr<IRenderTarget>&& target) override;
		virtual DirectX12RenderPassBuilder& attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& dependsOn(const IRenderPass* renderPass) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineBuilder : public RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline, DirectX12RenderPassBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineBuilderImpl);

	public:
		DirectX12RenderPipelineBuilder(DirectX12RenderPassBuilder& parent, UniquePtr<DirectX12RenderPipeline>&& instance);
		DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
		DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
		virtual ~DirectX12RenderPipelineBuilder() noexcept;

	public:
		virtual DirectX12RenderPipelineLayoutBuilder layout();
		virtual DirectX12RasterizerBuilder rasterizer();
		virtual DirectX12InputAssemblerBuilder inputAssembler();
		virtual DirectX12RenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);
		virtual DirectX12RenderPipelineBuilder& withInputAssembler(SharedPtr<IInputAssembler> inputAssembler);
		virtual DirectX12RenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);
		virtual DirectX12RenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);

	public:
		virtual DirectX12RenderPassBuilder& go() override;
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual void use(SharedPtr<IRasterizer> rasterizer) override;
		virtual void use(SharedPtr<IInputAssembler> inputAssembler) override;
		virtual void use(SharedPtr<IViewport> viewport) override;
		virtual void use(SharedPtr<IScissor> scissor) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<DirectX12RenderPipelineLayoutBuilder, DirectX12RenderPipelineLayout, DirectX12RenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutBuilderImpl);

	public:
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineBuilder& parent, UniquePtr<DirectX12RenderPipelineLayout>&& instance);
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineLayoutBuilder&&) = delete;
		DirectX12RenderPipelineLayoutBuilder(const DirectX12RenderPipelineLayoutBuilder&) = delete;
		virtual ~DirectX12RenderPipelineLayoutBuilder() noexcept;

	public:
		virtual DirectX12ShaderProgramBuilder shaderProgram();
		virtual DirectX12DescriptorSetLayoutBuilder addDescriptorSet(const UInt32& id, const ShaderStage& stages);

	public:
		virtual DirectX12RenderPipelineBuilder& go() override;

	public:
		virtual void use(UniquePtr<IShaderProgram>&& program) override;
		virtual void use(UniquePtr<IDescriptorSetLayout>&& layout) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RasterizerBuilder : public RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder> {
	public:
		using RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder>::RasterizerBuilder;

	public:
		virtual DirectX12RasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) override;
		virtual DirectX12RasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) override;
		virtual DirectX12RasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) override;
		virtual DirectX12RasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) override;
		virtual DirectX12RasterizerBuilder& enableDepthBias(const bool& enable = false) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12InputAssemblerBuilder : public InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder> {
	public:
		using InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder>::InputAssemblerBuilder;

	public:
		virtual DirectX12VertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

	public:
		virtual DirectX12InputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;
		virtual void use(UniquePtr<IVertexBufferLayout>&& layout) override;
		virtual void use(UniquePtr<IIndexBufferLayout>&& layout) override;

	public:
		virtual DirectX12InputAssemblerBuilder& withIndexType(const IndexType& type);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12ShaderProgramBuilder : public ShaderProgramBuilder<DirectX12ShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder> {
	public:
		using ShaderProgramBuilder<DirectX12ShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder>::ShaderProgramBuilder;

	public:
		virtual DirectX12ShaderProgramBuilder& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<DirectX12DescriptorSetLayoutBuilder, DirectX12DescriptorSetLayout, DirectX12RenderPipelineLayoutBuilder> {
	public:
		using DescriptorSetLayoutBuilder<DirectX12DescriptorSetLayoutBuilder, DirectX12DescriptorSetLayout, DirectX12RenderPipelineLayoutBuilder>::DescriptorSetLayoutBuilder;

	public:
		virtual DirectX12RenderPipelineLayoutBuilder& go() override;

	public:
		virtual DirectX12DescriptorSetLayoutBuilder& addDescriptor(UniquePtr<IDescriptorLayout>&& layout) override;
		virtual DirectX12DescriptorSetLayoutBuilder& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize) override;
	};


	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Device : public GraphicsDevice, public ComResource<ID3D12Device5> {
		LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);

	public:
		explicit DirectX12Device(const IRenderBackend* backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers);
		DirectX12Device(const DirectX12Device&) = delete;
		DirectX12Device(DirectX12Device&&) = delete;
		virtual ~DirectX12Device() noexcept;

	public:
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
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;

	public:
		DirectX12RenderPassBuilder buildRenderPass() const;
		//DirectX12ComputePassBuilder buildComputePass() const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Queue : public ICommandQueue, public ComResource<ID3D12CommandQueue> {
		LITEFX_IMPLEMENTATION(DirectX12QueueImpl);

	public:
		explicit DirectX12Queue(const IGraphicsDevice* device, const QueueType& type, const QueuePriority& priority);
		virtual ~DirectX12Queue() noexcept;

	public:
		virtual bool isBound() const noexcept override;
		virtual QueuePriority getPriority() const noexcept override;
		virtual QueueType getType() const noexcept override;
		virtual const IGraphicsDevice* getDevice() const noexcept override;

	public:
		virtual void bind() override;
		virtual void release() override;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12SwapChain : public virtual DirectX12RuntimeObject, public ISwapChain, public ComResource<IDXGISwapChain4> {
		LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);

	public:
		explicit DirectX12SwapChain(const DirectX12Device* device, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Format& format = Format::B8G8R8A8_SRGB);
		virtual ~DirectX12SwapChain() noexcept;

	public:
		virtual const Size2d& getBufferSize() const noexcept override;
		virtual size_t getWidth() const noexcept override;
		virtual size_t getHeight() const noexcept override;
		virtual const Format& getFormat() const noexcept override;
		virtual UInt32 swapBackBuffer() const override;
		virtual void reset(const Size2d& frameBufferSize, const UInt32& frameBuffers) override;
		virtual UInt32 getBuffers() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPass : public virtual DirectX12RuntimeObject, public IRenderPass {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
		LITEFX_BUILDER(DirectX12RenderPassBuilder);

	public:
		explicit DirectX12RenderPass(const IGraphicsDevice* device);	// Adapter for builder interface.
		explicit DirectX12RenderPass(const DirectX12Device* device);
		DirectX12RenderPass(const DirectX12RenderPass&) = delete;
		DirectX12RenderPass(DirectX12RenderPass&&) = delete;
		virtual ~DirectX12RenderPass() noexcept;

	public:
		virtual const DirectX12Device* getDevice() const noexcept;
		virtual const DirectX12CommandBuffer* getDXCommandBuffer() const noexcept;

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
	class LITEFX_DIRECTX12_API DirectX12RenderPipeline : public virtual DirectX12RuntimeObject, public IRenderPipeline, public ComResource<ID3D12PipelineState> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineBuilder);

	public:
		explicit DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const UInt32& id, const String& name = "");
		DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = delete;
		DirectX12RenderPipeline(const DirectX12RenderPipeline&) noexcept = delete;
		virtual ~DirectX12RenderPipeline() noexcept;

		// IRequiresInitialization
	public:
		virtual bool isInitialized() const noexcept override;

		// IRenderPipeline
	public:
		/// <inheritdoc />
		virtual const IRenderPass& renderPass() const noexcept override;

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
		virtual UniquePtr<IDescriptorSet> makeDescriptorSet(const UInt32& bufferSet) const override;
		virtual void bind(const IVertexBuffer* buffer) const override;
		virtual void bind(const IIndexBuffer* buffer) const override;
		virtual void bind(IDescriptorSet* buffer) const override;
		virtual void use() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayout : public virtual DirectX12RuntimeObject, public IRenderPipelineLayout, public ComResource<ID3D12RootSignature> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineLayoutBuilder);

	public:
		explicit DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline);
		DirectX12RenderPipelineLayout(DirectX12RenderPipelineLayout&&) noexcept = delete;
		DirectX12RenderPipelineLayout(const DirectX12RenderPipelineLayout&) noexcept = delete;
		virtual ~DirectX12RenderPipelineLayout() noexcept;

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
	class LITEFX_DIRECTX12_API DirectX12InputAssembler : public virtual DirectX12RuntimeObject, public InputAssembler {
		LITEFX_BUILDER(DirectX12InputAssemblerBuilder);

	public:
		explicit DirectX12InputAssembler(const DirectX12RenderPipeline& pipeline) noexcept;
		DirectX12InputAssembler(DirectX12InputAssembler&&) noexcept = delete;
		DirectX12InputAssembler(const DirectX12InputAssembler&) noexcept = delete;
		virtual ~DirectX12InputAssembler() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Rasterizer : public virtual DirectX12RuntimeObject, public Rasterizer {
		LITEFX_BUILDER(DirectX12RasterizerBuilder);

	public:
		explicit DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline) noexcept;
		DirectX12Rasterizer(DirectX12Rasterizer&&) noexcept = delete;
		DirectX12Rasterizer(const DirectX12Rasterizer&) noexcept = delete;
		virtual ~DirectX12Rasterizer() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12ShaderModule : public IShaderModule, public IResource<D3D12_SHADER_BYTECODE> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderModuleImpl);

	public:
		explicit DirectX12ShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main");
		virtual ~DirectX12ShaderModule() noexcept;

	public:
		virtual const String& getFileName() const noexcept override;
		virtual const String& getEntryPoint() const noexcept override;
		virtual const ShaderStage& getType() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12ShaderProgram : public virtual DirectX12RuntimeObject, public IShaderProgram {
		LITEFX_IMPLEMENTATION(DirectX12ShaderProgramImpl);
		LITEFX_BUILDER(DirectX12ShaderProgramBuilder);

	public:
		explicit DirectX12ShaderProgram(const DirectX12RenderPipelineLayout& pipelineLayout);
		DirectX12ShaderProgram(DirectX12ShaderProgram&&) noexcept = delete;
		DirectX12ShaderProgram(const DirectX12ShaderProgram&) noexcept = delete;
		virtual ~DirectX12ShaderProgram() noexcept;

	public:
		virtual Array<const IShaderModule*> getModules() const noexcept override;
		virtual void use(UniquePtr<IShaderModule>&& module) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayout : public virtual DirectX12RuntimeObject, public IDescriptorSetLayout {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetLayoutImpl);
		LITEFX_BUILDER(DirectX12DescriptorSetLayoutBuilder);

	public:
		explicit DirectX12DescriptorSetLayout(const DirectX12RenderPipelineLayout& pipelineLayout, const UInt32& id, const ShaderStage& stages);
		DirectX12DescriptorSetLayout(DirectX12DescriptorSetLayout&&) = delete;
		DirectX12DescriptorSetLayout(const DirectX12DescriptorSetLayout&) = delete;
		virtual ~DirectX12DescriptorSetLayout() noexcept;

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

	class LITEFX_DIRECTX12_API DirectX12DescriptorSet : public virtual DirectX12RuntimeObject, public IDescriptorSet, ComResource<ID3D12DescriptorHeap> {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetImpl);

	public:
		explicit DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& bufferSet);
		DirectX12DescriptorSet(DirectX12DescriptorSet&&) = delete;
		DirectX12DescriptorSet(const DirectX12DescriptorSet&) = delete;
		virtual ~DirectX12DescriptorSet() noexcept;

	public:
		virtual const IDescriptorSetLayout* getDescriptorSetLayout() const noexcept override;
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
	class LITEFX_DIRECTX12_API DirectX12DescriptorLayout : public virtual DirectX12RuntimeObject, public IDescriptorLayout {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorLayoutImpl);

	public:
		explicit DirectX12DescriptorLayout(const DirectX12DescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize);
		DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) = delete;
		DirectX12DescriptorLayout(const DirectX12DescriptorLayout&) = delete;
		virtual ~DirectX12DescriptorLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;

	public:
		virtual const IDescriptorSetLayout* getDescriptorSet() const noexcept override;
		virtual DescriptorType getDescriptorType() const noexcept override;
	};


	/// <summary>
	/// Implements the DirectX12 <see cref="IRenderBackend" />.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Backend : public IRenderBackend<DirectX12Device>, public ComResource<IDXGIFactory7> {
		LITEFX_IMPLEMENTATION(DirectX12BackendImpl);
		LITEFX_BUILDER(DirectX12BackendBuilder);

	public:
		explicit DirectX12Backend(const App& app, const bool& advancedSoftwareRasterizer = false);
		DirectX12Backend(const DirectX12Backend&) noexcept = delete;
		DirectX12Backend(DirectX12Backend&&) noexcept = delete;
		virtual ~DirectX12Backend();

		// IBackend interface.
	public:
		/// <inheritdoc />
		virtual BackendType getType() const noexcept override;

		// IRenderBackend interface.
	public:
		/// <inheritdoc />
		virtual Array<const DirectX12GraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const DirectX12GraphicsAdapter* findAdapter(const Optional<UInt32>& adapterId = std::nullopt) const override;

	public:
		/// <summary>
		/// Enables <a href="https://docs.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp" target="_blank">Windows Advanced Software Rasterization (WARP)</a>.
		/// </summary>
		/// <remarks>
		/// Enabling software rasterization disables hardware rasterization. Requesting adapters using <see cref="findAdapter" /> or <see cref="listAdapters" />
		/// will only return WARP-compatible adapters.
		/// </remarks>
		/// <param name="enable"><c>true</c>, if advanced software rasterization should be used.</param>
		virtual void enableAdvancedSoftwareRasterizer(const bool& enable = false);
	};
}