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

		class LITEFX_RENDERING_API RenderBackend {
		private:
			const App& m_app;

		public:
			explicit RenderBackend(const App& app);
			RenderBackend(const RenderBackend&) = delete;
			RenderBackend(RenderBackend&&) = delete;
			virtual ~RenderBackend() = default;

		public:
			const App& getApp() const;
		};

	}
}