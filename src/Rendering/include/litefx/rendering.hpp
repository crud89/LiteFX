#pragma once

#include <litefx/app.hpp>
#include <litefx/math.hpp>

#if !defined (LITEFX_RENDERING_API)
#  if defined(LiteFX_Rendering_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_RENDERING_API __declspec(dllexport)
#  elif (defined(LiteFX_Rendering_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_RENDERING_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Rendering_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_RENDERING_API __declspec(dllimport)
#  endif
#endif

#ifndef LITEFX_RENDERING_API
#  define LITEFX_RENDERING_API
#endif

namespace LiteFX::Rendering {
	using namespace LiteFX;
	using namespace LiteFX::Math;

	// Forward declarations.
	class ITexture;
	class ISwapChain;
	class ISurface;
	class ICommandPool;
	class ICommandQueue;
	class IGraphicsDevice;
	class IGraphicsAdapter;
	class IRenderBackend;

	// Define enumerations.
	enum class LITEFX_RENDERING_API GraphicsAdapterType {
		None = 0x00000000,
		GPU = 0x00000001,
		CPU = 0x00000002,
		Other = 0x7FFFFFFF,
	};

	enum class LITEFX_RENDERING_API QueueType {
		None = 0x00000000,
		Graphics = 0x00000001,
		Compute = 0x00000002,
		Transfer = 0x00000004,
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_RENDERING_API Format {
		None = 0x00000000,
		B8G8R8A8_UNORM = 0x00000001,
		B8G8R8A8_UNORM_SRGB = 0x00000011,
		Other = 0x7FFFFFFF,
	};

	// Define flags.
	LITEFX_DEFINE_FLAGS(QueueType);
	// ...


	// Define interfaces.
	class LITEFX_RENDERING_API ITexture {
	public:
		virtual ~ITexture() noexcept = default;

	public:
		virtual Size2d getSize() const noexcept = 0;
		virtual Format getFormat() const noexcept = 0;
	};

	class LITEFX_RENDERING_API ISwapChain {
	public:
		virtual ~ISwapChain() noexcept = default;

	public:
		virtual const IGraphicsDevice* getDevice() const noexcept = 0;
	};

	class LITEFX_RENDERING_API ISurface {
	public:
		virtual ~ISurface() noexcept = default;
	};

	class LITEFX_RENDERING_API ICommandPool {
	public:
		virtual ~ICommandPool() noexcept = default;
	};

	class LITEFX_RENDERING_API ICommandQueue {
	public:
		virtual ~ICommandQueue() noexcept = default;

	public:
		virtual QueueType getType() const noexcept = 0;
	};

	class LITEFX_RENDERING_API IGraphicsDevice {
	public:
		virtual ~IGraphicsDevice() noexcept = default;

	public:
		virtual const IGraphicsAdapter* getAdapter() const noexcept= 0;
		virtual const ISurface* getSurface() const noexcept = 0;
		virtual const ISwapChain* getSwapChain() const noexcept = 0;
		virtual Array<Format> getSurfaceFormats() const = 0;
	};

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
		virtual const ICommandQueue* findQueue(const QueueType& queueType) const = 0;
	};

	class LITEFX_RENDERING_API IRenderBackend {
	public:
		virtual ~IRenderBackend() noexcept = default;

	public:
		virtual Array<const IGraphicsAdapter*> getAdapters() const = 0;
		virtual const IGraphicsAdapter* getAdapter(Optional<uint32_t> adapterId = std::nullopt) const = 0;
	};

	// Base classes.
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

	class LITEFX_RENDERING_API RenderBackend : public IRenderBackend {
		LITEFX_IMPLEMENTATION(RenderBackendImpl)

	public:
		explicit RenderBackend(const App& app) noexcept;
		RenderBackend(const RenderBackend&) noexcept = delete;
		RenderBackend(RenderBackend&&) noexcept = delete;
		virtual ~RenderBackend() noexcept;

	public:
		const App& getApp() const noexcept;
	};

}