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
	class IScissor;
    class IRenderPipeline;
    class IRenderPipelineLayout;
    class IShaderModule;
    class IShaderProgram;
    class ITexture;
	class IRenderPass;
	class IBufferLayout;
	class IVertexBufferLayout;
	class IIndexBufferLayout;
	class IDescriptorLayout;
	class IDescriptorSetLayout;
	class IMappable;
	class IBindable;
	class ITransferable;
	class IDeviceMemory;
	class IBuffer;
	class IVertexBuffer;
	class IIndexBuffer;
	class IConstantBuffer;
	class IDescriptor;
	class IImage;
	class ITexture;
	class ISampler;
	class IDescriptorSet;

	class Viewport;
	class Scissor;
	class Buffer;
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class Image;
	class Texture;
	class Sampler;

	// Common interface declarations.

	// Define enumerations.
	/// <summary>
	/// Defines different types of graphics adapters.
	/// </summary>
	enum class LITEFX_RENDERING_API GraphicsAdapterType {
		/// <summary>
		/// The adapter is not a valid graphics adapter.
		/// </summary>
		None = 0x00000000,
		
		/// <summary>
		/// The adapter is a dedicated GPU adapter.
		/// </summary>
		GPU = 0x00000001,

		/// <summary>
		/// The adapter is an integrated CPU.
		/// </summary>
		CPU = 0x00000002,

		/// <summary>
		/// The adapter type is not captured by this enum. This value is used internally to mark invalid adapters and should not be used.
		/// </summary>
		Other = 0x7FFFFFFF,
	};

	enum class LITEFX_RENDERING_API QueueType {
		None = 0x00000000,
		Graphics = 0x00000001,
		Compute = 0x00000002,
		Transfer = 0x00000004,
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_RENDERING_API QueuePriority {
		Normal = 33,
		High = 66,
		Realtime = 100
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

	enum class LITEFX_RENDERING_API AttributeSemantic {
		Binormal = 0x00000001,
		BlendIndices = 0x00000002,
		BlendWeight = 0x00000003,
		Color = 0x00000004,
		Normal = 0x00000005,
		Position = 0x00000006,
		TransformedPosition = 0x00000007,
		PointSize = 0x00000008,
		Tangent = 0x00000009,
		TextureCoordinate = 0x0000000A,
		Unknown = 0x7FFFFFFF
	};

	enum class LITEFX_RENDERING_API DescriptorType {
		Uniform         = 0x00000001,
		Storage         = 0x00000002,
		Image           = 0x00000003,
		Sampler         = 0x00000004,
		InputAttachment = 0x00000005
	};

	enum class LITEFX_RENDERING_API BufferType {
		Vertex = 0x00000001,
		Index = 0x00000002,
		Uniform = 0x00000003,
		Storage = 0x00000004,
		Other = 0x7FFFFFFF
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

	enum class LITEFX_RENDERING_API ShaderStage {
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
		DepthStencil = 0x00000002,

		/// <summary>
		/// Represents a color target that should be presented.
		/// </summary>
		/// <remarks>
		/// This is similar to <see cref="RenderTargetType::Color" />, but is used to optimize the memory layout of the target for it to be pushed to a swap chain.
		/// </remarks>
		Present = 0x00000004
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

	enum class LITEFX_RENDERING_API FilterMode {
		Nearest = 0x00000001,
		Linear = 0x00000002
	};

	enum class LITEFX_RENDERING_API MipMapMode {
		Nearest = 0x00000001,
		Linear = 0x00000002
	};

	enum class LITEFX_RENDERING_API BorderMode {
		Repeat = 0x00000001,
		RepeatMirrored = 0x00010001,
		ClampToEdge = 0x00000002,
		ClampToEdgeMirrored = 0x00010002,
		ClampToBorder = 0x00000003,
	};

	// Define flags.
	LITEFX_DEFINE_FLAGS(QueueType);
	LITEFX_DEFINE_FLAGS(ShaderStage);
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

	/// <summary>
	/// 
	/// </summary>
	size_t LITEFX_RENDERING_API getSize(const Format& format);

	/// <summary>
	/// 
	/// </summary>
	bool LITEFX_RENDERING_API hasDepth(const Format& format);

	/// <summary>
	/// 
	/// </summary>
	bool LITEFX_RENDERING_API hasStencil(const Format& format);

	/// <summary>
	/// Represents a physical graphics adapter.
	/// </summary>
	/// <remarks>
	/// A graphics adapter can be seen as an actual phyiscal device that can run graphics computations. Typically this resembles a GPU that is connected
	/// to the bus. However, it can also represent an emulated, virtual adapter, such as a software rasterizer.
	/// </remarks>
	class LITEFX_RENDERING_API IGraphicsAdapter {
	public:
		virtual ~IGraphicsAdapter() noexcept = default;

	public:
		/// <summary>
		/// Retrieves the name of the graphics adapter.
		/// </summary>
		/// <returns>The name of the graphics adapter.</returns>
		virtual String getName() const noexcept = 0;

		/// <summary>
		/// Returns a unique identifier, that identifies the vendor of the graphics adapter.
		/// </summary>
		/// <returns>A unique identifier, that identifies the vendor of the graphics adapter.</returns>
		virtual UInt32 getVendorId() const noexcept = 0;

		/// <summary>
		/// Returns a unique identifier, that identifies the product.
		/// </summary>
		/// <returns>A unique identifier, that identifies the product.</returns>
		virtual UInt32 getDeviceId() const noexcept = 0;

		/// <summary>
		/// Returns the type of the graphics adapter.
		/// </summary>
		/// <returns>The type of the graphics adapter.</returns>
		virtual GraphicsAdapterType getType() const noexcept = 0;

		/// <summary>
		/// Returns the graphics driver version.
		/// </summary>
		/// <returns>The graphics driver version.</returns>
		virtual UInt32 getDriverVersion() const noexcept = 0;

		/// <summary>
		/// Returns the graphics API version.
		/// </summary>
		/// <returns>The graphics API version.</returns>
		virtual UInt32 getApiVersion() const noexcept = 0;

		/// <summary>
		/// Returns the amount of dedicated graphics memory (in bytes), this adapter can use.
		/// </summary>
		/// <returns>The amount of dedicated graphics memory (in bytes), this adapter can use.</returns>
		virtual UInt64 getDedicatedMemory() const noexcept = 0;
	};

	/// <summary>
	/// Represents a surface to render to.
	/// </summary>
	/// <remarks>
	/// A surface can be seen as a window or area on the screen, the renderer can draw to. Note that the interface does not make any constraints on the surface
	/// to allow for portability. A surface implementation may provide access to the actual handle to use. Surface instances are responsible for owning the handle.
	/// </remarks>
	class LITEFX_RENDERING_API ISurface {
	public:
		virtual ~ISurface() noexcept = default;
	};

	/// <summary>
	/// Represents a render target, i.e. an abstract view of the output of an <see cref="IRenderPass" />.
	/// </remarks>
	/// <remarks>
	/// A render target represents one output of a render pass, stored within an <see cref="IImage" />. It is contained by a <see cref="IFrameBuffer" />, that records 
	/// draw commands for it.
	/// </remarks>
	/// <seealso cref="RenderTarget" />
	/// <seealso cref="IFrameBuffer" />
	/// <seealso cref="IImage" />
	class LITEFX_RENDERING_API IRenderTarget {
	public:
		virtual ~IRenderTarget() noexcept = default;

	public:
		/// <summary>
		/// Returns the location of the render target output attachment within the fragment shader.
		/// </summary>
		/// <returns>The location of the render target output attachment within the fragment shader</returns>
		virtual const UInt32& location() const noexcept = 0;

		/// <summary>
		/// Returns the render target image resource.
		/// </summary>
		/// <remarks>
		/// The render target image is not automatically generated when the render target itself gets generated. Instead, it is either provided by a <see cref="IFrameBuffer" /> or 
		/// a <see cref="ISwapChain" /> by calling the <see cref="reset" /> method.
		/// </remarks>
		/// <returns>The render target image resource.</returns>
		/// <seealso cref="reset" />
		virtual const IImage* image() const noexcept = 0;

		/// <summary>
		/// Returns the type of the render target.
		/// </summary>
		/// <returns>The type of the render target.</returns>
		virtual const RenderTargetType& type() const noexcept = 0;

		/// <summary>
		/// Returns the number of samples of the render target when used for multi-sampling.
		/// </summary>
		/// <returns>The number of samples of the render target.</returns>
		virtual const MultiSamplingLevel& samples() const noexcept = 0;

		/// <summary>
		/// Returns the internal format of the render target.
		/// </summary>
		/// <returns>The internal format of the render target.</returns>
		virtual const Format& format() const noexcept = 0;

		/// <summary>
		/// Returns <c>true</c>, if the render target should be cleared, when the render pass is started. If the <see cref="format" /> is set to a depth format, this clears the
		/// depth buffer. Otherwise it clears the color buffer.
		/// </summary>
		/// <returns><c>true</c>, if the render target should be cleared, when the render pass is started</returns>
		/// <seealso cref="clearStencil" />
		/// <seealso cref="clearValues" />
		virtual const bool& clearBuffer() const noexcept = 0;

		/// <summary>
		/// Returns <c>true</c>, if the render target stencil should be cleared, when the render pass is started. If the <see cref="format" /> is does not contain a stencil channel,
		/// this has no effect.
		/// </summary>
		/// <returns><c>true</c>, if the render target stencil should be cleared, when the render pass is started</returns>
		/// <seealso cref="clearStencil" />
		/// <seealso cref="clearValues" />
		virtual const bool& clearStencil() const noexcept = 0;

		/// <summary>
		/// Returns the value, the render target is cleared with, if <see cref="clearBuffer" /> either or <see cref="clearStencil" /> is specified.
		/// </summary>
		/// <remarks>
		/// If the <see cref="format" /> is a color format and <see cref="clearBuffer" /> is specified, this contains the clear color. However, if the format is a depth/stencil 
		/// format, the R and G channels contain the depth and stencil value to clear the buffer with. Note that the stencil buffer is only cleared, if <see cref="clearStencil" />
		/// is specified and vice versa.
		/// </remarks>
		/// <returns>The value, the render target is cleared with, if <see cref="clearBuffer" /> either or <see cref="clearStencil" /> is specified.</returns>
		virtual const Vector4f& clearValues() const noexcept = 0;

		/// <summary>
		/// Returns <c>true</c>, if the target should not be made persistent for access after the render pass has finished.
		/// </summary>
		/// <remarks>
		/// A render target can be marked as volatile if it does not need to be accessed after the render pass has finished. This can be used to optimize away unnecessary GPU/CPU 
		/// memory round-trips. For example a depth buffer may only be used as an input for the lighting stage of a deferred renderer, but is not required after this. So instead
		/// of reading it from the GPU after the lighting pass has finished and then discarding it anyway, it can be marked as volatile in order to prevent it from being read from
		/// the GPU memory again in the first place.
		/// </remarks>
		/// <returns><c>true</c>, if the target should not be made persistent for access after the render pass has finished.</returns>
		virtual const bool& isVolatile() const noexcept = 0;

	public:
		/// <summary>
		/// Resets the render targets image resource.
		/// </summary>
		/// <param name="image">The image resource to use.</param>
		/// <seealso cref="image" />
		virtual void reset(UniquePtr<IImage>&& image) = 0;
	};

	/// <summary>
	/// Implements a render target.
	/// </summary>
	/// <see cref="IRenderTarget" />
	class LITEFX_RENDERING_API RenderTarget : public IRenderTarget {
		LITEFX_IMPLEMENTATION(RenderTargetImpl);

	public:
		/// <summary>
		/// Initializes the render target.
		/// </summary>
		/// <param name="location">The location of the render target ouput attachment.</param>
		/// <param name="type">The type of the render target.</param>
		/// <param name="format">The format of the render target.</param>
		/// <param name="clearBuffer"><c>true</c>, if the render target should be cleared, when a render pass is started.</param>
		/// <param name="clearValues">The values with which the render target gets cleared.</param>
		/// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared, when a render pass is started.</param>
		/// <param name="samples">The number of samples of the render target when used with multi-sampling.</param>
		/// <param name="isVolatile"><c>true</c>, if the target should not be made persistent for access after the render pass has finished.</param>
		explicit RenderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const bool& clearBuffer, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const bool& clearStencil = true, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& isVolatile = false);
		RenderTarget(const RenderTarget&) noexcept;
		RenderTarget(RenderTarget&&) noexcept;
		virtual ~RenderTarget() noexcept;

	public:
		/// <inheritdoc />
		virtual const UInt32& location() const noexcept override;

		/// <inheritdoc />
		virtual const IImage* image() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTargetType& type() const noexcept override;

		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

		/// <inheritdoc />
		virtual const Format& format() const noexcept override;

		/// <inheritdoc />
		virtual const bool& clearBuffer() const noexcept override;

		/// <inheritdoc />
		virtual const bool& clearStencil() const noexcept override;

		/// <inheritdoc />
		virtual const Vector4f& clearValues() const noexcept override;

		/// <inheritdoc />
		virtual const bool& isVolatile() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void reset(UniquePtr<IImage>&& image) override;
	};

	/// <summary>
	/// Represents a command buffer, that buffers commands that should be submitted to a <see cref="ICommandQueue" />.
	/// </summary>
	class LITEFX_RENDERING_API ICommandBuffer {
	public:
		virtual ~ICommandBuffer() noexcept = default;

	public:
		/// <summary>
		/// Waits for the command buffer to be executed.
		/// </summary>
		/// <remarks>
		/// If the command buffer gets submitted, it does not necessarily get executed straight away. If you depend on a command buffer to be finished, you can call this method.
		/// </remarks>
		virtual void wait() const = 0;

		/// <summary>
		/// Sets the command buffer into recording state, so that it can receive command that should be submitted to the parent <see cref="ICommandQueue" />.
		/// </summary>
		/// <remarks>
		/// Note that, if a command buffer has been submitted before, this method waits for the earlier commands to be executed by calling <see cref="wait" />.
		/// </remarks>
		virtual void begin() const = 0;

		/// <summary>
		/// Ends recording commands on the command buffer.
		/// </summary>
		/// <param name="submit">If set to <c>true</c>, the command buffer is automatically submitted by calling the <see cref="submit" /> method.</param>
		/// <param name="wait">If <paramref name="submit" /> is set to <c>true</c>, this parameter gets passed to the <see cref="submit" /> method.</param>
		/// <seealso cref="submit" />
		virtual void end(const bool& submit = true, const bool& wait = false) const = 0;

		/// <summary>
		/// Submits the command buffer to the parent command queue.
		/// </summary>
		/// <remarks>
		/// </remarks>
		/// <param name="wait">If set to <c>true</c>, the command buffer blocks, until the submitted commands have been executed.</param>
		/// <seealso cref="wait" />
		virtual void submit(const bool& wait = false) const = 0;
	};


	// TODO: Remove me.
	class LITEFX_RENDERING_API IRequiresInitialization {
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		virtual bool isInitialized() const noexcept = 0;
	};

	//template <typename TOwner, typename TDevice>
	//class LITEFX_RENDERING_API IDeviceContext {
	//private:
	//	const TOwner& m_parent;
	//	const TDevice& m_device;

	//public:
	//	explicit VulkanRuntimeObject(const TParent& parent, const TDevice& device) :
	//		m_parent(parent), m_device(device)
	//	{
	//		if (device == nullptr)
	//			throw RuntimeException("The device must be initialized.");
	//	}

	//	VulkanRuntimeObject(VulkanRuntimeObject&&) = delete;
	//	VulkanRuntimeObject(const VulkanRuntimeObject&) = delete;
	//	virtual ~VulkanRuntimeObject() noexcept = default;

	//public:
	//	virtual const TParent& parent() const noexcept { return m_parent; }
	//	virtual const VulkanDevice* getDevice() const noexcept { return m_device; };
	//};

}