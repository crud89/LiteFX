#pragma once

#include <litefx/rendering.hpp>

#include "dx12_api.hpp"
#include "dx12_builders.hpp"
#include "dx12_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Backend : public RenderBackend, public IComResource<IDXGIFactory7> {
		LITEFX_IMPLEMENTATION(DirectX12BackendImpl);
		LITEFX_BUILDER(DirectX12BackendBuilder);

	public:
		explicit DirectX12Backend(const App& app, const bool& advancedSoftwareRasterizer = false);
		DirectX12Backend(const DirectX12Backend&) noexcept = delete;
		DirectX12Backend(DirectX12Backend&&) noexcept = delete;
		virtual ~DirectX12Backend();

	public:
		virtual Array<const IGraphicsAdapter*> listAdapters() const override;
		virtual const IGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const override;
		virtual const ISurface* getSurface() const noexcept override;
		virtual const IGraphicsAdapter* getAdapter() const noexcept override;

	public:
		virtual void use(const IGraphicsAdapter* adapter) override;
		virtual void use(UniquePtr<ISurface>&& surface) override;

	public:
		virtual void enableAdvancedSoftwareRasterizer(const bool& enable = false);
	};
}