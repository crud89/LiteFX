#pragma once

#if !defined (LITEFX_APPMODEL_API)
#  if defined(LiteFX_AppModel_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_APPMODEL_API __declspec(dllexport)
#  elif (defined(LiteFX_AppModel_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_APPMODEL_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_AppModel_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_APPMODEL_API __declspec(dllimport)
#  endif
#endif

#ifndef LITEFX_APPMODEL_API
#  define LITEFX_APPMODEL_API
#endif

#include <litefx/core.h>
#include <litefx/logging.hpp>

namespace LiteFX {

	class IBackend;
	class AppVersion;
	class App;
	class AppBuilder;

	enum class LITEFX_APPMODEL_API Platform {
		None = 0x00000000,
		Win32 = 0x00000001,
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_APPMODEL_API BackendType {
		Rendering = 0x01,
		Other = 0xFF
	};

	enum class LITEFX_APPMODEL_API BackendState {
		Inactive = 0x01,
		Active = 0x02
	};
}