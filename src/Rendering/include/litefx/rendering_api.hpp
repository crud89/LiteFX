#pragma once

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

#include <litefx/core.h>
#include <litefx/math.hpp>

namespace LiteFX::Rendering {
	using namespace LiteFX;
	using namespace LiteFX::Math;

	// Forward declarations.
	class IInputAssembler;
	class IRasterizer;
	class IViewport;
    class IRenderPipeline;
    class IRenderPipelineLayout;
    class IShaderModule;
    class IShaderProgram;
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
		B8G8R8A8_UNORM_SRGB = 0x01000001,
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_RENDERING_API BufferFormat {
		None = 0x00000000,
		X16F = 0x10000101,
		X16I = 0x10000201,
		X16U = 0x10000401,
		XY16F = 0x10000102,
		XY16I = 0x10000202,
		XY16U = 0x10000402,
		XYZ16F = 0x10000103,
		XYZ16I = 0x10000203,
		XYZ16U = 0x10000403,
		XYZW16F = 0x10000104,
		XYZW16I = 0x10000204,
		XYZW16U = 0x10000404,
		X32F = 0x20000101,
		X32I = 0x20000201,
		X32U = 0x20000401,
		XY32F = 0x20000102,
		XY32I = 0x20000202,
		XY32U = 0x20000402,
		XYZ32F = 0x20000103,
		XYZ32I = 0x20000203,
		XYZ32U = 0x20000403,
		XYZW32F = 0x20000104,
		XYZW32I = 0x20000204,
		XYZW32U = 0x20000404
	};

	enum class LITEFX_RENDERING_API ShaderType {
		Vertex = 0x00000001,
		TessellationControl = 0x00000002,
		TessellationEvaluation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_RENDERING_API PolygonMode {
		Solid = 0x00000001,
		Wireframe = 0x00000002,
		Point = 0x00000004
	};

	enum class LITEFX_RENDERING_API CullMode {
		FrontFaces = 0x00000001,
		BackFaces = 0x00000002,
		Both = 0x00000004,
		Disabled = 0x0000000F
	};

	enum class LITEFX_RENDERING_API CullOrder {
		ClockWise = 0x00000001,
		CounterClockWise = 0x00000002
	};

	// Define flags.
	LITEFX_DEFINE_FLAGS(QueueType);
	LITEFX_DEFINE_FLAGS(ShaderType);
	LITEFX_DEFINE_FLAGS(BufferFormat);

	// Helper functions.
	inline UInt32 getBufferFormatChannels(const BufferFormat& format) {
		return static_cast<UInt32>(format) & 0x000000FF;
	}

	inline UInt32 getBufferFormatChannelSize(const BufferFormat& format) {
		return (static_cast<UInt32>(format) & 0xFF000000) >> 24;
	}

	inline UInt32 getBufferFormatType(const BufferFormat& format) {
		return static_cast<UInt32>(format) & 0x0000FF00 >> 8;
	}
}