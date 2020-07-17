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
		R4G4_UNORM,
		R4G4B4A4_UNORM,
		B4G4R4A4_UNORM,
		R5G6B5_UNORM,
		B5G6R5_UNORM,
		R5G5B5A1_UNORM,
		B5G5R5A1_UNORM,
		A1R5G5B5_UNORM,
		R8_UNORM,
		R8_SNORM,
		R8_USCALED,
		R8_SSCALED,
		R8_UINT,
		R8_SINT,
		R8_SRGB,
		R8G8_UNORM,
		R8G8_SNORM,
		R8G8_USCALED,
		R8G8_SSCALED,
		R8G8_UINT,
		R8G8_SINT,
		R8G8_SRGB,
		R8G8B8_UNORM,
		R8G8B8_SNORM,
		R8G8B8_USCALED,
		R8G8B8_SSCALED,
		R8G8B8_UINT,
		R8G8B8_SINT,
		R8G8B8_SRGB,
		B8G8R8_UNORM,
		B8G8R8_SNORM,
		B8G8R8_USCALED,
		B8G8R8_SSCALED,
		B8G8R8_UINT,
		B8G8R8_SINT,
		B8G8R8_SRGB,
		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R8G8B8A8_USCALED,
		R8G8B8A8_SSCALED,
		R8G8B8A8_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_SRGB,
		B8G8R8A8_UNORM,
		B8G8R8A8_SNORM,
		B8G8R8A8_USCALED,
		B8G8R8A8_SSCALED,
		B8G8R8A8_UINT,
		B8G8R8A8_SINT,
		B8G8R8A8_SRGB,
		A8B8G8R8_UNORM,
		A8B8G8R8_SNORM,
		A8B8G8R8_USCALED,
		A8B8G8R8_SSCALED,
		A8B8G8R8_UINT,
		A8B8G8R8_SINT,
		A8B8G8R8_SRGB,
		A2R10G10B10_UNORM,
		A2R10G10B10_SNORM,
		A2R10G10B10_USCALED,
		A2R10G10B10_SSCALED,
		A2R10G10B10_UINT,
		A2R10G10B10_SINT,
		A2B10G10R10_UNORM,
		A2B10G10R10_SNORM,
		A2B10G10R10_USCALED,
		A2B10G10R10_SSCALED,
		A2B10G10R10_UINT,
		A2B10G10R10_SINT,
		R16_UNORM,
		R16_SNORM,
		R16_USCALED,
		R16_SSCALED,
		R16_UINT,
		R16_SINT,
		R16_SFLOAT,
		R16G16_UNORM,
		R16G16_SNORM,
		R16G16_USCALED,
		R16G16_SSCALED,
		R16G16_UINT,
		R16G16_SINT,
		R16G16_SFLOAT,
		R16G16B16_UNORM,
		R16G16B16_SNORM,
		R16G16B16_USCALED,
		R16G16B16_SSCALED,
		R16G16B16_UINT,
		R16G16B16_SINT,
		R16G16B16_SFLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_SNORM,
		R16G16B16A16_USCALED,
		R16G16B16A16_SSCALED,
		R16G16B16A16_UINT,
		R16G16B16A16_SINT,
		R16G16B16A16_SFLOAT,
		R32_UINT,
		R32_SINT,
		R32_SFLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G32_SFLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,
		R32G32B32_SFLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32A32_SFLOAT,
		R64_UINT,
		R64_SINT,
		R64_SFLOAT,
		R64G64_UINT,
		R64G64_SINT,
		R64G64_SFLOAT,
		R64G64B64_UINT,
		R64G64B64_SINT,
		R64G64B64_SFLOAT,
		R64G64B64A64_UINT,
		R64G64B64A64_SINT,
		R64G64B64A64_SFLOAT,
		B10G11R11_UFLOAT,
		E5B9G9R9_UFLOAT,
		D16_UNORM,
		X8_D24_UNORM,
		D32_SFLOAT,
		S8_UINT,
		D16_UNORM_S8_UINT,
		D24_UNORM_S8_UINT,
		D32_SFLOAT_S8_UINT,
		BC1_RGB_UNORM,
		BC1_RGB_SRGB,
		BC1_RGBA_UNORM,
		BC1_RGBA_SRGB,
		BC2_UNORM,
		BC2_SRGB,
		BC3_UNORM,
		BC3_SRGB,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UFLOAT,
		BC6H_SFLOAT,
		BC7_UNORM,
		BC7_SRGB,
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