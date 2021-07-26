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

    /// <summary>
    /// Represents the type of a <see cref="ICommandQueue" />.
    /// </summary>
    /// <remarks>
    /// There are three major queue types: `Graphics`, `Compute` and `Transfer`. Each queue type has a larger subset of commands it is allowed to execute. For example, a 
    /// graphics queue can be used to execute transfer or compute commands, whilst a compute queue may not execute graphics commands (such as *draw*). 
    /// 
    /// You should always aim to use a queue that is dedicated for the workload you want to submit. For example, if you want to upload resources to the GPU, use the a
    /// dedicated transfer queue and synchronize access to the resource by waiting for the queue to finish at the point of time you need to access the resource. You can,
    /// however, also use the graphics queue for transfers. This can be more efficient, if you have resources that require to be updated with each frame. The performance 
    /// impact of synchronizing two queues may be larger than simply using the graphics queue to begin with.
    /// 
    /// The advantage of using dedicated queues is, that they do not necessarily block execution. For example, when performing a compute or transfer workload on a 
    /// graphics queue, you do not need to synchronize in order to wait for the result, however this also means that no rendering can take place until the workloads have
    /// finished.
    /// </remarks>
    enum class LITEFX_RENDERING_API QueueType {
        /// <summary>
        /// Describes an unspecified command queue. It is not valid to create a queue instance with this type.
        /// </summary>
        None = 0x00000000,

        /// <summary>
        /// Represents a queue that can execute graphics, compute and transfer workloads.
        /// </summary>
        Graphics = 0x00000001,

        /// <summary>
        /// Represents a queue that can execute compute and transfer workloads.
        /// </summary>
        Compute = 0x00000002,

        /// <summary>
        /// Represents a queue that can execute only transfer workloads.
        /// </summary>
        Transfer = 0x00000004,

        /// <summary>
        /// Represents an invalid queue type.
        /// </summary>
        Other = 0x7FFFFFFF
    };

    /// <summary>
    /// Specifies the priority with which a queue is scheduled on the GPU.
    /// </summary>
    enum class LITEFX_RENDERING_API QueuePriority {
        /// <summary>
        /// The default queue priority.
        /// </summary>
        Normal = 33,

        /// <summary>
        /// A high queue priority.
        /// </summary>
        High = 66,

        /// <summary>
        /// The highest possible queue priority. Submitting work to this queue might block other queues.
        /// </summary>
        Realtime = 100
    };

    /// <summary>
    /// Describes a texel format.
    /// </summary>
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

    /// <summary>
    /// Describes a buffer attribute format.
    /// </summary>
    /// <seealso cref="getBufferFormatChannels" />
    /// <seealso cref="getBufferFormatChannelSize" />
    /// <seealso cref="getBufferFormatType" />
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

    /// <summary>
    /// Describes the semantic of a buffer attribute.
    /// </summary>
    /// <seealso cref="BufferAttribute" />
    enum class LITEFX_RENDERING_API AttributeSemantic {
        /// <summary>
        /// The attribute contains a bi-normal vector.
        /// </summary>
        Binormal = 0x00000001,

        /// <summary>
        /// The attribute contains blend indices.
        /// </summary>
        BlendIndices = 0x00000002,

        /// <summary>
        /// The attribute contains blend weights.
        /// </summary>
        BlendWeight = 0x00000003,

        /// <summary>
        /// The attribute contains a color value.
        /// </summary>
        Color = 0x00000004,

        /// <summary>
        /// The attribute contains a normal vector.
        /// </summary>
        Normal = 0x00000005,

        /// <summary>
        /// The attribute contains a position vector.
        /// </summary>
        Position = 0x00000006,

        /// <summary>
        /// The attribute contains a pre-transformed position vector.
        /// </summary>
        TransformedPosition = 0x00000007,

        /// <summary>
        /// The attribute contains a point size.
        /// </summary>
        PointSize = 0x00000008,

        /// <summary>
        /// The attribute contains a tangent vector.
        /// </summary>
        Tangent = 0x00000009,

        /// <summary>
        /// The attribute contains a texture coordinate.
        /// </summary>
        TextureCoordinate = 0x0000000A,

        /// <summary>
        /// The attribute is a generic, unknown semantic.
        /// </summary>
        Unknown = 0x7FFFFFFF
    };

    /// <summary>
    /// Describes the type of a <see cref="IDescriptor" />.
    /// </summary>
    /// <remarks>
    /// Note that, while in theory you can declare a writable descriptor in any format, the rendering back-end might not necessarily support writing in a specific format.
    /// </remarks>
    /// <seealso cref="IDescriptorLayout" />
    /// <seealso href="https://docs.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads#supported-formats-and-api-calls" />
    enum class LITEFX_RENDERING_API DescriptorType {
        /// <summary>
        /// A uniform buffer in Vulkan. Maps to a constant buffer in DirectX.
        /// </summary>
        /// <remarks>
        /// A uniform or constant buffer is read-only. In GLSL, use the <c>uniform</c> keyword to access a uniform buffer. In HLSL, use the <c>ConstantBuffer</c> keyword.
        /// </remarks>
        Uniform         = 0x00000001,

        /// <summary>
        /// A shader storage buffer object in Vulkan. Maps to a structured buffer in DirectX.
        /// </summary>
        /// <remarks>
        /// A storage buffer is read-only by default. If you want to create a writable storage buffer, use <see cref="WritableStorage"> instead.
        /// 
        /// In GLSL, use the <c>buffer</c> keyword to access storage buffers. In HLSL, use the <c>StructuredBuffer</c> keyword.
        /// 
        /// The difference between <see cref="Uniform" /> and storage buffers is, that storage buffers can have variable length. However, they are typically less efficient.
        /// </remarks>
        Storage         = 0x00000002,

        /// <summary>
        /// A writable shader storage object in Vulkan. Maps to a read/write structured buffer in DirectX.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>buffer</c> keyword to access storage buffers. In HLSL, use the <c>RWStructuredBuffer</c> keyword.
        /// </remarks>
        WritableStorage = 0x00000012,

        /// <summary>
        /// A read-only sampled image.
        /// </summary>
        /// <remarks>
        /// Textures are read-only by default. If you want to create a writable texture, use the <see cref="WritableTexture"> instead.
        /// 
        /// In GLSL, use the <c>uniform texture</c> keywords to access the texture. In HLSL, use the <c>Texture</c> keywords.
        /// 
        /// Note, that textures are typically not be accessed directly, but instead are sampled using a <see cref="Sampler" />.
        /// </remarks>
        Texture         = 0x00000003,

        /// <summary>
        /// A writable image.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>uniform image</c> keywords to access the texture. In HLSL, use the <c>RWTexture</c> keywords.
        /// </remaks>
        WritableTexture = 0x00000013,
        
        /// <summary>
        /// A sampler state of a texture or image.
        /// </summary>
        Sampler         = 0x00000004,

        /// <summary>
        /// The result of a render target from an earlier render pass. Maps to a <c>SubpassInput</c> in HLSL.
        /// </summary>
        InputAttachment = 0x00000005,

        /// <summary>
        /// Represents a read-only texel buffer.
        /// </summary>
        /// <remarks>
        /// Use the <c>uniform samplerBuffer</c> keyword in GLSL to access the buffer. In HLSL, use the <c>Buffer</c> keyword.
        /// </remarks>
        Buffer          = 0x00000006,

        /// <summary>
        /// Represents a writable texel buffer.
        /// </summary>
        /// <remarks>
        /// Use the <c>uniform imageBuffer</c> keyword in GLSL to access the buffer. In HLSL, use the <c>RWBuffer</c> keyword.
        /// </remarks>
        WritableBuffer  = 0x00000016
    };

    /// <summary>
    /// Describes the type of a <see cref="IBuffer" />.
    /// </summary>
    /// <seealso cref="IBufferLayout" />
    enum class LITEFX_RENDERING_API BufferType {
        /// <summary>
        /// Describes a vertex buffer.
        /// </summary>
        Vertex = 0x00000001,
        
        /// <summary>
        /// Describes an index buffer.
        /// </summary>
        Index = 0x00000002,

        /// <summary>
        /// Describes an uniform buffer object (Vulkan) or constant buffer view (DirectX).
        /// </summary>
        Uniform = 0x00000003,

        /// <summary>
        /// Describes a shader storage buffer object (Vulkan) or unordered access view (DirectX).
        /// </summary>
        Storage = 0x00000004,

        /// <summary>
        /// Describes a shader texel storage buffer object (Vulkan) or unordered access view (DirectX).
        /// </summary>
        Texel = 0x00000005,

        /// <summary>
        /// Describes another type of buffer, such as samplers or images.
        /// </summary>
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

    /// <summary>
    /// Describes the element type of an index buffer.
    /// </summary>
    enum class LITEFX_RENDERING_API IndexType {
        /// <summary>
        /// Indices are stored as 2 byte unsigned integers.
        /// </summary>
        UInt16 = 0x00000010,

        /// <summary>
        /// Indices are stored as 4 byte unsigned integers.
        /// </summary>
        UInt32 = 0x00000020
    };

    /// <summary>
    /// Describes the valid shader stages of a graphics pipeline.
    /// </summary>
    enum class LITEFX_RENDERING_API ShaderStage {
        /// <summary>
        /// Represents the vertex shader stage.
        /// </summary>
        Vertex = 0x00000001,

        /// <summary>
        /// Represents the tessellation control or hull shader stage.
        /// </summary>
        TessellationControl = 0x00000002,

        /// <summary>
        /// Represents the tessellation evaluation or domain shader stage.
        /// </summary>
        TessellationEvaluation = 0x00000004,

        /// <summary>
        /// Represents the  geometry shader stage.
        /// </summary>
        /// <remarks>
        /// Note that geometry shaders come with a performance penalty and might not be supported on all platforms. If you can, avoid using them.
        /// </remarks>
        Geometry = 0x00000008,

        /// <summary>
        /// Represents the fragment or vertex shader stage.
        /// </summary>
        Fragment = 0x00000010,

        /// <summary>
        /// Represents the compute shader stage.
        /// </summary>
        Compute = 0x00000020,

        /// <summary>
        /// Represents an unknown shader stage.
        /// </summary>
        Other = 0x7FFFFFFF
    };

    /// <summary>
    /// Describes the draw mode for polygons.
    /// </summary>
    /// <seealso cref="InputAssembler" />
    enum class LITEFX_RENDERING_API PolygonMode {
        /// <summary>
        /// Polygons are drawn as solid surfaces.
        /// </summary>
        Solid = 0x00000001,

        /// <summary>
        /// Polygons are only drawn as wireframes.
        /// </summary>
        Wireframe = 0x00000002,

        /// <summary>
        /// Polygons are drawn as points at the vertex positions.
        /// </summary>
        Point = 0x00000004
    };

    /// <summary>
    /// Describes which faces are culled by the <see cref="Rasterizer" /> stage.
    /// </summary>
    /// <seealso cref="Rasterizer" />
    enum class LITEFX_RENDERING_API CullMode {
        /// <summary>
        /// The rasterizer will discard front-facing polygons.
        /// </summary>
        FrontFaces = 0x00000001,

        /// <summary>
        /// The rasterizer wll discard back-facing polygons.
        /// </summary>
        BackFaces = 0x00000002,

        /// <summary>
        /// The rasterizer will discard front and back-facing polygons.
        /// </summary>
        Both = 0x00000004,

        /// <summary>
        /// The rasterizer will not discard any polygons.
        /// </summary>
        Disabled = 0x0000000F
    };

    /// <summary>
    /// Describes the order or vertex winding, that is used to determine, whether a polygon is facing towards or away from the camera.
    /// </summary>
    /// <seealso cref="CullMode" />
    /// <seealso cref="Rasterizer" />
    enum class LITEFX_RENDERING_API CullOrder {
        /// <summary>
        /// Vertices are evaluated in a clock-wise manner.
        /// </summary>
        ClockWise = 0x00000001,

        /// <summary>
        /// Vertices are evaluated in a counter clock-wise manner.
        /// </summary>
        CounterClockWise = 0x00000002
    };

    /// <summary>
    /// Describes the type of a render target.
    /// </summary>
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

    /// <summary>
    /// Describes the dimensions of a image resource, i.e. the dimensions that are required to access a texel or describe the image extent.
    /// </summary>
    /// <seealso cref="IImage" />
    enum class LITEFX_RENDERING_API ImageDimensions {
        /// <summary>
        /// Represents a 1D image.
        /// </summary>
        DIM_1 = 0x01,

        /// <summary>
        /// Represents a 2D image.
        /// </summary>
        DIM_2 = 0x02,

        /// <summary>
        /// Represents a 3D image.
        /// </summary>
        DIM_3 = 0x03,

        /// <summary>
        /// Represents a set of six 2D images that are used to build a cube map.
        /// </summary>
        CUBE = 0x04
    };

    /// <summary>
    /// Describes the number of samples with which a <see cref="ITexture" /> is sampled.
    /// </summary>
    enum class LITEFX_RENDERING_API MultiSamplingLevel {
        /// <summary>
        /// The default number of samples. Multi-sampling will be de-activated, if this sampling level is used.
        /// </summary>
        x1 = 0x00000001,

        /// <summary>
        /// Use 2 samples per pixel.
        /// </summary>
        x2 = 0x00000002,

        /// <summary>
        /// Use 4 samples per pixel.
        /// </summary>
        x4 = 0x00000004,

        /// <summary>
        /// Use 8 samples per pixel.
        /// </summary>
        x8 = 0x00000008,

        /// <summary>
        /// Use 16 samples per pixel.
        /// </summary>
        x16 = 0x00000010,

        /// <summary>
        /// Use 32 samples per pixel.
        /// </summary>
        x32 = 0x00000020,

        /// <summary>
        /// Use 64 samples per pixel.
        /// </summary>
        x64 = 0x00000040
    };

    /// <summary>
    /// Describes the filter operation when accessing a pixel from a texture coordinate.
    /// </summary>
    /// <seealso cref="IImage" />
    enum class LITEFX_RENDERING_API FilterMode {
        /// <summary>
        /// Take the nearest texel with respect to the texture coordinate.
        /// </summary>
        Nearest = 0x00000001,

        /// <summary>
        /// Linearly interpolate between the two closest texels with respect to the texture coordinate.
        /// </summary>
        Linear = 0x00000002
    };

    /// <summary>
    /// Describes the filter operation between two mip-map levels.
    /// </summary>
    /// <seealso cref="IImage" />
    /// <seealso cref="FilterMode" />
    enum class LITEFX_RENDERING_API MipMapMode {
        /// <summary>
        /// Take the texel from the mip-map level that is closest to the actual depth.
        /// </summary>
        Nearest = 0x00000001,

        /// <summary>
        /// Linearly interpolate between the texels of the two neighboring mip-map levels.
        /// </summary>
        Linear = 0x00000002
    };

    /// <summary>
    /// Describes how to treat texture coordinates that are outside the domain `[0..1]`.
    /// </summary>
    enum class LITEFX_RENDERING_API BorderMode {
        /// <summary>
        /// Repeat the texture.
        /// </summary>
        Repeat = 0x00000001,

        /// <summary>
        /// Mirror the texture.
        /// </summary>
        RepeatMirrored = 0x00010001,

        /// <summary>
        /// Take the closest edge texel.
        /// </summary>
        ClampToEdge = 0x00000002,

        /// <summary>
        /// Take the closest edge texel from the opposite site.
        /// </summary>
        ClampToEdgeMirrored = 0x00010002,

        /// <summary>
        /// Return a pre-specified border color.
        /// </summary>
        ClampToBorder = 0x00000003,
    };

    /// <summary>
    /// Describes the operation used to compare depth or stencil values during depth/stencil tests.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    enum class LITEFX_RENDERING_API CompareOperation {
        /// <summary>
        /// The test always fails.
        /// </summary>
        Never = 0x00000000,

        /// <summary>
        /// The test succeeds, if the current value is less than the stencil ref or previous depth value.
        /// </summary>
        Less = 0x00000001,

        /// <summary>
        /// The test succeeds, if the current value is greater than the stencil ref or previous depth value.
        /// </summary>
        Greater = 0x0000002,

        /// <summary>
        /// The test succeeds, if the current value is equal to the stencil ref or previous depth value.
        /// </summary>
        Equal = 0x00000003,

        /// <summary>
        /// The test succeeds, if the current value is less or equal to the stencil ref or previous depth value.
        /// </summary>
        LessEqual = 0x00000004,

        /// <summary>
        /// The test succeeds, if the current value is greater or euql to the stencil ref or previous depth value.
        /// </summary>
        GreaterEqual = 0x00000005,

        /// <summary>
        /// The test succeeds, if the current value is not equal to the stencil ref or previous depth value.
        /// </summary>
        NotEqual = 0x00000006,

        /// <summary>
        /// The test always succeeds.
        /// </summary>
        Always = 0x00000007
    };

    /// <summary>
    /// An operation that is applied to the stencil buffer.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    enum class LITEFX_RENDERING_API StencilOperation {
        /// <summary>
        /// Keep the current stencil value.
        /// </summary>
        Keep = 0x00000000,
        
        /// <summary>
        /// Set the stencil value to `0`.
        /// </summary>
        Zero = 0x00000001,

        /// <summary>
        /// Replace the current stencil value with the stencil ref.
        /// </summary>
        Replace = 0x00000002,

        /// <summary>
        /// Increment the current stencil value.
        /// </summary>
        IncrementClamp = 0x00000003,

        /// <summary>
        /// Decrement the current stencil value.
        /// </summary>
        DecrementClamp = 0x00000004,

        /// <summary>
        /// Bitwise invert the current stencil value.
        /// </summary>
        Invert = 0x00000005,

        /// <summary>
        /// Increment the current stencil value and wrap it, if it goes out of bounds.
        /// </summary>
        IncrementWrap = 0x00000006,

        /// <summary>
        /// Decrement the current stencil value and wrap it, if it goes out of bounds.
        /// </summary>
        DecrementWrap = 0x00000007
    };

    /// <summary>
    /// Specifies a blend factor.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    enum class LITEFX_RENDERING_API BlendFactor {
        Zero = 0,
        One = 1,
        SourceColor = 2,
        OneMinusSourceColor = 3,
        DestinationColor = 4,
        OneMinusDestinationColor = 5,
        SourceAlpha = 6,
        OneMinusSourceAlpha = 7,
        DestinationAlpha = 8,
        OneMinusDestinationAlpha = 9,
        ConstantColor = 10,
        OneMinusConstantColor = 11,
        ConstantAlpha = 12,
        OneMinusConstantAlpha = 13,
        SourceAlphaSaturate = 14,
        Source1Color = 15,
        OneMinusSource1Color = 16,
        Source1Alpha = 17,
        OneMinusSource1Alpha = 18
    };

    /// <summary>
    /// Specifies a write mask for a color buffer.
    /// </summary>
    /// <seealso cref="RenderTargetType" />
    enum class LITEFX_RENDERING_API WriteMask {
        /// <summary>
        /// Write into the red channel.
        /// </summary>
        R = 0x01,

        /// <summary>
        /// Write into the green channel.
        /// </summary>
        G = 0x02,

        /// <summary>
        /// Write into the blue channel.
        /// </summary>
        B = 0x04,

        /// <summary>
        /// Write into the alpha channel.
        /// </summary>
        A = 0x08
    };

    /// <summary>
    /// Specifies a blend operation.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    enum class LITEFX_RENDERING_API BlendOperation {
        Add = 0x01,
        Subtract = 0x02,
        ReverseSubtract = 0x03,
        Minimum = 0x04,
        Maximum = 0x05
    };

    /// <summary>
    /// Specifies the state of a resource.
    /// </summary>
    /// <seealso cref="IDeviceMemory" />
    enum class LITEFX_RENDERING_API ResourceState {
        /// <summary>
        /// The state of the resource is undefined or does not matter.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_COMMON`</term>
        ///         <term>`VK_ACCESS_NONE_KHR`</term>
        ///         <term>`VK_IMAGE_LAYOUT_UNDEFINED`</term>
        ///     </item>
        /// </list>
        /// </remarks>
        Common = 0x00000001,

        /// <summary>
        /// The resource is used as a read-only vertex buffer.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER`</term>
        ///         <term>`VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT`</term>
        ///         <term>–</term>
        ///     </item>
        /// </list>
        /// </remarks>
        VertexBuffer = 0x00000002,

        /// <summary>
        /// The resource is used as a read-only index buffer.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_INDEX_BUFFER`</term>
        ///         <term>`VK_ACCESS_INDEX_READ_BIT`</term>
        ///         <term>–</term>
        ///     </item>
        /// </list>
        /// </remarks>
        IndexBuffer = 0x0000003,

        /// <summary>
        /// The resource is used as a read-only uniform or constant buffer.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER`</term>
        ///         <term>`VK_ACCESS_UNIFORM_READ_BIT`</term>
        ///         <term>–</term>
        ///     </item>
        /// </list>
        /// </remarks>
        UniformBuffer = 0x00000004,

        /// <summary>
        /// The resource is used as a read-only storage or texel buffer.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE`</term>
        ///         <term>`VK_ACCESS_SHADER_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// </remarks>
        ReadOnly = 0x00000005,

        /// <summary>
        /// The resource is used as a read-write storage or texel buffer.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_UNORDERED_ACCESS`</term>
        ///         <term>`VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_GENERAL`</term>
        ///     </item>
        /// </list>
        /// </remarks>
        ReadWrite = 0x00000006,

        /// <summary>
        /// The resource is used as a copy source.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_COPY_SOURCE`</term>
        ///         <term>`VK_ACCESS_TRANSFER_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// </remarks>
        CopySource = 0x00000010,

        /// <summary>
        /// The resource is used as a copy destination.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_COPY_DEST`</term>
        ///         <term>`VK_ACCESS_TRANSFER_WRITE_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// </remarks>
        CopyDestination = 0x00000011,

        /// <summary>
        /// The resource is used as a render target.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_RENDER_TARGET`</term>
        ///         <term>`VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        RenderTarget = 0x00000020,

        /// <summary>
        /// The resource is used as a read-only depth/stencil target.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_DEPTH_READ`</term>
        ///         <term>`VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        DepthRead = 0x00000021,

        /// <summary>
        /// The resource is used as a write-only depth/stencil target.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_DEPTH_WRITE`</term>
        ///         <term>`VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        DepthWrite = 0x00000022,

        /// <summary>
        /// The resource is presented on a swap chain.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_PRESENT`</term>
        ///         <term>`VK_ACCESS_MEMORY_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        Present = 0x00000023,

        /// <summary>
        /// The resource is a multi-sampled image that will be resolved into a present target.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_RESOLVE_SOURCE`</term>
        ///         <term>`VK_ACCESS_MEMORY_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        ResolveSource = 0x00000024,

        /// <summary>
        /// The resource is resolved from a multi-sampled image.
        /// </summary>
        /// <remarks>
        /// The following table contains the API-specific flags for each supported back-end.
        /// 
        /// <list type="table">
        ///     <listheader>
        ///         <term>DirectX 12 ❎</term>
        ///         <term>Vulkan 🌋 (`VkAccessFlags`)</term>
        ///         <term>Vulkan 🌋 (`VkImageLayout`) </term>
        ///     </listheader>
        ///     <item>
        ///         <term>`D3D12_RESOURCE_STATE_RESOLVE_DEST`</term>
        ///         <term>`VK_ACCESS_MEMORY_WRITE_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`</term>
        ///     </item>
        /// </list>
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="IRenderPass" />es.
        /// </remarks>
        ResolveDestination = 0x00000025,

        /// <summary>
        /// The state of the resource is not known by the engine.
        /// </summary>
        /// <remarks>
        /// A resource with an unknown state is not invalid. However, it is not valid to transition a resource out of or into this state. If a resource ends up
        /// in this state, the state of the <see cref="IDeviceMemory" /> should be manually set.
        /// </remarks>
        Undefined = 0x7FFFFFFF
    };

    // Define flags.
    LITEFX_DEFINE_FLAGS(QueueType);
    LITEFX_DEFINE_FLAGS(ShaderStage);
    LITEFX_DEFINE_FLAGS(BufferFormat);
    LITEFX_DEFINE_FLAGS(WriteMask);

    // Helper functions.

    /// <summary>
    /// Returns the number of channels for a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    inline UInt32 getBufferFormatChannels(const BufferFormat& format) {
        return static_cast<UInt32>(format) & 0x000000FF;
    }

    /// <summary>
    /// Returns the number of bytes used by a channel of a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    inline UInt32 getBufferFormatChannelSize(const BufferFormat& format) {
        return (static_cast<UInt32>(format) & 0xFF000000) >> 24;
    }

    /// <summary>
    /// Returns the underlying data type of a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    inline UInt32 getBufferFormatType(const BufferFormat& format) {
        return (static_cast<UInt32>(format) & 0x0000FF00) >> 8;
    }

    /// <summary>
    /// Returns the size of an element of a specified format.
    /// </summary>
    size_t LITEFX_RENDERING_API getSize(const Format& format);

    /// <summary>
    /// Returns <c>true</c>, if the format contains a depth channel.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    bool LITEFX_RENDERING_API hasDepth(const Format& format);

    /// <summary>
    /// Returns <c>true</c>, if the format contains a stencil channel.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    bool LITEFX_RENDERING_API hasStencil(const Format& format);

    /// <summary>
    /// Represents a physical graphics adapter.
    /// </summary>
    /// <remarks>
    /// A graphics adapter can be seen as an actual physical device that can run graphics computations. Typically this resembles a GPU that is connected
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
    /// Represents a single shader module, i.e. a part of a <see cref="IShaderProgram" />.
    /// </summary>
    /// <remarks>
    /// A shader module corresponds to a single shader source file.
    /// </remarks>
    class LITEFX_RENDERING_API IShaderModule {
    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the shader module.
        /// </summary>
        /// <returns>The type of the shader module.</returns>
        virtual const ShaderStage& type() const noexcept = 0;

        /// <summary>
        /// Returns the file name of the shader module.
        /// </summary>
        /// <returns>The file name of the shader module.</returns>
        virtual const String& fileName() const noexcept = 0;

        /// <summary>
        /// Returns the name of the shader module entry point.
        /// </summary>
        /// <returns>The name of the shader module entry point.</returns>
        virtual const String& entryPoint() const noexcept = 0;
    };

    /// <summary>
    /// Represents a render target, i.e. an abstract view of the output of an <see cref="IRenderPass" />.
    /// </remarks>
    /// <remarks>
    /// A render target represents one output of a render pass, stored within an <see cref="IImage" />. It is contained by a <see cref="IRenderPass" />, that contains 
    /// the <see cref="IFrameBuffer" />, that stores the actual render target image resource.
    /// </remarks>
    /// <seealso cref="RenderTarget" />
    /// <seealso cref="IRenderPass" />
    /// <seealso cref="IFrameBuffer" />
    /// <seealso cref="IImage" />
    class LITEFX_RENDERING_API IRenderTarget {
    public:
        /// <summary>
        /// Describes the blend state of the render target.
        /// </summary>
        struct BlendState {
        public:
            /// <summary>
            /// Specifies, if the target should be blended (default: <c>false</c>).
            /// </summary>
            bool Enable{ false };

            /// <summary>
            /// The blend factor for the source color channels (default: <c>BlendFactor::One</c>).
            /// </summary>
            BlendFactor SourceColor{ BlendFactor::One };

            /// <summary>
            /// The blend factor for the destination color channels (default: <c>BlendFactor::Zero</c>).
            /// </summary>
            BlendFactor DestinationColor{ BlendFactor::Zero };

            /// <summary>
            /// The blend factor for the source alpha channel (default: <c>BlendFactor::One</c>).
            /// </summary>
            BlendFactor SourceAlpha{ BlendFactor::One };

            /// <summary>
            /// The blend factor for the destination alpha channels (default: <c>BlendFactor::Zero</c>).
            /// </summary>
            BlendFactor DestinationAlpha{ BlendFactor::Zero };

            /// <summary>
            /// The blend operation for the color channels (default: <c>BlendOperation::Add</c>).
            /// </summary>
            BlendOperation ColorOperation{ BlendOperation::Add };

            /// <summary>
            /// The blend operation for the alpha channel (default: <c>BlendOperation::Add</c>).
            /// </summary>
            BlendOperation AlphaOperation{ BlendOperation::Add };

            /// <summary>
            /// The channel write mask, determining which channels are written to (default: <c>WriteMask::R | WriteMask::G | WriteMask::B | WriteMask::A</c>).
            /// </summary>
            WriteMask WriteMask{ WriteMask::R | WriteMask::G | WriteMask::B | WriteMask::A };
        };

    public:
        virtual ~IRenderTarget() noexcept = default;

    public:
        /// <summary>
        /// Returns the location of the render target output attachment within the fragment shader.
        /// </summary>
        /// <remarks>
        /// The locations of all render targets of a frame buffer must be within a continuous domain, starting at <c>0</c>. A frame buffer validates the render target locations
        /// when it is initialized and will raise an exception, if a location is either not mapped or assigned multiple times.
        /// </remarks>
        /// <returns>The location of the render target output attachment within the fragment shader</returns>
        virtual const UInt32& location() const noexcept = 0;

        /// <summary>
        /// Returns the type of the render target.
        /// </summary>
        /// <returns>The type of the render target.</returns>
        virtual const RenderTargetType& type() const noexcept = 0;

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

        /// <summary>
        /// Returns the render targets blend state.
        /// </summary>
        /// <returns>The render targets blend state.</returns>
        virtual const BlendState& blendState() const noexcept = 0;
    };

    /// <summary>
    /// Implements a render target.
    /// </summary>
    /// <see cref="IRenderTarget" />
    class LITEFX_RENDERING_API RenderTarget : public IRenderTarget {
        LITEFX_IMPLEMENTATION(RenderTargetImpl);

    public:
        RenderTarget() noexcept;

        /// <summary>
        /// Initializes the render target.
        /// </summary>
        /// <param name="location">The location of the render target output attachment.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The format of the render target.</param>
        /// <param name="clearBuffer"><c>true</c>, if the render target should be cleared, when a render pass is started.</param>
        /// <param name="clearValues">The values with which the render target gets cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared, when a render pass is started.</param>
        /// <param name="isVolatile"><c>true</c>, if the target should not be made persistent for access after the render pass has finished.</param>
        /// <param name="blendState">The render target blend state.</param>
        explicit RenderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const bool& clearBuffer, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const bool& clearStencil = true, const bool& isVolatile = false, const BlendState& blendState = {});
        RenderTarget(const RenderTarget&) noexcept;
        RenderTarget(RenderTarget&&) noexcept;
        virtual ~RenderTarget() noexcept;

    public:
        inline RenderTarget& operator=(const RenderTarget&) noexcept;
        inline RenderTarget& operator=(RenderTarget&&) noexcept;

    public:
        /// <inheritdoc />
        virtual const UInt32& location() const noexcept override;

        /// <inheritdoc />
        virtual const RenderTargetType& type() const noexcept override;

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

        /// <inheritdoc />
        virtual const BlendState& blendState() const noexcept override;
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

    /// <summary>
    /// Stores the depth/stencil state of a see <see cref="IRasterizer" />.
    /// </summary>
    class LITEFX_RENDERING_API DepthStencilState {
        LITEFX_IMPLEMENTATION(DepthStencilStateImpl);

    public:
        /// <summary>
        /// Describes the rasterizer depth state.
        /// </summary>
        struct DepthState {
        public:
            /// <summary>
            /// Specifies, if depth testing should be enabled (default: <c>true</c>).
            /// </summary>
            bool Enable{ true };

            /// <summary>
            /// Specifies, if depth should be written (default: <c>true</c>).
            /// </summary>
            bool Write{ true };

            /// <summary>
            /// The compare operation used to pass the depth test (default: <c>CompareOperation::Always</c>).
            /// </summary>
            CompareOperation Operation{ CompareOperation::Always };
        };

        /// <summary>
        /// Describes the rasterizer depth bias.
        /// </summary>
        /// <remarks>
        /// The depth bias can be used to alter the depth value function, i.e. how the values within the depth buffer are distributed. By default, the depth buffer
        /// uses an exponential function scale to increase precision for closer objects. The values provided with <see cref="depthBiasClamp" />, 
        /// <see cref="depthBiasConstantFactor" /> and <see cref="depthBiasSlopeFactor" /> are used to change the domain clamping, offset and steepness of the depth
        /// value distribution.
        /// </remarks>
        struct DepthBias {
        public:
            /// <summary>
            /// Specifies, if depth bias should be used (default: <c>false</c>).
            /// </summary>
            bool Enable{ false };

            /// <summary>
            /// Specifies the depth bias clamp (default: <c>0.0</c>).
            /// </summary>
            Float Clamp{ 0.f };

            /// <summary>
            /// Specifies the depth bias slope factor (default: <c>0.0</c>).
            /// </summary>
            Float SlopeFactor{ 0.f };

            /// <summary>
            /// Specifies the depth bias constant factor (default: <c>0.0</c>).
            /// </summary>
            Float ConstantFactor{ 0.f };
        };

        /// <summary>
        /// Describes a stencil test for either front or back faces.
        /// </summary>
        struct StencilTest {
        public:
            /// <summary>
            /// The operation to apply to the stencil buffer, if the stencil test fails (default: <c>StencilOperation::Keep</c>).
            /// </summary>
            StencilOperation StencilFailOp{ StencilOperation::Keep };

            /// <summary>
            /// The operation to apply to the stencil buffer, if the stencil test passes (default: <c>StencilOperation::Keep</c>).
            /// </summary>
            StencilOperation StencilPassOp{ StencilOperation::Replace };

            /// <summary>
            /// The operation to apply to the stencil buffer, if the depth test fails (default: <c>StencilOperation::Keep</c>).
            /// </summary>
            StencilOperation DepthFailOp{ StencilOperation::Keep };

            /// <summary>
            /// The operation use for stencil testing (default: <c>CompareOperation::Never</c>).
            /// </summary>
            CompareOperation Operation{ CompareOperation::Never };
        };

        /// <summary>
        /// Describes the rasterizer stencil state.
        /// </summary>
        struct StencilState {
        public:
            /// <summary>
            /// Specifies, if stencil state should be used (default: <c>false</c>).
            /// </summary>
            bool Enable{ false };

            /// <summary>
            /// Specifies the bits to write to the stencil state (default: <c>0xFF</c>).
            /// </summary>
            Byte WriteMask{ 0xFF };

            /// <summary>
            /// Specifies the bits to read from the stencil state (default: <c>0xFF</c>).
            /// </summary>
            Byte ReadMask{ 0xFF };

            /// <summary>
            /// Describes the stencil test for faces that point towards the camera.
            /// </summary>
            StencilTest FrontFace{};

            /// <summary>
            /// Describes the stencil test for faces that point away from the camera.
            /// </summary>
            StencilTest BackFace{};
        };

    public:
        /// <summary>
        /// Initializes a new rasterizer depth/stencil state.
        /// </summary>
        /// <param name="depthState">The depth state of the rasterizer.</param>
        /// <param name="depthBias">The depth bias configuration of the rasterizer.</param>
        /// <param name="stencilState">The stencil state of the rasterizer.</param>
        explicit DepthStencilState(const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) noexcept;

        /// <summary>
        /// Initializes a new rasterizer depth/stencil state.
        /// </summary>
        DepthStencilState() noexcept;

        /// <summary>
        /// Creates a copy of a depth/stencil state.
        /// </summary>
        DepthStencilState(const DepthStencilState&) noexcept;

        /// <summary>
        /// Moves a depth/stencil state.
        /// </summary>
        DepthStencilState(DepthStencilState&&) noexcept;

        /// <summary>
        /// Destroys a depth/stencil state.
        /// </summary>
        virtual ~DepthStencilState() noexcept;

        /// <summary>
        /// Copies a depth/stencil state.
        /// </summary>
        /// <returns>A reference to the current depth/stencil state instance.</returns>
        DepthStencilState& operator=(const DepthStencilState&) noexcept;

        /// <summary>
        /// Moves a depth/stencil state.
        /// </summary>
        /// <returns>A reference to the current depth/stencil state instance.</returns>
        DepthStencilState& operator=(DepthStencilState&&) noexcept;

    public:
        /// <summary>
        /// Returns the depth state.
        /// </summary>
        /// <returns>The depth state.</returns>
        virtual DepthState& depthState() const noexcept;

        /// <summary>
        /// Returns the depth bias.
        /// </summary>
        /// <returns>The depth bias.</returns>
        virtual DepthBias& depthBias() const noexcept;

        /// <summary>
        /// Returns the stencil state.
        /// </summary>
        /// <returns>The stencil state.</returns>
        virtual StencilState& stencilState() const noexcept;
    };

    /// <summary>
    /// Represents the rasterizer state of a <see cref="IRenderPipeline" />.
    /// </summary>
    class LITEFX_RENDERING_API IRasterizer {
    public:
        virtual ~IRasterizer() noexcept = default;

    public:
        /// <summary>
        /// Returns the polygon mode of the rasterizer state.
        /// </summary>
        /// <returns>The polygon mode of the rasterizer state.</returns>
        virtual const PolygonMode& polygonMode() const noexcept = 0;

        /// <summary>
        /// Returns the cull mode of the rasterizer state.
        /// </summary>
        /// <returns>The cull mode of the rasterizer state.</returns>
        virtual const CullMode& cullMode() const noexcept = 0;

        /// <summary>
        /// Returns the cull mode of the rasterizer state.
        /// </summary>
        /// <returns>The cull mode of the rasterizer state.</returns>
        virtual const CullOrder& cullOrder() const noexcept = 0;

        /// <summary>
        /// Returns the line width of the rasterizer state.
        /// </summary>
        /// <remarks>
        /// Note that line width is not supported in DirectX and is only emulated under Vulkan. Instead of forcing this value, it is recommended to 
        /// use a custom shader for it.
        /// </remarks>
        /// <returns>The line width of the rasterizer state.</returns>
        virtual const Float& lineWidth() const noexcept = 0;

        /// <summary>
        /// Returns the depth/stencil state of the rasterizer.
        /// </summary>
        /// <returns>The depth/stencil state of the rasterizer.</returns>
        virtual const DepthStencilState& depthStencilState() const noexcept = 0;
    };

    /// <summary>
    /// Implements a <see cref="IRasterizer" />.
    /// </summary>
    class LITEFX_RENDERING_API Rasterizer : public IRasterizer {
        LITEFX_IMPLEMENTATION(RasterizerImpl);

    public:
        /// <summary>
        /// Initializes a new rasterizer instance.
        /// </summary>
        /// <param name="polygonMode">The polygon mode of the rasterizer state.</param>
        /// <param name="cullMode">The cull mode of the rasterizer state.</param>
        /// <param name="cullOrder">The cull order of the rasterizer state.</param>
        /// <param name="lineWidth">The line width of the rasterizer state.</param>
        /// <param name="depthStencilState">The rasterizer depth/stencil state.</param>
        explicit Rasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;
        Rasterizer(Rasterizer&&) noexcept;
        Rasterizer(const Rasterizer&) noexcept;
        virtual ~Rasterizer() noexcept;

    public:
        /// <inheritdoc />
        virtual const PolygonMode& polygonMode() const noexcept override;

        /// <inheritdoc />
        virtual const CullMode& cullMode() const noexcept override;

        /// <inheritdoc />
        virtual const CullOrder& cullOrder() const noexcept override;

        /// <inheritdoc />
        virtual const Float& lineWidth() const noexcept override;

        /// <inheritdoc />
        virtual const DepthStencilState& depthStencilState() const noexcept override;

    protected:
        virtual PolygonMode& polygonMode() noexcept;
        virtual CullMode& cullMode() noexcept;
        virtual CullOrder& cullOrder() noexcept;
        virtual Float& lineWidth() noexcept;
        virtual DepthStencilState& depthStencilState() noexcept;
    };

    /// <summary>
    /// Builds a <see cref="Rasterizer" />.
    /// </summary>
    template <typename TDerived, typename TRasterizer, typename TParent> requires
        rtti::implements<TRasterizer, IRasterizer>
    class RasterizerBuilder : public Builder<TDerived, TRasterizer, TParent, SharedPtr<TRasterizer>> {
    public:
        using Builder<TDerived, TRasterizer, TParent, SharedPtr<TRasterizer>>::Builder;

    public:
        /// <summary>
        /// Initializes the rasterizer state with the provided polygon mode.
        /// </summary>
        /// <param name="mode">The polygon mode to initialize the rasterizer state with.</param>
        virtual TDerived& withPolygonMode(const PolygonMode& mode) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided cull mode.
        /// </summary>
        /// <param name="mode">The cull mode to initialize the rasterizer state with.</param>
        virtual TDerived& withCullMode(const CullMode& mode) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided cull order.
        /// </summary>
        /// <param name="order">The cull order to initialize the rasterizer state with.</param>
        virtual TDerived& withCullOrder(const CullOrder& order) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided line width.
        /// </summary>
        /// <param name="width">The line width to initialize the rasterizer state with.</param>
        virtual TDerived& withLineWidth(const Float& width) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer depth bias.
        /// </summary>
        /// <param name="depthBias">The depth bias the rasterizer should use.</param>
        virtual TDerived& withDepthBias(const DepthStencilState::DepthBias& depthBias) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer depth state.
        /// </summary>
        /// <param name="depthState">The depth state of the rasterizer.</param>
        virtual TDerived& withDepthState(const DepthStencilState::DepthState& depthState) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer stencil state.
        /// </summary>
        /// <param name="stencilState">The stencil state of the rasterizer.</param>
        virtual TDerived& withStencilState(const DepthStencilState::StencilState& stencilState) noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IViewport {
    public:
        virtual ~IViewport() noexcept = default;

    public:
        virtual RectF getRectangle() const noexcept = 0;
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
        virtual float getMinDepth() const noexcept = 0;
        virtual void setMinDepth(const float& depth) const noexcept = 0;
        virtual float getMaxDepth() const noexcept = 0;
        virtual void setMaxDepth(const float& depth) const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Viewport : public IViewport {
        LITEFX_IMPLEMENTATION(ViewportImpl);

    public:
        explicit Viewport(const RectF& clientRect = { }, const Float& minDepth = 0.f, const Float& maxDepth = 1.f);
        Viewport(Viewport&&) noexcept = delete;
        Viewport(const Viewport&) noexcept = delete;
        virtual ~Viewport() noexcept;

    public:
        virtual RectF getRectangle() const noexcept override;
        virtual void setRectangle(const RectF& rectangle) noexcept override;
        virtual Float getMinDepth() const noexcept override;
        virtual void setMinDepth(const Float& depth) const noexcept override;
        virtual Float getMaxDepth() const noexcept override;
        virtual void setMaxDepth(const Float& depth) const noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IScissor {
    public:
        virtual ~IScissor() noexcept = default;

    public:
        virtual RectF getRectangle() const noexcept = 0;
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Scissor : public IScissor {
        LITEFX_IMPLEMENTATION(ScissorImpl);

    public:
        explicit Scissor(const RectF& scissorRect = { });
        Scissor(Scissor&&) noexcept = delete;
        Scissor(const Scissor&) noexcept = delete;
        virtual ~Scissor() noexcept;

    public:
        virtual RectF getRectangle() const noexcept override;
        virtual void setRectangle(const RectF& rectangle) noexcept override;
    };

    /// <summary>
    /// Stores meta data about a buffer attribute, i.e. a member or field of a descriptor or buffer.
    /// </summary>
    class LITEFX_RENDERING_API BufferAttribute {
        LITEFX_IMPLEMENTATION(BufferAttributeImpl);

    public:
        /// <summary>
        /// Initializes an empty buffer attribute.
        /// </summary>
        BufferAttribute();

        /// <summary>
        /// Initializes a new buffer attribute.
        /// </summary>
        /// <param name="location">The location the buffer attribute is bound to.</param>
        /// <param name="offset">The offset of the attribute relative to the buffer.</param>
        /// <param name="format">The format of the buffer attribute.</param>
        /// <param name="semantic">The semantic of the buffer attribute.</param>
        /// <param name="semanticIndex">The semantic index of the buffer attribute.</param>
        BufferAttribute(const UInt32& location, const UInt32& offset, const BufferFormat& format, const AttributeSemantic& semantic, const UInt32& semanticIndex = 0);
        BufferAttribute(BufferAttribute&&) noexcept;
        BufferAttribute(const BufferAttribute&);
        virtual ~BufferAttribute() noexcept;

    public:
        /// <summary>
        /// Returns the location of the buffer attribute.
        /// </summary>
        /// <remarks>
        /// Locations can only be specified in Vulkan and are implicitly generated based on semantics for DirectX. However, it is a good practice to provide them anyway.
        /// </remarks>
        /// <returns>The location of the buffer attribute.</returns>
        virtual const UInt32& location() const noexcept;

        /// <summary>
        /// Returns the format of the buffer attribute.
        /// </summary>
        /// <returns>The format of the buffer attribute.</returns>
        virtual const BufferFormat& format() const noexcept;

        /// <summary>
        /// Returns the offset of the buffer attribute.
        /// </summary>
        /// <returns>The offset of the buffer attribute.</returns>
        virtual const UInt32& offset() const noexcept;

        /// <summary>
        /// Returns the semantic of the buffer attribute.
        /// </summary>
        /// <remarks>
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        /// </remarks>
        /// <returns>The semantic of the buffer attribute.</returns>
        /// <seealso cref="semanticIndex" />
        virtual const AttributeSemantic& semantic() const noexcept;

        /// <summary>
        /// Returns the semantic index of the buffer attribute.
        /// </summary>
        /// <remarks>
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        /// </remarks>
        /// <returns>The semantic index of the buffer attribute.</returns>
        /// <seealso cref="semantic" />
        virtual const UInt32& semanticIndex() const noexcept;
    };
}