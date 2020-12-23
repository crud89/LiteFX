#pragma once

#include <litefx/rendering.hpp>
#include "dx12_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class DirectX12BackendBuilder;
	//class DirectX12RenderPipelineBuilder;
	//class DirectX12RenderPipelineLayoutBuilder;
	//class DirectX12ShaderProgramBuilder;
	//class DirectX12RenderPassBuilder;
	//class DirectX12RasterizerBuilder;
	//class DirectX12ViewportBuilder;
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

}