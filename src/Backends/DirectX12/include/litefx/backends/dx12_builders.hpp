#pragma once

#include <litefx/rendering_builders.hpp>
#include "dx12_api.hpp"
#include "dx12.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Builds a DirectX 12  <see cref="Barrier" />.
	/// </summary>
	/// <seealso cref="DirectX12Barrier" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12BarrierBuilder final : public BarrierBuilder<DirectX12Barrier> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 barrier builder.
		/// </summary>
		constexpr inline explicit DirectX12BarrierBuilder();
		DirectX12BarrierBuilder(const DirectX12BarrierBuilder&) = delete;
		DirectX12BarrierBuilder(DirectX12BarrierBuilder&&) = delete;
		constexpr inline virtual ~DirectX12BarrierBuilder() noexcept;

		// BarrierBuilder interface.
	public:
		/// <inheritdoc />
		constexpr inline void setupStages(PipelineStage waitFor, PipelineStage continueWith) override;

		/// <inheritdoc />
		constexpr inline void setupGlobalBarrier(ResourceAccess before, ResourceAccess after) override;

		/// <inheritdoc />
		constexpr inline void setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after) override;

		/// <inheritdoc />
		constexpr inline void setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgram" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12ShaderProgramBuilder final : public ShaderProgramBuilder<DirectX12ShaderProgram> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderProgramBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX 12  shader program builder.
		/// </summary>
		/// <param name="device">The parent device that hosts the shader program.</param>
		constexpr inline explicit DirectX12ShaderProgramBuilder(const DirectX12Device& device);
		DirectX12ShaderProgramBuilder(const DirectX12ShaderProgramBuilder&) = delete;
		DirectX12ShaderProgramBuilder(DirectX12ShaderProgramBuilder&&) = delete;
		constexpr inline virtual ~DirectX12ShaderProgramBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;

		// ShaderProgramBuilder interface.
	protected:
		/// <inheritdoc />
		constexpr inline UniquePtr<DirectX12ShaderModule> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint) override;

		/// <inheritdoc />
		constexpr inline UniquePtr<DirectX12ShaderModule> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="DirectX12Rasterizer" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12RasterizerBuilder final : public RasterizerBuilder<DirectX12Rasterizer> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 input assembler builder.
		/// </summary>
		constexpr inline explicit DirectX12RasterizerBuilder() noexcept;
		DirectX12RasterizerBuilder(const DirectX12RasterizerBuilder&) noexcept = delete;
		DirectX12RasterizerBuilder(DirectX12RasterizerBuilder&&) noexcept = delete;
		constexpr inline virtual ~DirectX12RasterizerBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;
	};
	
	/// <summary>
	/// Builds a see <see cref="DirectX12VertexBufferLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12VertexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12VertexBufferLayoutBuilder final : public VertexBufferLayoutBuilder<DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder>::VertexBufferLayoutBuilder;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;

		// DirectX12VertexBufferLayoutBuilder interface.
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
		template <typename TSelf>
		constexpr inline [[nodiscard]] auto withAttribute(this TSelf&& self, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Unknown, UInt32 semanticIndex = 0) -> TSelf&& {
			self.addAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(this->instance()->attributes().size()), offset, format, semantic, semanticIndex)));
			return self;
		}

		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <param name="location">The location, the attribute is bound to.</param>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		template <typename TSelf>
		constexpr inline [[nodiscard]] auto withAttribute(this TSelf&& self, UInt32 location, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Unknown, UInt32 semanticIndex = 0) -> TSelf&& {
			self.addAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format, semantic, semanticIndex)));
			return self;
		}
	};

	/// <summary>
	/// Builds a <see cref="DirectX12InputAssembler" />.
	/// </summary>
	/// <seealso cref="DirectX12InputAssembler" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12InputAssemblerBuilder final : public InputAssemblerBuilder<DirectX12InputAssembler> {
		LITEFX_IMPLEMENTATION(DirectX12InputAssemblerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX 12 input assembler builder.
		/// </summary>
		constexpr inline explicit DirectX12InputAssemblerBuilder() noexcept;
		DirectX12InputAssemblerBuilder(const DirectX12InputAssemblerBuilder&) noexcept = delete;
		DirectX12InputAssemblerBuilder(DirectX12InputAssemblerBuilder&&) noexcept = delete;
		constexpr inline virtual ~DirectX12InputAssemblerBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline virtual void build() override;

	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		constexpr inline DirectX12VertexBufferLayoutBuilder vertexBuffer(size_t elementSize, UInt32 binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		template <typename TSelf>
		constexpr inline auto indexType(this TSelf&& self, IndexType type) -> TSelf&& {
			self.use(makeUnique<DirectX12IndexBufferLayout>(type));
			return self;
		}
	};

	/// <summary>
	/// Builds a <see cref="DirectX12DescriptorSetLayout" /> for a pipeline layout.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="DirectX12RenderPipeline" />
	/// <seealso cref="DirectX12ComputePipeline" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12DescriptorSetLayoutBuilder final : public DescriptorSetLayoutBuilder<DirectX12DescriptorSetLayout, DirectX12PipelineLayoutBuilder> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 descriptor set layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="space">The space the descriptor set is bound to.</param>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		/// <param name="poolSize">Ignored for DirectX 12, but required for compatibility.</param>
		/// <param name="maxUnboundedArraySize">Ignored for DirectX 12, but required for compatibility.</param>
		constexpr inline explicit DirectX12DescriptorSetLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 space = 0, ShaderStage stages = ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, UInt32 poolSize = 0, UInt32 maxUnboundedArraySize = 0);
		DirectX12DescriptorSetLayoutBuilder(const DirectX12DescriptorSetLayoutBuilder&) = delete;
		DirectX12DescriptorSetLayoutBuilder(DirectX12DescriptorSetLayoutBuilder&&) = delete;
		constexpr inline virtual ~DirectX12DescriptorSetLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;

		// DescriptorSetLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		constexpr inline UniquePtr<DirectX12DescriptorLayout> makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors) override;

		/// <inheritdoc />
		constexpr inline UniquePtr<DirectX12DescriptorLayout> makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="PushConstantsLayout" /> for a pipeline layout.
	/// </summary>
	/// <seealso cref="DirectX12PushConstantsLayout" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12PushConstantsLayoutBuilder final : public PushConstantsLayoutBuilder<DirectX12PushConstantsLayout, DirectX12PipelineLayoutBuilder> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 render pipeline push constants layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="size">The size of the push constants backing memory.</param>
		constexpr inline explicit DirectX12PushConstantsLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 size);
		DirectX12PushConstantsLayoutBuilder(const DirectX12PushConstantsLayoutBuilder&) = delete;
		DirectX12PushConstantsLayoutBuilder(DirectX12PushConstantsLayoutBuilder&&) = delete;
		constexpr inline virtual ~DirectX12PushConstantsLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;

		// PushConstantsLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		constexpr inline UniquePtr<DirectX12PushConstantsRange> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="PipelineLayout" /> for a pipeline.
	/// </summary>
	/// <seealso cref="DirectX12PipelineLayout" />
	/// <seealso cref="DirectX12RenderPipeline" />
	/// <seealso cref="DirectX12ComputePipeline" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12PipelineLayoutBuilder final : public PipelineLayoutBuilder<DirectX12PipelineLayout> {
		LITEFX_IMPLEMENTATION(DirectX12PipelineLayoutBuilderImpl);
		friend class DirectX12DescriptorSetLayoutBuilder;

	public:
		/// <summary>
		/// Initializes a new DirectX 12 pipeline layout builder.
		/// </summary>
		constexpr inline DirectX12PipelineLayoutBuilder(const DirectX12Device& device);
		DirectX12PipelineLayoutBuilder(DirectX12PipelineLayoutBuilder&&) = delete;
		DirectX12PipelineLayoutBuilder(const DirectX12PipelineLayoutBuilder&) = delete;
		constexpr inline virtual ~DirectX12PipelineLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		constexpr inline void build() override;

		// DirectX12PipelineLayoutBuilder.
	public:
		/// <summary>
		/// Builds a new descriptor set for the pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		/// <param name="poolSize">Unused for this backend.</param>
		constexpr inline DirectX12DescriptorSetLayoutBuilder descriptorSet(UInt32 space = 0, ShaderStage stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, UInt32 poolSize = 0);

		/// <summary>
		/// Builds a new push constants layout for the pipeline layout.
		/// </summary>
		/// <param name="size">The size of the push constants backing memory.</param>
		constexpr inline DirectX12PushConstantsLayoutBuilder pushConstants(UInt32 size);

	private:
		/// <summary>
		/// Returns the device, the builder has been initialized with.
		/// </summary>
		/// <returns>A reference of the device, the builder has been initialized with.</returns>
		constexpr inline const DirectX12Device& device() const noexcept;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipeline" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineBuilder : public RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX 12 render pipeline builder.
		/// </summary>
		/// <param name="renderPass">The parent render pass</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		explicit DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const String& name = "");
		DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
		DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
		virtual ~DirectX12RenderPipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// RenderPipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& shaderProgram(SharedPtr<DirectX12ShaderProgram> shaderProgram) override;

		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& layout(SharedPtr<DirectX12PipelineLayout> layout) override;

		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& rasterizer(SharedPtr<DirectX12Rasterizer> rasterizer) override;

		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& inputAssembler(SharedPtr<DirectX12InputAssembler> inputAssembler) override;

		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& enableAlphaToCoverage(const bool& enable = true) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12ComputePipeline" />
	class LITEFX_DIRECTX12_API DirectX12ComputePipelineBuilder : public ComputePipelineBuilder<DirectX12ComputePipelineBuilder, DirectX12ComputePipeline> {
		LITEFX_IMPLEMENTATION(DirectX12ComputePipelineBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX 12 compute pipeline builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the compute pipeline.</param>
		explicit DirectX12ComputePipelineBuilder(const DirectX12Device& device, const String& name = "");
		DirectX12ComputePipelineBuilder(DirectX12ComputePipelineBuilder&&) = delete;
		DirectX12ComputePipelineBuilder(const DirectX12ComputePipelineBuilder&) = delete;
		virtual ~DirectX12ComputePipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// ComputePipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12ComputePipelineBuilder& shaderProgram(SharedPtr<DirectX12ShaderProgram> program) override;

		/// <inheritdoc />
		virtual DirectX12ComputePipelineBuilder& layout(SharedPtr<DirectX12PipelineLayout> layout) override;
	};

	/// <summary>
	/// Implements the DirectX 12 <see cref="RenderPassBuilder" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	class LITEFX_DIRECTX12_API DirectX12RenderPassBuilder : public RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassBuilderImpl)

	public:
		/// <summary>
		/// Initializes a DirectX 12 render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a DirectX 12 render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="samples">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a DirectX 12 render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const UInt32& commandBuffers, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a DirectX 12 render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="multiSamplingLevel">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device, const UInt32& commandBuffers, const MultiSamplingLevel& multiSamplingLevel, const String& name = "") noexcept;
		DirectX12RenderPassBuilder(const DirectX12RenderPassBuilder&) noexcept = delete;
		DirectX12RenderPassBuilder(DirectX12RenderPassBuilder&&) noexcept = delete;
		virtual ~DirectX12RenderPassBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual void build() override;

		// RenderPassBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& commandBuffers(const UInt32& count) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& multiSamplingLevel(const MultiSamplingLevel& samples) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const String& name, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const String& name, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const String& name, DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& renderTarget(const String& name, DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& inputAttachment(const DirectX12InputAttachmentMapping& inputAttachment) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const UInt32& outputLocation) override;

		/// <inheritdoc />
		virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget) override;
	};
}
#endif // defined(BUILD_DEFINE_BUILDERS)