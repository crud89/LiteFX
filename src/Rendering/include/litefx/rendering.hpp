#pragma once

#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>
#include <litefx/rendering_pipelines.hpp>

namespace LiteFX::Rendering {
	using namespace LiteFX;
	using namespace LiteFX::Math;

	// Define interfaces.
	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IInputAssembler {
	public:
		virtual ~IInputAssembler() noexcept = default;

	public:
		virtual const BufferLayout* getLayout() const = 0;
		virtual void use(UniquePtr<BufferLayout>&& layout) = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IRasterizer {
	public:
		virtual ~IRasterizer() noexcept = default;

	public:
		virtual PolygonMode getPolygonMode() const noexcept = 0;
		virtual void setPolygonMode(const PolygonMode& mode) noexcept = 0;
		virtual CullMode getCullMode() const noexcept = 0;
		virtual void setCullMode(const CullMode& mode) noexcept = 0;
		virtual CullOrder getCullOrder() const noexcept = 0;
		virtual void setCullOrder(const CullOrder& order) noexcept = 0;
		virtual Float getLineWidth() const noexcept = 0;
		virtual void setLineWidth(const Float& width) noexcept = 0;
		virtual bool getDepthBiasEnabled() const noexcept = 0;
		virtual void setDepthBiasEnabled(const bool& enable) noexcept = 0;
		virtual float getDepthBiasClamp() const noexcept = 0;
		virtual void setDepthBiasClamp(const float& clamp) noexcept = 0;
		virtual float getDepthBiasConstantFactor() const noexcept = 0;
		virtual void setDepthBiasConstantFactor(const float& factor) noexcept = 0;
		virtual float getDepthBiasSlopeFactor() const noexcept = 0;
		virtual void setDepthBiasSlopeFactor(const float& factor) noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IViewport {
	public:
		virtual ~IViewport() noexcept = default;

	public:
		virtual RectF getRectangle() const noexcept = 0;
		virtual void setRectangle(const RectF& rectangle) noexcept = 0;
		virtual const Array<RectF>& getScissors() const noexcept = 0;
		virtual Array<RectF>& getScissors() noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ITexture {
	public:
		virtual ~ITexture() noexcept = default;

	public:
		virtual Size2d getSize() const noexcept = 0;
		virtual Format getFormat() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ISwapChain {
	public:
		virtual ~ISwapChain() noexcept = default;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept = 0;
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
	class LITEFX_RENDERING_API ICommandPool {
	public:
		virtual ~ICommandPool() noexcept = default;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ICommandQueue {
	public:
		virtual ~ICommandQueue() noexcept = default;

	public:
		virtual QueueType getType() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IGraphicsDevice {
	public:
		virtual ~IGraphicsDevice() noexcept = default;

	public:
		virtual const IGraphicsAdapter* getAdapter() const noexcept= 0;
		virtual const ISurface* getSurface() const noexcept = 0;
		virtual const ISwapChain* getSwapChain() const noexcept = 0;
		virtual Array<Format> getSurfaceFormats() const = 0;

	public:
		//virtual UniquePtr<ITexture> createTexture2d(const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) const = 0;
		virtual UniquePtr<IShaderModule> loadShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main") const = 0;
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

	public:
		virtual UniquePtr<IGraphicsDevice> createDevice(const ISurface* surface, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Array<String>& extensions = { }) const = 0;
		virtual ICommandQueue* findQueue(const QueueType& queueType) const = 0;
		virtual ICommandQueue* findQueue(const QueueType& queueType, const ISurface* forSurface) const = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API GraphicsDevice : public IGraphicsDevice {
		LITEFX_IMPLEMENTATION(GraphicsDeviceImpl)

	public:
		GraphicsDevice(const IGraphicsAdapter* adapter, const ISurface* surface);
		GraphicsDevice(const GraphicsDevice&) noexcept = delete;
		GraphicsDevice(GraphicsDevice&&) noexcept = delete;
		virtual ~GraphicsDevice() noexcept;

	public:
		virtual const IGraphicsAdapter* getAdapter() const noexcept override;
		virtual const ISurface* getSurface() const noexcept override;
	};

	/// <summary>
	/// 
	/// </summary>
	template <typename TDerived, typename TDevice>
	class GraphicsDeviceBuilder : public Builder<TDerived, TDevice> {
	public:
		using builder_type::Builder;

	public:
		virtual TDerived& withFormat(const Format& format) = 0;
		virtual TDerived& withQueue(const QueueType& queueType) = 0;
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
		template <typename T, typename ...TArgs, std::enable_if_t<std::is_convertible_v<T*, IGraphicsDevice*>, int> = 0, typename TBuilder = T::builder>
		TBuilder build(TArgs&&... _args) const {
			return TBuilder(makeUnique<T>(this->getAdapter(), this->getSurface(), std::forward<TArgs>(_args)...));
		}
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API RenderBackend : public IRenderBackend {
		LITEFX_IMPLEMENTATION(RenderBackendImpl)

	public:
		explicit RenderBackend(const App& app) noexcept;
		RenderBackend(const RenderBackend&) noexcept = delete;
		RenderBackend(RenderBackend&&) noexcept = delete;
		virtual ~RenderBackend() noexcept;

	public:
		virtual BackendType getType() const noexcept override;
		const App& getApp() const noexcept;
	};
}