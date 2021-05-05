#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class VulkanBackendBuilder;
	class VulkanRenderPipelineBuilder;
	class VulkanRenderPipelineLayoutBuilder;
	class VulkanShaderProgramBuilder;
	class VulkanRenderPassBuilder;
	class VulkanRasterizerBuilder;
	class VulkanViewportBuilder;
	class VulkanInputAssemblerBuilder;
	class VulkanDescriptorSetLayoutBuilder;
	class VulkanVertexBufferLayoutBuilder;
	class VulkanIndexBufferLayoutBuilder;
	class VulkanDescriptorLayoutBuilder;

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBackendBuilder : public Builder<VulkanBackendBuilder, VulkanBackend, AppBuilder> {
	public:
		using builder_type::Builder;

	public:
		virtual AppBuilder& go() override;

	public:
		VulkanBackendBuilder& withSurface(UniquePtr<ISurface>&& surface);
		VulkanBackendBuilder& withSurface(VulkanSurface::surface_callback callback);
		VulkanBackendBuilder& withAdapter(const UInt32& adapterId);
		VulkanBackendBuilder& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);
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
		virtual VulkanShaderProgramBuilder setShaderProgram();
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
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPassBuilder : public RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass> {
	public:
		VulkanRenderPassBuilder(UniquePtr<VulkanRenderPass>&& instance);
		virtual ~VulkanRenderPassBuilder() noexcept;

	public:
		virtual UniquePtr<VulkanRenderPass> go() override;

	public:
		virtual VulkanRenderPipelineBuilder addPipeline(const UInt32& id, const String& name = "");

	public:
		virtual void use(UniquePtr<IRenderPipeline>&& pipeline) override;
		virtual void use(UniquePtr<IRenderTarget>&& target) override;
		virtual VulkanRenderPassBuilder& attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clear = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual VulkanRenderPassBuilder& dependsOn(const IRenderPass* renderPass) override;
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
		virtual VulkanRenderPipelineBuilder& withInputAssembler(SharedPtr<IInputAssembler> inputAssembler);
		virtual VulkanRenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);
		virtual VulkanRenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);

	public:
		virtual VulkanRenderPassBuilder& go() override;
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual void use(SharedPtr<IRasterizer> rasterizer) override;
		virtual void use(SharedPtr<IInputAssembler> inputAssembler) override;
		virtual void use(SharedPtr<IViewport> viewport) override;
		virtual void use(SharedPtr<IScissor> scissor) override;
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
	class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder> {
	public:
		using InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder>::InputAssemblerBuilder;

	public:
		virtual VulkanVertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

	public:
		virtual VulkanInputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;
		virtual void use(UniquePtr<IVertexBufferLayout>&& layout) override;
		virtual void use(UniquePtr<IIndexBufferLayout>&& layout) override;

	public:
		virtual VulkanInputAssemblerBuilder& withIndexType(const IndexType& type);
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
		virtual VulkanVertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset);
		virtual VulkanVertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanIndexBufferLayoutBuilder : public Builder<VulkanIndexBufferLayoutBuilder, VulkanIndexBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using Builder<VulkanIndexBufferLayoutBuilder, VulkanIndexBufferLayout, VulkanInputAssemblerBuilder>::Builder;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanDescriptorLayoutBuilder : public Builder<VulkanDescriptorLayoutBuilder, VulkanDescriptorLayout, VulkanDescriptorSetLayoutBuilder> {
	public:
		using Builder<VulkanDescriptorLayoutBuilder, VulkanDescriptorLayout, VulkanDescriptorSetLayoutBuilder>::Builder;
	};
}