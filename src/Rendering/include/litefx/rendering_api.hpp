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

namespace LiteFX::Rendering {
	using namespace LiteFX;

	// Forward declarations.
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

	enum class LITEFX_RENDERING_API ShaderType {
		Vertex = 0x00000001,
		TessellationControl = 0x00000002,
		TessellationEvaluation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		Other = 0x7FFFFFFF
	};

	// Define flags.
	LITEFX_DEFINE_FLAGS(QueueType);
	LITEFX_DEFINE_FLAGS(ShaderType);
	// ...

}