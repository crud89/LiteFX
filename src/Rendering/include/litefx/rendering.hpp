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
		GPU = 0x01,
		CPU = 0x02,
		Other = 0xFF,
	};

	enum class LITEFX_RENDERING_API QueueType {
		Graphics = 0x01,
		Compute = 0x02,
		Transfer = 0x04,
		Present = 0x10,
		Other = 0xFF
	};

	LITEFX_DEFINE_FLAGS(QueueType)

	class LITEFX_RENDERING_API ISurface {
	};

	class LITEFX_RENDERING_API ICommandPool {
	};

	class LITEFX_RENDERING_API ICommandQueue {
	};

	class LITEFX_RENDERING_API IGraphicsDevice {
	};

	class LITEFX_RENDERING_API IGraphicsAdapter {
	public:
		virtual String getName() const noexcept = 0;
		virtual uint32_t getVendorId() const noexcept = 0;
		virtual uint32_t getDeviceId() const noexcept = 0;
		virtual GraphicsAdapterType getType() const noexcept = 0;
		virtual uint32_t getDriverVersion() const noexcept = 0;
		virtual uint32_t getApiVersion() const noexcept = 0;

	public:
		virtual UniquePtr<IGraphicsDevice> createDevice() const = 0;
	};

	class LITEFX_RENDERING_API IRenderBackend {
	public:
		virtual Array<UniquePtr<IGraphicsAdapter>> getAdapters() const = 0;
		virtual UniquePtr<IGraphicsAdapter> getAdapter(Optional<uint32_t> adapterId = std::nullopt) const = 0;
		virtual UniquePtr<ICommandQueue> createQueue(const QueueType& queueType) const = 0;
		//virtual UniquePtr<ISurface> createSurface() const = 0;
		//virtual void useAdapter(const GraphicsAdapter* adapter) const = 0;
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