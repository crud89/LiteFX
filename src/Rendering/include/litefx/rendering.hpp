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

namespace LiteFX {
	namespace Rendering {

		using namespace LiteFX;

		enum class LITEFX_RENDERING_API GraphicsAdapterType 
		{
			GPU = 0x01,
			CPU = 0x02,
			Other = 0xFF,
		};

		enum class LITEFX_RENDERING_API QueueType 
		{
			Graphics = 0x01,
			Compute = 0x02,
			Transfer = 0x04,
			Present = 0x10,
			Other = 0xFF
		};

		class LITEFX_RENDERING_API Surface
		{
		public:
			Surface() = default;
			Surface(const Surface&) = delete;
			Surface(Surface&&) = delete;
			virtual ~Surface() = default;
		};

		class LITEFX_RENDERING_API CommandPool
		{
		public:
			CommandPool() = default;
			CommandPool(const CommandPool&) = delete;
			CommandPool(CommandPool&&) = delete;
			virtual ~CommandPool() = default;
		};

		class LITEFX_RENDERING_API CommandQueue
		{
		public:
			CommandQueue() = default;
			CommandQueue(const CommandQueue&) = delete;
			CommandQueue(CommandQueue&&) = delete;
			virtual ~CommandQueue() = default;
		};

		class LITEFX_RENDERING_API GraphicsDevice
		{
		public:
			GraphicsDevice() = default;
			GraphicsDevice(const GraphicsDevice&) = delete;
			GraphicsDevice(GraphicsDevice&&) = delete;
			virtual ~GraphicsDevice() = default;
		};

		class LITEFX_RENDERING_API GraphicsAdapter
		{
		public:
			GraphicsAdapter() = default;
			GraphicsAdapter(const GraphicsAdapter&) = delete;
			GraphicsAdapter(GraphicsAdapter&&) = delete;
			virtual ~GraphicsAdapter() = default;

		public:
			virtual String getName() const = 0;
			virtual uint32_t getVendorId() const = 0;
			virtual uint32_t getDeviceId() const = 0;
			virtual GraphicsAdapterType getType() const = 0;
			virtual uint32_t getDriverVersion() const = 0;
			virtual uint32_t getApiVersion() const = 0;

		public:
			virtual UniquePtr<GraphicsDevice> createDevice() const = 0;
		};

		class LITEFX_RENDERING_API RenderBackend
		{
		private:
			const App& m_app;

		public:
			explicit RenderBackend(const App& app);
			RenderBackend(const RenderBackend&) = delete;
			RenderBackend(RenderBackend&&) = delete;
			virtual ~RenderBackend() = default;

		public:
			const App& getApp() const;

		public:
			virtual void getAdapters(Array<SharedPtr<GraphicsAdapter>>& adapters, bool forceReload = false) const = 0;
			virtual UniquePtr<CommandQueue> createQueue(const QueueType& queueType) const = 0;
			virtual UniquePtr<Surface> createSurface() const = 0;
			//virtual void useAdapter(const GraphicsAdapter* adapter) const = 0;
		};

	}
}