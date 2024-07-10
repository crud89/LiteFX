#pragma once

#include <litefx/config.h>

#if !defined (LITEFX_GRAPHICS_API)
#  if defined(LiteFX_Graphics_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_GRAPHICS_API __declspec(dllexport)
#  elif (defined(LiteFX_Graphics_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_GRAPHICS_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Graphics_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_GRAPHICS_API __declspec(dllimport)
#  endif
#endif 

#ifndef LITEFX_GRAPHICS_API
#  define LITEFX_GRAPHICS_API
#endif

#include <litefx/core.h>

namespace LiteFX::Graphics {
	using namespace LiteFX;

	enum class PrimitiveTopology {
		PointList = 0x00010001,
		LineList = 0x00020001,
		TriangleList = 0x00040001,
		LineStrip = 0x00020002,
		TriangleStrip = 0x00040002
	};

}