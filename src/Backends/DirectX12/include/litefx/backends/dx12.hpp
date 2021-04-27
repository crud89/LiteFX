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

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12GraphicsAdapter : public IGraphicsAdapter, public IComResource<IDXGIAdapter4> {
		LITEFX_IMPLEMENTATION(DirectX12GraphicsAdapterImpl);

	public:
		DirectX12GraphicsAdapter(ComPtr<IDXGIAdapter4> adapter);
		DirectX12GraphicsAdapter(const DirectX12GraphicsAdapter&) = delete;
		DirectX12GraphicsAdapter(DirectX12GraphicsAdapter&&) = delete;
		virtual ~DirectX12GraphicsAdapter() noexcept;

	public:
		virtual String getName() const noexcept override;
		virtual uint32_t getVendorId() const noexcept override;
		virtual uint32_t getDeviceId() const noexcept override;
		virtual GraphicsAdapterType getType() const noexcept override;
		
		/// <inheritdoc />
		/// <remarks>
		/// This property is not supported by DirectX 12. The method always returns `0`.
		/// </remarks>
		virtual uint32_t getDriverVersion() const noexcept override;

		/// <inheritdoc />
		/// <remarks>
		/// This property is not supported by DirectX 12. The method always returns `0`.
		/// </remarks>
		virtual uint32_t getApiVersion() const noexcept override;
		virtual uint32_t getDedicatedMemory() const noexcept override;

	public:
		virtual ICommandQueue* findQueue(const QueueType& queueType) const override;
		virtual ICommandQueue* findQueue(const QueueType& queueType, const ISurface* forSurface) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Surface : public ISurface, public IResource<HWND> {
	public:
		DirectX12Surface(const HWND& hwnd) noexcept;
		DirectX12Surface(const DirectX12Surface&) = delete;
		DirectX12Surface(DirectX12Surface&&) = delete;
		virtual ~DirectX12Surface() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Device : public GraphicsDevice, public IComResource<ID3D12Device5> {
		LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);

	public:
		explicit DirectX12Device(const IRenderBackend* backend, const Format& format);
		DirectX12Device(const DirectX12Device&) = delete;
		DirectX12Device(DirectX12Device&&) = delete;
		virtual ~DirectX12Device() noexcept;

	public:
		virtual size_t getBufferWidth() const noexcept override;
		virtual size_t getBufferHeight() const noexcept override;
		virtual void wait() override;
		virtual void resize(int width, int height) override;
		virtual UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements = 1) const override;
		virtual UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements = 1) const override;
		virtual UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const override;
		virtual UniquePtr<IConstantBuffer> createConstantBuffer(const IDescriptorLayout* layout, const BufferUsage& usage, const UInt32& elements) const override;
		virtual UniquePtr<IImage> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<IImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<ITexture> createTexture(const IDescriptorLayout* layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;
		virtual UniquePtr<ISampler> createSampler(const IDescriptorLayout* layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") const override;
		virtual Array<UniquePtr<IImage>> createSwapChainImages(const ISwapChain* swapChain) const override;
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;

	public:
		//DirectX12RenderPassBuilder buildRenderPass() const;
		////DirectX12ComputePassBuilder buildComputePass() const;
	};

}