#pragma once

#include <litefx/rendering_builders.hpp>
#include "vulkan_api.hpp"
#include "vulkan.hpp"

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// @brief Builds a Vulkan @ref Barrier.
	///
	/// @see VulkanBarrier
	class LITEFX_VULKAN_API [[nodiscard]] VulkanBarrierBuilder final : public BarrierBuilder<VulkanBarrier> {
	public:
		/// @brief Initializes a Vulkan barrier builder.
		explicit VulkanBarrierBuilder();
		~VulkanBarrierBuilder() noexcept override;

		VulkanBarrierBuilder(VulkanBarrierBuilder&&) noexcept = default;
		VulkanBarrierBuilder(const VulkanBarrierBuilder&) = delete;
		VulkanBarrierBuilder& operator=(VulkanBarrierBuilder&&) noexcept = delete;
		VulkanBarrierBuilder& operator=(const VulkanBarrierBuilder&) = delete;

		// BarrierBuilder interface.
	public:
		/// @copydoc BarrierBuilder::setupStages
		void setupStages(PipelineStage waitFor, PipelineStage continueWith) override;

		/// @copydoc BarrierBuilder::setupGlobalBarrier
		void setupGlobalBarrier(ResourceAccess before, ResourceAccess after) override;

		/// @copydoc BarrierBuilder::setupBufferBarrier
		void setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after) override;

		/// @copydoc BarrierBuilder::setupImageBarrier
		void setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) override;
	};

	/// @brief Builds a Vulkan @ref ShaderProgram.
	///
	/// @see VulkanShaderProgram
	class LITEFX_VULKAN_API [[nodiscard]] VulkanShaderProgramBuilder final : public ShaderProgramBuilder<VulkanShaderProgram> {
	public:
		/// @brief Initializes a Vulkan graphics shader program builder.
		///
		/// @param device The parent device that hosts the shader program.
		explicit VulkanShaderProgramBuilder(const VulkanDevice& device);
		~VulkanShaderProgramBuilder() noexcept override;

		VulkanShaderProgramBuilder(VulkanShaderProgramBuilder&&) noexcept = delete;
		VulkanShaderProgramBuilder(const VulkanShaderProgramBuilder&) = delete;
		VulkanShaderProgramBuilder& operator=(VulkanShaderProgramBuilder&&) noexcept = delete;
		VulkanShaderProgramBuilder& operator=(const VulkanShaderProgramBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

		// ShaderProgramBuilder interface.
	protected:
		/// @copydoc ShaderProgramBuilder::makeShaderModule(ShaderStage, const String&, const String&, const Optional<DescriptorBindingPoint>&)
		UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;

		/// @copydoc ShaderProgramBuilder::makeShaderModule(ShaderStage, std::istream&, const String&, const String&, const Optional<DescriptorBindingPoint>&)
		UniquePtr<VulkanShaderModule> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) override;
	};

	/// @brief Builds a Vulkan @ref IRasterizer.
	///
	/// @see VulkanRasterizer
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRasterizerBuilder final : public RasterizerBuilder<VulkanRasterizer> {
	public:
		/// @brief Initializes a Vulkan input assembler builder.
		explicit VulkanRasterizerBuilder();
		~VulkanRasterizerBuilder() noexcept override;

		VulkanRasterizerBuilder(VulkanRasterizerBuilder&&) noexcept = delete;
		VulkanRasterizerBuilder(const VulkanRasterizerBuilder&) = delete;
		VulkanRasterizerBuilder& operator=(VulkanRasterizerBuilder&&) noexcept = delete;
		VulkanRasterizerBuilder& operator=(const VulkanRasterizerBuilder&) = delete;

		// Builder interface.
	public:
		/// @copydoc Builder::build
		void build() override;
	};

	/// @brief Builds a @ref VulkanInputAssembler.
	///
	/// @see VulkanInputAssembler
	class LITEFX_VULKAN_API [[nodiscard]] VulkanInputAssemblerBuilder final : public InputAssemblerBuilder<VulkanInputAssembler> {
		LITEFX_IMPLEMENTATION(VulkanInputAssemblerBuilderImpl);

	public:
		/// @brief Initializes a Vulkan input assembler builder.
		explicit VulkanInputAssemblerBuilder();
		~VulkanInputAssemblerBuilder() noexcept override;

		VulkanInputAssemblerBuilder(VulkanInputAssemblerBuilder&&) noexcept = delete;
		VulkanInputAssemblerBuilder(const VulkanInputAssemblerBuilder&) = delete;
		VulkanInputAssemblerBuilder& operator=(VulkanInputAssemblerBuilder&&) noexcept = delete;
		VulkanInputAssemblerBuilder& operator=(const VulkanInputAssemblerBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

	public:
		/// @brief Starts building a vertex buffer layout.
		///
		/// @param elementSize The size of a vertex within the vertex buffer.
		/// @param binding The binding point to bind the vertex buffer to.
		VulkanVertexBufferLayoutBuilder vertexBuffer(size_t elementSize, UInt32 binding = 0);

		/// @brief Starts building an index buffer layout.
		///
		/// @param type The type of the index buffer.
		template <typename TSelf>
		auto indexType(this TSelf&& self, IndexType type) -> TSelf&& {
			self.use(VulkanIndexBufferLayout::create(type));
			return std::forward<TSelf>(self);
		}
	};
	
	/// @brief Builds a see @ref VulkanVertexBufferLayout.
	///
	/// @see VulkanVertexBuffer
	/// @see VulkanVertexBufferLayout
	class LITEFX_VULKAN_API [[nodiscard]] VulkanVertexBufferLayoutBuilder final : public VertexBufferLayoutBuilder<VulkanVertexBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<VulkanVertexBufferLayout, VulkanInputAssemblerBuilder>::VertexBufferLayoutBuilder;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;
	};

	/// @brief Builds a Vulkan @ref PipelineLayout for a pipeline.
	///
	/// @see VulkanPipelineLayout
	/// @see VulkanRenderPipeline
	/// @see VulkanComputePipeline
	class LITEFX_VULKAN_API [[nodiscard]] VulkanPipelineLayoutBuilder final : public PipelineLayoutBuilder<VulkanPipelineLayout> {
		friend class VulkanDescriptorSetLayoutBuilder;

	public:
		/// @brief Initializes a new Vulkan pipeline layout builder.
		explicit VulkanPipelineLayoutBuilder(const VulkanDevice& device);
		~VulkanPipelineLayoutBuilder() noexcept override;

		VulkanPipelineLayoutBuilder(VulkanPipelineLayoutBuilder&&) noexcept = delete;
		VulkanPipelineLayoutBuilder(const VulkanPipelineLayoutBuilder&) = delete;
		VulkanPipelineLayoutBuilder& operator=(VulkanPipelineLayoutBuilder&&) noexcept = delete;
		VulkanPipelineLayoutBuilder& operator=(const VulkanPipelineLayoutBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

		// VulkanPipelineLayoutBuilder.
	public:
		/// @brief Builds a new descriptor set for the pipeline layout.
		///
		/// @param space The space, the descriptor set is bound to.
		/// @param stages The stages, the descriptor set will be accessible from.
		VulkanDescriptorSetLayoutBuilder descriptorSet(UInt32 space = 0, ShaderStage stages = ShaderStage::Any);

		/// @brief Builds a new push constants layout for the pipeline layout.
		///
		/// @param size The size of the push constants backing memory.
		VulkanPushConstantsLayoutBuilder pushConstants(UInt32 size);
	};

	/// @brief Builds a @ref VulkanDescriptorSetLayout for a @ref VulkanPipelineLayout.
	///
	/// @see VulkanDescriptorSetLayout
	/// @see VulkanRenderPipeline
	/// @see VulkanComputePipeline
	class LITEFX_VULKAN_API [[nodiscard]] VulkanDescriptorSetLayoutBuilder final : public DescriptorSetLayoutBuilder<VulkanDescriptorSetLayout, VulkanPipelineLayoutBuilder> {
	public:
		/// @brief Initializes a Vulkan descriptor set layout builder.
		///
		/// @param parent The parent pipeline layout builder.
		/// @param space The space the descriptor set is bound to.
		/// @param stages The shader stages, the descriptor set is accessible from.
		explicit VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 space = 0, ShaderStage stages = ShaderStage::Any);
		~VulkanDescriptorSetLayoutBuilder() noexcept override;

		VulkanDescriptorSetLayoutBuilder(VulkanDescriptorSetLayoutBuilder&&) noexcept = delete;
		VulkanDescriptorSetLayoutBuilder(const VulkanDescriptorSetLayoutBuilder&) = delete;
		VulkanDescriptorSetLayoutBuilder& operator=(VulkanDescriptorSetLayoutBuilder&&) noexcept = delete;
		VulkanDescriptorSetLayoutBuilder& operator=(const VulkanDescriptorSetLayoutBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

		// DescriptorSetLayoutBuilder interface.
	protected:
		/// @copydoc DescriptorSetLayoutBuilder::makeDescriptor(DescriptorType, UInt32, UInt32, UInt32, bool)
		VulkanDescriptorLayout makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors, bool unbounded) override;

		/// @copydoc DescriptorSetLayoutBuilder::makeDescriptor(UInt32, FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float)
		VulkanDescriptorLayout makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) override;
	};

	/// @brief Builds a Vulkan @ref PushConstantsLayout for a @ref VulkanPipelineLayout.
	///
	/// @see VulkanPushConstantsLayout
	class LITEFX_VULKAN_API [[nodiscard]] VulkanPushConstantsLayoutBuilder final : public PushConstantsLayoutBuilder<VulkanPushConstantsLayout, VulkanPipelineLayoutBuilder> {
	public:
		/// @brief Initializes a Vulkan render pipeline push constants layout builder.
		///
		/// @param parent The parent pipeline layout builder.
		/// @param size The size of the push constants backing memory.
		explicit VulkanPushConstantsLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 size);
		~VulkanPushConstantsLayoutBuilder() noexcept override;

		VulkanPushConstantsLayoutBuilder(VulkanPushConstantsLayoutBuilder&&) noexcept = delete;
		VulkanPushConstantsLayoutBuilder(const VulkanPushConstantsLayoutBuilder&) = delete;
		VulkanPushConstantsLayoutBuilder& operator=(VulkanPushConstantsLayoutBuilder&&) noexcept = delete;
		VulkanPushConstantsLayoutBuilder& operator=(const VulkanPushConstantsLayoutBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

		// PushConstantsLayoutBuilder interface.
	protected:
		/// @copydoc PushConstantsLayoutBuilder::makeRange
		UniquePtr<VulkanPushConstantsRange> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) override;
	};

	/// @brief Builds a Vulkan @ref RenderPipeline.
	///
	/// @see VulkanRenderPipeline
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRenderPipelineBuilder final : public RenderPipelineBuilder<VulkanRenderPipeline> {
	public:
		/// @brief Initializes a Vulkan render pipeline builder.
		///
		/// @param renderPass The parent render pass
		/// @param name A debug name for the render pipeline.
		explicit VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name = "");
		~VulkanRenderPipelineBuilder() noexcept override;

		VulkanRenderPipelineBuilder(VulkanRenderPipelineBuilder&&) noexcept = delete;
		VulkanRenderPipelineBuilder(const VulkanRenderPipelineBuilder&) = delete;
		VulkanRenderPipelineBuilder& operator=(VulkanRenderPipelineBuilder&&) noexcept = delete;
		VulkanRenderPipelineBuilder& operator=(const VulkanRenderPipelineBuilder&) = delete;

		// Builder interface.
	public:
		/// @copydoc Builder::build
		void build() override;
	};

	/// @brief Builds a Vulkan @ref ComputePipeline.
	///
	/// @see VulkanComputePipeline
	class LITEFX_VULKAN_API [[nodiscard]] VulkanComputePipelineBuilder final : public ComputePipelineBuilder<VulkanComputePipeline> {
	public:
		/// @brief Initializes a Vulkan compute pipeline builder.
		///
		/// @param device The parent device
		/// @param name A debug name for the compute pipeline.
		explicit VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name = "");
		~VulkanComputePipelineBuilder() noexcept override;

		VulkanComputePipelineBuilder(VulkanComputePipelineBuilder&&) noexcept = delete;
		VulkanComputePipelineBuilder(const VulkanComputePipelineBuilder&) = delete;
		VulkanComputePipelineBuilder& operator=(VulkanComputePipelineBuilder&&) noexcept = delete;
		VulkanComputePipelineBuilder& operator=(const VulkanComputePipelineBuilder&) = delete;

		// Builder interface.
	public:
		/// @copydoc Builder::build
		void build() override;
	};

	/// @brief Builds a Vulkan @ref RayTracingPipeline.
	///
	/// @see VulkanRayTracingPipeline
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRayTracingPipelineBuilder final : public RayTracingPipelineBuilder<VulkanRayTracingPipeline> {
	public:
		/// @brief Initializes a Vulkan ray-tracing pipeline builder.
		///
		/// @param device The parent device
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
		/// @param name A debug name for the ray-tracing pipeline.
		explicit VulkanRayTracingPipelineBuilder(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords, const String& name = "");
		~VulkanRayTracingPipelineBuilder() noexcept override;

		VulkanRayTracingPipelineBuilder(VulkanRayTracingPipelineBuilder&&) noexcept = delete;
		VulkanRayTracingPipelineBuilder(const VulkanRayTracingPipelineBuilder&) = delete;
		VulkanRayTracingPipelineBuilder& operator=(VulkanRayTracingPipelineBuilder&&) noexcept = delete;
		VulkanRayTracingPipelineBuilder& operator=(const VulkanRayTracingPipelineBuilder&) = delete;

		// Builder interface.
	public:
		/// @copydoc Builder::build
		void build() override;
	};

	/// @brief Implements the Vulkan @ref RenderPassBuilder.
	///
	/// @see VulkanRenderPass
	class LITEFX_VULKAN_API [[nodiscard]] VulkanRenderPassBuilder final : public RenderPassBuilder<VulkanRenderPass> {
	public:
		/// @brief Initializes a Vulkan render pass builder.
		///
		/// @param device The parent device.
		/// @param name A debug name for the render pass.
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, const String& name = "");

		/// @brief Initializes a Vulkan render pass builder.
		///
		/// @param device The parent device.
		/// @param commandBuffers The number of command buffers to initialize.
		/// @param name A debug name for the render pass.
		explicit VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name = "");

		/// @copydoc Builder::~Builder
		~VulkanRenderPassBuilder() noexcept override;

		VulkanRenderPassBuilder(VulkanRenderPassBuilder&&) noexcept = delete;
		VulkanRenderPassBuilder(const VulkanRenderPassBuilder&) = delete;
		VulkanRenderPassBuilder& operator=(VulkanRenderPassBuilder&&) noexcept = delete;
		VulkanRenderPassBuilder& operator=(const VulkanRenderPassBuilder&) = delete;

		// Builder interface.
	protected:
		/// @copydoc Builder::build
		void build() override;

		// RenderPassBuilder interface.
	protected:
		/// @copydoc RenderPassBuilder::makeInputAttachment
		RenderPassDependency makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget) override;
	};

}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)