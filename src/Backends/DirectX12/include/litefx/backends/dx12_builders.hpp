#pragma once

#include <litefx/rendering.hpp>
#include "dx12_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class DirectX12BackendBuilder;
	class DirectX12RenderPipelineBuilder;
	//class DirectX12RenderPipelineLayoutBuilder;
	//class DirectX12ShaderProgramBuilder;
	class DirectX12RenderPassBuilder;
	//class DirectX12RasterizerBuilder;
	class DirectX12ViewportBuilder;
	//class DirectX12InputAssemblerBuilder;
	//class DirectX12DescriptorSetLayoutBuilder;
	//class DirectX12VertexBufferLayoutBuilder;
	//class DirectX12IndexBufferLayoutBuilder;
	//class DirectX12DescriptorLayoutBuilder;

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12BackendBuilder : public Builder<DirectX12BackendBuilder, DirectX12Backend, AppBuilder> {
	public:
		using builder_type::Builder;

	public:
		virtual AppBuilder& go() override;

	public:
		DirectX12BackendBuilder& useAdvancedSoftwareRasterizer(const bool& enable = false);
		DirectX12BackendBuilder& withSurface(UniquePtr<ISurface>&& surface);
		DirectX12BackendBuilder& withAdapter(const UInt32& adapterId);
		DirectX12BackendBuilder& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPassBuilder : public RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass> {
	public:
		DirectX12RenderPassBuilder(UniquePtr<DirectX12RenderPass>&& instance);
		virtual ~DirectX12RenderPassBuilder() noexcept;

	public:
		virtual UniquePtr<DirectX12RenderPass> go() override;

	public:
		virtual DirectX12RenderPipelineBuilder addPipeline();

	public:
		virtual void use(UniquePtr<IRenderPipeline>&& pipeline) override;
		virtual void use(UniquePtr<IRenderTarget>&& target) override;
		virtual DirectX12RenderPassBuilder& attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& dependsOn(const IRenderPass* renderPass) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineBuilder : public RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline, DirectX12RenderPassBuilder> {
	public:
		using RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline, DirectX12RenderPassBuilder>::RenderPipelineBuilder;

	public:
		//virtual DirectX12RenderPipelineLayoutBuilder defineLayout();

	public:
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
	};


	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12ViewportBuilder : public ViewportBuilder<DirectX12ViewportBuilder, DirectX12Viewport, DirectX12RenderPipelineLayoutBuilder> {
	public:
		using ViewportBuilder<DirectX12ViewportBuilder, DirectX12Viewport, DirectX12RenderPipelineLayoutBuilder>::ViewportBuilder;

	public:
		virtual DirectX12ViewportBuilder& withRectangle(const RectF& rectangle) override;
		virtual DirectX12ViewportBuilder& addScissor(const RectF& scissor) override;
	};
}