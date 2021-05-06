#pragma once

#include <litefx/rendering.hpp>
#include "dx12_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class DirectX12BackendBuilder;
	class DirectX12RenderPipelineBuilder;
	class DirectX12RenderPipelineLayoutBuilder;
	class DirectX12ShaderProgramBuilder;
	class DirectX12RenderPassBuilder;
	class DirectX12RasterizerBuilder;
	class DirectX12ViewportBuilder;
	class DirectX12InputAssemblerBuilder;
	//class DirectX12DescriptorSetLayoutBuilder;
	class DirectX12VertexBufferLayoutBuilder;
	class DirectX12IndexBufferLayoutBuilder;
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
		virtual DirectX12RenderPipelineBuilder addPipeline(const UInt32& id, const String& name = "");

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
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineBuilderImpl);

	public:
		DirectX12RenderPipelineBuilder(DirectX12RenderPassBuilder& parent, UniquePtr<DirectX12RenderPipeline>&& instance);
		DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
		DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
		virtual ~DirectX12RenderPipelineBuilder() noexcept;

	public:
		virtual DirectX12RenderPipelineLayoutBuilder layout();
		virtual DirectX12RasterizerBuilder rasterizer();
		virtual DirectX12InputAssemblerBuilder inputAssembler();
		virtual DirectX12RenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);
		virtual DirectX12RenderPipelineBuilder& withInputAssembler(SharedPtr<IInputAssembler> inputAssembler);
		virtual DirectX12RenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);
		virtual DirectX12RenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);

	public:
		virtual DirectX12RenderPassBuilder& go() override;
		virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual void use(SharedPtr<IRasterizer> rasterizer) override;
		virtual void use(SharedPtr<IInputAssembler> inputAssembler) override;
		virtual void use(SharedPtr<IViewport> viewport) override;
		virtual void use(SharedPtr<IScissor> scissor) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<DirectX12RenderPipelineLayoutBuilder, DirectX12RenderPipelineLayout, DirectX12RenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutBuilderImpl);

	public:
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineBuilder& parent, UniquePtr<DirectX12RenderPipelineLayout>&& instance);
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineLayoutBuilder&&) = delete;
		DirectX12RenderPipelineLayoutBuilder(const DirectX12RenderPipelineLayoutBuilder&) = delete;
		virtual ~DirectX12RenderPipelineLayoutBuilder() noexcept;

	public:
		virtual DirectX12ShaderProgramBuilder shaderProgram();
		//virtual DirectX12DescriptorSetLayoutBuilder addDescriptorSet(const UInt32& id, const ShaderStage& stages);

	public:
		virtual DirectX12RenderPipelineBuilder& go() override;

	public:
		virtual void use(UniquePtr<IShaderProgram>&& program) override;
		virtual void use(UniquePtr<IDescriptorSetLayout>&& layout) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RasterizerBuilder : public RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder> {
	public:
		using RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder>::RasterizerBuilder;

	public:
		virtual DirectX12RasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) override;
		virtual DirectX12RasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) override;
		virtual DirectX12RasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) override;
		virtual DirectX12RasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) override;
		virtual DirectX12RasterizerBuilder& enableDepthBias(const bool& enable = false) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) override;
		virtual DirectX12RasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12InputAssemblerBuilder : public InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder> {
	public:
		using InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder>::InputAssemblerBuilder;

	public:
		virtual DirectX12VertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

	public:
		virtual DirectX12InputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;
		virtual void use(UniquePtr<IVertexBufferLayout>&& layout) override;
		virtual void use(UniquePtr<IIndexBufferLayout>&& layout) override;

	public:
		virtual DirectX12InputAssemblerBuilder& withIndexType(const IndexType& type);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder>::VertexBufferLayoutBuilder;

	public:
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

		/// <summary>
		/// 
		/// </summary>
		/// <reamrks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </reamrks>
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex = 0);
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex = 0);
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12IndexBufferLayoutBuilder : public Builder<DirectX12IndexBufferLayoutBuilder, DirectX12IndexBufferLayout, DirectX12InputAssemblerBuilder> {
	public:
		using Builder<DirectX12IndexBufferLayoutBuilder, DirectX12IndexBufferLayout, DirectX12InputAssemblerBuilder>::Builder;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12ShaderProgramBuilder : public ShaderProgramBuilder<DirectX12ShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder> {
	public:
		using ShaderProgramBuilder<DirectX12ShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder>::ShaderProgramBuilder;

	public:
		virtual DirectX12ShaderProgramBuilder& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;
		virtual DirectX12ShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	};
}