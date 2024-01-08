#pragma once

#include <litefx/rendering_builders.hpp>
#include "vulkan_api.hpp"
#include "vulkan.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
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
		constexpr inline explicit VulkanBarrierBuilder();
		constexpr inline VulkanBarrierBuilder(const VulkanBarrierBuilder&) = delete;
		constexpr inline VulkanBarrierBuilder(VulkanBarrierBuilder&&) = default;
		constexpr inline virtual ~VulkanBarrierBuilder() noexcept;

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
	/// Builds a Vulkan  <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="VulkanShaderProgram" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanShaderProgramBuilder final : public ShaderProgramBuilder<VulkanShaderProgram> {
		LITEFX_IMPLEMENTATION(VulkanShaderProgramBuilderImpl);

	public:
		/// <summary>
		/// Initializes a Vulkan graphics shader program builder.
		/// </summary>
		/// <param name="device">The parent device that hosts the shader program.</param>
		constexpr inline explicit VulkanShaderProgramBuilder(const VulkanDevice& device);
		VulkanShaderProgramBuilder(const VulkanShaderProgramBuilder&) = delete;
		VulkanShaderProgramBuilder(VulkanShaderProgramBuilder&&) = delete;
		constexpr inline virtual ~VulkanShaderProgramBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

		// ShaderProgramBuilder interface.
	protected:
		/// <inheritdoc />
		constexpr inline UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint) override;

		/// <inheritdoc />
		constexpr inline UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint) override;
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
		constexpr inline explicit VulkanRasterizerBuilder() noexcept;
		VulkanRasterizerBuilder(const VulkanRasterizerBuilder&) noexcept = delete;
		VulkanRasterizerBuilder(VulkanRasterizerBuilder&&) noexcept = delete;
		constexpr inline virtual ~VulkanRasterizerBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		inline void build() override;
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
		inline void build() override;
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
		constexpr inline explicit VulkanInputAssemblerBuilder() noexcept;
		VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) noexcept = delete;
		VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
		constexpr inline virtual ~VulkanInputAssemblerBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		constexpr inline VulkanVertexBufferLayoutBuilder vertexBuffer(size_t elementSize, UInt32 binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		template <typename TSelf>
		constexpr inline auto indexType(this TSelf&& self, IndexType type) -> TSelf&& {
			self.use(makeUnique<VulkanIndexBufferLayout>(type));
			return self;
		}
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
		/// <param name="maxUnboundedArraySize">The maximum array size of unbounded descriptor arrays in this descriptor set.</param>
		constexpr inline explicit VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 space = 0, ShaderStage stages = ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, UInt32 maxUnboundedArraySize = 0);
		VulkanDescriptorSetLayoutBuilder(const VulkanDescriptorSetLayoutBuilder&) = delete;
		VulkanDescriptorSetLayoutBuilder(VulkanDescriptorSetLayoutBuilder&&) = delete;
		constexpr inline virtual ~VulkanDescriptorSetLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

		// DescriptorSetLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		constexpr inline UniquePtr<VulkanDescriptorLayout> makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors) override;

		/// <inheritdoc />
		constexpr inline UniquePtr<VulkanDescriptorLayout> makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) override;
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
		constexpr inline explicit VulkanPushConstantsLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 size);
		VulkanPushConstantsLayoutBuilder(const VulkanPushConstantsLayoutBuilder&) = delete;
		VulkanPushConstantsLayoutBuilder(VulkanPushConstantsLayoutBuilder&&) = delete;
		constexpr inline virtual ~VulkanPushConstantsLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

		// PushConstantsLayoutBuilder interface.
	protected:
		/// <inheritdoc />
		inline UniquePtr<VulkanPushConstantsRange> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) override;
	};

	/// <summary>
	/// Builds a Vulkan <see cref="PipelineLayout" /> for a pipeline.
	/// </summary>
	/// <seealso cref="VulkanPipelineLayout" />
	/// <seealso cref="VulkanRenderPipeline" />
	/// <seealso cref="VulkanComputePipeline" />
	class LITEFX_VULKAN_API [[nodiscard]] VulkanPipelineLayoutBuilder final : public PipelineLayoutBuilder<VulkanPipelineLayout> {
		LITEFX_IMPLEMENTATION(VulkanPipelineLayoutBuilderImpl);
		friend class VulkanDescriptorSetLayoutBuilder;

	public:
		/// <summary>
		/// Initializes a new Vulkan pipeline layout builder.
		/// </summary>
		constexpr inline VulkanPipelineLayoutBuilder(const VulkanDevice& device);
		VulkanPipelineLayoutBuilder(VulkanPipelineLayoutBuilder&&) = delete;
		VulkanPipelineLayoutBuilder(const VulkanPipelineLayoutBuilder&) = delete;
		constexpr inline virtual ~VulkanPipelineLayoutBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

		// VulkanPipelineLayoutBuilder.
	public:
		/// <summary>
		/// Builds a new descriptor set for the pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		/// <param name="maxUnboundedArraySize">The maximum array size of unbounded descriptor arrays in this descriptor set.</param>
		constexpr inline VulkanDescriptorSetLayoutBuilder descriptorSet(UInt32 space = 0, ShaderStage stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex, UInt32 maxUnboundedArraySize = 0);

		/// <summary>
		/// Builds a new push constants layout for the pipeline layout.
		/// </summary>
		/// <param name="size">The size of the push constants backing memory.</param>
		constexpr inline VulkanPushConstantsLayoutBuilder pushConstants(UInt32 size);

	private:
		/// <summary>
		/// Returns the device, the builder has been initialized with.
		/// </summary>
		/// <returns>A reference of the device, the builder has been initialized with.</returns>
		constexpr inline const VulkanDevice& device() const noexcept;
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
		constexpr inline explicit VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name = "");
		VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) = delete;
		VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) = delete;
		constexpr inline virtual ~VulkanRenderPipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		inline void build() override;
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
		constexpr inline explicit VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name = "");
		VulkanComputePipelineBuilder(VulkanComputePipelineBuilder&&) = delete;
		VulkanComputePipelineBuilder(const VulkanComputePipelineBuilder&) = delete;
		constexpr inline virtual ~VulkanComputePipelineBuilder() noexcept;

		// Builder interface.
	public:
		/// <inheritdoc />
		inline void build() override;
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
		/// <param name="device">The parent device</param>
		/// <param name="name">A debug name for the render pass.</param>
		constexpr inline explicit VulkanRenderPassBuilder(const VulkanDevice& device, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="samples">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		constexpr inline explicit VulkanRenderPassBuilder(const VulkanDevice& device, MultiSamplingLevel samples = MultiSamplingLevel::x1, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="name">A debug name for the render pass.</param>
		constexpr inline explicit VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name = "") noexcept;

		/// <summary>
		/// Initializes a Vulkan render pass builder.
		/// </summary>
		/// <param name="device">The parent device</param>
		/// <param name="commandBuffers">The number of command buffers to initialize.</param>
		/// <param name="multiSamplingLevel">The multi-sampling level for the render targets.</param>
		/// <param name="name">A debug name for the render pass.</param>
		constexpr inline explicit VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, MultiSamplingLevel multiSamplingLevel, const String& name = "") noexcept;
		VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) noexcept = delete;
		VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
		constexpr inline virtual ~VulkanRenderPassBuilder() noexcept;

		// Builder interface.
	protected:
		/// <inheritdoc />
		inline void build() override;

		// RenderPassBuilder interface.
	protected:
		/// <inheritdoc />
		inline VulkanInputAttachmentMapping makeInputAttachment(UInt32 inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget) override;
	};

}
#endif // defined(BUILD_DEFINE_BUILDERS)