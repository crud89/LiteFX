#pragma once

#include <litefx/rendering_builders.hpp>
#include "vulkan_api.hpp"
#include "vulkan.hpp"

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Builds a Vulkan  <see cref="Barrier" />.
	/// </summary>
	/// <seealso cref="VulkanBarrier" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanBarrierBuilder final : public BarrierBuilder<VulkanBarrier> {
	public:
		/// <summary>
		/// Initializes a Vulkan barrier builder.
		/// </summary>
		explicit VulkanBarrierBuilder();
		~VulkanBarrierBuilder() noexcept override;

		VulkanBarrierBuilder(VulkanBarrierBuilder&&) noexcept = delete;
		VulkanBarrierBuilder(const VulkanBarrierBuilder&) noexcept = delete;
		VulkanBarrierBuilder& operator=(VulkanBarrierBuilder&&) noexcept = delete;
		VulkanBarrierBuilder& operator=(const VulkanBarrierBuilder&) noexcept = delete;

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
	/// Builds a Vulkan  <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="VulkanShaderProgram" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanShaderProgramBuilder final : public ShaderProgramBuilder<VulkanShaderProgram> {
	public:
		/// <summary>
		/// Initializes a Vulkan graphics shader program builder.
		/// </summary>
		/// <param name="device">The parent device that hosts the shader program.</param>
		explicit VulkanShaderProgramBuilder(const VulkanDevice& device);
		~VulkanShaderProgramBuilder() noexcept override;

		VulkanShaderProgramBuilder(VulkanShaderProgramBuilder&&) noexcept = delete;
		VulkanShaderProgramBuilder(const VulkanShaderProgramBuilder&) noexcept = delete;
		VulkanShaderProgramBuilder& operator=(VulkanShaderProgramBuilder&&) noexcept = delete;
		VulkanShaderProgramBuilder& operator=(const VulkanShaderProgramBuilder&) noexcept = delete;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// ShaderProgramBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;

		/// <inheritdoc />
		UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="VulkanRasterizer" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRasterizerBuilder final : public RasterizerBuilder<VulkanRasterizer> {
	public:
		/// <summary>
		/// Initializes a Vulkan input assembler builder.
		/// </summary>
		explicit VulkanRasterizerBuilder();
		~VulkanRasterizerBuilder() noexcept override;

		VulkanRasterizerBuilder(VulkanRasterizerBuilder&&) noexcept = delete;
		VulkanRasterizerBuilder(const VulkanRasterizerBuilder&) noexcept = delete;
		VulkanRasterizerBuilder& operator=(VulkanRasterizerBuilder&&) noexcept = delete;
		VulkanRasterizerBuilder& operator=(const VulkanRasterizerBuilder&) noexcept = delete;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Builds a <see cref="VulkanInputAssembler" />.
	/// </summary>
	/// <seealso cref="VulkanInputAssembler" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanInputAssemblerBuilder final : public InputAssemblerBuilder<VulkanInputAssembler> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan input assembler builder.
		/// </summary>
		explicit VulkanInputAssemblerBuilder();
		~VulkanInputAssemblerBuilder() noexcept override;

		VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
		VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) noexcept = delete;
		VulkanInputAssemblerBuilder& operator=(VulkanInputAssemblerBuilder&&) noexcept = delete;
		VulkanInputAssemblerBuilder& operator=(const VulkanInputAssemblerBuilder&) noexcept = delete;

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
		VulkanVertexBufferLayoutBuilder vertexBuffer(size_t elementSize, UInt32 binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		template <typename TSelf>
		auto indexType(this TSelf&& self, IndexType type) -> TSelf&& {
			self.use(makeUnique<VulkanIndexBufferLayout>(type));
			return std::forward<TSelf>(self);
		}
	};
	
	/// <summary>
	/// Builds a see <see cref="VulkanVertexBufferLayout" />.
	/// </summary>
	/// <seealso cref="VulkanVertexBuffer" />
	/// <seealso cref="VulkanVertexBufferLayout" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanVertexBufferLayoutBuilder final : public VertexBufferLayoutBuilder<VulkanVertexBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<VulkanVertexBufferLayout, VulkanInputAssemblerBuilder>::VertexBufferLayoutBuilder;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="PipelineLayout" /> for a pipeline.
	/// </summary>
	/// <seealso cref="VulkanPipelineLayout" />
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanPipelineLayoutBuilder final : public PipelineLayoutBuilder<VulkanPipelineLayout> {
		friend class VulkanDescriptorSetLayoutBuilder;

	public:
		/// <summary>
		/// Initializes a new Vulkan pipeline layout builder.
		/// </summary>
		VulkanPipelineLayoutBuilder(const VulkanDevice& device);
		~VulkanPipelineLayoutBuilder() noexcept override;

		VulkanPipelineLayoutBuilder(VulkanPipelineLayoutBuilder&&) noexcept = delete;
		VulkanPipelineLayoutBuilder(const VulkanPipelineLayoutBuilder&) noexcept = delete;
		VulkanPipelineLayoutBuilder& operator=(VulkanPipelineLayoutBuilder&&) noexcept = delete;
		VulkanPipelineLayoutBuilder& operator=(const VulkanPipelineLayoutBuilder&) noexcept = delete;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// VulkanPipelineLayoutBuilder.
	public:
		/// <summary>
		/// Builds a new descriptor set for the pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		VulkanDescriptorSetLayoutBuilder descriptorSet(UInt32 space = 0, ShaderStage stages = ShaderStage::Any);

		/// <summary>
		/// Builds a new push constants layout for the pipeline layout.
		/// </summary>
		/// <param name="size">The size of the push constants backing memory.</param>
		VulkanPushConstantsLayoutBuilder pushConstants(UInt32 size);
	};

	/// <summary>
	/// Builds a <see cref="VulkanDescriptorSetLayout" /> for a <see cref="VulkanPipelineLayout" />.
	/// </summary>
	/// <seealso cref="VulkanDescriptorSetLayout" />
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanDescriptorSetLayoutBuilder final : public DescriptorSetLayoutBuilder<VulkanDescriptorSetLayout, VulkanPipelineLayoutBuilder> {
	public:
		/// <summary>
		/// Initializes a Vulkan descriptor set layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="space">The space the descriptor set is bound to.</param>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		explicit VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 space = 0, ShaderStage stages = ShaderStage::Any);
		~VulkanDescriptorSetLayoutBuilder() noexcept override;

		VulkanDescriptorSetLayoutBuilder(VulkanDescriptorSetLayoutBuilder&&) noexcept = delete;
		VulkanDescriptorSetLayoutBuilder(const VulkanDescriptorSetLayoutBuilder&) noexcept = delete;
		VulkanDescriptorSetLayoutBuilder& operator=(VulkanDescriptorSetLayoutBuilder&&) noexcept = delete;
		VulkanDescriptorSetLayoutBuilder& operator=(const VulkanDescriptorSetLayoutBuilder&) noexcept = delete;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// DescriptorSetLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<VulkanDescriptorLayout> makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors) override;

		/// <inheritdoc />
		UniquePtr<VulkanDescriptorLayout> makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="PushConstantsLayout" /> for a <see cref="VulkanPipelineLayout" />.
	/// </summary>
	/// <seealso cref="VulkanPushConstantsLayout" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanPushConstantsLayoutBuilder final : public PushConstantsLayoutBuilder<VulkanPushConstantsLayout, VulkanPipelineLayoutBuilder> {
	public:
		/// <summary>
		/// Initializes a Vulkan render pipeline push constants layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="size">The size of the push constants backing memory.</param>
		explicit VulkanPushConstantsLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 size);
		~VulkanPushConstantsLayoutBuilder() noexcept override;

		VulkanPushConstantsLayoutBuilder(VulkanPushConstantsLayoutBuilder&&) noexcept = delete;
		VulkanPushConstantsLayoutBuilder(const VulkanPushConstantsLayoutBuilder&) noexcept = delete;
		VulkanPushConstantsLayoutBuilder& operator=(VulkanPushConstantsLayoutBuilder&&) noexcept = delete;
		VulkanPushConstantsLayoutBuilder& operator=(const VulkanPushConstantsLayoutBuilder&) noexcept = delete;

		// Builder interface.
	protected:
		/// <inheritdoc />
		void build() override;

		// PushConstantsLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		UniquePtr<VulkanPushConstantsRange> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRenderPipelineBuilder final : public RenderPipelineBuilder<VulkanRenderPipeline> {
	public:
		/// <summary>
		/// Initializes a Vulkan render pipeline builder.
		/// </summary>
		/// <param name="renderPass">The parent render pass</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		explicit VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name = "");
		~VulkanRenderPipelineBuilder() noexcept override;

		VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) noexcept = delete;
		VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) noexcept = delete;
		VulkanRenderPipelineBuilder& operator=(VulkanRenderPipelineBuilder&&) noexcept = delete;
		VulkanRenderPipelineBuilder& operator=(const VulkanRenderPipelineBuilder&) noexcept = delete;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanComputePipelineBuilder final : public ComputePipelineBuilder<VulkanComputePipeline> {
	public:
		/// <summary>
		/// Initializes a Vulkan compute pipeline builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the compute pipeline.</param>
		explicit VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name = "");
		~VulkanComputePipelineBuilder() noexcept override;

		VulkanComputePipelineBuilder(VulkanComputePipelineBuilder&&) noexcept = delete;
		VulkanComputePipelineBuilder(const VulkanComputePipelineBuilder&) noexcept = delete;
		VulkanComputePipelineBuilder& operator=(VulkanComputePipelineBuilder&&) noexcept = delete;
		VulkanComputePipelineBuilder& operator=(const VulkanComputePipelineBuilder&) noexcept = delete;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="RayTracingPipeline" />.
	/// </summary>
	/// <seealso cref="VulkanRayTracingPipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRayTracingPipelineBuilder final : public RayTracingPipelineBuilder<VulkanRayTracingPipeline> {
	public:
		/// <summary>
		/// Initializes a Vulkan ray-tracing pipeline builder.
		/// </summary>
		/// <param name="device">The parent device</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
		/// <param name="name">A debug name for the ray-tracing pipeline.</param>
		explicit VulkanRayTracingPipelineBuilder(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords, const String& name = "");
		~VulkanRayTracingPipelineBuilder() noexcept override;

		VulkanRayTracingPipelineBuilder(VulkanRayTracingPipelineBuilder&&) noexcept = delete;
		VulkanRayTracingPipelineBuilder(const VulkanRayTracingPipelineBuilder&) noexcept = delete;
		VulkanRayTracingPipelineBuilder& operator=(VulkanRayTracingPipelineBuilder&&) noexcept = delete;
		VulkanRayTracingPipelineBuilder& operator=(const VulkanRayTracingPipelineBuilder&) noexcept = delete;

		// Builder interface.
	public:
		/// <inheritdoc />
		void build() override;
	};

	/// <summary>
	/// Implements the Vulkan <see cref="RenderPassBuilder" />.
	/// </summary>
	/// <seealso cref="VulkanRenderPass" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRenderPassBuilder final : public RenderPassBuilder<VulkanRenderPass> {
	public:
		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const String& name = "");

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="name">A debug name for the render pass.</param>
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name = "");

		/// <inheritdoc />
		~VulkanRenderPassBuilder() noexcept override;

		VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
		VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) noexcept = delete;
		VulkanRenderPassBuilder& operator=(VulkanRenderPassBuilder&&) noexcept = delete;
		VulkanRenderPassBuilder& operator=(const VulkanRenderPassBuilder&) noexcept = delete;

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