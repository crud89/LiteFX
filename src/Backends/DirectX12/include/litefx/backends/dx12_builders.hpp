#pragma once

#include <litefx/rendering_builders.hpp>
#include "dx12_api.hpp"
#include "dx12.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	///// <summary>
	///// Builds a DirectX 12 <see cref="ShaderProgram" /> for graphics rendering.
	///// </summary>
	///// <seealso cref="DirectX12ShaderProgram" />
	///// <seealso cref="DirectX12RenderPipeline" />
	//class LITEFX_DIRECTX12_API DirectX12GraphicsShaderProgramBuilder : public GraphicsShaderProgramBuilder<DirectX12GraphicsShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12GraphicsShaderProgramBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 graphics shader program builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	explicit DirectX12GraphicsShaderProgramBuilder(DirectX12RenderPipelineLayoutBuilder& parent);
	//	DirectX12GraphicsShaderProgramBuilder(const DirectX12GraphicsShaderProgramBuilder&) = delete;
	//	DirectX12GraphicsShaderProgramBuilder(DirectX12GraphicsShaderProgramBuilder&&) = delete;
	//	virtual ~DirectX12GraphicsShaderProgramBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual DirectX12RenderPipelineLayoutBuilder& go() override;

	//	// ShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;

	//	// GraphicsShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual DirectX12GraphicsShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual DirectX12ComputeShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	//};

	///// <summary>
	///// Builds a see <see cref="DirectX12VertexBufferLayout" />.
	///// </summary>
	///// <seealso cref="DirectX12VertexBuffer" />
	///// <seealso cref="DirectX12VertexBufferLayout" />
	//class LITEFX_DIRECTX12_API DirectX12VertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder> {
	//public:
	//	using VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder>::VertexBufferLayoutBuilder;

	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12VertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

	//public:
	//	/// <summary>
	//	/// Adds an attribute to the vertex buffer layout.
	//	/// </summary>
	//	/// <remarks>
	//	/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
	//	/// </remarks>
	//	/// <param name="format">The format of the attribute.</param>
	//	/// <param name="offset">The offset of the attribute within a buffer element.</param>
	//	/// <param name="semantic">The semantic of the attribute.</param>
	//	/// <param name="semanticIndex">The semantic index of the attribute.</param>
	//	virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);

	//	/// <summary>
	//	/// Adds an attribute to the vertex buffer layout.
	//	/// </summary>
	//	/// <param name="location">The location, the attribute is bound to.</param>
	//	/// <param name="format">The format of the attribute.</param>
	//	/// <param name="offset">The offset of the attribute within a buffer element.</param>
	//	/// <param name="semantic">The semantic of the attribute.</param>
	//	/// <param name="semanticIndex">The semantic index of the attribute.</param>
	//	virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	//};

	///// <summary>
	///// Builds a <see cref="DirectX12DescriptorSetLayout" /> for a render pipeline.
	///// </summary>
	///// <seealso cref="DirectX12DescriptorSetLayout" />
	///// <seealso cref="DirectX12RenderPipeline" />
	//class LITEFX_DIRECTX12_API DirectX12RenderPipelineDescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<DirectX12RenderPipelineDescriptorSetLayoutBuilder, DirectX12DescriptorSetLayout, DirectX12RenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12RenderPipelineDescriptorSetLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 descriptor set layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="space">The space the descriptor set is bound to.</param>
	//	/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
	//	explicit DirectX12RenderPipelineDescriptorSetLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex);
	//	DirectX12RenderPipelineDescriptorSetLayoutBuilder(const DirectX12RenderPipelineDescriptorSetLayoutBuilder&) = delete;
	//	DirectX12RenderPipelineDescriptorSetLayoutBuilder(DirectX12RenderPipelineDescriptorSetLayoutBuilder&&) = delete;
	//	virtual ~DirectX12RenderPipelineDescriptorSetLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual DirectX12RenderPipelineLayoutBuilder& go() override;

	//	// DescriptorSetLayoutBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12RenderPipelineDescriptorSetLayoutBuilder& addDescriptor(UniquePtr<DirectX12DescriptorLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual DirectX12RenderPipelineDescriptorSetLayoutBuilder& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) override;

	//	// DirectX12RenderPipelineDescriptorSetLayoutBuilder.
	//public:
	//	/// <summary>
	//	/// Sets the space, the descriptor set is bound to.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	virtual DirectX12RenderPipelineDescriptorSetLayoutBuilder& space(const UInt32& space) noexcept;

	//	/// <summary>
	//	/// Sets the shader stages, the descriptor set is accessible from.
	//	/// </summary>
	//	/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
	//	virtual DirectX12RenderPipelineDescriptorSetLayoutBuilder& shaderStages(const ShaderStage& stages) noexcept;
	//};

	///// <summary>
	///// Builds a DirectX 12 <see cref="PushConstantsLayout" /> for a <see cref="RenderPipeline" />.
	///// </summary>
	///// <seealso cref="DirectX12PushConstantsLayout" />
	//class LITEFX_DIRECTX12_API DirectX12PushConstantsLayoutBuilder : public PushConstantsLayoutBuilder<DirectX12RenderPipelinePushConstantsLayoutBuilder, DirectX12PushConstantsLayout, DirectX12RenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12PushConstantsLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 render pipeline push constants layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	explicit DirectX12PushConstantsLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& size);
	//	DirectX12PushConstantsLayoutBuilder(const DirectX12RenderPipelinePushConstantsLayoutBuilder&) = delete;
	//	DirectX12PushConstantsLayoutBuilder(DirectX12RenderPipelinePushConstantsLayoutBuilder&&) = delete;
	//	virtual ~DirectX12PushConstantsLayoutBuilder() noexcept;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// PushConstantsLayoutBuilder interface.
	//public:
	//	virtual DirectX12PushConstantsLayoutBuilder& addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) override;
	//};

	///// <summary>
	///// Builds a DirectX 12 <see cref="PipelineLayout" /> for a render pipeline.
	///// </summary>
	///// <seealso cref="DirectX12PipelineLayout" />
	///// <seealso cref="DirectX12RenderPipeline" />
	//class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayoutBuilder : public PipelineLayoutBuilder<DirectX12RenderPipelineLayoutBuilder, DirectX12PipelineLayout, DirectX12RenderPipelineBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a new DirectX 12 render pipeline layout builder.
	//	/// </summary>
	//	DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineBuilder& parent);
	//	DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineLayoutBuilder&&) = delete;
	//	DirectX12RenderPipelineLayoutBuilder(const DirectX12RenderPipelineLayoutBuilder&) = delete;
	//	virtual ~DirectX12RenderPipelineLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual DirectX12RenderPipelineBuilder& go() override;

	//	// RenderPipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12ShaderProgram>&& program) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12DescriptorSetLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12PushConstantsLayout>&& layout) override;

	//	// DirectX12RenderPipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a shader program for the render pipeline layout.
	//	/// </summary>
	//	virtual DirectX12GraphicsShaderProgramBuilder shaderProgram();

	//	/// <summary>
	//	/// Builds a new descriptor set for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
	//	/// <param name="poolSize">Unused for this backend.</param>
	//	virtual DirectX12RenderPipelineDescriptorSetLayoutBuilder addDescriptorSet(const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, const UInt32& poolSize = 0);

	//	/// <summary>
	//	/// Builds a new push constants layout for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	virtual DirectX12RenderPipelinePushConstantsLayoutBuilder addPushConstants(const UInt32& size);
	//};

	///// <summary>
	///// Builds a <see cref="DirectX12InputAssembler" />.
	///// </summary>
	///// <seealso cref="DirectX12InputAssembler" />
	//class LITEFX_DIRECTX12_API DirectX12InputAssemblerBuilder : public InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12InputAssemblerBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 input assembler builder.
	//	/// </summary>
	//	/// <param name="parent">The parent render pipeline builder.</param>
	//	explicit DirectX12InputAssemblerBuilder(DirectX12RenderPipelineBuilder& parent) noexcept;
	//	DirectX12InputAssemblerBuilder(const DirectX12InputAssemblerBuilder&) noexcept = delete;
	//	DirectX12InputAssemblerBuilder(DirectX12InputAssemblerBuilder&&) noexcept = delete;
	//	virtual ~DirectX12InputAssemblerBuilder() noexcept;

	//public:
	//	/// <summary>
	//	/// Starts building a vertex buffer layout.
	//	/// </summary>
	//	/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
	//	/// <param name="binding">The binding point to bind the vertex buffer to.</param>
	//	virtual DirectX12VertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

	//	/// <summary>
	//	/// Starts building an index buffer layout.
	//	/// </summary>
	//	/// <param name="type">The type of the index buffer.</param>
	//	virtual DirectX12InputAssemblerBuilder& withIndexType(const IndexType& type);

	//	// IInputAssemblerBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12InputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12VertexBufferLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12IndexBufferLayout>&& layout) override;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual DirectX12RenderPipelineBuilder& go() override;
	//};

	///// <summary>
	///// Builds a DirectX 12 <see cref="IRasterizer" />.
	///// </summary>
	///// <seealso cref="DirectX12Rasterizer" />
	//class LITEFX_DIRECTX12_API DirectX12RasterizerBuilder : public RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder> {
	//	LITEFX_IMPLEMENTATION(DirectX12RasterizerBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 input assembler builder.
	//	/// </summary>
	//	/// <param name="parent">The parent render pipeline builder.</param>
	//	explicit DirectX12RasterizerBuilder(DirectX12RenderPipelineBuilder& parent) noexcept;
	//	DirectX12RasterizerBuilder(const DirectX12RasterizerBuilder&) noexcept = delete;
	//	DirectX12RasterizerBuilder(DirectX12RasterizerBuilder&&) noexcept = delete;
	//	virtual ~DirectX12RasterizerBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual DirectX12RenderPipelineBuilder& go() override;

	//	// RasterizerBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withDepthBias(const DepthStencilState::DepthBias& depthBias) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withDepthState(const DepthStencilState::DepthState& depthState) noexcept override;

	//	/// <inheritdoc />
	//	virtual DirectX12RasterizerBuilder& withStencilState(const DepthStencilState::StencilState& stencilState) noexcept override;
	//};

	///// <summary>
	///// Builds a DirectX 12 <see cref="RenderPipeline" />.
	///// </summary>
	///// <seealso cref="DirectX12RenderPipeline" />
	//class LITEFX_DIRECTX12_API DirectX12RenderPipelineBuilder : public RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline> {
	//	LITEFX_IMPLEMENTATION(DirectX12RenderPipelineBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 render pipeline builder.
	//	/// </summary>
	//	/// <param name="renderPass">The parent render pass</param>
	//	/// <param name="id">A unique identifier for the render pipeline.</param>
	//	/// <param name="name">A debug name for the render pipeline.</param>
	//	explicit DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const UInt32& id, const String& name = "");
	//	DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
	//	DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
	//	virtual ~DirectX12RenderPipelineBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual UniquePtr<DirectX12RenderPipeline> go() override;

	//	// RenderPipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12PipelineLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IRasterizer> rasterizer) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<DirectX12InputAssembler> inputAssembler) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IViewport> viewport) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IScissor> scissor) override;

	//	/// <inheritdoc />
	//	virtual DirectX12RenderPipelineBuilder& enableAlphaToCoverage(const bool& enable = true) override;

	//	// DirectX12RenderPipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a <see cref="DirectX12PipelineLayout" /> for the render pipeline.
	//	/// </summary>
	//	virtual DirectX12RenderPipelineLayoutBuilder layout();

	//	/// <summary>
	//	/// Builds a <see cref="DirectX12Rasterizer" /> for the render pipeline.
	//	/// </summary>
	//	virtual DirectX12RasterizerBuilder rasterizer();

	//	/// <summary>
	//	/// Builds a <see cref="DirectX12InputAssembler" /> for the render pipeline.
	//	/// </summary>
	//	virtual DirectX12InputAssemblerBuilder inputAssembler();

	//	/// <summary>
	//	/// Uses the provided rasterizer state for the render pipeline.
	//	/// </summary>
	//	/// <param name="rasterizer">The rasterizer state used by the render pipeline.</param>
	//	virtual DirectX12RenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);

	//	/// <summary>
	//	/// Uses the provided input assembler state for the render pipeline.
	//	/// </summary>
	//	/// <param name="inputAssembler">The input assembler state used by the render pipeline.</param>
	//	virtual DirectX12RenderPipelineBuilder& withInputAssembler(SharedPtr<DirectX12InputAssembler> inputAssembler);

	//	/// <summary>
	//	/// Adds the provided viewport to the render pipeline.
	//	/// </summary>
	//	/// <param name="viewport">The viewport to add to the render pipeline.</param>
	//	virtual DirectX12RenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);

	//	/// <summary>
	//	/// Adds the provided scissor to the render pipeline.
	//	/// </summary>
	//	/// <param name="scissor">The scissor to add to the render pipeline.</param>
	//	virtual DirectX12RenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);
	//};

	///// <summary>
	///// Builds a DirectX 12 <see cref="ComputePipeline" />.
	///// </summary>
	///// <seealso cref="DirectX12ComputePipeline" />
	//class LITEFX_DIRECTX12_API DirectX12ComputePipelineBuilder : public ComputePipelineBuilder<DirectX12ComputePipelineBuilder, DirectX12ComputePipeline> {
	//	LITEFX_IMPLEMENTATION(DirectX12ComputePipelineBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a DirectX 12 compute pipeline builder.
	//	/// </summary>
	//	/// <param name="device">The parent device</param>
	//	/// <param name="name">A debug name for the compute pipeline.</param>
	//	explicit DirectX12ComputePipelineBuilder(const DirectX12Device& device, const String& name = "");
	//	DirectX12ComputePipelineBuilder(DirectX12ComputePipelineBuilder&&) = delete;
	//	DirectX12ComputePipelineBuilder(const DirectX12ComputePipelineBuilder&) = delete;
	//	virtual ~DirectX12ComputePipelineBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual UniquePtr<DirectX12ComputePipeline> go() override;

	//	// ComputePipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<DirectX12PipelineLayout>&& layout) override;

	//	// DirectX12ComputePipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a <see cref="DirectX12PipelineLayout" /> for the compute pipeline.
	//	/// </summary>
	//	virtual DirectX12ComputePipelineLayoutBuilder layout();
	//};

	///// <summary>
	///// Implements the DirectX 12 <see cref="RenderPassBuilder" />.
	///// </summary>
	///// <seealso cref="DirectX12RenderPass" />
	//class LITEFX_DIRECTX12_API DirectX12RenderPassBuilder : public RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass> {
	//	LITEFX_IMPLEMENTATION(DirectX12RenderPassBuilderImpl)

	//public:
	//	explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const String& name = "") noexcept;
	//	explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const MultiSamplingLevel& multiSamplingLevel = MultiSamplingLevel::x1, const String& name = "") noexcept;
	//	explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const UInt32& commandBuffers, const String& name = "") noexcept;
	//	explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const UInt32& commandBuffers, const MultiSamplingLevel& multiSamplingLevel = MultiSamplingLevel::x1, const String& name = "") noexcept;
	//	DirectX12RenderPassBuilder(const DirectX12RenderPassBuilder&) noexcept = delete;
	//	DirectX12RenderPassBuilder(DirectX12RenderPassBuilder&&) noexcept = delete;
	//	virtual ~DirectX12RenderPassBuilder() noexcept;

	//public:
	//	virtual void use(RenderTarget&& target) override;
	//	virtual void use(DirectX12InputAttachmentMapping&& inputAttachment) override;

	//public:
	//	virtual DirectX12RenderPassBuilder& commandBuffers(const UInt32& count) override;
	//	virtual DirectX12RenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual DirectX12RenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual DirectX12RenderPassBuilder& setMultiSamplingLevel(const MultiSamplingLevel& samples = MultiSamplingLevel::x4) override;
	//	virtual DirectX12RenderPassBuilder& inputAttachment(const DirectX12InputAttachmentMapping& inputAttachment) override;
	//	virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const UInt32& outputLocation) override;
	//	virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget) override;

	//public:
	//	[[nodiscard]] virtual UniquePtr<DirectX12RenderPass> go() override;
	//};
}
#endif // defined(BUILD_DEFINE_BUILDERS)