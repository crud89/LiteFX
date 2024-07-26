#pragma once

#include <litefx/rendering_builders.hpp>
#include "dx12_api.hpp"
#include "dx12.hpp"

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
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
		explicit DirectX12BarrierBuilder();
		DirectX12BarrierBuilder(const DirectX12BarrierBuilder&) = delete;
		DirectX12BarrierBuilder(DirectX12BarrierBuilder&&) = default;
		virtual ~DirectX12BarrierBuilder() noexcept;

		// BarrierBuilder interface.
	public:
		/// <inheritdoc />
		void setupStages(PipelineStage waitFor, PipelineStage continueWith) override;

		/// <inheritdoc />
		void setupGlobalBarrier(ResourceAccess before, ResourceAccess after) override;

		/// <inheritdoc />
		void setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after) override;

		/// <inheritdoc />
		void setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) override;
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
		explicit DirectX12ShaderProgramBuilder(const DirectX12Device& device);
		DirectX12ShaderProgramBuilder(const DirectX12ShaderProgramBuilder&) = delete;
		DirectX12ShaderProgramBuilder(DirectX12ShaderProgramBuilder&&) = delete;
		virtual ~DirectX12ShaderProgramBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// ShaderProgramBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<DirectX12ShaderModule> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;

		/// <inheritdoc />
		UniquePtr<DirectX12ShaderModule> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;
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
		explicit DirectX12RasterizerBuilder() noexcept;
		DirectX12RasterizerBuilder(const DirectX12RasterizerBuilder&) noexcept = delete;
		DirectX12RasterizerBuilder(DirectX12RasterizerBuilder&&) noexcept = delete;
		virtual ~DirectX12RasterizerBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;
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
		explicit DirectX12InputAssemblerBuilder() noexcept;
		DirectX12InputAssemblerBuilder(const DirectX12InputAssemblerBuilder&) noexcept = delete;
		DirectX12InputAssemblerBuilder(DirectX12InputAssemblerBuilder&&) noexcept = delete;
		virtual ~DirectX12InputAssemblerBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		DirectX12VertexBufferLayoutBuilder vertexBuffer(size_t elementSize, UInt32 binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		template <typename TSelf>
		inline auto indexType(this TSelf&& self, IndexType type) -> TSelf&& {
			self.use(makeUnique<DirectX12IndexBufferLayout>(type));
			return std::forward<TSelf>(self);
		}
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
		void build() override;
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
		DirectX12PipelineLayoutBuilder(const DirectX12Device& device);
		DirectX12PipelineLayoutBuilder(DirectX12PipelineLayoutBuilder&&) = delete;
		DirectX12PipelineLayoutBuilder(const DirectX12PipelineLayoutBuilder&) = delete;
		virtual ~DirectX12PipelineLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// DirectX12PipelineLayoutBuilder.
	public:
		/// <summary>
		/// Builds a new descriptor set for the pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		DirectX12DescriptorSetLayoutBuilder descriptorSet(UInt32 space = 0, ShaderStage stages = ShaderStage::Any);

		/// <summary>
		/// Builds a new push constants layout for the pipeline layout.
		/// </summary>
		/// <param name="size">The size of the push constants backing memory.</param>
		DirectX12PushConstantsLayoutBuilder pushConstants(UInt32 size);

	private:
		/// <summary>
		/// Returns the device, the builder has been initialized with.
		/// </summary>
		/// <returns>A reference of the device, the builder has been initialized with.</returns>
		const DirectX12Device& device() const noexcept;
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
		explicit DirectX12DescriptorSetLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 space = 0, ShaderStage stages = ShaderStage::Any);
		DirectX12DescriptorSetLayoutBuilder(const DirectX12DescriptorSetLayoutBuilder&) = delete;
		DirectX12DescriptorSetLayoutBuilder(DirectX12DescriptorSetLayoutBuilder&&) = delete;
		virtual ~DirectX12DescriptorSetLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// DescriptorSetLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<DirectX12DescriptorLayout> makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors) override;

		/// <inheritdoc />
		UniquePtr<DirectX12DescriptorLayout> makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) override;
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
		explicit DirectX12PushConstantsLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 size);
		DirectX12PushConstantsLayoutBuilder(const DirectX12PushConstantsLayoutBuilder&) = delete;
		DirectX12PushConstantsLayoutBuilder(DirectX12PushConstantsLayoutBuilder&&) = delete;
		virtual ~DirectX12PushConstantsLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// PushConstantsLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<DirectX12PushConstantsRange> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipeline" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12RenderPipelineBuilder final : public RenderPipelineBuilder<DirectX12RenderPipeline> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 render pipeline builder.
		/// </summary>
		/// <param name="renderPass">The parent render pass.</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		explicit DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const String& name = "");
		DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
		DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
		virtual ~DirectX12RenderPipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12ComputePipeline" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12ComputePipelineBuilder final : public ComputePipelineBuilder<DirectX12ComputePipeline> {
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
		void build() override;
	};

	/// <summary>
	/// Builds a DirectX 12 <see cref="RayTracingPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RayTracingPipeline" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12RayTracingPipelineBuilder final : public RayTracingPipelineBuilder<DirectX12RayTracingPipeline> {
	public:
		/// <summary>
		/// Initializes a DirectX 12 ray-tracing pipeline builder.
		/// </summary>
		/// <param name="device">The parent device</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
		/// <param name="name">A debug name for the ray-tracing pipeline.</param>
		explicit DirectX12RayTracingPipelineBuilder(const DirectX12Device& device, ShaderRecordCollection&& shaderRecords, const String& name = "");
		DirectX12RayTracingPipelineBuilder(DirectX12RayTracingPipelineBuilder&&) = delete;
		DirectX12RayTracingPipelineBuilder(const DirectX12RayTracingPipelineBuilder&) = delete;
		virtual ~DirectX12RayTracingPipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Implements the DirectX 12 <see cref="RenderPassBuilder" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	class LITEFX_DIRECTX12_API [[nodiscard]] DirectX12RenderPassBuilder final : public RenderPassBuilder<DirectX12RenderPass> {
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
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device, UInt32 commandBuffers, const String& name = "") noexcept;

		DirectX12RenderPassBuilder(const DirectX12RenderPassBuilder&) noexcept = delete;
		DirectX12RenderPassBuilder(DirectX12RenderPassBuilder&&) noexcept = delete;
		virtual ~DirectX12RenderPassBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// RenderPassBuilder interface.
	protected:
		/// <inheritdoc />
		RenderPassDependency makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget) override;
	};

}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)