#pragma once

#include <litefx/core_types.hpp>

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


	LITEFX_DEFINE_FLAGS(QueueType);
	// ...


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
		virtual UniquePtr<IGraphicsDevice> createDevice(const ISurface* surface) const = 0;
		virtual SharedPtr<ICommandQueue> findQueue(const QueueType& queueType) const = 0;
	};

	class LITEFX_RENDERING_API IRenderBackend {
	public:
		virtual ~IRenderBackend() noexcept = default;

	public:
		virtual Array<UniquePtr<IGraphicsAdapter>> getAdapters() const = 0;
		virtual UniquePtr<IGraphicsAdapter> getAdapter(Optional<uint32_t> adapterId = std::nullopt) const = 0;

		// Platform specific code.
	public:
#if (defined _WIN32 || defined WINCE)
		virtual UniquePtr<ISurface> createSurfaceWin32(HWND hwnd) const = 0;
#else 
#pragma message ("IRenderBackend: No supported surface platform detected.")
#endif
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