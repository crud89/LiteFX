#pragma once

#include "vulkan_api.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	///// <summary>
	///// Builds a see <see cref="VulkanVertexBufferLayout" />.
	///// </summary>
	///// <seealso cref="VulkanVertexBuffer" />
	///// <seealso cref="VulkanVertexBufferLayout" />
	//class LITEFX_VULKAN_API VulkanVertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder> {
	//public:
	//	using VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder>::VertexBufferLayoutBuilder;

	//public:
	//	/// <inheritdoc />
	//	virtual VulkanVertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

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
	//	virtual VulkanVertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);

	//	/// <summary>
	//	/// Adds an attribute to the vertex buffer layout.
	//	/// </summary>
	//	/// <param name="location">The location, the attribute is bound to.</param>
	//	/// <param name="format">The format of the attribute.</param>
	//	/// <param name="offset">The offset of the attribute within a buffer element.</param>
	//	/// <param name="semantic">The semantic of the attribute.</param>
	//	/// <param name="semanticIndex">The semantic index of the attribute.</param>
	//	virtual VulkanVertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	//};

	///// <summary>
	///// Builds a <see cref="VulkanDescriptorSetLayout" /> for a render pipeline.
	///// </summary>
	///// <seealso cref="VulkanDescriptorSetLayout" />
	///// <seealso cref="VulkanRenderPipeline" />
	//class LITEFX_VULKAN_API VulkanRenderPipelineDescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<VulkanRenderPipelineDescriptorSetLayoutBuilder, VulkanDescriptorSetLayout, VulkanRenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanRenderPipelineDescriptorSetLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan descriptor set layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="space">The space the descriptor set is bound to.</param>
	//	/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
	//	explicit VulkanRenderPipelineDescriptorSetLayoutBuilder(VulkanRenderPipelineLayoutBuilder& parent, const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, const UInt32& poolSize = 1024);
	//	VulkanRenderPipelineDescriptorSetLayoutBuilder(const VulkanRenderPipelineDescriptorSetLayoutBuilder&) = delete;
	//	VulkanRenderPipelineDescriptorSetLayoutBuilder(VulkanRenderPipelineDescriptorSetLayoutBuilder&&) = delete;
	//	virtual ~VulkanRenderPipelineDescriptorSetLayoutBuilder() noexcept;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// DescriptorSetLayoutBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder& withDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout) override;
	//	
	//	/// <inheritdoc />
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder& withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) override;

	//	// VulkanRenderPipelineDescriptorSetLayoutBuilder.
	//public:
	//	/// <summary>
	//	/// Sets the space, the descriptor set is bound to.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder& space(const UInt32& space) noexcept;

	//	/// <summary>
	//	/// Sets the shader stages, the descriptor set is accessible from.
	//	/// </summary>
	//	/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder& shaderStages(const ShaderStage& stages) noexcept;

	//	/// <summary>
	//	/// Sets the size of the descriptor pools used for descriptor set allocations.
	//	/// </summary>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder& poolSize(const UInt32& poolSize) noexcept;
	//};

	///// <summary>
	///// Builds a <see cref="VulkanDescriptorSetLayout" /> for a compute pipeline.
	///// </summary>
	///// <seealso cref="VulkanDescriptorSetLayout" />
	///// <seealso cref="VulkanComputePipeline" />
	//class LITEFX_VULKAN_API VulkanComputePipelineDescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<VulkanComputePipelineDescriptorSetLayoutBuilder, VulkanDescriptorSetLayout, VulkanComputePipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanComputePipelineDescriptorSetLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan descriptor set layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="space">The space the descriptor set is bound to.</param>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
	//	explicit VulkanComputePipelineDescriptorSetLayoutBuilder(VulkanComputePipelineLayoutBuilder& parent, const UInt32& space = 0, const UInt32& poolSize = 1024);
	//	VulkanComputePipelineDescriptorSetLayoutBuilder(const VulkanComputePipelineDescriptorSetLayoutBuilder&) = delete;
	//	VulkanComputePipelineDescriptorSetLayoutBuilder(VulkanComputePipelineDescriptorSetLayoutBuilder&&) = delete;
	//	virtual ~VulkanComputePipelineDescriptorSetLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// DescriptorSetLayoutBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanComputePipelineDescriptorSetLayoutBuilder& withDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual VulkanComputePipelineDescriptorSetLayoutBuilder& withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) override;

	//	// VulkanComputePipelineDescriptorSetLayoutBuilder.
	//public:
	//	/// <summary>
	//	/// Sets the space, the descriptor set is bound to.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	virtual VulkanComputePipelineDescriptorSetLayoutBuilder& space(const UInt32& space) noexcept;

	//	/// <summary>
	//	/// Sets the size of the descriptor pools used for descriptor set allocations.
	//	/// </summary>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
	//	virtual VulkanComputePipelineDescriptorSetLayoutBuilder& poolSize(const UInt32& poolSize) noexcept;
	//};

	///// <summary>
	///// Builds a Vulkan graphics <see cref="ShaderProgram" />.
	///// </summary>
	///// <seealso cref="VulkanShaderProgram" />
	//class LITEFX_VULKAN_API VulkanGraphicsShaderProgramBuilder : public GraphicsShaderProgramBuilder<VulkanGraphicsShaderProgramBuilder, VulkanShaderProgram, VulkanRenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanGraphicsShaderProgramBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan graphics shader program builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	explicit VulkanGraphicsShaderProgramBuilder(VulkanRenderPipelineLayoutBuilder& parent);
	//	VulkanGraphicsShaderProgramBuilder(const VulkanGraphicsShaderProgramBuilder&) = delete;
	//	VulkanGraphicsShaderProgramBuilder(VulkanGraphicsShaderProgramBuilder&&) = delete;
	//	virtual ~VulkanGraphicsShaderProgramBuilder() noexcept;

	//	// Builder interface.
	//protected:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// ShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;

	//	// GraphicsShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;

	//	/// <inheritdoc />
	//	virtual VulkanGraphicsShaderProgramBuilder& withFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;
	//};

	///// <summary>
	///// Builds a Vulkan compute <see cref="ShaderProgram" />.
	///// </summary>
	///// <seealso cref="VulkanShaderProgram" />
	//class LITEFX_VULKAN_API VulkanComputeShaderProgramBuilder : public ComputeShaderProgramBuilder<VulkanComputeShaderProgramBuilder, VulkanShaderProgram, VulkanComputePipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanComputeShaderProgramBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan compute shader program builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	explicit VulkanComputeShaderProgramBuilder(VulkanComputePipelineLayoutBuilder& parent);
	//	VulkanComputeShaderProgramBuilder(const VulkanComputeShaderProgramBuilder&) = delete;
	//	VulkanComputeShaderProgramBuilder(VulkanComputeShaderProgramBuilder&&) = delete;
	//	virtual ~VulkanComputeShaderProgramBuilder() noexcept;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// ShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanComputeShaderProgramBuilder& withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;

	//	// ComputeShaderProgramBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanComputeShaderProgramBuilder& withComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="PushConstantsLayout" /> for a <see cref="RenderPipeline" />.
	///// </summary>
	///// <seealso cref="VulkanPushConstantsLayout" />
	//class LITEFX_VULKAN_API VulkanRenderPipelinePushConstantsLayoutBuilder : public PushConstantsLayoutBuilder<VulkanRenderPipelinePushConstantsLayoutBuilder, VulkanPushConstantsLayout, VulkanRenderPipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanRenderPipelinePushConstantsLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan render pipeline push constants layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	explicit VulkanRenderPipelinePushConstantsLayoutBuilder(VulkanRenderPipelineLayoutBuilder& parent, const UInt32& size);
	//	VulkanRenderPipelinePushConstantsLayoutBuilder(const VulkanRenderPipelinePushConstantsLayoutBuilder&) = delete;
	//	VulkanRenderPipelinePushConstantsLayoutBuilder(VulkanRenderPipelinePushConstantsLayoutBuilder&&) = delete;
	//	virtual ~VulkanRenderPipelinePushConstantsLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual VulkanRenderPipelineLayoutBuilder& go() override;

	//	// PushConstantsLayoutBuilder interface.
	//public:
	//	virtual VulkanRenderPipelinePushConstantsLayoutBuilder& addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) override;
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="PushConstantsLayout" /> for a <see cref="ComputePipeline" />.
	///// </summary>
	///// <seealso cref="VulkanPushConstantsLayout" />
	//class LITEFX_VULKAN_API VulkanComputePipelinePushConstantsLayoutBuilder : public PushConstantsLayoutBuilder<VulkanComputePipelinePushConstantsLayoutBuilder, VulkanPushConstantsLayout, VulkanComputePipelineLayoutBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanComputePipelinePushConstantsLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan compute pipeline push constants layout builder.
	//	/// </summary>
	//	/// <param name="parent">The parent pipeline layout builder.</param>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	explicit VulkanComputePipelinePushConstantsLayoutBuilder(VulkanComputePipelineLayoutBuilder& parent, const UInt32& size);
	//	VulkanComputePipelinePushConstantsLayoutBuilder(const VulkanComputePipelinePushConstantsLayoutBuilder&) = delete;
	//	VulkanComputePipelinePushConstantsLayoutBuilder(VulkanComputePipelinePushConstantsLayoutBuilder&&) = delete;
	//	virtual ~VulkanComputePipelinePushConstantsLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual VulkanComputePipelineLayoutBuilder& go() override;

	//	// PushConstantsLayoutBuilder interface.
	//public:
	//	virtual VulkanComputePipelinePushConstantsLayoutBuilder& addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) override;
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="PipelineLayout" /> for a render pipeline.
	///// </summary>
	///// <seealso cref="VulkanPipelineLayout" />
	///// <seealso cref="VulkanRenderPipeline" />
	//class LITEFX_VULKAN_API VulkanRenderPipelineLayoutBuilder : public PipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanPipelineLayout> {
	//	LITEFX_IMPLEMENTATION(VulkanRenderPipelineLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a new Vulkan render pipeline layout builder.
	//	/// </summary>
	//	explicit VulkanRenderPipelineLayoutBuilder();
	//	VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineLayoutBuilder&&) = delete;
	//	VulkanRenderPipelineLayoutBuilder(const VulkanRenderPipelineLayoutBuilder&) = delete;
	//	virtual ~VulkanRenderPipelineLayoutBuilder() noexcept;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// RenderPipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanShaderProgram>&& program) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanDescriptorSetLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanPushConstantsLayout>&& layout) override;

	//	// VulkanRenderPipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a shader program for the render pipeline layout.
	//	/// </summary>
	//	virtual VulkanGraphicsShaderProgramBuilder shaderProgram();

	//	/// <summary>
	//	/// Builds a new descriptor set for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocation.</param>
	//	virtual VulkanRenderPipelineDescriptorSetLayoutBuilder descriptorSet(const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, const UInt32& poolSize = 1024);

	//	/// <summary>
	//	/// Builds a new push constants layout for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	virtual VulkanRenderPipelinePushConstantsLayoutBuilder pushConstants(const UInt32& size);
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="PipelineLayout" /> for a compute pipeline.
	///// </summary>
	///// <seealso cref="VulkanPipelineLayout" />
	///// <seealso cref="VulkanComputePipeline" />
	//class LITEFX_VULKAN_API VulkanComputePipelineLayoutBuilder : public PipelineLayoutBuilder<VulkanComputePipelineLayoutBuilder, VulkanPipelineLayout> {
	//	LITEFX_IMPLEMENTATION(VulkanComputePipelineLayoutBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a new Vulkan compute pipeline layout builder.
	//	/// </summary>
	//	explicit VulkanComputePipelineLayoutBuilder();
	//	VulkanComputePipelineLayoutBuilder(VulkanComputePipelineLayoutBuilder&&) = delete;
	//	VulkanComputePipelineLayoutBuilder(const VulkanComputePipelineLayoutBuilder&) = delete;
	//	virtual ~VulkanComputePipelineLayoutBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void build() override;

	//	// ComputePipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanShaderProgram>&& program) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanDescriptorSetLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanPushConstantsLayout>&& layout) override;

	//	// VulkanComputePipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a shader program for the render pipeline layout.
	//	/// </summary>
	//	virtual VulkanComputeShaderProgramBuilder shaderProgram();

	//	/// <summary>
	//	/// Builds a new descriptor set for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="space">The space, the descriptor set is bound to.</param>
	//	/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocation.</param>
	//	virtual VulkanComputePipelineDescriptorSetLayoutBuilder descriptorSet(const UInt32& space = 0, const UInt32& poolSize = 1024);

	//	/// <summary>
	//	/// Builds a new push constants layout for the render pipeline layout.
	//	/// </summary>
	//	/// <param name="size">The size of the push constants backing memory.</param>
	//	virtual VulkanComputePipelinePushConstantsLayoutBuilder pushConstants(const UInt32& size);
	//};



	////
	//// 
	//// TODO: Refactor the following:
	////
	////



	///// <summary>
	///// Builds a <see cref="VulkanInputAssembler" />.
	///// </summary>
	///// <seealso cref="VulkanInputAssembler" />
	//class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanInputAssemblerBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan input assembler builder.
	//	/// </summary>
	//	/// <param name="parent">The parent render pipeline builder.</param>
	//	explicit VulkanInputAssemblerBuilder(VulkanRenderPipelineBuilder& parent) noexcept;
	//	VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) noexcept = delete;
	//	VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
	//	virtual ~VulkanInputAssemblerBuilder() noexcept;

	//public:
	//	/// <summary>
	//	/// Starts building a vertex buffer layout.
	//	/// </summary>
	//	/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
	//	/// <param name="binding">The binding point to bind the vertex buffer to.</param>
	//	virtual VulkanVertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

	//	/// <summary>
	//	/// Starts building an index buffer layout.
	//	/// </summary>
	//	/// <param name="type">The type of the index buffer.</param>
	//	virtual VulkanInputAssemblerBuilder& withIndexType(const IndexType& type);

	//	// IInputAssemblerBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanInputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanVertexBufferLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanIndexBufferLayout>&& layout) override;

	//	// Builder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual VulkanRenderPipelineBuilder& go() override;
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="IRasterizer" />.
	///// </summary>
	///// <seealso cref="VulkanRasterizer" />
	//class LITEFX_VULKAN_API VulkanRasterizerBuilder : public RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineBuilder> {
	//	LITEFX_IMPLEMENTATION(VulkanRasterizerBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan input assembler builder.
	//	/// </summary>
	//	/// <param name="parent">The parent render pipeline builder.</param>
	//	explicit VulkanRasterizerBuilder(VulkanRenderPipelineBuilder& parent) noexcept;
	//	VulkanRasterizerBuilder(const VulkanRasterizerBuilder&) noexcept = delete;
	//	VulkanRasterizerBuilder(VulkanRasterizerBuilder&&) noexcept = delete;
	//	virtual ~VulkanRasterizerBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual VulkanRenderPipelineBuilder& go() override;

	//	// RasterizerBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withDepthBias(const DepthStencilState::DepthBias& depthBias) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withDepthState(const DepthStencilState::DepthState& depthState) noexcept override;

	//	/// <inheritdoc />
	//	virtual VulkanRasterizerBuilder& withStencilState(const DepthStencilState::StencilState& stencilState) noexcept override;
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="RenderPipeline" />.
	///// </summary>
	///// <seealso cref="VulkanRenderPipeline" />
	//class LITEFX_VULKAN_API VulkanRenderPipelineBuilder : public RenderPipelineBuilder<VulkanRenderPipelineBuilder, VulkanRenderPipeline> {
	//	LITEFX_IMPLEMENTATION(VulkanRenderPipelineBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan render pipeline builder.
	//	/// </summary>
	//	/// <param name="renderPass">The parent render pass</param>
	//	/// <param name="id">A unique identifier for the render pipeline.</param>
	//	/// <param name="name">A debug name for the render pipeline.</param>
	//	explicit VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const UInt32& id, const String& name = "");
	//	VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) = delete;
	//	VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) = delete;
	//	virtual ~VulkanRenderPipelineBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual UniquePtr<VulkanRenderPipeline> go() override;

	//	// RenderPipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanPipelineLayout>&& layout) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IRasterizer> rasterizer) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<VulkanInputAssembler> inputAssembler) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IViewport> viewport) override;

	//	/// <inheritdoc />
	//	virtual void use(SharedPtr<IScissor> scissor) override;

	//	/// <inheritdoc />
	//	virtual VulkanRenderPipelineBuilder& enableAlphaToCoverage(const bool& enable = true) override;

	//	// VulkanRenderPipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a <see cref="VulkanPipelineLayout" /> for the render pipeline.
	//	/// </summary>
	//	virtual VulkanRenderPipelineLayoutBuilder layout();

	//	/// <summary>
	//	/// Builds a <see cref="VulkanRasterizer" /> for the render pipeline.
	//	/// </summary>
	//	virtual VulkanRasterizerBuilder rasterizer();

	//	/// <summary>
	//	/// Builds a <see cref="VulkanInputAssembler" /> for the render pipeline.
	//	/// </summary>
	//	virtual VulkanInputAssemblerBuilder inputAssembler();

	//	/// <summary>
	//	/// Uses the provided rasterizer state for the render pipeline.
	//	/// </summary>
	//	/// <param name="rasterizer">The rasterizer state used by the render pipeline.</param>
	//	virtual VulkanRenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);

	//	/// <summary>
	//	/// Uses the provided input assembler state for the render pipeline.
	//	/// </summary>
	//	/// <param name="inputAssembler">The input assembler state used by the render pipeline.</param>
	//	virtual VulkanRenderPipelineBuilder& withInputAssembler(SharedPtr<VulkanInputAssembler> inputAssembler);

	//	/// <summary>
	//	/// Adds the provided viewport to the render pipeline.
	//	/// </summary>
	//	/// <param name="viewport">The viewport to add to the render pipeline.</param>
	//	virtual VulkanRenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);

	//	/// <summary>
	//	/// Adds the provided scissor to the render pipeline.
	//	/// </summary>
	//	/// <param name="scissor">The scissor to add to the render pipeline.</param>
	//	virtual VulkanRenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);
	//};

	///// <summary>
	///// Builds a Vulkan <see cref="ComputePipeline" />.
	///// </summary>
	///// <seealso cref="VulkanComputePipeline" />
	//class LITEFX_VULKAN_API VulkanComputePipelineBuilder : public ComputePipelineBuilder<VulkanComputePipelineBuilder, VulkanComputePipeline> {
	//	LITEFX_IMPLEMENTATION(VulkanComputePipelineBuilderImpl);

	//public:
	//	/// <summary>
	//	/// Initializes a Vulkan compute pipeline builder.
	//	/// </summary>
	//	/// <param name="device">The parent device</param>
	//	/// <param name="name">A debug name for the compute pipeline.</param>
	//	explicit VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name = "");
	//	VulkanComputePipelineBuilder(VulkanComputePipelineBuilder&&) = delete;
	//	VulkanComputePipelineBuilder(const VulkanComputePipelineBuilder&) = delete;
	//	virtual ~VulkanComputePipelineBuilder() noexcept;

	//	// IBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	[[nodiscard]] virtual UniquePtr<VulkanComputePipeline> go() override;

	//	// ComputePipelineBuilder interface.
	//public:
	//	/// <inheritdoc />
	//	virtual void use(UniquePtr<VulkanPipelineLayout>&& layout) override;

	//	// VulkanComputePipelineBuilder.
	//public:
	//	/// <summary>
	//	/// Builds a <see cref="VulkanPipelineLayout" /> for the compute pipeline.
	//	/// </summary>
	//	virtual VulkanComputePipelineLayoutBuilder layout();
	//};

	///// <summary>
	///// Implements the Vulkan <see cref="RenderPassBuilder" />.
	///// </summary>
	///// <seealso cref="VulkanRenderPass" />
	//class LITEFX_VULKAN_API VulkanRenderPassBuilder : public RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass> {
	//	LITEFX_IMPLEMENTATION(VulkanRenderPassBuilderImpl)

	//public:
	//	explicit VulkanRenderPassBuilder(const VulkanDevice& device, const String& name = "") noexcept;
	//	explicit VulkanRenderPassBuilder(const VulkanDevice& device, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const String& name = "") noexcept;
	//	explicit VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const String& name = "") noexcept;
	//	explicit VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const MultiSamplingLevel& multiSamplingLevel, const String& name = "") noexcept;
	//	VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) noexcept = delete;
	//	VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
	//	virtual ~VulkanRenderPassBuilder() noexcept;
	//public:
	//	virtual void use(RenderTarget&& target) override;
	//	virtual void use(VulkanInputAttachmentMapping&& inputAttachment) override;

	//public:
	//	virtual VulkanRenderPassBuilder& commandBuffers(const UInt32& count) override;
	//	virtual VulkanRenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual VulkanRenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual VulkanRenderPassBuilder& renderTarget(VulkanInputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual VulkanRenderPassBuilder& renderTarget(VulkanInputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	//	virtual VulkanRenderPassBuilder& setMultiSamplingLevel(const MultiSamplingLevel& samples = MultiSamplingLevel::x4) override;
	//	virtual VulkanRenderPassBuilder& inputAttachment(const VulkanInputAttachmentMapping& inputAttachment) override;
	//	virtual VulkanRenderPassBuilder& inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const UInt32& outputLocation) override;
	//	virtual VulkanRenderPassBuilder& inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget) override;

	//public:
	//	[[nodiscard]] virtual UniquePtr<VulkanRenderPass> go() override;
	//};
}
#endif // defined(BUILD_DEFINE_BUILDERS)