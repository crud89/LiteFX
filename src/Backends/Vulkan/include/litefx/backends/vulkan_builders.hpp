#pragma once

#include <litefx/rendering_builders.hpp>
#include "vulkan_api.hpp"
#include "vulkan.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Builds a Vulkan  <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="VulkanShaderProgram" />
	class LITEFX_VULKAN_API VulkanShaderProgramBuilder : public ShaderProgramBuilder<VulkanShaderProgramBuilder, VulkanShaderProgram> {
		LITEFX_IMPLEMENTATION(VulkanShaderProgramBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan graphics shader program builder.
		/// </summary>
		/// <param name="device">The parent device that hosts the shader program.</param>
		explicit VulkanShaderProgramBuilder(const VulkanDevice& device);
		VulkanShaderProgramBuilder(const VulkanShaderProgramBuilder&) = delete;
		VulkanShaderProgramBuilder(VulkanShaderProgramBuilder&&) = delete;
		virtual ~VulkanShaderProgramBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		virtual void build() override;

		// ShaderProgramBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual VulkanShaderProgramBuilder& withComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="VulkanRasterizer" />
	class LITEFX_VULKAN_API VulkanRasterizerBuilder : public RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer> {
		LITEFX_IMPLEMENTATION(VulkanRasterizerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan input assembler builder.
		/// </summary>
		explicit VulkanRasterizerBuilder() noexcept;
		VulkanRasterizerBuilder(const VulkanRasterizerBuilder&) noexcept = delete;
		VulkanRasterizerBuilder(VulkanRasterizerBuilder&&) noexcept = delete;
		virtual ~VulkanRasterizerBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// RasterizerBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& polygonMode(const PolygonMode& mode = PolygonMode::Solid) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& cullMode(const CullMode& cullMode = CullMode::BackFaces) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& cullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& lineWidth(const Float& lineWidth = 1.f) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& depthBias(const DepthStencilState::DepthBias& depthBias) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& depthState(const DepthStencilState::DepthState& depthState) noexcept override;

		/// <inheritdoc />
		virtual VulkanRasterizerBuilder& stencilState(const DepthStencilState::StencilState& stencilState) noexcept override;
	};

	/// <summary>
	/// Builds a see <see cref="VulkanVertexBufferLayout" />.
	/// </summary>
	/// <seealso cref="VulkanVertexBuffer" />
	/// <seealso cref="VulkanVertexBufferLayout" />
	class LITEFX_VULKAN_API VulkanVertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<VulkanVertexBufferLayoutBuilder, VulkanVertexBufferLayout, VulkanInputAssemblerBuilder>::VertexBufferLayoutBuilder;

	public:
		/// <inheritdoc />
		virtual VulkanVertexBufferLayoutBuilder& withAttribute(UniquePtr<BufferAttribute>&& attribute) override;

	public:
		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <remarks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </remarks>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual VulkanVertexBufferLayoutBuilder& withAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);

		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <param name="location">The location, the attribute is bound to.</param>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual VulkanVertexBufferLayoutBuilder& withAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	};

	/// <summary>
	/// Builds a <see cref="VulkanInputAssembler" />.
	/// </summary>
	/// <seealso cref="VulkanInputAssembler" />
	class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan input assembler builder.
		/// </summary>
		explicit VulkanInputAssemblerBuilder() noexcept;
		VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) noexcept = delete;
		VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
		virtual ~VulkanInputAssemblerBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// InputAssemblerBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanInputAssemblerBuilder& topology(const PrimitiveTopology& topology) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanVertexBufferLayout>&& layout) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanIndexBufferLayout>&& layout) override;

		// VulkanInputAssemblerBuilder interface.
	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		virtual VulkanVertexBufferLayoutBuilder vertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		virtual VulkanInputAssemblerBuilder& indexType(const IndexType& type);
	};

	/// <summary>
	/// Builds a <see cref="VulkanDescriptorSetLayout" /> for a <see cref="VulkanPipelineLayout" />.
	/// </summary>
	/// <seealso cref="VulkanDescriptorSetLayout" />
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API VulkanDescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<VulkanDescriptorSetLayoutBuilder, VulkanDescriptorSetLayout, VulkanPipelineLayoutBuilder> {
		LITEFX_IMPLEMENTATION(VulkanDescriptorSetLayoutBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan descriptor set layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="space">The space the descriptor set is bound to.</param>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
		explicit VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, const UInt32& poolSize = 1024);
		VulkanDescriptorSetLayoutBuilder(const VulkanDescriptorSetLayoutBuilder&) = delete;
		VulkanDescriptorSetLayoutBuilder(VulkanDescriptorSetLayoutBuilder&&) = delete;
		virtual ~VulkanDescriptorSetLayoutBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// DescriptorSetLayoutBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanDescriptorSetLayoutBuilder& withDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout) override;
		
		/// <inheritdoc />
		virtual VulkanDescriptorSetLayoutBuilder& withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) override;

		/// <inheritdoc />
		virtual VulkanDescriptorSetLayoutBuilder& withStaticSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f) override;

		// VulkanDescriptorSetLayoutBuilder.
	public:
		/// <summary>
		/// Sets the space, the descriptor set is bound to.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		virtual VulkanDescriptorSetLayoutBuilder& space(const UInt32& space) noexcept;

		/// <summary>
		/// Sets the shader stages, the descriptor set is accessible from.
		/// </summary>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		virtual VulkanDescriptorSetLayoutBuilder& shaderStages(const ShaderStage& stages) noexcept;

		/// <summary>
		/// Sets the size of the descriptor pools used for descriptor set allocations.
		/// </summary>
		/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
		virtual VulkanDescriptorSetLayoutBuilder& poolSize(const UInt32& poolSize) noexcept;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="PushConstantsLayout" /> for a <see cref="VulkanPipelineLayout" />.
	/// </summary>
	/// <seealso cref="VulkanPushConstantsLayout" />
	class LITEFX_VULKAN_API VulkanPushConstantsLayoutBuilder : public PushConstantsLayoutBuilder<VulkanPushConstantsLayoutBuilder, VulkanPushConstantsLayout, VulkanPipelineLayoutBuilder> {
		LITEFX_IMPLEMENTATION(VulkanPushConstantsLayoutBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan  pipeline push constants layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="size">The size of the push constants backing memory.</param>
		explicit VulkanPushConstantsLayoutBuilder(VulkanPipelineLayoutBuilder& parent, const UInt32& size);
		VulkanPushConstantsLayoutBuilder(const VulkanPushConstantsLayoutBuilder&) = delete;
		VulkanPushConstantsLayoutBuilder(VulkanPushConstantsLayoutBuilder&&) = delete;
		virtual ~VulkanPushConstantsLayoutBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// PushConstantsLayoutBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanPushConstantsLayoutBuilder& withRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="PipelineLayout" /> for a pipeline.
	/// </summary>
	/// <seealso cref="VulkanPipelineLayout" />
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API VulkanPipelineLayoutBuilder : public PipelineLayoutBuilder<VulkanPipelineLayoutBuilder, VulkanPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanPipelineLayoutBuilderImpl);
		friend class VulkanDescriptorSetLayoutBuilder;

	public:
		/// <summary>
		/// Initializes a new Vulkan pipeline layout builder.
		/// </summary>
		/// <param name="device">The parent device, the pipeline layout is created from.</param>
		explicit VulkanPipelineLayoutBuilder(const VulkanDevice& device);
		VulkanPipelineLayoutBuilder(VulkanPipelineLayoutBuilder&&) = delete;
		VulkanPipelineLayoutBuilder(const VulkanPipelineLayoutBuilder&) = delete;
		virtual ~VulkanPipelineLayoutBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// PipelineLayoutBuilder interface.
	public:
		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanDescriptorSetLayout>&& layout) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<VulkanPushConstantsLayout>&& layout) override;

		// VulkanPipelineLayoutBuilder.
	public:
		/// <summary>
		/// Builds a new descriptor set for the pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		/// <param name="poolSize">The size of the descriptor pools used for descriptor set allocation.</param>
		virtual VulkanDescriptorSetLayoutBuilder descriptorSet(const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, const UInt32& poolSize = 1024);

		/// <summary>
		/// Builds a new push constants layout for the pipeline layout.
		/// </summary>
		/// <param name="size">The size of the push constants backing memory.</param>
		virtual VulkanPushConstantsLayoutBuilder pushConstants(const UInt32& size);

	private:
		/// <summary>
		/// Returns the device, the builder has been initialized with.
		/// </summary>
		/// <returns>A reference of the device, the builder has been initialized with.</returns>
		virtual const VulkanDevice& device() const noexcept;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPipeline" />
	class LITEFX_VULKAN_API VulkanRenderPipelineBuilder : public RenderPipelineBuilder<VulkanRenderPipelineBuilder, VulkanRenderPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan render pipeline builder.
		/// </summary>
		/// <param name="renderPass">The parent render pass</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		explicit VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name = "");
		VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) = delete;
		VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) = delete;
		virtual ~VulkanRenderPipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// RenderPipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& shaderProgram(SharedPtr<VulkanShaderProgram> shaderProgram) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& layout(SharedPtr<VulkanPipelineLayout> layout) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& rasterizer(SharedPtr<VulkanRasterizer> rasterizer) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& inputAssembler(SharedPtr<VulkanInputAssembler> inputAssembler) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& viewport(SharedPtr<IViewport> viewport) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& scissor(SharedPtr<IScissor> scissor) override;

		/// <inheritdoc />
		virtual VulkanRenderPipelineBuilder& enableAlphaToCoverage(const bool& enable = true) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API VulkanComputePipelineBuilder : public ComputePipelineBuilder<VulkanComputePipelineBuilder, VulkanComputePipeline> {
		LITEFX_IMPLEMENTATION(VulkanComputePipelineBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan compute pipeline builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the compute pipeline.</param>
		explicit VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name = "");
		VulkanComputePipelineBuilder(VulkanComputePipelineBuilder&&) = delete;
		VulkanComputePipelineBuilder(const VulkanComputePipelineBuilder&) = delete;
		virtual ~VulkanComputePipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// ComputePipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanComputePipelineBuilder& shaderProgram(SharedPtr<VulkanShaderProgram> program) override;

		/// <inheritdoc />
		virtual VulkanComputePipelineBuilder& layout(SharedPtr<VulkanPipelineLayout> layout) override;
	};

	/// <summary>
	/// Implements the Vulkan <see cref="RenderPassBuilder" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPass" />
	class LITEFX_VULKAN_API VulkanRenderPassBuilder : public RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass> {
		LITEFX_IMPLEMENTATION(VulkanRenderPassBuilderImpl)

	public:
		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="samples">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="multiSamplingLevel">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const MultiSamplingLevel& multiSamplingLevel, const String& name = "") noexcept;
		VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) noexcept = delete;
		VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
		virtual ~VulkanRenderPassBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// RenderPassBuilder interface.
	public:
		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& commandBuffers(const UInt32& count) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& multiSamplingLevel(const MultiSamplingLevel& samples) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& renderTarget(VulkanInputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& renderTarget(VulkanInputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& inputAttachment(const VulkanInputAttachmentMapping& inputAttachment) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const UInt32& outputLocation) override;

		/// <inheritdoc />
		virtual VulkanRenderPassBuilder& inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget) override;
	};
}
#endif // defined(BUILD_DEFINE_BUILDERS)