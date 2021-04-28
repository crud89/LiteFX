#pragma once

#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>
#include <litefx/rendering_pipelines.hpp>

namespace LiteFX::Rendering {
	using namespace LiteFX;
	using namespace LiteFX::Math;

	class LITEFX_RENDERING_API ICommandBuffer {
	public:
		virtual ~ICommandBuffer() noexcept = default;

	public:
		virtual const ICommandQueue* getQueue() const noexcept = 0;

	public:
		virtual void begin() const = 0;
		virtual void end() const = 0;
		virtual void submit(const bool& waitForQueue = false) const = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IRenderTarget {
	public:
		virtual ~IRenderTarget() noexcept = default;

	public:
		virtual RenderTargetType getType() const noexcept = 0;
		virtual void setType(const RenderTargetType& type) = 0;
		virtual MultiSamplingLevel getSamples() const noexcept = 0;
		virtual void setSamples(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) = 0;
		virtual bool getClearBuffer() const noexcept = 0;
		virtual void setClearBuffer(const bool& clear = true) = 0;
		virtual bool getClearStencil() const noexcept = 0;
		virtual void setClearStencil(const bool& clear = true) = 0;
		virtual Format getFormat() const noexcept = 0;
		virtual void setFormat(const Format& format) = 0;

		/// <summary>
		/// Gets whether the target should be made persistent for access after the render pass has finished.
		/// </summary>
		/// <remarks>
		/// A render target can be marked as volatile if it does not need to be accessed after the render pass has finished. This can be used to optimize away unnecessary GPU/CPU 
		/// memory round-trips. For example a depth buffer may only be used as an input for the lighting stage of a deferred renderer, but is not required after this. So instead
		/// of reading it from the GPU after the lighting pass has finished and then discarding it anyway, it can be marked as volatile in order to prevent it from being read from
		/// the GPU memory again in the first place.
		/// </remarks>
		/// <seealso cref="IRenderTarget::setVolatile" />
		virtual bool getVolatile() const noexcept = 0;

		/// <summary>
		/// Sets whether the target should be made persistent for access after the render pass has finished.
		/// </summary>
		/// <seealso cref="IRenderTarget::getVolatile" />
		virtual void setVolatile(const bool& isVolatile = false) = 0;

		virtual const Vector4f& getClearValues() const noexcept = 0;
		virtual void setClearValues(const Vector4f& values) = 0;
	};

	class LITEFX_RENDERING_API RenderTarget : public IRenderTarget {
		LITEFX_IMPLEMENTATION(RenderTargetImpl);

	public:
		RenderTarget();
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget(RenderTarget&&) = delete;
		virtual ~RenderTarget() noexcept;

	public:
		/// <inheritdoc />
		virtual RenderTargetType getType() const noexcept override;

		/// <inheritdoc />
		virtual void setType(const RenderTargetType& type) override;

		/// <inheritdoc />
		virtual MultiSamplingLevel getSamples() const noexcept override;

		/// <inheritdoc />
		virtual void setSamples(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) override;

		/// <inheritdoc />
		virtual bool getClearBuffer() const noexcept override;

		/// <inheritdoc />
		virtual void setClearBuffer(const bool& clear = true) override;

		/// <inheritdoc />
		virtual bool getClearStencil() const noexcept override;

		/// <inheritdoc />
		virtual void setClearStencil(const bool& clear = true) override;

		/// <inheritdoc />
		virtual Format getFormat() const noexcept override;

		/// <inheritdoc />
		virtual void setFormat(const Format& format) override;

		/// <inheritdoc />
		virtual bool getVolatile() const noexcept override;

		/// <inheritdoc />
		virtual void setVolatile(const bool& isVolatile = false) override;

		/// <inheritdoc />
		virtual const Vector4f& getClearValues() const noexcept override;

		/// <inheritdoc />
		virtual void setClearValues(const Vector4f& values) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ISwapChain {
	public:
		virtual ~ISwapChain() noexcept = default;

	public:
		virtual const Size2d& getBufferSize() const noexcept = 0;
		virtual size_t getWidth() const noexcept = 0;
		virtual size_t getHeight() const noexcept = 0;
		virtual const Format& getFormat() const noexcept = 0;
		virtual Array<const IImage*> getFrames() const noexcept = 0;
		virtual UInt32 swapBackBuffer() const = 0;
		virtual void reset() = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ISurface {
	public:
		virtual ~ISurface() noexcept = default;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ICommandQueue {
	public:
		virtual ~ICommandQueue() noexcept = default;

	public:
		virtual bool isBound() const noexcept = 0;
		virtual QueuePriority getPriority() const noexcept = 0;
		virtual QueueType getType() const noexcept = 0;
		virtual const IGraphicsDevice* getDevice() const noexcept = 0;

	public:
		virtual void bind() = 0;
		virtual void release() = 0;
		virtual UniquePtr<ICommandBuffer> createCommandBuffer() const = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IGraphicsDevice {
	public:
		virtual ~IGraphicsDevice() noexcept = default;

	public:
		virtual Array<UniquePtr<IImage>> createSwapChainImages(const ISwapChain* swapChain) const = 0;
		virtual UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements = 1) const = 0;
		virtual UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;
		virtual UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout* layout, const BufferUsage& usage, const UInt32& elements) const = 0;
		virtual UniquePtr<IConstantBuffer> createConstantBuffer(const IDescriptorLayout* layout, const BufferUsage& usage, const UInt32& elements) const = 0;
		virtual UniquePtr<IImage> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;
		virtual UniquePtr<IImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;
		virtual UniquePtr<ITexture> createTexture(const IDescriptorLayout* layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;
		virtual UniquePtr<ISampler> createSampler(const IDescriptorLayout* layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") const = 0;
		virtual void wait() = 0;
		virtual void resize(int width, int height) = 0;

		virtual const IRenderBackend* getBackend() const noexcept = 0;
		virtual const ISwapChain* getSwapChain() const noexcept = 0;
		virtual Array<Format> getSurfaceFormats() const = 0;
		virtual size_t getBufferWidth() const noexcept = 0;
		virtual size_t getBufferHeight() const noexcept = 0;
		//virtual Color getBackColor() const noexcept = 0;
		//virtual void setBackColor(const Color& color) = 0;

		/// <summary>
		/// Returns the instance of the queue, used to process draw calls.
		/// </summary>
		virtual const ICommandQueue* graphicsQueue() const noexcept = 0;

		/// <summary>
		/// Returns the instance of the queue used for device-device transfers (e.g. between render-passes).
		/// </summary>
		/// <remarks>
		/// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
		/// </remarks>
		virtual const ICommandQueue* transferQueue() const noexcept = 0;

		/// <summary>
		/// Returns the instance of the queue used for host-device transfers.
		/// </summary>
		/// <remarks>
		/// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
		/// </remarks>
		//virtual const ICommandQueue* bufferQueue() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IGraphicsAdapter {
	public:
		virtual ~IGraphicsAdapter() noexcept = default;

	public:
		virtual String getName() const noexcept = 0;
		virtual uint32_t getVendorId() const noexcept = 0;
		virtual uint32_t getDeviceId() const noexcept = 0;
		virtual GraphicsAdapterType getType() const noexcept = 0;
		virtual uint32_t getDriverVersion() const noexcept = 0;
		virtual uint32_t getApiVersion() const noexcept = 0;
		virtual uint32_t getDedicatedMemory() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API GraphicsDevice : public IGraphicsDevice {
		LITEFX_IMPLEMENTATION(GraphicsDeviceImpl);

	public:
		GraphicsDevice(const IRenderBackend* backend);
		GraphicsDevice(const GraphicsDevice&) noexcept = delete;
		GraphicsDevice(GraphicsDevice&&) noexcept = delete;
		virtual ~GraphicsDevice() noexcept;

	public:
		virtual const IRenderBackend* getBackend() const noexcept override;

	public:
		/// <summary>
		/// Initializes a new render pass of type <typeparamref name="T"/> and returns a builder instance for it.
		/// </summary>
		/// <param name="_args">The arguments which are passed to the constructor of the render pass.</param>
		/// <typeparam name="T">The type of the render pass. The type must implement <see cref="IRenderPass" /> interface.</typeparam>
		template <typename T, typename ...TArgs, std::enable_if_t<std::is_convertible_v<T*, IRenderPass*>, int> = 0, typename TBuilder = T::builder>
		TBuilder build(TArgs&&... _args) const {
			return TBuilder(makeUnique<T>(this, std::forward<TArgs>(_args)...));
		}
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IRenderBackend : public IBackend {
	public:
		virtual ~IRenderBackend() noexcept = default;

	public:
		virtual Array<const IGraphicsAdapter*> listAdapters() const = 0;
		virtual const IGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;
		virtual const ISurface* getSurface() const noexcept = 0;
		virtual const IGraphicsAdapter* getAdapter() const noexcept = 0;

	public:
		virtual void use(const IGraphicsAdapter* adapter) = 0;
		virtual void use(UniquePtr<ISurface>&& surface) = 0;

	public:
		template <typename T, typename ...TArgs, std::enable_if_t<std::is_convertible_v<T*, IGraphicsDevice*>, int> = 0>
		UniquePtr<T> createDevice(TArgs&&... _args) const {
			return makeUnique<T>(this, std::forward<TArgs>(_args)...);
		}
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API RenderBackend : public IRenderBackend {
		LITEFX_IMPLEMENTATION(RenderBackendImpl);

	public:
		explicit RenderBackend(const App& app);
		RenderBackend(const RenderBackend&) noexcept = delete;
		RenderBackend(RenderBackend&&) noexcept = delete;
		virtual ~RenderBackend() noexcept;

	public:
		virtual BackendType getType() const noexcept override;
		const App& getApp() const noexcept;
	};
}