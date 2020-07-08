#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

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
	class LITEFX_VULKAN_API VulkanRenderPipelineBuilder : public RenderPipelineBuilder<VulkanRenderPipelineBuilder, VulkanRenderPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineBuilderImpl)

	public:
		VulkanRenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>&& instance);
		virtual ~VulkanRenderPipelineBuilder() noexcept;

	public:
		virtual UniquePtr<VulkanRenderPipeline> go() override;

	public:
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual void use(UniquePtr<IShaderProgram>&& program) override;
		virtual void use(UniquePtr<IRenderPass>&& renderPass) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanShaderProgramBuilder : public ShaderProgramBuilder<VulkanShaderProgramBuilder, VulkanShaderProgram, VulkanRenderPipelineBuilder> {
	public:
		using ShaderProgramBuilder<VulkanShaderProgramBuilder, VulkanShaderProgram, VulkanRenderPipelineBuilder>::ShaderProgramBuilder;

	public:
		virtual VulkanShaderProgramBuilder& addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main") override;
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
	class LITEFX_VULKAN_API VulkanRenderPassBuilder : public RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass, VulkanRenderPipelineBuilder> {
	public:
		using RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass, VulkanRenderPipelineBuilder>::RenderPassBuilder;

	public:
		virtual VulkanRenderPipelineBuilder& go() override;

	public:
		virtual void use(UniquePtr<IRenderTarget>&& target) override;
		virtual VulkanRenderPassBuilder& withColorTarget(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) override;
		virtual VulkanRenderPassBuilder& addTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanRenderPipelineLayout, VulkanRenderPipelineBuilder> {
	public:
		using RenderPipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanRenderPipelineLayout, VulkanRenderPipelineBuilder>::RenderPipelineLayoutBuilder;

	public:
		virtual VulkanRenderPipelineBuilder& go() override;

	public:
		virtual void use(UniquePtr<IRasterizer>&& rasterizer) override;
		virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) override;
		virtual void use(UniquePtr<IViewport>&& viewport) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRasterizerBuilder : public RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineLayoutBuilder> {
	public:
		using RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineLayoutBuilder>::RasterizerBuilder;

	public:
		virtual VulkanRasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) override;
		virtual VulkanRasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) override;
		virtual VulkanRasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) override;
		virtual VulkanRasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) override;
		virtual VulkanRasterizerBuilder& withDepthBias(const bool& enable = false) override;
		virtual VulkanRasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) override;
		virtual VulkanRasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) override;
		virtual VulkanRasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanViewportBuilder : public ViewportBuilder<VulkanViewportBuilder, VulkanViewport, VulkanRenderPipelineLayoutBuilder> {
	public:
		using ViewportBuilder<VulkanViewportBuilder, VulkanViewport, VulkanRenderPipelineLayoutBuilder>::ViewportBuilder;

	public:
		virtual VulkanViewportBuilder& withRectangle(const RectF& rectangle) override;
		virtual VulkanViewportBuilder& addScissor(const RectF& scissor) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineLayoutBuilder> {
	public:
		using InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineLayoutBuilder>::InputAssemblerBuilder;

	public:
		virtual VulkanInputAssemblerBuilder& withBufferLayout(UniquePtr<BufferLayout>&& layout) override;
		virtual VulkanInputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanBufferLayoutBuilder : public BufferLayoutBuilder<VulkanBufferLayoutBuilder, VulkanBufferLayout, VulkanInputAssemblerBuilder> {
	public:
		using BufferLayoutBuilder<VulkanBufferLayoutBuilder, VulkanBufferLayout, VulkanInputAssemblerBuilder>::BufferLayoutBuilder;

	public:
		virtual VulkanBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

		/// <summary>
		/// 
		/// </summary>
		/// <reamrks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </reamrks>
		virtual VulkanBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset);
		virtual VulkanBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset);
	};
}