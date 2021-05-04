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
		explicit DirectX12Device(const IRenderBackend* backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers);
		DirectX12Device(const DirectX12Device&) = delete;
		DirectX12Device(DirectX12Device&&) = delete;
		virtual ~DirectX12Device() noexcept;

	public:
		virtual size_t getBufferWidth() const noexcept override;
		virtual size_t getBufferHeight() const noexcept override;
		virtual const ICommandQueue* graphicsQueue() const noexcept override;
		virtual const ICommandQueue* transferQueue() const noexcept override;
		virtual const ICommandQueue* bufferQueue() const noexcept override;
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
		virtual Array<Format> getSurfaceFormats() const override;
		virtual const ISwapChain* getSwapChain() const noexcept override;

	public:
		DirectX12RenderPassBuilder buildRenderPass() const;
		//DirectX12ComputePassBuilder buildComputePass() const;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Queue : public ICommandQueue, public IComResource<ID3D12CommandQueue> {
		LITEFX_IMPLEMENTATION(DirectX12QueueImpl);

	public:
		DirectX12Queue(const IGraphicsDevice* device, const QueueType& type, const QueuePriority& priority);
		virtual ~DirectX12Queue() noexcept;

	public:
		virtual bool isBound() const noexcept override;
		virtual QueuePriority getPriority() const noexcept override;
		virtual QueueType getType() const noexcept override;
		virtual const IGraphicsDevice* getDevice() const noexcept override;

	public:
		virtual void bind() override;
		virtual void release() override;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12CommandBuffer : public ICommandBuffer, public IComResource<ID3D12GraphicsCommandList4> {
		LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);

	public:
		DirectX12CommandBuffer(const DirectX12Queue* queue);
		DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;
		DirectX12CommandBuffer(DirectX12CommandBuffer&&) = delete;
		virtual ~DirectX12CommandBuffer() noexcept;

	public:
		virtual const ICommandQueue* getQueue() const noexcept override;

	public:
		virtual void begin() const override;
		virtual void end() const override;
		virtual void submit(const bool& waitForQueue = false) const override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12SwapChain : public ISwapChain, public IComResource<IDXGISwapChain4> {
		LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);

	public:
		DirectX12SwapChain(const DirectX12Device* device, const Size2d& frameBufferSize, const UInt32& frameBuffers, const Format& format = Format::B8G8R8A8_SRGB);
		virtual ~DirectX12SwapChain() noexcept;

	public:
		virtual const Size2d& getBufferSize() const noexcept override;
		virtual size_t getWidth() const noexcept override;
		virtual size_t getHeight() const noexcept override;
		virtual const Format& getFormat() const noexcept override;
		virtual UInt32 swapBackBuffer() const override;
		virtual void reset(const Size2d& frameBufferSize, const UInt32& frameBuffers) override;
		virtual UInt32 getBuffers() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPass : public IRenderPass {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
		LITEFX_BUILDER(DirectX12RenderPassBuilder);

	public:
		DirectX12RenderPass(const IGraphicsDevice* device);	// Adapter for builder interface.
		DirectX12RenderPass(const DirectX12Device* device);
		DirectX12RenderPass(const DirectX12RenderPass&) = delete;
		DirectX12RenderPass(DirectX12RenderPass&&) = delete;
		virtual ~DirectX12RenderPass() noexcept;

	public:
		virtual const ICommandBuffer* getCommandBuffer() const noexcept override;

	public:
		virtual void addTarget(UniquePtr<IRenderTarget>&& target) override;
		virtual const Array<const IRenderTarget*> getTargets() const noexcept override;
		virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) override;
		virtual void setDependency(const IRenderPass* renderPass = nullptr) override;
		virtual const IRenderPass* getDependency() const noexcept override;
		virtual const IRenderPipeline* getPipeline() const noexcept override;
		virtual IRenderPipeline* getPipeline() noexcept override;
		virtual void begin() const override;
		virtual void end(const bool& present = false) override;
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const override;
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const override;
		virtual const IImage* getAttachment(const UInt32& attachmentId) const override;

	public:
		virtual void bind(const IVertexBuffer* buffer) const override;
		virtual void bind(const IIndexBuffer* buffer) const override;
		virtual void bind(IDescriptorSet* buffer) const override;
		virtual UniquePtr<IVertexBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const override;
		virtual UniquePtr<IIndexBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const override;
		virtual UniquePtr<IDescriptorSet> makeBufferPool(const UInt32& bufferSet) const override;

	public:
		virtual const DirectX12Device* getDevice() const noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipeline : public IRenderPipeline, public IComResource<ID3D12PipelineState> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineBuilder);

	public:
		DirectX12RenderPipeline(const DirectX12RenderPass& renderPass);
		DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = delete;
		DirectX12RenderPipeline(const DirectX12RenderPipeline&) noexcept = delete;
		virtual ~DirectX12RenderPipeline() noexcept;

	public:
		virtual const IRenderPipelineLayout* getLayout() const noexcept override;
		virtual IRenderPipelineLayout* getLayout() noexcept override;
		virtual void setLayout(UniquePtr<IRenderPipelineLayout>&& layout) override;
		virtual const IRenderPass& renderPass() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayout : public RenderPipelineLayout, public IComResource<ID3D12RootSignature> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineLayoutBuilder);

	public:
		DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline);
		DirectX12RenderPipelineLayout(DirectX12RenderPipelineLayout&&) noexcept = delete;
		DirectX12RenderPipelineLayout(const DirectX12RenderPipelineLayout&) noexcept = delete;
		virtual ~DirectX12RenderPipelineLayout() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12InputAssembler : public InputAssembler {
		LITEFX_BUILDER(DirectX12InputAssemblerBuilder);

	public:
		DirectX12InputAssembler(const DirectX12RenderPipelineLayout& layout) noexcept;
		DirectX12InputAssembler(DirectX12InputAssembler&&) noexcept = delete;
		DirectX12InputAssembler(const DirectX12InputAssembler&) noexcept = delete;
		virtual ~DirectX12InputAssembler() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Rasterizer : public Rasterizer {
		LITEFX_BUILDER(DirectX12RasterizerBuilder);

	public:
		DirectX12Rasterizer(const DirectX12RenderPipelineLayout& layout) noexcept;
		DirectX12Rasterizer(DirectX12Rasterizer&&) noexcept = delete;
		DirectX12Rasterizer(const DirectX12Rasterizer&) noexcept = delete;
		virtual ~DirectX12Rasterizer() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Viewport : public Viewport {
		LITEFX_BUILDER(DirectX12ViewportBuilder);

	public:
		DirectX12Viewport(const DirectX12RenderPipelineLayout& layout) noexcept;
		DirectX12Viewport(DirectX12Viewport&&) noexcept = delete;
		DirectX12Viewport(const DirectX12Viewport&) noexcept = delete;
		virtual ~DirectX12Viewport() noexcept;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout : public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
		LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);

	public:
		DirectX12VertexBufferLayout(const DirectX12InputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding = 0);
		DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) = delete;
		DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = delete;
		virtual ~DirectX12VertexBufferLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;

	public:
		virtual Array<const BufferAttribute*> getAttributes() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout : public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);
		LITEFX_BUILDER(DirectX12IndexBufferLayoutBuilder);

	public:
		DirectX12IndexBufferLayout(const DirectX12InputAssembler& inputAssembler, const IndexType& type);
		DirectX12IndexBufferLayout(DirectX12IndexBufferLayout&&) = delete;
		DirectX12IndexBufferLayout(const DirectX12IndexBufferLayout&) = delete;
		virtual ~DirectX12IndexBufferLayout() noexcept;

	public:
		virtual size_t getElementSize() const noexcept override;
		virtual UInt32 getBinding() const noexcept override;
		virtual BufferType getType() const noexcept override;

	public:
		virtual const IndexType& getIndexType() const noexcept override;
	};

}