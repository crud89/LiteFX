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
#include <litefx/graphics.hpp>

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
	class ICommandQueue;
	class ICommandBuffer;
    class IGraphicsDevice;
    class IGraphicsAdapter;
	class IRenderBackend;
	class IRenderPass;
	class IRenderTarget;
	class IBuffer;
	class IBufferLayout;
	class IBufferSet;
	class IBufferPool;

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
		B8G8R8A8_SRGB = 0x01000001,
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

	enum class LITEFX_RENDERING_API BufferSetType {
		VertexData = 0x00000001,
		Resource = 0x00000002
	};

	enum class LITEFX_RENDERING_API BufferType {
		Uniform = 0x00000010,
		Storage = 0x00000020,
		Index = 0x00000040,
		Vertex = 0x00000080,
		Sampler = 0x000000F0
	};

	/// <summary>
	/// Defines how a buffer is used and describes how its memory is managed.
	/// </summary>
	/// <remarks>
	/// There are three common buffer usage scenarios that are supported by the library:
	///
	/// <list type="number">
	/// <item>
	/// <description>
	/// <strong>Static resources</strong>: such as vertex/index/constant buffers, textures or other infrequently updated buffers. In this case, the most efficient 
	/// approach is to create a buffer using <see cref="BufferUsage::Staging" /> and map it from the CPU. Create a second buffer using 
	/// <see cref="BufferUsage::Resource" /> and transfer the staging buffer into it.
	/// </description>
	/// </item>
	/// <item>
	/// <description>
	/// <strong>Dynamic resources</strong>: such as deformable meshes or buffers that need to be updated every frame. For such buffers use the
	/// <see cref="BufferUsage::Dynamic" /> mode to prevent regular transfer overhead.
	/// </description>
	/// </item>
	/// <item>
	/// <description>
	/// <strong>Readbacks</strong>: or resources that are written on the GPU and read by the CPU. The usage mode <see cref="BufferUsage::Readback" /> is designed to 
	/// provide the best performance for this special case.
	/// </description>
	/// </item>
	/// </list>
	/// </remarks>
	enum class LITEFX_RENDERING_API BufferUsage {
		/// <summary>
		/// Creates a buffer that can optimally be mapped from the CPU in order to be transferred to the GPU later.
		/// </summary>
		/// <remarks>
		/// The memory for the buffer will be allocated in the DRAM (CPU or host memory). It can be optimally accessed by the CPU in order to be written. However,
		/// reading it from the GPU may be inefficient. This usage mode should be used to create a staging buffer, i.e. a buffer that is written infrequently and
		/// then transferred to another buffer, that uses <see cref="BufferUsage::Resource" />.
		/// </remarks>
		Staging = 0x00000001,

		/// <summary>
		/// Creates a buffer that can optimally be read by the GPU.
		/// </summary>
		/// <remarks>
		/// The memory for the buffer will be allocated on the VRAM (GPU or device memory). It can be optimally accessed by the GPU in order to be read frequently.
		/// It can be written by a transfer call. Note that those come with an overhead and should only occur infrequently.
		/// </remarks>
		Resource = 0x00000002,

		/// <summary>
		/// Creates a buffer that can be optimally mapped by the CPU and is preferred to be optimally read by the GPU.
		/// </summary>
		/// <remarks>
		/// Dynamic buffers are used when the content is expected to be changed every frame. They do not require transfer calls, but may not be read as efficiently
		/// as <see cref="BufferUsage::Resource" /> buffers.
		/// </remarks>
		Dynamic = 0x00000010,

		/// <summary>
		/// Creates a buffer that can be written by the GPU and read by the CPU.
		/// </summary>
		Readback = 0x00000100
	};

	enum class LITEFX_RENDERING_API IndexType {
		UInt16 = 0x00000010,
		UInt32 = 0x00000020
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

	enum class LITEFX_RENDERING_API RenderTargetType {
		/// <summary>
		/// Represents a color target.
		/// </summary>
		Color = 0x00000001,

		/// <summary>
		/// Represents a depth/stencil target.
		/// </summary>
		Depth = 0x00000002,

		/// <summary>
		/// Represents a color target that should be presented.
		/// </summary>
		/// <remarks>
		/// This is similar to <see cref="RenderTargetType::Color" />, but is used to optimize the memory layout of the target for it to be pushed to a swap chain.
		/// </remarks>
		Present = 0x00000004,

		/// <summary>
		/// Represents a target that should receive a copy of another image through a transfer operation.
		/// </summary>
		Transfer = 0x00000008
	};

	enum class LITEFX_RENDERING_API MultiSamplingLevel {
		x1 = 0x00000001,
		x2 = 0x00000002,
		x4 = 0x00000004,
		x8 = 0x00000008,
		x16 = 0x00000010,
		x32 = 0x00000020,
		x64 = 0x00000040
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
		return (static_cast<UInt32>(format) & 0x0000FF00) >> 8;
	}
}