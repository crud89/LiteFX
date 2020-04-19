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
	class LITEFX_VULKAN_API VulkanDeviceBuilder : public GraphicsDeviceBuilder<VulkanDeviceBuilder, VulkanDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceBuilderImpl)

	public:
		typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

	public:
		VulkanDeviceBuilder(UniquePtr<VulkanDevice>&& instance) noexcept;
		virtual ~VulkanDeviceBuilder() noexcept;

	public:
		virtual UniquePtr<VulkanDevice> go() override;

	public:
		virtual VulkanDeviceBuilder& withFormat(const Format& format);
		virtual VulkanDeviceBuilder& withQueue(const QueueType& queueType);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineBuilder : public RenderPipelineBuilder<VulkanRenderPipelineBuilder, VulkanRenderPipeline> {
		LITEFX_IMPLEMENTATION(VulkanRenderPipelineBuilderImpl)

	public:
		VulkanRenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>&& instance) noexcept;
		virtual ~VulkanRenderPipelineBuilder() noexcept;

	public:
		virtual UniquePtr<VulkanRenderPipeline> go() override;

	public:
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_VULKAN_API VulkanRenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanRenderPipelineLayout, VulkanRenderPipelineBuilder> {
	public:
		using RenderPipelineLayoutBuilder<VulkanRenderPipelineLayoutBuilder, VulkanRenderPipelineLayout, VulkanRenderPipelineBuilder>::RenderPipelineLayoutBuilder;

	public:
		virtual void use(UniquePtr<IRasterizer>&& rasterizer) override;
		virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) override;
		virtual void use(UniquePtr<IViewport>&& viewport) override;
	};

	///// <summary>
	///// 
	///// </summary>
	//class LITEFX_VULKAN_API VulkanRasterizerBuilder : public RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineLayoutBuilder> {
	//public:
	//	using RasterizerBuilder<VulkanRasterizerBuilder, VulkanRasterizer, VulkanRenderPipelineLayoutBuilder>::RasterizerBuilder;

	//public:
	//	virtual VulkanRasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) override;
	//	virtual VulkanRasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) override;
	//	virtual VulkanRasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) override;
	//	virtual VulkanRasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) override;
	//	virtual VulkanRasterizerBuilder& withDepthBias(const bool& enable = false) override;
	//	virtual VulkanRasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) override;
	//	virtual VulkanRasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) override;
	//	virtual VulkanRasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) override;
	//};

	///// <summary>
	///// 
	///// </summary>
	//class LITEFX_VULKAN_API VulkanViewportBuilder : public ViewportBuilder<VulkanViewportBuilder, VulkanViewport, VulkanRenderPipelineLayoutBuilder> {
	//public:
	//	using ViewportBuilder<VulkanViewportBuilder, VulkanViewport, VulkanRenderPipelineLayoutBuilder>::ViewportBuilder;

	//public:
	//	virtual VulkanViewportBuilder& withRectangle(const RectF& rectangle) override;
	//	virtual VulkanViewportBuilder& addScissor(const RectF& scissor) override;
	//};

	///// <summary>
	///// 
	///// </summary>
	//class LITEFX_VULKAN_API VulkanInputAssemblerBuilder : public InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineLayoutBuilder> {
	//public:
	//	using InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineLayoutBuilder>::InputAssemblerBuilder;

	//public:
	//	virtual VulkanInputAssemblerBuilder& withBufferLayout(UniquePtr<BufferLayout>&& layout) override;
	//};

}