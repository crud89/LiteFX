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

#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/graphics.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    class IGraphicsAdapter;
    class ISurface;
    class IShaderModule;
    class IRenderTarget;
    class IRasterizer;
    class IViewport;
    class IScissor;
    class IBufferLayout;
    class IVertexBufferLayout;
    class IIndexBufferLayout;
    class IDescriptorLayout;
    class IMappable;
    class IDeviceMemory;
    class IBuffer;
    class IImage;
    class ISampler;
    class IBarrier;
    class IDescriptorSet;
    class IDescriptorSetLayout;
    class IPushConstantsRange;
    class IPushConstantsLayout;
    class IShaderProgram;
    class IPipelineLayout;
    class IVertexBuffer;
    class IIndexBuffer;
    class IInputAssembler;
    class IPipeline;
    class ICommandBuffer;
    class IRenderPipeline;
    class IComputePipeline;
    class IFrameBuffer;
    class IRenderPass;
    class ISwapChain;
    class ICommandQueue;
    class IGraphicsFactory;
    class IGraphicsDevice;
    class IRenderBackend;

#pragma region "Enumerations"

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
    /// Represents the type of a <see cref="CommandQueue" />.
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
    enum class LITEFX_RENDERING_API QueueType : UInt32 {
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
        /// Represents a queue that can perform hardware video decoding.
        /// </summary>
        /// <remarks>
        /// Video encoding/decoding is currently not a supported feature, but knowing all the capabilities of a queue is useful to select the best queue family for a particular task.
        /// </remarks>
        VideoDecode = 0x00000010,

        /// <summary>
        /// Represents a queue that can perform hardware video encoding.
        /// </summary>
        /// <remarks>
        /// Video encoding/decoding is currently a supported feature, but knowing all the capabilities of a queue is useful to select the best queue family for a particular task.
        /// </remarks>
        VideoEncode = 0x00000020,

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
        /// <remarks>
        /// Do not use this queue priority when creating queues, as it is reserved for the default (builtin) queues.
        /// </remarks>
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
    enum class LITEFX_RENDERING_API BufferFormat : UInt32 {
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
    enum class LITEFX_RENDERING_API AttributeSemantic : UInt32 {
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
        ConstantBuffer = 0x00000001,

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
        StructuredBuffer = 0x00000002,

        /// <summary>
        /// A writable shader storage object in Vulkan. Maps to a read/write structured buffer in DirectX.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>buffer</c> keyword to access storage buffers. In HLSL, use the <c>RWStructuredBuffer</c> keyword.
        /// </remarks>
        RWStructuredBuffer = 0x00000012,

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
        Texture = 0x00000003,

        /// <summary>
        /// A writable image.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>uniform image</c> keywords to access the texture. In HLSL, use the <c>RWTexture</c> keywords.
        /// </remaks>
        RWTexture = 0x00000013,
        
        /// <summary>
        /// A sampler state of a texture or image.
        /// </summary>
        Sampler = 0x00000004,

        /// <summary>
        /// The result of a render target from an earlier render pass. Maps to a <c>SubpassInput</c> in HLSL.
        /// </summary>
        InputAttachment = 0x00000005,

        /// <summary>
        /// Represents a read-only texel buffer (uniform texel buffer).
        /// </summary>
        /// <remarks>
        /// Use the <c>uniform imageBuffer</c> keyword in GLSL to access the buffer. In HLSL, use the <c>Buffer</c> keyword.
        /// </remarks>
        Buffer = 0x00000006,

        /// <summary>
        /// Represents a writable texel buffer (storage texel buffer).
        /// </summary>
        /// <remarks>
        /// Use the <c>uniform imageBuffer</c> keyword in GLSL to access the buffer. In HLSL, use the <c>RWBuffer</c> keyword.
        /// </remarks>
        RWBuffer = 0x00000016,

        /// <summary>
        /// Represents an unformatted buffer.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>buffer</c> keyword to access byte address buffers. In HLSL, use the <c>ByteAddressBuffer</c> keyword.
        /// </remarks>
        ByteAddressBuffer = 0x00000007,

        /// <summary>
        /// Represents an unformatted writable buffer.
        /// </summary>
        /// <remarks>
        /// In GLSL, use the <c>buffer</c> keyword to access byte address buffers. In HLSL, use the <c>RWByteAddressBuffer</c> keyword.
        /// </remarks>
        RWByteAddressBuffer = 0x00000017,
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
        /// <remarks>
        /// Buffers of this type can be bound to `ConstantBuffer` descriptors.
        /// </remarks>
        Uniform = 0x00000003,

        /// <summary>
        /// Describes a shader storage buffer object (Vulkan) or unordered access view (DirectX).
        /// </summary>
        /// <remarks>
        /// Buffers of this type can be bound to `StructuredBuffer`/`RWStructuredBuffer` or `ByteAddressBuffer`/`RWByteAddressBuffer` descriptors.
        /// </remarks>
        Storage = 0x00000004,

        /// <summary>
        /// Describes a shader texel storage buffer object (Vulkan) or unordered access view (DirectX).
        /// </summary>
        /// <remarks>
        /// Buffers of this type can be bound to `Buffer`/`RWBuffer` descriptors.
        /// </remarks>
        Texel = 0x00000005,

        /// <summary>
        /// Describes another type of buffer, such as samplers or images.
        /// </summary>
        /// <remarks>
        /// Buffers of this type must not be bound to any descriptor, but can be used as copy/transfer targets and sources.
        /// </remarks>
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
    enum class LITEFX_RENDERING_API ShaderStage : UInt32 {
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
    /// Describes the number of samples with which a <see cref="IImage" /> is sampled.
    /// </summary>
    enum class LITEFX_RENDERING_API MultiSamplingLevel : UInt32 {
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
    /// Defines pipeline stages as points where synchronization may occur.
    /// </summary>
    /// <seealso cref="IBarrier" />
    /// <seealso cref="ResourceAccess" />
    /// <seealso cref="ImageLayout" />
    enum class LITEFX_RENDERING_API PipelineStage {
        /// <summary>
        /// Represents no-blocking behavior.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_NONE` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_NONE` in DirectX 12 ❎.
        /// 
        /// This stage flag is special, as it cannot be combined with other stage flags.
        /// </remarks>
        None = 0x00000000,

        /// <summary>
        /// Waits for all previous commands to be finished, or blocks all following commands until the barrier is executed.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_ALL` in DirectX 12 ❎.
        /// 
        /// This stage flag is special, as it cannot be combined with other stage flags.
        /// </remarks>
        All = 0x00000001,

        /// <summary>
        /// Waits for previous commands to finish all graphics stages, or blocks following commands until the graphics stages has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_DRAW` in DirectX 12 ❎.
        /// 
        /// This stage flag is special, as it cannot be combined with other stage flags.
        /// </remarks>
        Draw = 0x00000002,

        /// <summary>
        /// Waits for previous commands to finish the input assembly stage, or blocks following commands until the input assembly stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_VERTEX_INPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_INDEX_INPUT` in DirectX 12 ❎.
        /// </remarks>
        InputAssembly = 0x00000004,

        /// <summary>
        /// Waits for previous commands to finish the vertex shader stage, or blocks following commands until the vertex shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_VERTEX_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        /// </remarks>
        Vertex = 0x00000006,

        /// <summary>
        /// Waits for previous commands to finish the tessellation control/hull shader stage, or blocks following commands until the tessellation control/hull shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        /// </remarks>
        TessellationControl = 0x00000008,

        /// <summary>
        /// Waits for previous commands to finish the tessellation evaluation/domain shader stage, or blocks following commands until the tessellation evaluation/domain shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        /// </remarks>
        TessellationEvaluation = 0x00000010,

        /// <summary>
        /// Waits for previous commands to finish the geometry shader stage, or blocks following commands until the geometry shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        /// </remarks>
        Geometry = 0x00000020,

        /// <summary>
        /// Waits for previous commands to finish the fragment/pixel shader stage, or blocks following commands until the fragment/pixel shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_PIXEL_SHADING` in DirectX 12 ❎.
        /// </remarks>
        Fragment = 0x00000040,

        /// <summary>
        /// Waits for previous commands to finish the depth/stencil stage, or blocks following commands until the depth/stencil stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_DEPTH_STENCIL` in DirectX 12 ❎.
        /// </remarks>
        DepthStencil = 0x00000080,

        /// <summary>
        /// Waits for previous commands to finish the draw indirect stage, or blocks following commands until the draw indirect stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_EXECUTE_INDIRECT` in DirectX 12 ❎.
        /// </remarks>
        Indirect = 0x00000100,

        /// <summary>
        /// Waits for previous commands to finish the output merger stage, or blocks following commands until the output merger stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RENDER_TARGET` in DirectX 12 ❎.
        /// </remarks>
        RenderTarget = 0x00000200,

        /// <summary>
        /// Waits for previous commands to finish the compute shader stage, or blocks following commands until the compute shader stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_COMPUTE_SHADING` in DirectX 12 ❎.
        /// 
        /// This stage flag is special, as it cannot be combined with other stage flags.
        /// </remarks>
        Compute = 0x00000400,

        /// <summary>
        /// Waits for previous commands to finish the transfer stage, or blocks following commands until the transfer stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_TRANSFER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_COPY` in DirectX 12 ❎.
        /// </remarks>
        Transfer = 0x00000800,

        /// <summary>
        /// Waits for previous commands to finish the multi-sampling resolution stage, or blocks following commands until the multi-sampling resolution stage has finished.
        /// </summary>
        /// <remarks>
        /// Translates to `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RESOLVE` in DirectX 12 ❎.
        /// </remarks>
        Resolve = 0x00001000
    };

    /// <summary>
    /// Defines how a <see cref="IBuffer" /> or <see cref="IImage" /> resource is accessed.
    /// </summary>
    /// <seealso cref="IBarrier" />
    /// <seealso cref="IImage" />
    /// <seealso cref="IBuffer" />
    /// <seealso cref="PipelineStage" />
    /// <seealso cref="ImageLayout" />
    enum class LITEFX_RENDERING_API ResourceAccess {
        /// <summary>
        /// Indicates that a resource is not accessed.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_NO_ACCESS` in the DirectX 12 ❎ backend and `VK_ACCESS_NONE` in the Vulkan 🌋 backend.
        /// 
        /// This access flag is special, as it cannot be combined with other access flags.
        /// </remarks>
        None = 0x7FFFFFFF,

        /// <summary>
        /// Indicates that a resource is accessed as a vertex buffer.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_VERTEX_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        VertexBuffer = 0x00000001,

        /// <summary>
        /// Indicates that a resource is accessed as an index buffer.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_INDEX_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_INDEX_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        IndexBuffer = 0x00000002,

        /// <summary>
        /// Indicates that a resource is accessed as an uniform/constant buffer.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_CONSTANT_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_UNIFORM_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        UniformBuffer = 0x00000004,

        /// <summary>
        /// Indicates that a resource is accessed as a render target.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RENDER_TARGET` in the DirectX 12 ❎ backend and `VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        RenderTarget = 0x00000008,

        /// <summary>
        /// Indicates that a resource is accessed as to read depth/stencil values.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ` in the DirectX 12 ❎ backend and `VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        DepthStencilRead = 0x00000010,

        /// <summary>
        /// Indicates that a resource is accessed as to write depth/stencil values.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE` in the DirectX 12 ❎ backend and `VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        DepthStencilWrite = 0x00000020,

        /// <summary>
        /// Indicates that a resource is accessed as a read-only shader resource.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_SHADER_RESOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_SHADER_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        ShaderRead = 0x00000040,

        /// <summary>
        /// Indicates that a resource is accessed as a read-write shader resource.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_UNORDERED_ACCESS` in the DirectX 12 ❎ backend and `VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        ShaderReadWrite = 0x00000080,

        /// <summary>
        /// Indicates that a resource is accessed as to read indirect draw commands.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT` in the DirectX 12 ❎ backend and `VK_ACCESS_INDIRECT_COMMAND_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        Indirect = 0x00000100,

        /// <summary>
        /// Indicates that a resource is accessed as to read during a transfer operation.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COPY_SOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_TRANSFER_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        TransferRead = 0x00000200,

        /// <summary>
        /// Indicates that a resource is accessed as to write during a transfer operation.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COPY_DEST` in the DirectX 12 ❎ backend and `VK_ACCESS_TRANSFER_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        TransferWrite = 0x00000400,

        /// <summary>
        /// Indicates that a resource is accessed as to read during a resolve operation.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RESOLVE_SOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_READ_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        ResolveRead = 0x00000800,

        /// <summary>
        /// Indicates that a resource is accessed as to write during a resolve operation.
        /// </summary>
        /// <remarks>
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RESOLVE_DEST` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        ResolveWrite = 0x00001000,
        
        /// <summary>
        /// Indicates that a resource can be accessed in any way, compatible to the layout.
        /// </summary>
        /// <remarks>
        /// Note that you have to ensure that you do not access the resource in an incompatible way manually.
        /// 
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COMMON` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT` in the Vulkan 🌋 backend.
        /// </remarks>
        Common = 0x00002000
    };

    /// <summary>
    /// Specifies the layout of an <see cref="IImage" /> resource.
    /// </summary>
    /// <seealso cref="IImage" />
    /// <seealso cref="IBarrier" />
    /// <seealso cref="ResourceAccess" />
    /// <seealso cref="PipelineStage" />
    enum class LITEFX_RENDERING_API ImageLayout {
        /// <summary>
        /// A common image layout that allows for all types of access (shader resource, transfer destination, transfer source).
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COMMON` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_GENERAL` in the Vulkan 🌋 backend.
        /// </remarks>
        Common = 0x00000001,

        /// <summary>
        /// Indicates that the image is used as a read-only storage or texel buffer.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_SHADER_RESOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        ShaderResource = 0x00000002,

        /// <summary>
        /// Indicates that the image is used as a read-write storage or texel buffer.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_GENERAL` in the Vulkan 🌋 backend.
        /// </remarks>
        ReadWrite = 0x00000003,

        /// <summary>
        /// Allows the image to be used as a source for transfer operations.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COPY_SOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        CopySource = 0x00000010,

        /// <summary>
        /// Allows the image to be used as a destination for transfer operations.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COPY_DEST` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        CopyDestination = 0x00000011,

        /// <summary>
        /// Indicates that the image is used as a render target.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RENDER_TARGET` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        RenderTarget = 0x00000020,

        /// <summary>
        /// Indicates that image is used as a read-only depth/stencil target.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        DepthRead = 0x00000021,

        /// <summary>
        /// Indicates that the image is used as a write-only depth/stencil target.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        DepthWrite = 0x00000022,

        /// <summary>
        /// Indicates that the image is presented on a swap chain.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_PRESENT` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` in the Vulkan 🌋 backend.
        /// 
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
        /// </remarks>
        Present = 0x00000023,

        /// <summary>
        /// Indicates that the image is resolved from a multi-sampled image.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        ResolveSource = 0x00000024,

        /// <summary>
        /// Indicates that the image is a render-target that a multi-sampled image is resolved into.
        /// </summary>
        /// <remarks>
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RESOLVE_DEST` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        /// </remarks>
        ResolveDestination = 0x00000025,

        /// <summary>
        /// The layout of the image is not known by the engine.
        /// </summary>
        /// <remarks>
        /// Indicates that an image's layout is not known, which typically happens after creating image resources. It is not valid to transition any resource into this state.
        /// 
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_UNDEFINED` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_UNDEFINED` in the Vulkan 🌋 backend.
        /// </remarks>
        Undefined = 0x7FFFFFFF
    };

#pragma endregion

#pragma region "Flags"

    LITEFX_DEFINE_FLAGS(QueueType);
    LITEFX_DEFINE_FLAGS(ShaderStage);
    LITEFX_DEFINE_FLAGS(PipelineStage);
    LITEFX_DEFINE_FLAGS(ResourceAccess);
    LITEFX_DEFINE_FLAGS(BufferFormat);
    LITEFX_DEFINE_FLAGS(WriteMask);

#pragma endregion

#pragma region "Helper Functions"

    /// <summary>
    /// Returns the number of channels for a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    constexpr inline UInt32 getBufferFormatChannels(BufferFormat format) {
        return static_cast<UInt32>(format) & 0x000000FF;
    }

    /// <summary>
    /// Returns the number of bytes used by a channel of a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    constexpr inline UInt32 getBufferFormatChannelSize(BufferFormat format) {
        return (static_cast<UInt32>(format) & 0xFF000000) >> 24;
    }

    /// <summary>
    /// Returns the underlying data type of a buffer format.
    /// </summary>
    /// <seealso cref="BufferFormat" />
    constexpr inline UInt32 getBufferFormatType(BufferFormat format) {
        return (static_cast<UInt32>(format) & 0x0000FF00) >> 8;
    }

    /// <summary>
    /// Returns the size of an element of a specified format.
    /// </summary>
    constexpr inline size_t LITEFX_RENDERING_API getSize(Format format);

    /// <summary>
    /// Returns <c>true</c>, if the format contains a depth channel.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    constexpr inline bool LITEFX_RENDERING_API hasDepth(Format format);

    /// <summary>
    /// Returns <c>true</c>, if the format contains a stencil channel.
    /// </summary>
    /// <seealso cref="DepthStencilState" />
    constexpr inline bool LITEFX_RENDERING_API hasStencil(Format format);

#pragma endregion

    /// <summary>
    /// The interface for a state resource.
    /// </summary>
    class LITEFX_RENDERING_API IStateResource {
    public:
        virtual ~IStateResource() noexcept = default;

    public:
        /// <summary>
        /// Returns the name of the resource.
        /// </summary>
        /// <returns>The name of the resource.</returns>
        virtual const String& name() const noexcept = 0;
    };

    /// <summary>
    /// Base class for a resource that can be identified by a name string within a <see cref="DeviceState" />.
    /// </summary>
    class LITEFX_RENDERING_API StateResource : public virtual IStateResource {
        LITEFX_IMPLEMENTATION(StateResourceImpl);

    protected:
        StateResource() noexcept;

    public:
        /// <summary>
        /// Initializes a new state resource instance.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        explicit StateResource(StringView name);
        StateResource(StateResource&&) = delete;
        StateResource(const StateResource&) = delete;
        virtual ~StateResource() noexcept;

    protected:
        /// <inheritdoc />
        String& name() noexcept;

    public:
        /// <inheritdoc />
        const String& name() const noexcept override;
    };

    /// <summary>
    /// A class that can be used to manage the state of a <see cref="IGraphicsDevice" />.
    /// </summary>
    /// <remarks>
    /// The device state makes managing resources created by a device easier, since you do not have to worry about storage and release order. Note,
    /// however, that this is not free. Requesting a resource requires a lookup within a hash-map. Also device states are not specialized for the 
    /// concrete device, so you can only work with interfaces. This implies potentially inefficient upcasting of the state resource when its passed to 
    /// another object. You have to decide if or to which degree you want to rely on storing resources in a device state.
    /// </remarks>
    /// <seealso cref="StateResource" />
    /// <seealso cref="IGraphicsDevice" />
    class LITEFX_RENDERING_API DeviceState {
        LITEFX_IMPLEMENTATION(DeviceStateImpl);
        friend class IGraphicsDevice;

    public:
        /// <summary>
        /// Creates a new device state instance.
        /// </summary>
        explicit DeviceState() noexcept;
        DeviceState(DeviceState&&) = delete;
        DeviceState(const DeviceState&) = delete;
        virtual ~DeviceState() noexcept;

    public:
        /// <summary>
        /// Release all resources managed by the device state.
        /// </summary>
        void clear();

        /// <summary>
        /// Adds a new render pass to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="renderPass">The render pass to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another render pass with the same identifier has already been added.</exception>
        void add(UniquePtr<IRenderPass>&& renderPass);

        /// <summary>
        /// Adds a new render pass to the device state.
        /// </summary>
        /// <param name="id">The identifier for the render pass.</param>
        /// <param name="renderPass">The render pass to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another render pass with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IRenderPass>&& renderPass);

        /// <summary>
        /// Adds a new pipeline to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="pipeline">The pipeline to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another pipeline with the same identifier has already been added.</exception>
        void add(UniquePtr<IPipeline>&& pipeline);

        /// <summary>
        /// Adds a new pipeline to the device state.
        /// </summary>
        /// <param name="id">The identifier for the pipeline.</param>
        /// <param name="pipeline">The pipeline to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another pipeline with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IPipeline>&& pipeline);

        /// <summary>
        /// Adds a new buffer to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="buffer">The buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another buffer with the same identifier has already been added.</exception>
        void add(UniquePtr<IBuffer>&& buffer);

        /// <summary>
        /// Adds a new buffer to the device state.
        /// </summary>
        /// <param name="id">The identifier for the buffer.</param>
        /// <param name="buffer">The buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another buffer with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IBuffer>&& buffer);

        /// <summary>
        /// Adds a new vertex buffer to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another vertex buffer with the same identifier has already been added.</exception>
        void add(UniquePtr<IVertexBuffer>&& vertexBuffer);

        /// <summary>
        /// Adds a new vertex buffer to the device state.
        /// </summary>
        /// <param name="id">The identifier for the vertex buffer.</param>
        /// <param name="vertexBuffer">The vertex buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another vertex buffer with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IVertexBuffer>&& vertexBuffer);

        /// <summary>
        /// Adds a new index buffer to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="indexBuffer">The index buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another index buffer with the same identifier has already been added.</exception>
        void add(UniquePtr<IIndexBuffer>&& indexBuffer);

        /// <summary>
        /// Adds a new index buffer to the device state.
        /// </summary>
        /// <param name="id">The identifier for the index buffer.</param>
        /// <param name="indexBuffer">The index buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another index buffer with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IIndexBuffer>&& indexBuffer);

        /// <summary>
        /// Adds a new image to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="image">The image to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another image with the same identifier has already been added.</exception>
        void add(UniquePtr<IImage>&& image);

        /// <summary>
        /// Adds a new image to the device state.
        /// </summary>
        /// <param name="id">The identifier for the image.</param>
        /// <param name="image">The image to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another image with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IImage>&& image);

        /// <summary>
        /// Adds a new sampler to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="sampler">The sampler to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another sampler with the same identifier has already been added.</exception>
        void add(UniquePtr<ISampler>&& sampler);

        /// <summary>
        /// Adds a new sampler to the device state.
        /// </summary>
        /// <param name="id">The identifier for the sampler.</param>
        /// <param name="sampler">The sampler to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another sampler with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<ISampler>&& sampler);
        
        /// <summary>
        /// Adds a new descriptor set to the device state.
        /// </summary>
        /// <param name="id">The identifier for the descriptor set.</param>
        /// <param name="sampler">The descriptor set to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another descriptor set with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IDescriptorSet>&& descriptorSet);

        /// <summary>
        /// Returns a render pass from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the render pass.</param>
        /// <returns>A reference of the render pass.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no render pass has been added for the provided <paramref name="id" />.</exception>
        IRenderPass& renderPass(const String& id) const;

        /// <summary>
        /// Returns a pipeline from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the pipeline.</param>
        /// <returns>A reference of the pipeline.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no pipeline has been added for the provided <paramref name="id" />.</exception>
        IPipeline& pipeline(const String& id) const;

        /// <summary>
        /// Returns a buffer from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the buffer.</param>
        /// <returns>A reference of the buffer.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no buffer has been added for the provided <paramref name="id" />.</exception>
        IBuffer& buffer(const String& id) const;

        /// <summary>
        /// Returns a vertex buffer from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the vertex buffer.</param>
        /// <returns>A reference of the vertex buffer.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no vertex buffer has been added for the provided <paramref name="id" />.</exception>
        IVertexBuffer& vertexBuffer(const String& id) const;

        /// <summary>
        /// Returns an index buffer from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the index buffer.</param>
        /// <returns>A reference of the index buffer.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no index buffer has been added for the provided <paramref name="id" />.</exception>
        IIndexBuffer& indexBuffer(const String& id) const;

        /// <summary>
        /// Returns an image from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the image.</param>
        /// <returns>A reference of the image.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no image has been added for the provided <paramref name="id" />.</exception>
        IImage& image(const String& id) const;

        /// <summary>
        /// Returns an sampler from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the sampler.</param>
        /// <returns>A reference of the sampler.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no sampler has been added for the provided <paramref name="id" />.</exception>
        ISampler& sampler(const String& id) const;

        /// <summary>
        /// Returns an descriptor set from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the descriptor set.</param>
        /// <returns>A reference of the descriptor set.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no descriptor set has been added for the provided <paramref name="id" />.</exception>
        IDescriptorSet& descriptorSet(const String& id) const;

        /// <summary>
        /// Releases a render pass.
        /// </summary>
        /// <remarks>
        /// Calling this method will destroy the render pass. Before calling it, the render pass must be requested using <see cref="renderPass" />. After 
        /// this method has been executed, all references (including the <paramref name="renderPass" /> parameter) will be invalid. If the render pass is
        /// not managed by the device state, this method will do nothing and return <c>false</c>.
        /// </remarks>
        /// <param name="renderPass">The render pass to release.</param>
        /// <returns><c>true</c>, if the render pass was properly released, <c>false</c> otherwise.</returns>
        bool release(const IRenderPass& renderPass);

        /// <summary>
        /// Releases a pipeline.
        /// </summary>
        /// <param name="pipeline">The pipeline to release.</param>
        /// <returns><c>true</c>, if the pipeline was properly released, <c>false</c> otherwise.</returns>
        bool release(const IPipeline& pipeline);

        /// <summary>
        /// Releases a buffer.
        /// </summary>
        /// <param name="buffer">The buffer to release.</param>
        /// <returns><c>true</c>, if the buffer was properly released, <c>false</c> otherwise.</returns>
        bool release(const IBuffer& buffer);

        /// <summary>
        /// Releases a vertex buffer.
        /// </summary>
        /// <param name="buffer">The vertex buffer to release.</param>
        /// <returns><c>true</c>, if the vertex buffer was properly released, <c>false</c> otherwise.</returns>
        bool release(const IVertexBuffer& buffer);

        /// <summary>
        /// Releases a index buffer.
        /// </summary>
        /// <param name="buffer">The index buffer to release.</param>
        /// <returns><c>true</c>, if the index buffer was properly released, <c>false</c> otherwise.</returns>
        bool release(const IIndexBuffer& buffer);

        /// <summary>
        /// Releases an image.
        /// </summary>
        /// <param name="image">The image to release.</param>
        /// <returns><c>true</c>, if the image was properly released, <c>false</c> otherwise.</returns>
        bool release(const IImage& image);

        /// <summary>
        /// Releases a sampler.
        /// </summary>
        /// <param name="sampler">The sampler to release.</param>
        /// <returns><c>true</c>, if the sampler was properly released, <c>false</c> otherwise.</returns>
        bool release(const ISampler& sampler);

        /// <summary>
        /// Releases a descriptor set.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to release.</param>
        /// <returns><c>true</c>, if the descriptor set was properly released, <c>false</c> otherwise.</returns>
        bool release(const IDescriptorSet& descriptorSet);
    };

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
        virtual String name() const noexcept = 0;

        /// <summary>
        /// Returns a unique identifier, that identifies the device in the system.
        /// </summary>
        /// <returns>A unique identifier, that identifies the device in the system.</returns>
        virtual UInt64 uniqueId() const noexcept = 0;

        /// <summary>
        /// Returns a unique identifier, that identifies the vendor of the graphics adapter.
        /// </summary>
        /// <returns>A unique identifier, that identifies the vendor of the graphics adapter.</returns>
        virtual UInt32 vendorId() const noexcept = 0;

        /// <summary>
        /// Returns a unique identifier, that identifies the product.
        /// </summary>
        /// <returns>A unique identifier, that identifies the product.</returns>
        virtual UInt32 deviceId() const noexcept = 0;

        /// <summary>
        /// Returns the type of the graphics adapter.
        /// </summary>
        /// <returns>The type of the graphics adapter.</returns>
        virtual GraphicsAdapterType type() const noexcept = 0;

        /// <summary>
        /// Returns the graphics driver version.
        /// </summary>
        /// <returns>The graphics driver version.</returns>
        virtual UInt32 driverVersion() const noexcept = 0;

        /// <summary>
        /// Returns the graphics API version.
        /// </summary>
        /// <returns>The graphics API version.</returns>
        virtual UInt32 apiVersion() const noexcept = 0;

        /// <summary>
        /// Returns the amount of dedicated graphics memory (in bytes), this adapter can use.
        /// </summary>
        /// <returns>The amount of dedicated graphics memory (in bytes), this adapter can use.</returns>
        virtual UInt64 dedicatedMemory() const noexcept = 0;
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
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_RENDERING_API IShaderModule {
    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the shader module.
        /// </summary>
        /// <returns>The type of the shader module.</returns>
        virtual ShaderStage type() const noexcept = 0;

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
    /// Represents a render target, i.e. an abstract view of the output of an <see cref="RenderPass" />.
    /// </remarks>
    /// <remarks>
    /// A render target represents one output of a render pass, stored within an <see cref="IImage" />. It is contained by a <see cref="RenderPass" />, that contains 
    /// the <see cref="FrameBuffer" />, that stores the actual render target image resource.
    /// </remarks>
    /// <seealso cref="RenderTarget" />
    /// <seealso cref="RenderPass" />
    /// <seealso cref="FrameBuffer" />
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
        /// Returns the name of the render target.
        /// </summary>
        /// <returns>The name of the render target.</returns>
        virtual const String& name() const noexcept = 0;

        /// <summary>
        /// Returns the location of the render target output attachment within the fragment shader.
        /// </summary>
        /// <remarks>
        /// The locations of all render targets of a frame buffer must be within a continuous domain, starting at <c>0</c>. A frame buffer validates the render target locations
        /// when it is initialized and will raise an exception, if a location is either not mapped or assigned multiple times.
        /// </remarks>
        /// <returns>The location of the render target output attachment within the fragment shader</returns>
        virtual UInt32 location() const noexcept = 0;

        /// <summary>
        /// Returns the type of the render target.
        /// </summary>
        /// <returns>The type of the render target.</returns>
        virtual RenderTargetType type() const noexcept = 0;

        /// <summary>
        /// Returns the internal format of the render target.
        /// </summary>
        /// <returns>The internal format of the render target.</returns>
        virtual Format format() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the render target should be cleared, when the render pass is started. If the <see cref="format" /> is set to a depth format, this clears the
        /// depth buffer. Otherwise it clears the color buffer.
        /// </summary>
        /// <returns><c>true</c>, if the render target should be cleared, when the render pass is started</returns>
        /// <seealso cref="clearStencil" />
        /// <seealso cref="clearValues" />
        virtual bool clearBuffer() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the render target stencil should be cleared, when the render pass is started. If the <see cref="format" /> is does not contain a stencil channel,
        /// this has no effect.
        /// </summary>
        /// <returns><c>true</c>, if the render target stencil should be cleared, when the render pass is started</returns>
        /// <seealso cref="clearStencil" />
        /// <seealso cref="clearValues" />
        virtual bool clearStencil() const noexcept = 0;

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
        virtual bool isVolatile() const noexcept = 0;

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
        explicit RenderTarget(UInt32 location, RenderTargetType type, Format format, bool clearBuffer, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, bool clearStencil = true, bool isVolatile = false, const BlendState& blendState = {});

        /// <summary>
        /// Initializes the render target.
        /// </summary>
        /// <param name="location">The name of the render target.</param>
        /// <param name="location">The location of the render target output attachment.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The format of the render target.</param>
        /// <param name="clearBuffer"><c>true</c>, if the render target should be cleared, when a render pass is started.</param>
        /// <param name="clearValues">The values with which the render target gets cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared, when a render pass is started.</param>
        /// <param name="isVolatile"><c>true</c>, if the target should not be made persistent for access after the render pass has finished.</param>
        /// <param name="blendState">The render target blend state.</param>
        explicit RenderTarget(const String& name, UInt32 location, RenderTargetType type, Format format, bool clearBuffer, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, bool clearStencil = true, bool isVolatile = false, const BlendState& blendState = {});
        RenderTarget(const RenderTarget&) noexcept;
        RenderTarget(RenderTarget&&) noexcept;
        virtual ~RenderTarget() noexcept;

    public:
        inline RenderTarget& operator=(const RenderTarget&) noexcept;
        inline RenderTarget& operator=(RenderTarget&&) noexcept;

    public:
        /// <inheritdoc />
        const String& name() const noexcept override;

        /// <inheritdoc />
        UInt32 location() const noexcept override;

        /// <inheritdoc />
        RenderTargetType type() const noexcept override;

        /// <inheritdoc />
        Format format() const noexcept override;

        /// <inheritdoc />
        bool clearBuffer() const noexcept override;

        /// <inheritdoc />
        bool clearStencil() const noexcept override;

        /// <inheritdoc />
        const Vector4f& clearValues() const noexcept override;

        /// <inheritdoc />
        bool isVolatile() const noexcept override;

        /// <inheritdoc />
        const BlendState& blendState() const noexcept override;
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
    /// Represents the rasterizer state of a <see cref="RenderPipeline" />.
    /// </summary>
    class LITEFX_RENDERING_API IRasterizer {
    public:
        virtual ~IRasterizer() noexcept = default;

    public:
        /// <summary>
        /// Returns the polygon mode of the rasterizer state.
        /// </summary>
        /// <returns>The polygon mode of the rasterizer state.</returns>
        virtual PolygonMode polygonMode() const noexcept = 0;

        /// <summary>
        /// Returns the cull mode of the rasterizer state.
        /// </summary>
        /// <returns>The cull mode of the rasterizer state.</returns>
        virtual CullMode cullMode() const noexcept = 0;

        /// <summary>
        /// Returns the cull mode of the rasterizer state.
        /// </summary>
        /// <returns>The cull mode of the rasterizer state.</returns>
        virtual CullOrder cullOrder() const noexcept = 0;

        /// <summary>
        /// Returns the line width of the rasterizer state.
        /// </summary>
        /// <remarks>
        /// Note that line width is not supported in DirectX and is only emulated under Vulkan. Instead of forcing this value, it is recommended to 
        /// use a custom shader for it.
        /// </remarks>
        /// <returns>The line width of the rasterizer state.</returns>
        virtual Float lineWidth() const noexcept = 0;

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
        explicit Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;
        Rasterizer(Rasterizer&&) noexcept;
        Rasterizer(const Rasterizer&) noexcept;
        virtual ~Rasterizer() noexcept;

    public:
        /// <inheritdoc />
        PolygonMode polygonMode() const noexcept override;

        /// <inheritdoc />
        CullMode cullMode() const noexcept override;

        /// <inheritdoc />
        CullOrder cullOrder() const noexcept override;

        /// <inheritdoc />
        Float lineWidth() const noexcept override;

        /// <inheritdoc />
        const DepthStencilState& depthStencilState() const noexcept override;

    protected:
        virtual PolygonMode& polygonMode() noexcept;
        virtual CullMode& cullMode() noexcept;
        virtual CullOrder& cullOrder() noexcept;
        virtual Float& lineWidth() noexcept;
        virtual DepthStencilState& depthStencilState() noexcept;
    };

    /// <summary>
    /// Interface for a viewport.
    /// </summary>
    class LITEFX_RENDERING_API IViewport {
    public:
        virtual ~IViewport() noexcept = default;

    public:
        /// <summary>
        /// Gets the rectangle that defines the dimensions of the viewport.
        /// </summary>
        /// <returns>The rectangle that defines the dimensions of the viewport.</returns>
        virtual RectF getRectangle() const noexcept = 0;

        /// <summary>
        /// Sets the rectangle that defines the dimensions of the viewport.
        /// </summary>
        /// <param name="rectangle">The rectangle that defines the dimensions of the viewport.</param>
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;

        /// <summary>
        /// Gets the minimum depth of the viewport.
        /// </summary>
        /// <returns>The minimum depth of the viewport.</returns>
        virtual float getMinDepth() const noexcept = 0;

        /// <summary>
        /// Sets the minimum depth of the viewport.
        /// </summary>
        /// <param name="depth">The minimum depth of the viewport.</param>
        virtual void setMinDepth(Float depth) const noexcept = 0;

        /// <summary>
        /// Gets the maximum depth of the viewport.
        /// </summary>
        /// <returns>The maximum depth of the viewport.</returns>
        virtual float getMaxDepth() const noexcept = 0;

        /// <summary>
        /// Sets the maximum depth of the viewport.
        /// </summary>
        /// <param name="depth">The maximum depth of the viewport.</param>
        virtual void setMaxDepth(Float depth) const noexcept = 0;
    };

    /// <summary>
    /// Implements a viewport.
    /// </summary>
    class LITEFX_RENDERING_API Viewport : public IViewport {
        LITEFX_IMPLEMENTATION(ViewportImpl);

    public:
        /// <summary>
        /// Initializes a new viewport.
        /// </summary>
        /// <param name="clientRect">The rectangle that defines the dimensions of the viewport.</param>
        /// <param name="minDepth">The minimum depth of the viewport.</param>
        /// <param name="maxDepth">The maximum depth of the viewport.</param>
        explicit Viewport(const RectF& clientRect = { }, Float minDepth = 0.f, Float maxDepth = 1.f);

        Viewport(Viewport&&) noexcept = delete;
        Viewport(const Viewport&) noexcept = delete;
        virtual ~Viewport() noexcept;

    public:
        /// <inheritdoc />
        RectF getRectangle() const noexcept override;

        /// <inheritdoc />
        void setRectangle(const RectF& rectangle) noexcept override;

        /// <inheritdoc />
        Float getMinDepth() const noexcept override;

        /// <inheritdoc />
        void setMinDepth(Float depth) const noexcept override;

        /// <inheritdoc />
        Float getMaxDepth() const noexcept override;

        /// <inheritdoc />
        void setMaxDepth(Float depth) const noexcept override;
    };

    /// <summary>
    /// The interface of a scissor.
    /// </summary>
    class LITEFX_RENDERING_API IScissor {
    public:
        virtual ~IScissor() noexcept = default;

    public:
        /// <summary>
        /// Gets the rectangle that defines the scissor region.
        /// </summary>
        /// <returns>The rectangle that defines the scissor region.</returns>
        virtual RectF getRectangle() const noexcept = 0;

        /// <summary>
        /// Sets the rectangle that defines the scissor region.
        /// </summary>
        /// <param name="rectangle">The rectangle that defines the scissor region.</rectangle>
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
    };

    /// <summary>
    /// Implements a scissor.
    /// </summary>
    class LITEFX_RENDERING_API Scissor : public IScissor {
        LITEFX_IMPLEMENTATION(ScissorImpl);

    public:
        /// <summary>
        /// Initializes a new scissor.
        /// </summary>
        /// <param name="scissorRect">The rectangle that defines the scissor region.</param>
        explicit Scissor(const RectF& scissorRect = { });

        Scissor(Scissor&&) noexcept = delete;
        Scissor(const Scissor&) noexcept = delete;
        virtual ~Scissor() noexcept;

    public:
        /// <inheritdoc />
        RectF getRectangle() const noexcept override;

        /// <inheritdoc />
        void setRectangle(const RectF& rectangle) noexcept override;
    };

    /// <summary>
    /// An event that is used to measure timestamps in a command queue.
    /// </summary>
    /// <remarks>
    /// Timing events are used to collect GPU time stamps asynchronously. A timing event can be inserted to a <see cref="ICommandBuffer" /> by 
    /// calling <see cref="ICommandBuffer::writeTimingEvent" />. This will cause the GPU to write the current time stamp when the command gets 
    /// executed. Since command order is not preserved within command buffers, this is not guaranteed to be accurate an accurate point of time
    /// for when a certain command in order has been executed. However, since a time stamp is always written at the bottom of the pipe, the 
    /// difference between two timestamps resembles the actual time the GPU was occupied with the commands between them.
    /// 
    /// Timing events are asynchronous. They are set for a certain back buffer of a <see cref="ISwapChain" />. Reading the time stamp requires
    /// the back buffer to be ready (i.e., the frame in flight needs to have executed). The earliest point where this is guaranteed is, if the
    /// swap chain swaps to the back buffer again. This means that the time stamps issued in one frame can only be read the next time the 
    /// frame's back buffer is used again.
    /// 
    /// Note that timing events are only supported on graphics and compute <see cref="ICommandQueue" />s.
    /// </remarks>
    /// <seeaslo cref="ISwapChain" />
    class LITEFX_RENDERING_API TimingEvent : public std::enable_shared_from_this<TimingEvent> {
        LITEFX_IMPLEMENTATION(TimingEventImpl);
        friend class ISwapChain;

    public:
        /// <summary>
        /// Returns a pointer with shared ownership to the current instance.
        /// </summary>
        /// <returns>A pointer with shared ownership to the current instance.</returns>
        inline std::shared_ptr<TimingEvent> getptr() {
            return shared_from_this();
        }

    private:
        explicit TimingEvent(const ISwapChain& swapChain, StringView name = "") noexcept;

    public:
        TimingEvent(TimingEvent&&) = delete;
        TimingEvent(const TimingEvent&) = delete;
        virtual ~TimingEvent() noexcept;

    public:
        /// <summary>
        /// Gets the name of the timing event.
        /// </summary>
        /// <returns>The name of the timing event.</returns>
        String name() const noexcept;

        /// <summary>
        /// Reads the current timestamp (as a tick count) of the event.
        /// </summary>
        /// <remarks>
        /// In order to convert the number of ticks to (milli-)seconds, this value needs to be divided by <see cref="IGraphicsDevice::ticksPerMillisecond" />. To improve precision,
        /// calculate the difference between two time stamps in ticks first and only then convert them to seconds.
        /// </remarks>
        /// <returns>The current time stamp of the event as a tick count.</returns>
        /// <seealso cref="ISwapChain::readTimingEvent" />
        UInt64 readTimestamp() const noexcept;

        /// <summary>
        /// Returns the query ID for the timing event.
        /// </summary>
        /// <returns>The query ID for the timing event.</returns>
        /// <seealso cref="ISwapChain::resolveQueryId" />
        UInt32 queryId() const;
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
        BufferAttribute(UInt32 location, UInt32 offset, BufferFormat format, AttributeSemantic semantic, UInt32 semanticIndex = 0);
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
        virtual UInt32 location() const noexcept;

        /// <summary>
        /// Returns the format of the buffer attribute.
        /// </summary>
        /// <returns>The format of the buffer attribute.</returns>
        virtual BufferFormat format() const noexcept;

        /// <summary>
        /// Returns the offset of the buffer attribute.
        /// </summary>
        /// <returns>The offset of the buffer attribute.</returns>
        virtual UInt32 offset() const noexcept;

        /// <summary>
        /// Returns the semantic of the buffer attribute.
        /// </summary>
        /// <remarks>
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        /// </remarks>
        /// <returns>The semantic of the buffer attribute.</returns>
        /// <seealso cref="semanticIndex" />
        virtual AttributeSemantic semantic() const noexcept;

        /// <summary>
        /// Returns the semantic index of the buffer attribute.
        /// </summary>
        /// <remarks>
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        /// </remarks>
        /// <returns>The semantic index of the buffer attribute.</returns>
        /// <seealso cref="semantic" />
        virtual UInt32 semanticIndex() const noexcept;
    };

    /// <summary>
    /// Describes a buffer layout.
    /// </summary>
    /// <seealso cref="IVertexBufferLayout" />
    /// <seealso cref="IIndexBufferLayout" />
    /// <seealso cref="IDescriptorLayout" />
    class LITEFX_RENDERING_API IBufferLayout {
    public:
        virtual ~IBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the size of a single element within the buffer.
        /// </summary>
        /// <returns>The size of a single element within the buffer.</returns>
        virtual size_t elementSize() const noexcept = 0;

        /// <summary>
        /// Returns the binding point, the buffer will be bound to.
        /// </summary>
        /// <remarks>
        /// In GLSL, the binding point is identified by the <c>binding</c> keyword, whilst in HLSL the binding maps to a register.
        /// </remarks>
        /// <returns>The binding point, the buffer will be bound to.</returns>
        virtual UInt32 binding() const noexcept = 0;

        /// <summary>
        /// Returns the buffer type of the buffer.
        /// </summary>
        /// <returns>The buffer type of the buffer.</returns>
        virtual BufferType type() const noexcept = 0;
    };

    /// <summary>
    /// Describes a vertex buffer layout.
    /// </summary>
    /// <seealso cref="IVertexBuffer" />
    class LITEFX_RENDERING_API IVertexBufferLayout : public IBufferLayout {
    public:
        virtual ~IVertexBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the vertex buffer attributes.
        /// </summary>
        /// <returns>The vertex buffer attributes.</returns>
        virtual Enumerable<const BufferAttribute*> attributes() const noexcept = 0;
    };

    /// <summary>
    /// Describes a index buffer layout.
    /// </summary>
    /// <seealso cref="IIndexBuffer" />
    class LITEFX_RENDERING_API IIndexBufferLayout : public IBufferLayout {
    public:
        virtual ~IIndexBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the index type of the index buffer.
        /// </summary>
        /// <returns>The index type of the index buffer.</returns>
        virtual IndexType indexType() const noexcept = 0;
    };

    /// <summary>
    /// Describes a the layout of a single descriptor within a <see cref="DescriptorSet" />.
    /// </summary>
    /// <remarks>
    /// A common metaphor for a descriptor to think of it as a "pointer for the GPU". Basically, a descriptor points to a buffer in a shader. A descriptor 
    /// can have different types and sizes. The types a descriptor can have are described by the <see cref="DescriptorType" />.
    /// 
    /// If the descriptor is a sampler, it can either be a dynamic or static sampler. A dynamic sampler needs to be bound during runtime just like any other
    /// descriptor by calling <see cref="IDescriptorSet::update" />. A static sampler is defined alongside the descriptor layout and is automatically set
    /// when the pipeline that uses the descriptor layout gets bound. In this case, the descriptor must not be updated with another sampler. If a descriptor
    /// layout describes a static sampler, the <see cref="IDescriptorLayout::staticSampler" /> returns a pointer to the static sampler state.
    /// 
    /// Typically, a descriptor "points" to a singular buffer, i.e. a scalar. However, a descriptor can also resemble an array. In this case,
    /// <see cref="IDescriptorLayout::descriptors" /> returns the number of elements in the array. If it returns `-1` (or `0xFFFFFFFF`), the descriptor 
    /// array is called `unbounded`. In this case, the number of descriptors in the array can be specified when allocating the descriptor set. Unbounded
    /// descriptor arrays behave different to normal descriptor arrays in different ways. They are typically used for bindless descriptors. If a descriptor
    /// represents an unbounded array, it must be the only descriptor in this descriptor set. Furthermore, unbounded arrays are not cached by the descriptor
    /// set layout. Descriptors within unbounded arrays may be updated after binding them to a command buffer. However, this must be done with special care,
    /// to prevent descriptors that are in use to be overwritten. For more information on how to manage unbounded arrays, refer to 
    /// <see cref="IDescriptorSetLayout::allocate" />.
    /// </remarks>
    /// <seealso cref="DescriptorSetLayout" />
    class LITEFX_RENDERING_API IDescriptorLayout : public IBufferLayout {
    public:
        virtual ~IDescriptorLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the descriptor.
        /// </summary>
        /// <returns>The type of the descriptor.</returns>
        virtual DescriptorType descriptorType() const noexcept = 0;

        /// <summary>
        /// Returns the number of descriptors in the descriptor array, or `-1` if the array is unbounded.
        /// </summary>
        /// <remarks>
        /// If the number of descriptors is `-1` (or `0xFFFFFFFF`), the descriptor array is unbounded. In that case, the size of the array must be specified,
        /// when allocating the descriptor set. This can be done by specifying the `descriptors` parameter when calling 
        /// <see cref="IDescriptorSetLayout::allocate" />.
        /// </remarks>
        /// <returns>The number of descriptors in the descriptor array, or `-1` if the array is unbounded.</returns>
        /// <seealso cref="IDescriptorLayout" />
        virtual UInt32 descriptors() const noexcept = 0;

        /// <summary>
        /// If the descriptor describes a static sampler, this method returns the state of the sampler. Otherwise, it returns <c>nullptr</c>.
        /// </summary>
        /// <remarks>
        /// Static samplers are called immutable samplers in Vulkan and describe sampler states, that are defined along the pipeline layout. While they do
        /// occupy a descriptor, they must not be bound explicitly. Instead, static samplers are automatically bound if the pipeline gets used. If a static
        /// sampler is set, the <see cref="descriptorType" /> must be set to <see cref="DescriptorType::Sampler" />.
        /// </remarks>
        /// <returns>The state of the static sampler, or <c>nullptr</c>, if the descriptor is not a static sampler.</returns>
        virtual const ISampler* staticSampler() const noexcept = 0;
    };

    /// <summary>
    /// Allows for data to be mapped into the object.
    /// </summary>
    class LITEFX_RENDERING_API IMappable {
    public:
        virtual ~IMappable() noexcept = default;

    public:
        /// <summary>
        /// Maps the memory at <paramref name="data" /> to the internal memory of this object.
        /// </summary>
        /// <param name="data">The address that marks the beginning of the data to map.</param>
        /// <param name="size">The number of bytes to map.</param>
        /// <param name="element">The array element to map the data to.</param>
        virtual void map(const void* const data, size_t size, UInt32 element = 0) = 0;

        /// <summary>
        /// Maps the memory blocks within <paramref name="data" /> to the internal memory of an array.
        /// </summary>
        /// <param name="data">The data blocks to map.</param>
        /// <param name="size">The size of each data block within <paramref name="data" />.</param>
        /// <param name="firsElement">The first element of the array to map.</param>
        virtual void map(Span<const void* const> data, size_t elementSize, UInt32 firstElement = 0) = 0;

        /// <summary>
        /// Maps the memory at <paramref name="data" /> to the internal memory of this object.
        /// </summary>
        /// <param name="data">The address that marks the beginning of the data to map.</param>
        /// <param name="size">The number of bytes to map.</param>
        /// <param name="element">The array element to map the data to.</param>
        /// <param name="write">If `true`, <paramref name="data" /> is copied into the internal memory. If `false` the internal memory is copied into <paramref name="data" />.</param>
        virtual void map(void* data, size_t size, UInt32 element = 0, bool write = true) = 0;

        /// <summary>
        /// Maps the memory blocks within <paramref name="data" /> to the internal memory of an array.
        /// </summary>
        /// <param name="data">The data blocks to map.</param>
        /// <param name="size">The size of each data block within <paramref name="data" />.</param>
        /// <param name="firsElement">The first element of the array to map.</param>
        /// <param name="write">If `true`, <paramref name="data" /> is copied into the internal memory. If `false` the internal memory is copied into <paramref name="data" />.</param>
        virtual void map(Span<void*> data, size_t elementSize, UInt32 firstElement = 0, bool write = true) = 0;
    };

    /// <summary>
    /// Describes a chunk of device memory.
    /// </summary>
    class LITEFX_RENDERING_API IDeviceMemory {
    public:
        virtual ~IDeviceMemory() noexcept = default;

    public:
        /// <summary>
        /// Gets the number of sub-resources inside the memory chunk.
        /// </summary>
        /// <remarks>
        /// For buffers, this equals the number of array elements. For images, this equals the product of layers, levels and planes. This number represents the number of states, that 
        /// can be obtained by calling the <see cref="state" /> method.
        /// </remarks>
        /// <returns>The number of array elements inside the memory chunk.</returns>
        /// <seealso cref="state" />
        virtual UInt32 elements() const noexcept = 0;

        /// <summary>
        /// Gets the size (in bytes) of the aligned memory chunk.
        /// </summary>
        /// <remarks>
        /// The size of the device memory block depends on different factors. The actual used memory of one element can be obtained by calling by the <see cref="elementSize" />. For 
        /// different reasons, though, elements may be required to be aligned to a certain size. The size of one aligned element is returned by <see cref="alignedElementSize" />.
        /// The size of the memory block, the elements get aligned to is returned by <see cref="elementAlignment" />.
        /// </remarks>
        /// <returns>The size (in bytes) of the memory chunk.</returns>
        /// <seealso cref="elements" />
        /// <seealso cref="elementSize" />
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t size() const noexcept = 0;

        /// <summary>
        /// Returns the size of a single element within the buffer. If there is only one element, this is equal to <see cref="size" />.
        /// </summary>
        /// <remarks>
        /// For images, this method will return a value that equals the result of <see cref="size" />.
        /// </remarks>
        /// <returns>The size of a single element within the buffer</returns>
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t elementSize() const noexcept = 0;

        /// <summary>
        /// Returns the alignment of a single element.
        /// </summary>
        /// <returns>The alignment of a single element.</returns>
        /// <seealso cref="elementSize" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t elementAlignment() const noexcept = 0;

        /// <summary>
        /// Returns the actual size of the element in device memory.
        /// </summary>
        /// <remarks>
        /// For images, this method will return a value that equals the result of <see cref="size" />, aligned by <see cref="elementAlignment" />.
        /// </remarks>
        /// <returns>The actual size of the element in device memory.</returns>
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="elementSize" />
        virtual size_t alignedElementSize() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the resource can be bound to a read/write descriptor.
        /// </summary>
        /// <remarks>
        /// If the resource is not writable, attempting to bind it to a writable descriptor will result in an exception.
        /// </remarks>
        /// <returns><c>true</c>, if the resource can be bound to a read/write descriptor.</returns>
        virtual bool writable() const noexcept = 0;
    };

    /// <summary>
    /// Base interface for buffer objects.
    /// </summary>
    class LITEFX_RENDERING_API IBuffer : public virtual IDeviceMemory, public virtual IMappable, public virtual IStateResource {
    public:
        virtual ~IBuffer() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the buffer.
        /// </summary>
        /// <returns>The type of the buffer.</returns>
        virtual BufferType type() const noexcept = 0;
    };

    /// <summary>
    /// Describes a generic image.
    /// </summary>
    class LITEFX_RENDERING_API IImage : public virtual IDeviceMemory, public virtual IStateResource {
    public:
        virtual ~IImage() noexcept = default;

    public:
        /// <summary>
        /// Returns the size (in bytes) of an image at a specified mip map level. If the image does not contain the provided mip map level, the method returns <c>0</c>.
        /// </summary>
        /// <remarks>
        /// Note that the size will only be returned for one layer. You have to multiply this value by the number of layers, if you want to receive the size of all layers
        /// of a certain mip-map level. This is especially important, if you use cube mapping, because this method will only return the size of one face.
        /// </remarks>
        /// <param name="level">The mip map level to return the size for.</param>
        /// <returns>The size (in bytes) of an image at a specified mip map level.</returns>
        virtual size_t size(UInt32 level) const noexcept = 0;

        /// <summary>
        /// Gets the extent of the image at a certain mip-map level.
        /// </summary>
        /// <remarks>
        /// Not all components of the extent are actually used. Check the <see cref="dimensions" /> to see, which components are required. The extent will be 0 for invalid 
        /// mip-map levels and 1 or more for valid mip map levels.
        /// </remarks>
        /// <returns>The extent of the image at a certain mip-map level.</returns>
        /// <seealso cref="dimensions" />
        virtual Size3d extent(UInt32 level = 0) const noexcept = 0;

        /// <summary>
        /// Gets the internal format of the image.
        /// </summary>
        /// <returns>The internal format of the image.</returns>
        virtual Format format() const noexcept = 0;

        /// <summary>
        /// Gets the images dimensionality.
        /// </summary>
        /// <remarks>
        /// The dimensions imply various things, most importantly, which components of the <see cref="extent" /> are used. Note that cube maps behave like 2D images when the 
        /// extent is used.
        /// </remarks>
        /// <returns>The images dimensionality.</returns>
        virtual ImageDimensions dimensions() const noexcept = 0;

        /// <summary>
        /// Gets the number of mip-map levels of the image.
        /// </summary>
        /// <returns>The number of mip-map levels of the image.</returns>
        virtual UInt32 levels() const noexcept = 0;

        /// <summary>
        /// Gets the number of layers (slices) of the image.
        /// </summary>
        /// <returns>The number of layers (slices) of the image.</returns>
        virtual UInt32 layers() const noexcept = 0;

        /// <summary>
        /// Returns the number of planes of the image resource.
        /// </summary>
        /// <remarks>
        /// The number of planes is dictated by the image format.
        /// </remarks>
        /// <returns>The number of planes of the image resource.</returns>
        /// <seealso cref="format" />
        virtual UInt32 planes() const noexcept = 0;

        /// <summary>
        /// Gets the number of samples of the texture.
        /// </summary>
        /// <returns>The number of samples of the texture.</returns>
        virtual MultiSamplingLevel samples() const noexcept = 0;

        /// <summary>
        /// Returns the current image layout.
        /// </summary>
        /// <param name="subresource">The sub-resource ID for which to return the layout.</param>
        /// <returns>The current image layout.</returns>
        /// <seealso cref="subresourceId" />
        virtual ImageLayout layout(UInt32 subresource = 0) const = 0;

        // TODO: getSampler() for combined samplers?

    public:
        /// <summary>
        /// Returns the sub-resource ID for a combination of mip-map <paramref name="level" />, array <paramref name="layer" /> and <paramref name="plane" />.
        /// </summary>
        /// <param name="level">The mip map level of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <returns>The sub-resource ID for the sub-resource.</returns>
        /// <seealso cref="resolveSubresource" />
        inline virtual UInt32 subresourceId(UInt32 level, UInt32 layer, UInt32 plane) const noexcept {
            return level + (layer * this->levels()) + (plane * this->levels() * this->layers());
        }

        /// <summary>
        /// Returns the <paramref name="plane" />, <paramref name="layer" /> and <paramref name="level" /> for the provided <paramref name="subresource" />.
        /// </summary>
        /// <param name="subresource">The sub-resource ID.</param>
        /// <param name="plane">The plane index of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="level">The mip-map level of the sub-resource.</param>
        /// <seealso cref="subresourceId" />
        inline virtual void resolveSubresource(UInt32 subresource, UInt32& plane, UInt32& layer, UInt32& level) const noexcept {
            const auto levels = this->levels();
            const UInt32 resourcesPerPlane = levels * this->layers();
            plane = subresource / resourcesPerPlane;
            layer = (subresource % resourcesPerPlane) / levels;
            level = subresource % levels;
        }
    };

    /// <summary>
    /// Describes a texture sampler.
    /// </summary>
    class LITEFX_RENDERING_API ISampler : public virtual IStateResource {
    public:
        virtual ~ISampler() noexcept = default;

    public:
        /// <summary>
        /// Gets the filtering mode that is used for minifying lookups.
        /// </summary>
        /// <returns>The filtering mode that is used for minifying lookups.</returns>
        virtual FilterMode getMinifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the filtering mode that is used for magnifying lookups.
        /// </summary>
        /// <returns>The filtering mode that is used for magnifying lookups.</returns>
        virtual FilterMode getMagnifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the horizontal border.
        /// </summary>
        /// <returns>The addressing mode at the horizontal border.</returns>
        virtual BorderMode getBorderModeU() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the vertical border.
        /// </summary>
        /// <returns>The addressing mode at the vertical border.</returns>
        virtual BorderMode getBorderModeV() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the depth border.
        /// </summary>
        /// <returns>The addressing mode at the depth border.</returns>
        virtual BorderMode getBorderModeW() const noexcept = 0;

        /// <summary>
        /// Gets the anisotropy value used when sampling this texture.
        /// </summary>
        /// <remarks>
        /// Anisotropy will be disabled, if this value is set to <c>0.0</c>.
        /// </remarks>
        /// <returns>The anisotropy value used when sampling this texture.</returns>
        virtual Float getAnisotropy() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map selection mode.
        /// </summary>
        /// <returns>The mip-map selection mode.</returns>
        virtual MipMapMode getMipMapMode() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map level of detail bias.
        /// </summary>
        /// <returns>The mip-map level of detail bias.</returns>
        virtual Float getMipMapBias() const noexcept = 0;

        /// <summary>
        /// Gets the maximum texture level of detail.
        /// </summary>
        /// <returns>The maximum texture level of detail.</returns>
        virtual Float getMaxLOD() const noexcept = 0;

        /// <summary>
        /// Gets the minimum texture level of detail.
        /// </summary>
        /// <returns>The minimum texture level of detail.</returns>
        virtual Float getMinLOD() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a barrier.
    /// </summary>
    /// <remarks>
    /// Barriers are used to synchronize the GPU with itself in a command buffer. They are basically used to control the GPU command flow and ensure that resources are in 
    /// he right state before using them. Generally speaking, there are two related types of barriers:
    /// 
    /// - **Execution barriers** are enforcing command order by telling the GPU to wait for certain pipeline stages or which stages to block until an operation has finished.
    /// - **Memory barriers** are used to transition resources between states and are a subset of execution barriers. Each memory barrier is always also an execution barrier,
    ///   but the opposite is not true.
    /// 
    /// An execution barrier is simply a barrier without any *resource transitions* happening. The only properties that are set for an execution barrier are two pipeline 
    /// stages defined by <see cref="IBarrier::syncBefore" /> and <see cref="IBarrier::syncAfter" />. The first value defines the pipeline stages, all *previous* commands in 
    /// a command buffer need to finish before execution is allowed to continue. Similarly, the second value defines the stage, all *subsequent* commands need to wait for, 
    /// before they are allowed to continue execution. For example, setting `syncBefore` to `Compute` and `syncAfter` to `Vertex` logically translates to: *All subsequent 
    /// commands that want to pass the vertex stage need to wait there before all previous commands passed the compute stage*. To synchronize reads in previous commands with
    /// writes in subsequent commands, this is sufficient. However, in order to do the opposite, this is not enough. Instead resource memory needs to be *transitioned* by
    /// specifying the desired <see cref="ResourceAccess" />, alongside the <see cref="ImageLayout" /> for images (note that buffers always share a *common* layout that can
    /// not be changed). This is done using memory barriers. There are two types of memory barriers used for state transitions:
    /// 
    /// - **Global barriers** apply to all resource memory.
    /// - **Image and buffer barriers** apply to individual images or buffers or a sub-resource of those.
    /// 
    /// Any `IBarrier` can contain an arbitrary mix of one or more global and/or image/buffer barriers. A global barrier is inserted by calling <see cref="IBarrier::wait" />.
    /// This method accepts two parameters: a `before` and an `after` access mode. Those parameters specify the <see cref="ResourceAccess" /> for the previous and subsequent 
    /// commands. This makes it possible to describe scenarios like *wait for certain writes to finish before continuing with certain reads*. Note that a resource can be
    /// accessed in different ways at the same time (for example as copy source and shader resource) and specifying an access state will only wait for the specified subset. 
    /// As a rule of thumb, you should always specify as little access as possible in order to leave most room for optimization.
    /// 
    /// Image and buffer barriers additionally describe which (sub-)resources to apply the barrier to. For buffers this only applies to individual elements in a buffer array.
    /// However, due to [driver restrictions](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#buffer-barriers), buffers are always transitioned as a
    /// whole. This is different from image resources, which have addressable sub-resources (mip levels, planes and array elements). For images, it is possible to transition 
    /// individual sub-resources into different <see cref="ImageLayout" />s to indicate when and how a texture is used. An image in a certain layout poses restrictions on how
    /// it can be accessed. For example, a `ReadWrite` image written by a compute shader must be transitioned into a proper layout to be read by a graphics shader. To 
    /// facilitate such a transition, a barrier is required. Image barriers can be inserted by calling one of the overloads of <see cref="IBarrier::transition" /> that accepts
    /// an <see cref="IImage" /> parameter.
    /// </remarks>
    /// <seealso cref="PipelineStage" />
    /// <seealso cref="IBuffer" />
    /// <seealso cref="ResourceAccess" />
    /// <seealso cref="IImage" />
    /// <seealso cref="ImageLayout" />
    /// <seealso cref="ICommandBuffer" />
    class LITEFX_RENDERING_API IBarrier {
    public:
        virtual ~IBarrier() noexcept = default;

    public:
        /// <summary>
        /// Returns the stage that all previous commands need to reach before continuing execution.
        /// </summary>
        /// <returns>The stage that all previous commands need to reach before continuing execution.</returns>
        constexpr inline virtual PipelineStage syncBefore() const noexcept = 0;
        
        /// <summary>
        /// Returns the stage all subsequent commands need to wait for before continuing execution.
        /// </summary>
        /// <returns>The stage all subsequent commands need to wait for before continuing execution.</returns>
        constexpr inline virtual PipelineStage syncAfter() const noexcept = 0;

        /// <summary>
        /// Inserts a global barrier that waits for previous commands to finish accesses described by <paramref name="accessBefore" /> before subsequent commands can continue
        /// with accesses described by <paramref name="accessAfter" />.
        /// </summary>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        constexpr inline virtual void wait(ResourceAccess accessBefore, ResourceAccess accessAfter) noexcept = 0;

        /// <summary>
        /// Inserts a buffer barrier that blocks access to <paramref name="buffer"/> of types contained in <paramref name="accessAfter" /> for subsequent commands until 
        /// previous commands have finished accesses contained in <paramref name="accessBefore" />.
        /// </summary>
        /// <param name="buffer">The buffer resource to transition.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        constexpr inline void transition(IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) {
            this->doTransition(buffer, accessBefore, accessAfter);
        };

        /// <summary>
        /// Inserts a buffer barrier that blocks access to a <paramref name="buffer"/>s <paramref name="element" /> of types contained in <paramref name="accessAfter" /> for 
        /// subsequent commands until previous commands have finished accesses contained in <paramref name="accessBefore" />.
        /// </summary>
        /// <remarks>
        /// Due to [driver restrictions](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#buffer-barriers), this is overload is currently redundant, 
        /// but might be available in the future. Currently, calling this method ignores the <paramref name="element" /> parameter and transitions the whole buffer.
        /// </remarks>
        /// <param name="buffer">The buffer resource to transition.</param>
        /// <param name="element">The element of the resource to transition.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        constexpr inline void transition(IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) {
            this->doTransition(buffer, element, accessBefore, accessAfter);
        }

        /// <summary>
        /// Inserts an image barrier that blocks access to all sub-resources of <paramref name="image"/> of the types contained in <paramref name="accessAfter" /> for 
        /// subsequent commands until previous commands have finished accesses contained in <paramref name="accessBefore" /> and transitions all sub-resources into
        /// <paramref name="layout" />.
        /// </summary>
        /// <param name="image">The image resource to transition.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        /// <param name="layout">The image layout to transition into.</param>
        constexpr inline void transition(IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
            this->doTransition(image, accessBefore, accessAfter, layout);
        }

        /// <summary>
        /// Inserts an image barrier that blocks access to a sub-resource range of <paramref name="image"/> of the types contained in <paramref name="accessAfter" /> for 
        /// subsequent commands until previous commands have finished accesses contained in <paramref name="accessBefore" /> and transitions the sub-resource into
        /// <paramref name="layout" />.
        /// </summary>
        /// <param name="image">The image resource to transition.</param>
        /// <param name="level">The base mip-map level of the sub-resource range.</param>
        /// <param name="levels">The number of mip-map levels of the sub-resource range.</param>
        /// <param name="layer">The base array layer of the sub-resource range.</param>
        /// <param name="layers">The number of array layer of the sub-resource range.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        /// <param name="layout">The image layout to transition into.</param>
        constexpr inline void transition(IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
            this->doTransition(image, level, levels, layer, layers, plane, accessBefore, accessAfter, layout);
        }

        /// <summary>
        /// Inserts an image barrier that blocks access to all sub-resources of <paramref name="image"/> of the types contained in <paramref name="accessAfter" /> for 
        /// subsequent commands until previous commands have finished accesses contained in <paramref name="accessBefore" /> and transitions all sub-resources into
        /// <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// This overload let's you explicitly specify the <paramref name="fromLayout" />. This is required, if you use any external transition mechanism that causes the engine
        /// to lose track of the image layout. If you are not running into issues with the other overloads, you probably do not want to call this method.
        /// </remarks>
        /// <param name="image">The image resource to transition.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        /// <param name="fromLayout">The image layout to transition from.</param>
        /// <param name="toLayout">The image layout to transition into.</param>
        constexpr inline void transition(IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
            this->doTransition(image, accessBefore, accessAfter, fromLayout, toLayout);
        }

        /// <summary>
        /// Inserts an image barrier that blocks access to a sub-resource range of <paramref name="image"/> of the types contained in <paramref name="accessAfter" /> for 
        /// subsequent commands until previous commands have finished accesses contained in <paramref name="accessBefore" /> and transitions the sub-resource into
        /// <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// This overload let's you explicitly specify the <paramref name="fromLayout" />. This is required, if you use any external transition mechanism that causes the engine
        /// to lose track of the image layout. If you are not running into issues with the other overloads, you probably do not want to call this method.
        /// </remarks>
        /// <param name="image">The image resource to transition.</param>
        /// <param name="level">The base mip-map level of the sub-resource range.</param>
        /// <param name="levels">The number of mip-map levels of the sub-resource range.</param>
        /// <param name="layer">The base array layer of the sub-resource range.</param>
        /// <param name="layers">The number of array layer of the sub-resource range.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="accessBefore">The access types previous commands have to finish.</param>
        /// <param name="accessAfter">The access types that subsequent commands continue with.</param>
        /// <param name="fromLayout">The image layout to transition from.</param>
        /// <param name="toLayout">The image layout to transition into.</param>
        constexpr inline void transition(IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
            this->doTransition(image, level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
        }

    private:
        constexpr inline virtual void doTransition(IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr inline virtual void doTransition(IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr inline virtual void doTransition(IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr inline virtual void doTransition(IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
        constexpr inline virtual void doTransition(IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr inline virtual void doTransition(IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
    };

    /// <summary>
    /// The interface for a descriptor set.
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorSet {
    public:
        virtual ~IDescriptorSet() noexcept = default;

    public:
        /// <summary>
        /// Updates a constant buffer within the current descriptor set.
        /// </summary>
        /// <param name="binding">The buffer binding point.</param>
        /// <param name="buffer">The constant buffer to write to the descriptor set.</param>
        /// <param name="bufferElement">The index of the first element in the buffer to bind to the descriptor set.</param>
        /// <param name="elements">The number of elements from the buffer to bind to the descriptor set. A value of `0` binds all available elements, starting at <paramref name="bufferElement" />.</param>
        /// <param name="firstDescriptor">The index of the first descriptor in the descriptor array to update.</param>
        void update(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0) const {
            this->doUpdate(binding, buffer, bufferElement, elements, firstDescriptor);
        }

        /// <summary>
        /// Updates a texture within the current descriptor set.
        /// </summary>
        /// <remarks>
        /// The exact representation of the level and layer parameters depends on the dimension of the provided texture, as well as the type of the descriptor identified by the 
        /// <paramref name="binding" /> parameter.
        /// 
        /// If the texture itself is not an array (i.e. the number of layers equals `1`), the parameters <paramref name="firstLayer" /> and <paramref name="layers" /> are ignored.
        /// 
        /// The descriptor type dictates, how mip-maps can be provided. If the descriptor type identifies a *writable texture*, the <paramref name="firstLevel" /> parameter specifies 
        /// the mip-map level to write to (or read from). Multiple levels are not allowed in this case, so the <paramref name="levels" /> parameter is ignored. Instead, you have to 
        /// bind them to separate descriptors. Furthermore, the <paramref name="firstLayer" /> and <paramref name="layers" /> parameter can be used to specify the number of depth
        /// or W-slices of a writable 3D texture or the side(s) of a cube map.
        /// </remarks>
        /// <param name="binding">The texture binding point.</param>
        /// <param name="texture">The texture to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the texture to.</param>
        /// <param name="firstLevel">The index of the first mip-map level to bind.</param>
        /// <param name="levels">The number of mip-map levels to bind. A value of `0` binds all available levels, starting at <paramref name="firstLevel" />.</param>
        /// <param name="firstLayer">The index of the first layer to bind.</param>
        /// <param name="layers">The number of layers to bind. A value of `0` binds all available layers, starting at <paramref name="firstLayer" />.</param>
        void update(UInt32 binding, const IImage& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const {
            this->doUpdate(binding, texture, descriptor, firstLevel, levels, firstLayer, layers);
        }

        /// <summary>
        /// Updates a sampler within the current descriptor set.
        /// </summary>
        /// <param name="binding">The sampler binding point.</param>
        /// <param name="sampler">The sampler to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the sampler to.</param>
        void update(UInt32 binding, const ISampler& sampler, UInt32 descriptor = 0) const {
            this->doUpdate(binding, sampler, descriptor);
        }

        /// <summary>
        /// Attaches an image as an input attachment to a descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="image">The image to bind to the input attachment descriptor.</param>
        void attach(UInt32 binding, const IImage& image) const {
            this->doAttach(binding, image);
        }

    private:
        virtual void doUpdate(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const = 0;
        virtual void doUpdate(UInt32 binding, const IImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const = 0;
        virtual void doUpdate(UInt32 binding, const ISampler& sampler, UInt32 descriptor) const = 0;
        virtual void doAttach(UInt32 binding, const IImage& image) const = 0;
    };

    /// <summary>
    /// Describes a resource binding to a descriptor or descriptor set.
    /// </summary>
    /// <seealso cref="IDescriptorSet" />
    /// <seealso cref="IDescriptorSetLayout" />
    struct LITEFX_RENDERING_API DescriptorBinding {
    public:
        using resource_container = Variant<Ref<IBuffer>, Ref<IImage>, Ref<ISampler>>;
        
    public:
        /// <summary>
        /// The binding point to bind the resource at.
        /// </summary>
        UInt32 binding;

        /// <summary>
        /// The resource to bind.
        /// </summary>
        /// <seealso cref="IBuffer" />
        /// <seealso cref="IImage" />
        /// <seealso cref="ISampler" />
        resource_container resource;

        /// <summary>
        /// The index of the descriptor in a descriptor array at which binding the resource arrays starts.
        /// </summary>
        /// <remarks>
        /// If the resource contains an array, the individual elements (*layers* for images) will be be bound, starting at this descriptor. The first element/layer to be
        /// bound is identified by <see cref="firstElement" />. The number of elements/layers to be bound is stored in <see cref="elements" />.
        /// </remarks>
        /// <seealso cref="firstElement" />
        /// <seealso cref="elements" />
        UInt32 firstDescriptor = 0;

        /// <summary>
        /// The index of the first array element or image layer to bind, starting at <see cref="firstDescriptor" />.
        /// </summary>
        /// <remarks>
        /// This property is ignored, if the resource is a <see cref="ISampler" />.
        /// </remarks>
        /// <seealso cref="firstDescriptor" />
        UInt32 firstElement = 0;

        /// <summary>
        /// The number of array elements or image layers to bind, starting at <see cref="firstDescriptor" />.
        /// </summary>
        /// <remarks>
        /// This property is ignored, if the resource is a <see cref="ISampler" />.
        /// </remarks>
        /// <seealso cref="firstDescriptor" />
        UInt32 elements = 0;

        /// <summary>
        /// If the resource is an image, this describes the first level to be bound.
        /// </summary>
        /// <remarks>
        /// This property is ignored, if the resource is a <see cref="ISampler" /> or <see cref="IBuffer" />.
        /// </remarks>
        UInt32 firstLevel = 0;

        /// <summary>
        /// If the resource is an image, this describes the number of levels to be bound.
        /// </summary>
        /// <remarks>
        /// This property is ignored, if the resource is a <see cref="ISampler" /> or <see cref="IBuffer" />.
        /// </remarks>
        UInt32 levels = 0;
    };

    /// <summary>
    /// The interface for a descriptor set layout.
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorSetLayout {
    public:
        virtual ~IDescriptorSetLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the layouts of the descriptors within the descriptor set.
        /// </summary>
        /// <returns>The layouts of the descriptors within the descriptor set.</returns>
        inline Enumerable<const IDescriptorLayout*> descriptors() const noexcept {
            return this->getDescriptors();
        }

        /// <summary>
        /// Returns the descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the requested descriptor layout.</param>
        /// <returns>The descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.</returns>
        virtual const IDescriptorLayout& descriptor(UInt32 binding) const = 0;

        /// <summary>
        /// Returns the space index of the descriptor set.
        /// </summary>
        /// <remarks>
        /// The descriptor set space maps to the space index in HLSL and the set index in GLSL.
        /// </remarks>
        /// <returns>The space index of the descriptor set.</returns>
        virtual UInt32 space() const noexcept = 0;

        /// <summary>
        /// Returns the shader stages, the descriptor set is used in.
        /// </summary>
        /// <returns>The shader stages, the descriptor set is used in.</returns>
        virtual ShaderStage shaderStages() const noexcept = 0;

        /// <summary>
        /// Returns the number of uniform/constant buffer descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of uniform/constant buffer descriptors.</returns>
        virtual UInt32 uniforms() const noexcept = 0;

        /// <summary>
        /// Returns the number of structured and byte address buffer descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of structured and byte address buffer descriptors.</returns>
        virtual UInt32 storages() const noexcept = 0;

        /// <summary>
        /// Returns the number of image (i.e. texture) descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of image (i.e. texture) descriptors.</returns>
        virtual UInt32 images() const noexcept = 0;

        /// <summary>
        /// Returns the number of texel buffer descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of texel buffer descriptors.</returns>
        virtual UInt32 buffers() const noexcept = 0;

        /// <summary>
        /// Returns the number of dynamic sampler descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of dynamic sampler descriptors.</returns>
        /// <seealso cref="staticSamplers" />
        virtual UInt32 samplers() const noexcept = 0;

        /// <summary>
        /// Returns the number of static or immutable sampler descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of static or immutable sampler descriptors.</returns>
        /// <seealso cref="samplers" />
        virtual UInt32 staticSamplers() const noexcept = 0;

        /// <summary>
        /// Returns the number of input attachment descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of input attachment descriptors.</returns>
        virtual UInt32 inputAttachments() const noexcept = 0;

    public:
        /// <summary>
        /// Allocates a new descriptor set or returns an instance of an unused descriptor set.
        /// </summary>
        /// <param name="bindings">Optional default bindings for descriptors in the descriptor set.</param>
        /// <remarks>
        /// Allocating a new descriptor set may be an expensive operation. To improve performance, and prevent fragmentation, the descriptor set layout keeps track of
        /// created descriptor sets. It does this by never releasing them. Instead, when a <see cref="DescriptorSet" /> instance gets destroyed, it should call 
        /// <see cref="free" /> in order to mark itself (i.e. its handle) as not being used any longer.
        /// 
        /// Before allocating a new descriptor set from a pool (which may even result in the creation of a new pool, if the existing pools are full), the layout tries 
        /// to hand out descriptor sets that marked as unused. Descriptor sets are only deleted, if the whole layout instance and therefore the descriptor pools are 
        /// deleted.
        /// 
        /// The above does not apply to unbounded descriptor arrays. A unbounded descriptor array is one, for which <see cref="IDescriptorLayout::descriptors" /> 
        /// returns `-1` (or `0xFFFFFFFF`). They must be allocated by specifying the <paramref name="descriptors" /> parameter. This parameter defines the number of
        /// descriptors to allocate in the array. 
        /// 
        /// Note that descriptor sets, that contain an unbounded descriptor array must only contain one single descriptor (the one that identifies this array). Such 
        /// descriptor sets are never cached. Instead, they are released when calling <see cref="free" />. It is a good practice to cache such descriptor sets as 
        /// global descriptor tables once and never release them. They provide more flexibility than regular descriptor arrays, since they may be updated, even after
        /// they have been bound to a command buffer or from different threads. However, you must ensure yourself not to overwrite any descriptors that are currently
        /// in use. Because unbounded arrays are not cached, freeing and re-allocating such descriptor sets may leave the descriptor heap fragmented, which might cause
        /// the allocation to fail, if the heap is full.
        /// </remarks>
        /// <returns>The instance of the descriptor set.</returns>
        /// <seealso cref="IDescriptorLayout" />
        inline UniquePtr<IDescriptorSet> allocate(const Enumerable<DescriptorBinding>& bindings = { }) const {
            return this->allocate(0, bindings);
        }

        /// <summary>
        /// Allocates a new descriptor set or returns an instance of an unused descriptor set.
        /// </summary>
        /// <param name="descriptors">The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.</param>
        /// <param name="bindings">Optional default bindings for descriptors in the descriptor set.</param>
        /// <returns>The instance of the descriptor set.</returns>
        /// <seealso cref="IDescriptorLayout" />
        inline UniquePtr<IDescriptorSet> allocate(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const {
            return this->getDescriptorSet(descriptors, bindings);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="bindings">Optional default bindings for descriptors in each descriptor set.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        inline Enumerable<UniquePtr<IDescriptorSet>> allocateMultiple(UInt32 descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const {
            return this->allocateMultiple(descriptorSets, 0, bindings);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="bindingFactory">A factory function that is called for each descriptor set in order to provide the default bindings.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        inline Enumerable<UniquePtr<IDescriptorSet>> allocateMultiple(UInt32 descriptorSets, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const {
            return this->allocateMultiple(descriptorSets, 0, bindingFactory);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="descriptors">The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.</param>
        /// <param name="bindings">Optional default bindings for descriptors in each descriptor set.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        inline Enumerable<UniquePtr<IDescriptorSet>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const {
            return this->getDescriptorSets(descriptorSets, descriptors, bindings);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="descriptors">The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.</param>
        /// <param name="bindingFactory">A factory function that is called for each descriptor set in order to provide the default bindings.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        inline Enumerable<UniquePtr<IDescriptorSet>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const {
            return this->getDescriptorSets(descriptorSets, descriptors, bindingFactory);
        }

        /// <summary>
        /// Marks a descriptor set as unused, so that it can be handed out again instead of allocating a new one.
        /// </summary>
        /// <seealso cref="allocate" />
        inline void free(const IDescriptorSet& descriptorSet) const noexcept {
            this->releaseDescriptorSet(descriptorSet);
        }

    private:
        virtual Enumerable<const IDescriptorLayout*> getDescriptors() const noexcept = 0;
        virtual UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const = 0;
        virtual Enumerable<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const = 0;
        virtual Enumerable<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const = 0;
        virtual void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const noexcept = 0;
    };

    /// <summary>
    /// Describes a range within a <see cref="IPushConstantsLayout" />.
    /// </summary>
    class LITEFX_RENDERING_API IPushConstantsRange {
    public:
        virtual ~IPushConstantsRange() noexcept = default;

    public:
        /// <summary>
        /// Returns the shader space the push constants can be accessed from.
        /// </summary>
        /// <returns>The shader space the push constants can be accessed from.</returns>
        virtual UInt32 space() const noexcept = 0;

        /// <summary>
        /// Returns the binding point or register, the push constants are made available at.
        /// </summary>
        /// <returns>The binding point or register, the push constants are made available at.</returns>
        virtual UInt32 binding() const noexcept = 0;

        /// <summary>
        /// Returns the offset from the push constants backing memory block, the range starts at.
        /// </summary>
        /// <returns>The offset from the push constants backing memory block, the range starts at.</returns>
        /// <seealso cref="size" />
        virtual UInt32 offset() const noexcept = 0;

        /// <summary>
        /// Returns the size (in bytes) of the range.
        /// </summary>
        /// <returns>The size (in bytes) of the range.</returns>
        /// <seealso cref="offset" />
        virtual UInt32 size() const noexcept = 0;

        /// <summary>
        /// Returns the shader stage(s), the range is accessible from.
        /// </summary>
        /// <returns>The shader stage(s), the range is accessible from.</returns>
        virtual ShaderStage stage() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a push constants layout.
    /// </summary>
    class LITEFX_RENDERING_API IPushConstantsLayout {
    public:
        virtual ~IPushConstantsLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the size (in bytes) of the push constants backing memory.
        /// </summary>
        /// <returns>The size (in bytes) of the push constants backing memory.</returns>
        virtual UInt32 size() const noexcept = 0;

        /// <summary>
        /// Returns the push constant range associated with the shader stage provided in <paramref name="stage" />.
        /// </summary>
        /// <param name="stage">The shader stage to request the associated push constant range for. Specifying multiple stages is not supported and will raise an exception.</param>
        /// <returns>The push constant range associated with the provided shader stage.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if no range is mapped to the provided shader stage.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="stage" /> contains multiple shader stages.</exception>
        /// <seealso cref="ranges" />
        virtual const IPushConstantsRange& range(ShaderStage stage) const = 0;

        /// <summary>
        /// Returns all push constant ranges.
        /// </summary>
        /// <returns>All push constant ranges.</returns>
        /// <seealso cref="range" />
        inline Enumerable<const IPushConstantsRange*> ranges() const noexcept {
            return this->getRanges();
        }

    private:
        virtual Enumerable<const IPushConstantsRange*> getRanges() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a shader program.
    /// </summary>
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_RENDERING_API IShaderProgram {
    public:
        virtual ~IShaderProgram() noexcept = default;

    public:
        /// <summary>
        /// Returns the modules, the shader program is build from.
        /// </summary>
        /// <returns>The modules, the shader program is build from.</returns>
        inline Enumerable<const IShaderModule*> modules() const noexcept {
            return this->getModules();
        }

        /// <summary>
        /// Uses shader reflection to extract the pipeline layout of a shader. May not be available in all backends.
        /// </summary>
        /// <remarks>
        /// Note that shader reflection may not yield different results than you would expect, especially when using DirectX 12. For more information on how to use shader
        /// reflection and how to write portable shaders, refer to the [shader development guide](https://github.com/crud89/LiteFX/wiki/Shader-Development) in the wiki.
        /// 
        /// In particular, shader reflection is not able to restore:
        /// 
        /// <list type="bullet">
        /// <item>
        /// <description>
        /// Input attachments in DirectX. Instead, input attachments are treated as <c>DescriptorType::Texture</c>. This is usually not a problem, since DirectX does not
        /// have a concept of render pass outputs/inputs anyway. However, keep this in mind, if you want to filter descriptors based on their type, for example.
        /// </description>
        /// <description>
        /// Immutable sampler states in Vulkan. Those are only restored in DirectX, if an explicit root signature has been provided. For this reason, it is best not to use
        /// them, if you want to use shader reflection.
        /// </description>
        /// </item>
        /// </list>
        /// </remarks>
        /// <returns>The pipeline layout extracted from shader reflection.</returns>
        /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
        inline SharedPtr<IPipelineLayout> reflectPipelineLayout() const {
            return this->parsePipelineLayout();
        };

    private:
        virtual Enumerable<const IShaderModule*> getModules() const noexcept = 0;
        virtual SharedPtr<IPipelineLayout> parsePipelineLayout() const = 0;
    };

    /// <summary>
    /// The interface for a pipeline layout.
    /// </summary>
    class LITEFX_RENDERING_API IPipelineLayout {
    public:
        virtual ~IPipelineLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the descriptor set layout for the descriptor set that is bound to the space provided by <paramref name="space" />.
        /// </summary>
        /// <param name="space">The space to request the descriptor set layout for.</param>
        /// <returns>The descriptor set layout for the descriptor set that is bound to the space provided by <paramref name="space" />.</returns>
        virtual const IDescriptorSetLayout& descriptorSet(UInt32 space) const = 0;

        /// <summary>
        /// Returns all descriptor set layouts, the pipeline has been initialized with.
        /// </summary>
        /// <returns>All descriptor set layouts, the pipeline has been initialized with.</returns>
        inline Enumerable<const IDescriptorSetLayout*> descriptorSets() const noexcept {
            return this->getDescriptorSets();
        }

        /// <summary>
        /// Returns the push constants layout, or <c>nullptr</c>, if the pipeline does not use any push constants.
        /// </summary>
        /// <returns>The push constants layout, or <c>nullptr</c>, if the pipeline does not use any push constants.</returns>
        virtual const IPushConstantsLayout* pushConstants() const noexcept = 0;

    private:
        virtual Enumerable<const IDescriptorSetLayout*> getDescriptorSets() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a vertex buffer.
    /// </summary>
    class LITEFX_RENDERING_API IVertexBuffer : public virtual IBuffer {
    public:
        virtual ~IVertexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the vertex buffer.
        /// </summary>
        /// <returns>The layout of the vertex buffer.</returns>
        virtual const IVertexBufferLayout& layout() const noexcept = 0;
    };

    /// <summary>
    /// The interface for an index buffer.
    /// </summary>
    class LITEFX_RENDERING_API IIndexBuffer : public virtual IBuffer {
    public:
        virtual ~IIndexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the index buffer.
        /// </summary>
        /// <returns>The layout of the index buffer.</returns>
        virtual const IIndexBufferLayout& layout() const noexcept = 0;
    };

    /// <summary>
    /// The interface for an input assembler state.
    /// </summary>
    class LITEFX_RENDERING_API IInputAssembler {
    public:
        virtual ~IInputAssembler() noexcept = default;

    public:
        /// <summary>
        /// Returns all vertex buffer layouts of the input assembly.
        /// </summary>
        /// <returns>All vertex buffer layouts of the input assembly.</returns>
        inline Enumerable<const IVertexBufferLayout*> vertexBufferLayouts() const noexcept {
            return this->getVertexBufferLayouts();
        }

        /// <summary>
        /// Returns the vertex buffer layout for binding provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the vertex buffer layout.</param>
        /// <returns>The vertex buffer layout for binding provided with <paramref name="binding" />.</returns>
        virtual const IVertexBufferLayout& vertexBufferLayout(UInt32 binding) const = 0;

        /// <summary>
        /// Returns the index buffer layout.
        /// </summary>
        /// <returns>The index buffer layout.</returns>
        virtual const IIndexBufferLayout& indexBufferLayout() const = 0;

        /// <summary>
        /// Returns the primitive topology.
        /// </summary>
        /// <returns>The primitive topology.</returns>
        virtual PrimitiveTopology topology() const noexcept = 0;

    private:
        virtual Enumerable<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a pipeline.
    /// </summary>
    class LITEFX_RENDERING_API IPipeline : public virtual IStateResource {
    public:
        virtual ~IPipeline() noexcept = default;

    public:
        /// <summary>
        /// Returns the shader program used by the pipeline.
        /// </summary>
        /// <returns>The shader program used by the pipeline.</returns>
        inline SharedPtr<const IShaderProgram> program() const noexcept {
            return this->getProgram();
        }

        /// <summary>
        /// Returns the layout of the render pipeline.
        /// </summary>
        /// <returns>The layout of the render pipeline.</returns>
        inline SharedPtr<const IPipelineLayout> layout() const noexcept {
            return this->getLayout();
        }

    private:
        virtual SharedPtr<const IShaderProgram> getProgram() const noexcept = 0;
        virtual SharedPtr<const IPipelineLayout> getLayout() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a command buffer.
    /// </summary>
    class LITEFX_RENDERING_API ICommandBuffer {
    public:
        friend class ICommandQueue;

    public:
        virtual ~ICommandBuffer() noexcept = default;

    public:
        /// <summary>
        /// Sets the command buffer into recording state, so that it can receive command that should be submitted to the parent <see cref="CommandQueue" />.
        /// </summary>
        /// <remarks>
        /// Note that you have to wait for a command buffer to be executed on the parent <see cref="CommandQueue" /> before you can begin recording on it again.
        /// </remarks>
        /// <exception cref="RuntimeException">Thrown, if the command buffer is already recording.</exception>
        /// <seealso cref="end" />
        virtual void begin() const = 0;

        /// <summary>
        /// Ends recording commands on the command buffer.
        /// </summary>
        /// <remarks>
        /// It is valid to call this method multiple times. If a command buffer is already closed, nothing will happen.
        /// </remarks>
        /// <seealso cref="begin" />
        virtual void end() const = 0;

        /// <summary>
        /// Returns `true`, if the command buffer is a secondary command buffer, or `false` otherwise.
        /// </summary>
        /// <returns>`true`, if the command buffer is a secondary command buffer, or `false` otherwise.</returns>
        virtual bool isSecondary() const noexcept = 0;

    public:
        /// <summary>
        /// Executes the transitions that have been added to <paramref name="barrier" />.
        /// </summary>
        /// <remarks>
        /// Calling this method will also update the resource states of each resource within the barrier. However, the actual state of the resource does not change until the barrier
        /// is executed on the command queue. Keep this in mind when inserting multiple barriers from different threads or in different command buffers, which may not be executed in 
        /// order. You might have to manually synchronize barrier execution.
        /// </remarks>
        /// <param name="barrier">The barrier containing the transitions to perform.</param>
        inline void barrier(const IBarrier& barrier) const noexcept {
            this->cmdBarrier(barrier);
        }

        /// <summary>
        /// Uses the image at level *0* to generate mip-maps for the remaining levels.
        /// </summary>
        /// <remarks>
        /// It is strongly advised, not to generate mip maps at runtime. Instead, prefer using a format that supports pre-computed mip maps. If you have to, prefer computing
        /// mip maps in a pre-process.
        /// 
        /// Note that not all texture formats and sizes are supported for mip map generation and the result might not be satisfactory. For example, it is not possible to compute 
        /// proper mip maps for pre-compressed formats. Textures should have power of two sizes in order to not appear under-sampled.
        /// 
        /// Note that generating mip maps might require the texture to be writable. You can transfer the texture into a non-writable resource afterwards to improve performance.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to issue the transition and transfer operations.</param>
        inline void generateMipMaps(IImage& image) noexcept {
            this->cmdGenerateMipMaps(image);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="targetElement">The index of the first element in the target buffer to copy to.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target buffer.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        /// <seealso cref="IBarrier" />
        inline void transfer(IBuffer& source, IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, targetElement, elements);
        }
        
        /// <summary>
        /// Performs a buffer-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method takes shared ownership over <paramref name="source" />, which means that a reference is hold until the parent command queue finished using the command buffer. At this point,
        /// the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this
        /// method at some point after the command buffer has been executed.
        /// 
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging
        /// buffer and delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="targetElement">The index of the first element in the target buffer to copy to.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target buffer.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(SharedPtr<IBuffer> source, IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, targetElement, elements);
        }

        /// <summary>
        /// Performs a buffer-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="subresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(IBuffer& source, IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// <summary>
        /// Performs a buffer-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="subresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// 
        /// This method takes shared ownership over <paramref name="source" />, which means that a reference is hold until the parent command queue finished using the command buffer. At this point,
        /// the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this
        /// method at some point after the command buffer has been executed.
        /// 
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging
        /// buffer and delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(SharedPtr<IBuffer> source, IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// <summary>
        /// Performs an image-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetSubresource">The image of the first sub-resource in the target image to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy between the images.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(IImage& source, IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, sourceSubresource, targetSubresource, subresources);
        }

        /// <summary>
        /// Performs an image-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method takes shared ownership over <paramref name="source" />, which means that a reference is hold until the parent command queue finished using the command buffer. At this point,
        /// the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this
        /// method at some point after the command buffer has been executed.
        /// 
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging
        /// buffer and delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetSubresource">The image of the first sub-resource in the target image to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy between the images.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(SharedPtr<IImage> source, IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, sourceSubresource, targetSubresource, subresources);
        }

        /// <summary>
        /// Performs an image-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="firstSubresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetElement">The index of the first target element to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(IImage& source, IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// <summary>
        /// Performs an image-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="firstSubresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// 
        /// This method takes shared ownership over <paramref name="source" />, which means that a reference is hold until the parent command queue finished using the command buffer. At this point,
        /// the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this
        /// method at some point after the command buffer has been executed.
        /// 
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging
        /// buffer and delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        /// 
        /// Note that you have to manually ensure that <paramref name="source" /> and <paramref name="target" /> are in the proper state for transfer operations. You might have to
        /// use a <see cref="IBarrier" /> before starting the transfer.
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetElement">The index of the first target element to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        inline void transfer(SharedPtr<IImage> source, IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        inline void use(const IPipeline& pipeline) const noexcept {
            this->cmdUse(pipeline);
        }

        // TODO: Allow bind to last used pipeline (throw, if no pipeline is in use).
        //void bind(const IDescriptorSet& descriptorSet) const;

        /// <summary>
        /// Binds the provided descriptor set to the provided pipeline.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        inline void bind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept {
            this->cmdBind(descriptorSet, pipeline);
        }

        /// <summary>
        /// Binds a vertex buffer to the pipeline.
        /// </summary>
        /// <remarks>
        /// After binding the vertex buffer, the next call to <see cref="draw" /> or <see cref="drawIndexed" /> will read from it, until another vertex buffer is bound. 
        /// </remarks>
        /// <param name="buffer">The vertex buffer to bind to the pipeline.</param>
        /// <seealso cref="VertexBuffer" />
        /// <seealso cref="draw" />
        /// <seealso cref="drawIndexed" />
        inline void bind(const IVertexBuffer& buffer) const noexcept {
            this->cmdBind(buffer);
        }

        /// <summary>
        /// Binds a index buffer to the pipeline.
        /// </summary>
        /// <remarks>
        /// After binding the index buffer, the next call to <see cref="drawIndexed" /> will read from it, until another index buffer is bound. 
        /// </remarks>
        /// <param name="buffer">The index buffer to bind to the pipeline.</param>
        /// <seealso cref="IndexBuffer" />
        /// <seealso cref="drawIndexed" />
        inline void bind(const IIndexBuffer& buffer) const noexcept {
            this->cmdBind(buffer);
        }

        /// <summary>
        /// Executes a compute shader.
        /// </summary>
        /// <param name="threadCount">The number of thread groups per axis.</param>
        virtual void dispatch(const Vector3u& threadCount) const noexcept = 0;

        /// <summary>
        /// Draws a number of vertices from the currently bound vertex buffer.
        /// </summary>
        /// <param name="vertices">The number of vertices to draw.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstVertex">The index of the first vertex to start drawing from.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept = 0;

        /// <summary>
        /// Draws the currently bound vertex buffer with a set of indices from the currently bound index buffer.
        /// </summary>
        /// <param name="indices">The number of indices to draw.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept = 0;

        /// <summary>
        /// Pushes a block of memory into the push constants backing memory.
        /// </summary>
        /// <param name="layout">The layout of the push constants to update.</param>
        /// <param name="memory">A pointer to the source memory.</param>
        inline void pushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept {
            this->cmdPushConstants(layout, memory);
        }

        /// <summary>
        /// Draws all vertices from the vertex buffer provided in <paramref name="vertexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the vertex buffer and issues a draw command for all vertices.
        /// </remarks>
        /// <param name="vertexBuffer">The vertex buffer to draw from.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstVertex">The index of the first vertex to start drawing from.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        inline void draw(const IVertexBuffer& vertexBuffer, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const {
            this->cmdDraw(vertexBuffer, instances, firstVertex, firstInstance);
        }

        /// <summary>
        /// Draws the currently bound vertex buffer using the index buffer provided in <paramref name="indexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the index buffer and issues a draw command for all indices.
        /// </remarks>
        /// <param name="indexBuffer">The index buffer to draw with.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        inline void drawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->cmdDrawIndexed(indexBuffer, instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <summary>
        /// Draws the vertex buffer provided by <paramref name="vertexBuffer" /> using the index buffer, provided by <paramref name="indexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the provided vertex and index buffers and issues a draw command for all indices.
        /// </remarks>
        /// <param name="vertexBuffer">The vertex buffer to draw from.</param>
        /// <param name="indexBuffer">The index buffer to draw with.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        inline void drawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->cmdDrawIndexed(vertexBuffer, indexBuffer, instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <summary>
        /// Sets the viewports used for the subsequent draw calls.
        /// </summary>
        /// <param name="viewports">The viewports used for the subsequent draw calls.</param>
        virtual void setViewports(Span<const IViewport*> viewports) const noexcept = 0;

        /// <summary>
        /// Sets the viewport used for the subsequent draw calls.
        /// </summary>
        /// <param name="viewport">The viewport used for the subsequent draw calls.</param>
        virtual void setViewports(const IViewport* viewport) const noexcept = 0;

        /// <summary>
        /// Sets the scissor rectangles used for the subsequent draw calls.
        /// </summary>
        /// <param name="scissors">The scissor rectangles used for the subsequent draw calls.</param>
        virtual void setScissors(Span<const IScissor*> scissors) const noexcept = 0;

        /// <summary>
        /// Sets the scissor rectangle used for the subsequent draw calls.
        /// </summary>
        /// <param name="scissors">The scissor rectangle used for the subsequent draw calls.</param>
        virtual void setScissors(const IScissor* scissor) const noexcept = 0;

        /// <summary>
        /// Sets the blend factors for the subsequent draw calls.
        /// </summary>
        /// <remarks>
        /// Blend factors are set for all render targets that use the blend modes <c>BlendFactor::ConstantColor</c>, <c>BlendFactor::OneMinusConstantColor</c>, <c>BlendFactor::ConstantAlpha</c> or 
        /// <c>BlendFactor::OneMinusConstantAlpha</c>.
        /// </remarks>
        /// <param name="blendFactors">The blend factors for the subsequent draw calls.</param>
        virtual void setBlendFactors(const Vector4f& blendFactors) const noexcept = 0;

        /// <summary>
        /// Sets the stencil reference for the subsequent draw calls.
        /// </summary>
        /// <param name="stencilRef">The stencil reference for the subsequent draw calls.</param>
        virtual void setStencilRef(UInt32 stencilRef) const noexcept = 0;

        /// <summary>
        /// Submits the command buffer to parent command
        /// </summary>
        /// <exception cref="RuntimeException">Thrown, if the command buffer is a secondary command buffer.</exception>
        virtual UInt64 submit() const = 0;

        /// <summary>
        /// Writes the current GPU time stamp value for the timing event.
        /// </summary>
        /// <param name="timingEvent">The timing event for which the time stamp is written.</param>
        virtual void writeTimingEvent(SharedPtr<const TimingEvent> timingEvent) const = 0;

        /// <summary>
        /// Executes a secondary command buffer/bundle.
        /// </summary>
        /// <param name="commandBuffer">The secondary command buffer/bundle to execute.</param>
        inline void execute(SharedPtr<const ICommandBuffer> commandBuffer) const {
            this->cmdExecute(commandBuffer);
        }

        /// <summary>
        /// Executes a series of secondary command buffers/bundles.
        /// </summary>
        /// <param name="commandBuffers">The command buffers to execute.</param>
        inline void execute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const {
            this->cmdExecute(commandBuffers);
        }

    private:
        virtual void cmdBarrier(const IBarrier& barrier) const noexcept = 0;
        virtual void cmdGenerateMipMaps(IImage& image) noexcept = 0;
        virtual void cmdTransfer(IBuffer& source, IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(IBuffer& source, IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(IImage& source, IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(IImage& source, IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<IBuffer> source, IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(SharedPtr<IBuffer> source, IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(SharedPtr<IImage> source, IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<IImage> source, IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdUse(const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IVertexBuffer& buffer) const noexcept = 0;
        virtual void cmdBind(const IIndexBuffer& buffer) const noexcept = 0;
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept = 0;
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdExecute(SharedPtr<const ICommandBuffer> commandBuffer) const = 0;
        virtual void cmdExecute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffer) const = 0;

        /// <summary>
        /// Called by the parent command queue to signal that the command buffer should release it's shared state.
        /// </summary>
        virtual void releaseSharedState() const = 0;
    };

    /// <summary>
    /// The interface for a render pipeline.
    /// </summary>
    class LITEFX_RENDERING_API IRenderPipeline : public virtual IPipeline {
    public:
        virtual ~IRenderPipeline() noexcept = default;

    public:
        /// <summary>
        /// Returns the input assembler state used by the render pipeline.
        /// </summary>
        /// <returns>The input assembler state used by the render pipeline.</returns>
        inline SharedPtr<IInputAssembler> inputAssembler() const noexcept {
            return this->getInputAssembler();
        }

        /// <summary>
        /// Returns the rasterizer state used by the render pipeline.
        /// </summary>
        /// <returns>The rasterizer state used by the render pipeline.</returns>
        inline SharedPtr<IRasterizer> rasterizer() const noexcept {
            return this->getRasterizer();
        }

        /// <summary>
        /// Returns <c>true</c>, if the pipeline uses <i>Alpha-to-Coverage</i> multi-sampling.
        /// </summary>
        /// <remarks>
        /// Alpha-to-Coverage is a multi-sampling technique used for partially transparent sprites or textures (such as foliage) to prevent visible flickering 
        /// along edges. If enabled, the alpha-channel of the first (non-depth/stencil) render target is used to generate a temporary coverage mask that is combined
        /// with the fragment coverage mask using a logical <b>AND</b>.
        /// </remarks>
        /// <returns><c>true</c>, if the pipeline uses <i>Alpha-to-Coverage</i> multi-sampling.</returns>
        /// <seealso href="https://bgolus.medium.com/anti-aliased-alpha-test-the-esoteric-alpha-to-coverage-8b177335ae4f" />
        /// <seealso href="https://en.wikipedia.org/wiki/Alpha_to_coverage" />
        /// <seealso href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-covg" />
        /// <seealso href="https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-blend-state#alpha-to-coverage" />
        virtual bool alphaToCoverage() const noexcept = 0;

    private:
        virtual SharedPtr<IInputAssembler> getInputAssembler() const noexcept = 0;
        virtual SharedPtr<IRasterizer> getRasterizer() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a compute pipeline.
    /// </summary>
    class LITEFX_RENDERING_API IComputePipeline : public virtual IPipeline {
    public:
        virtual ~IComputePipeline() noexcept = default;
    };

    /// <summary>
    /// The interface for a frame buffer.
    /// </summary>
    class LITEFX_RENDERING_API IFrameBuffer {
    public:
        virtual ~IFrameBuffer() noexcept = default;

    public:
        /// <summary>
        /// Returns the index of the buffer within the <see cref="RenderPass" />.
        /// </summary>
        /// <remarks>
        /// A render pass stores multiple frame buffers, each with their own index. Calling <see cref="RenderPass::frameBuffer" /> with this index on the frame buffers render
        /// pass returns the current frame buffer instance (i.e. the same instance, as the one, the index has been requested from).
        /// </remarks>
        /// <returns>the index of the buffer within the <see cref="RenderPass" />.</returns>
        virtual UInt32 bufferIndex() const noexcept = 0;

        /// <summary>
        /// Returns the current size of the frame buffer.
        /// </summary>
        /// <returns>The current size of the frame buffer.</returns>
        /// <seealso cref="height" />
        /// <seealso cref="width" />
        /// <seealso cref="resize" />
        virtual const Size2d& size() const noexcept = 0;

        /// <summary>
        /// Returns the current width of the frame buffer.
        /// </summary>
        /// <returns>The current width of the frame buffer.</returns>
        /// <seealso cref="height" />
        /// <seealso cref="size" />
        /// <seealso cref="resize" />
        virtual size_t getWidth() const noexcept = 0;

        /// <summary>
        /// Returns the current height of the frame buffer.
        /// </summary>
        /// <returns>The current height of the frame buffer.</returns
        /// <seealso cref="width" />
        /// <seealso cref="size" />
        /// <seealso cref="resize" />
        virtual size_t getHeight() const noexcept = 0;

        /// <summary>
        /// Returns all command buffers, the frame buffer stores.
        /// </summary>
        /// <returns>All command buffers, the frame buffer stores.</returns>
        /// <seealso cref="commandBuffer" />
        inline Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers() const noexcept {
            return this->getCommandBuffers();
        }

        /// <summary>
        /// Returns a command buffer that records draw commands for the frame buffer.
        /// </summary>
        /// <param name="index">The index of the command buffer.</param>
        /// <returns>A command buffer that records draw commands for the frame buffer</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the frame buffer does not store a command buffer at <paramref name="index" />.</exception>
        /// <seealso cref="commandBuffers" />
        inline SharedPtr<const ICommandBuffer> commandBuffer(UInt32 index) const {
            return this->getCommandBuffer(index);
        }

        /// <summary>
        /// Returns the images that store the output attachments for the render targets of the <see cref="RenderPass" />.
        /// </summary>
        /// <returns>The images that store the output attachments for the render targets of the <see cref="RenderPass" />.</returns>
        inline Enumerable<const IImage*> images() const noexcept {
            return this->getImages();
        }

        /// <summary>
        /// Returns the image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.
        /// </summary>
        /// <returns>The image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.</returns>
        virtual const IImage& image(UInt32 location) const = 0;

    public:
        /// <summary>
        /// Causes the frame buffer to be invalidated and recreated with a new size.
        /// </summary>
        /// <remarks>
        /// A frame buffer resize causes all render target resources (i.e. images) to be re-created. This is done by the implementation itself, except for present targets, which require
        /// a view of an image created on a <see cref="ISwapChain" />. If the frame buffer has a present target, it calls <see cref="ISwapChain::images" /> on the parent devices' swap 
        /// chain. Note that there should only be one render pass, that contains present targets, otherwise the images are written by different render passes, which may result in 
        /// undefined behavior.
        /// </remarks>
        /// <param name="renderArea">The new dimensions of the frame buffer.</param>
        virtual void resize(const Size2d& renderArea) = 0;

    private:
        virtual SharedPtr<const ICommandBuffer> getCommandBuffer(UInt32 index) const noexcept = 0;
        virtual Enumerable<SharedPtr<const ICommandBuffer>> getCommandBuffers() const noexcept = 0;
        virtual Enumerable<const IImage*> getImages() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a render pass.
    /// </summary>
    class LITEFX_RENDERING_API IRenderPass : public virtual IStateResource {
    public:
        /// <summary>
        /// Event arguments that are published to subscribers when a render pass is beginning.
        /// </summary>
        /// <seealso cref="IRenderPass::beginning" />
        struct BeginRenderPassEventArgs : public EventArgs {
        private:
            UInt32 m_backBuffer;

        public:
            BeginRenderPassEventArgs(UInt32 backBuffer) : 
                EventArgs(), m_backBuffer(backBuffer) { }
            BeginRenderPassEventArgs(const BeginRenderPassEventArgs&) = default;
            BeginRenderPassEventArgs(BeginRenderPassEventArgs&&) = default;
            virtual ~BeginRenderPassEventArgs() noexcept = default;

        public:
            BeginRenderPassEventArgs& operator=(const BeginRenderPassEventArgs&) = default;
            BeginRenderPassEventArgs& operator=(BeginRenderPassEventArgs&&) = default;

        public:
            /// <summary>
            /// Gets the index of the next back-buffer used in the render pass.
            /// </summary>
            /// <returns>The index of the next back-buffer used in the render pass.</returns>
            inline UInt32 backBuffer() const noexcept {
                return m_backBuffer;
            }
        };

    public:
        virtual ~IRenderPass() noexcept = default;

    public:
        /// <summary>
        /// Returns the current frame buffer from of the render pass.
        /// </summary>
        /// <remarks>
        /// The frame buffer can only be obtained, if the render pass has been started by calling <see cref="begin" />. If the render pass has ended or not yet started, the
        /// method will instead raise an exception.
        /// </remarks>
        /// <param name="buffer">The index of the frame buffer.</param>
        /// <returns>A back buffer used by the render pass.</returns>
        virtual const IFrameBuffer& activeFrameBuffer() const = 0;

        /// <summary>
        /// Returns the command queue, the render pass is executing on.
        /// </summary>
        /// <returns>A reference of the command queue, the render pass is executing on.</returns>
        inline const ICommandQueue& commandQueue() const noexcept {
            // NOTE: This should be a covariant, though?!
            return this->getCommandQueue();
        }

        /// <summary>
        /// Returns a list of all frame buffers.
        /// </summary>
        /// <returns>A list of all frame buffers. </returns>
        inline Enumerable<const IFrameBuffer*> frameBuffers() const noexcept {
            return this->getFrameBuffers();
        }

        /// <summary>
        /// Returns an array of all render pipelines, owned by the render pass.
        /// </summary>
        /// <returns>An array of all render pipelines, owned by the render pass.</returns>
        /// <seealso cref="IRenderPipeline" />
        inline Enumerable<const IRenderPipeline*> pipelines() const noexcept {
            return this->getPipelines();
        }

        /// <summary>
        /// Returns the render target mapped to the location provided by <paramref name="location" />.
        /// </summary>
        /// <param name="location">The location to return the render target for.</param>
        /// <returns>The render target mapped to the location provided by <paramref name="location" />.</returns>
        virtual const RenderTarget& renderTarget(UInt32 location) const = 0;

        /// <summary>
        /// Returns the list of render targets, the render pass renders into.
        /// </summary>
        /// <remarks>
        /// Note that the actual render target image resources are stored within the individual <see cref="FrameBuffer" />s of the render pass.
        /// </remarks>
        /// <returns>A list of render targets, the render pass renders into.</returns>
        /// <seealso cref="FrameBuffer" />
        /// <seealso cref="frameBuffer" />
        virtual Span<const RenderTarget> renderTargets() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if one of the render targets is used for presentation on a swap chain.
        /// </summary>
        /// <returns><c>true</c>, if one of the render targets is used for presentation on a swap chain.</returns>
        /// <seealso cref="renderTargets" />
        virtual bool hasPresentTarget() const noexcept = 0;

        /// <summary>
        /// Returns the input attachment the render pass is consuming.
        /// </summary>
        /// <returns>An array of input attachment mappings, that are mapped to the render pass.</returns>
        //virtual Span<const IInputAttachmentMapping> inputAttachments() const noexcept = 0;

        /// <summary>
        /// Returns the number of samples, the render targets are sampled with.
        /// </summary>
        /// <returns>The number of samples, the render targets are sampled with.</returns>
        virtual MultiSamplingLevel multiSamplingLevel() const noexcept = 0;

    public:
        /// <summary>
        /// Invoked, when the render pass is beginning.
        /// </summary>
        /// <seealso cref="begin" />
        mutable Event<BeginRenderPassEventArgs> beginning;

        /// <summary>
        /// Invoked, when the render pass is ending.
        /// </summary>
        /// <seealso cref="end" />
        mutable Event<EventArgs> ending;

        /// <summary>
        /// Begins the render pass.
        /// </summary>
        /// <param name="buffer">The back buffer to use. Typically this is the same as the value returned from <see cref="ISwapChain::swapBackBuffer" />.</param>
        virtual void begin(UInt32 buffer) = 0;

        /// <summary>
        /// Ends the render pass.
        /// </summary>
        /// <remarks>
        /// If the frame buffer has a present render target, this causes the render pass to synchronize with the swap chain and issue a present command.
        /// </remarks>
        /// <param name="buffer">The back buffer to use. Typically this is the same as the value returned from <see cref="ISwapChain::swapBackBuffer" />.</param>
        virtual void end() const = 0;

        /// <summary>
        /// Resets the frame buffers of the render pass.
        /// </summary>
        /// <param name="renderArea">The size of the render area, the frame buffers will be resized to.</param>
        virtual void resizeFrameBuffers(const Size2d& renderArea) = 0;

        /// <summary>
        /// Changes the multi sampling level of the render pass.
        /// </summary>
        /// <remarks>
        /// The method causes the frame buffers to be re-created. It checks, if the <paramref name="samples" /> are supported by the device for each render target 
        /// format. If not, an exception will be thrown. To prevent this, call <see cref=IGraphicsDevice::maximumMultiSamplingLevel" /> for each render target format on 
        /// your own, in order to request the maximum number of samples supported.
        /// </remarks>
        /// <param name="samples">The number of samples per edge pixel.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if one or more of the render targets have a format, that does not support the provided multi-sampling level.</exception>
        virtual void changeMultiSamplingLevel(MultiSamplingLevel samples) = 0;

        /// <summary>
        /// Resolves the input attachments mapped to the render pass and updates them on the descriptor set provided with <see cref="descriptorSet" />.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to update the input attachments on.</param>
        inline void updateAttachments(const IDescriptorSet& descriptorSet) const {
            this->setAttachments(descriptorSet);
        }

    private:
        virtual Enumerable<const IFrameBuffer*> getFrameBuffers() const noexcept = 0;
        virtual Enumerable<const IRenderPipeline*> getPipelines() const noexcept = 0;
        virtual void setAttachments(const IDescriptorSet& descriptorSet) const = 0;
        virtual const ICommandQueue& getCommandQueue() const noexcept = 0;
    };

    /// <summary>
    /// Interface for a swap chain.
    /// </summary>
    class LITEFX_RENDERING_API ISwapChain {
    public:
        /// <summary>
        /// Event arguments for a <see cref="ISwapChain::reseted" /> event.
        /// </summary>
        struct SwapChainResetEventArgs : public EventArgs {
        private:
            Format m_surfaceFormat;
            const Size2d& m_renderArea;
            UInt32 m_buffers;

        public:
            SwapChainResetEventArgs(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers) :
                EventArgs(), m_surfaceFormat(surfaceFormat), m_renderArea(renderArea), m_buffers(buffers) { }
            SwapChainResetEventArgs(const SwapChainResetEventArgs&) = default;
            SwapChainResetEventArgs(SwapChainResetEventArgs&&) = default;
            virtual ~SwapChainResetEventArgs() noexcept = default;

        public:
            SwapChainResetEventArgs& operator=(const SwapChainResetEventArgs&) = default;
            SwapChainResetEventArgs& operator=(SwapChainResetEventArgs&&) = default;

        public:
            /// <summary>
            /// Gets the new surface format of the swap chain back-buffers.
            /// </summary>
            /// <returns>The new surface format of the swap chain back-buffers.</returns>
            inline Format surfaceFormat() const noexcept {
                return m_surfaceFormat;
            }

            /// <summary>
            /// Gets the new render area of the swap chain back-buffers.
            /// </summary>
            /// <returns>The size of the new render area of the swap chain back-buffers.</returns>
            inline const Size2d& renderArea() const noexcept {
                return m_renderArea;
            }

            /// <summary>
            /// Gets the number of back-buffers in the swap chain.
            /// </summary>
            /// <returns>The number of back-buffers in the swap chain.</returns>
            inline UInt32 buffers() const noexcept {
                return m_buffers;
            }
        };

    public:
        virtual ~ISwapChain() noexcept = default;

    public:
        /// <summary>
        /// Creates a new instance of a <see cref="TimingEvent" />.
        /// </summary>
        /// <remarks>
        /// Note that registering a new timing event does invalidate previously registered events, i.e. they do not return meaningful time stamps for
        /// the next frame. Timing events should only be registered during application startup, before the first frame is rendered.
        /// </remarks>
        /// <param name="name">The name of the timing event.</param>
        /// <returns>A pointer with shared ownership to the newly created timing event instance.</returns>
        [[nodiscard]] inline std::shared_ptr<TimingEvent> registerTimingEvent(StringView name = "") noexcept {
            auto timingEvent = SharedPtr<TimingEvent>(new TimingEvent(*this, name));
            this->addTimingEvent(timingEvent);
            return timingEvent;
        }

        /// <summary>
        /// Returns all registered timing events.
        /// </summary>
        /// <returns>An array, containing all registered timing events.</returns>
        virtual Enumerable<SharedPtr<TimingEvent>> timingEvents() const noexcept = 0;

        /// <summary>
        /// Returns the timing event registered for <paramref name="queryId" />.
        /// </summary>
        /// <param name="queryId">The query ID of the timing event.</param>
        /// <returns>The timing event registered for <paramref name="queryId" />.</returns>
        virtual SharedPtr<TimingEvent> timingEvent(UInt32 queryId) const = 0;

        /// <summary>
        /// Reads the current time stamp value (in ticks) of a timing event.
        /// </summary>
        /// <remarks>
        /// In order to convert the number of ticks to (milli-)seconds, this value needs to be divided by <see cref="IGraphicsDevice::ticksPerMillisecond" />. To improve precision,
        /// calculate the difference between two time stamps in ticks first and only then convert them to seconds.
        /// </remarks>
        /// <param name="timingEvent">The timing event to read the current value for.</param>
        /// <returns>The current time stamp value of the timing event in ticks.</returns>
        /// <seealso cref="TimingEvent::readTimestamp" />
        virtual UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const = 0;

        /// <summary>
        /// Returns the query ID for the timing event.
        /// </summary>
        /// <param name="timingEvent">The timing event to return the query ID for.</param>
        /// <returns>The query ID for the <paramref name="timingEvent" />.</returns>
        /// <seealso cref="TimingEvent::queryId" />
        virtual UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const = 0;

    public:
        /// <summary>
        /// Returns the swap chain image format.
        /// </summary>
        /// <returns>The swap chain image format.</returns>
        virtual Format surfaceFormat() const noexcept = 0;

        /// <summary>
        /// Returns the number of images in the swap chain.
        /// </summary>
        /// <returns>The number of images in the swap chain.</returns>
        virtual UInt32 buffers() const noexcept = 0;

        /// <summary>
        /// Returns the size of the render area.
        /// </summary>
        /// <returns>The size of the render area.</returns>
        virtual const Size2d& renderArea() const noexcept = 0;

        /// <summary>
        /// Returns the swap chain present image for <paramref name="backBuffer" />.
        /// </summary>
        /// <param name="backBuffer">The index of the back buffer for which to return the swap chain present image.</param>
        /// <returns>A pointer to the back buffers swap chain present image.</returns>
        virtual const IImage* image(UInt32 backBuffer) const = 0;

        /// <summary>
        /// Returns an array of the swap chain present images.
        /// </summary>
        /// <returns>Returns an array of the swap chain present images.</returns>
        inline Enumerable<const IImage*> images() const noexcept {
            return this->getImages();
        };

        /// <summary>
        /// Queues a present that gets executed after <paramref name="frameBuffer" /> signals its readiness.
        /// </summary>
        /// <param name="frameBuffer">The frame buffer for which the present should wait.</param>
        virtual void present(const IFrameBuffer& frameBuffer) const = 0;

    public:
        /// <summary>
        /// Invoked, when the swap chain has swapped the back buffers.
        /// </summary>
        /// <seealso cref="swapBackBuffer" />
        mutable Event<EventArgs> swapped;

        /// <summary>
        /// Invoked, after the swap chain has been reseted.
        /// </summary>
        /// <seealso cref="reset" />
       mutable Event<SwapChainResetEventArgs> reseted;

        /// <summary>
        /// Returns an array of supported formats, that can be drawn to the surface.
        /// </summary>
        /// <returns>An array of supported formats, that can be drawn to the surface.</returns>
        /// <see cref="surface" />
        /// <seealso cref="ISurface" />
        virtual Enumerable<Format> getSurfaceFormats() const noexcept = 0;

        /// <summary>
        /// Causes the swap chain to be re-created. All frame and command buffers will be invalidated and rebuilt.
        /// </summary>
        /// <remarks>
        /// There is no guarantee, that the swap chain images will end up in the exact format, as specified by <paramref name="surfaceFormat" />. If the format itself is not
        /// supported, a compatible format may be looked up. If the lookup fails, the method may raise an exception.
        /// 
        /// Similarly, it is not guaranteed, that the number of images returned by <see cref="images" /> matches the number specified in <paramref name="buffers" />. A swap chain may 
        /// require a minimum number of images or may constraint a maximum number of images. In both cases, <paramref name="buffers" /> will be clamped.
        /// </remarks>
        /// <param name="surfaceFormat">The swap chain image format.</param>
        /// <param name="renderArea">The dimensions of the frame buffers.</param>
        /// <param name="buffers">The number of buffers in the swap chain.</param>
        /// <seealso cref="multiSamplingLevel" />
        virtual void reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers) = 0;

        /// <summary>
        /// Swaps the front buffer with the next back buffer in order.
        /// </summary>
        /// <returns>A reference of the front buffer after the buffer swap.</returns>
        [[nodiscard]] virtual UInt32 swapBackBuffer() const = 0;

    private:
        virtual Enumerable<const IImage*> getImages() const noexcept = 0;
        virtual void addTimingEvent(SharedPtr<TimingEvent> timingEvent) = 0;
    };

    /// <summary>
    /// The interface for a command queue.
    /// </summary>
    class LITEFX_RENDERING_API ICommandQueue {
    public:
        /// <summary>
        /// Event arguments for a <see cref="ICommandQueue::submitting" /> event.
        /// </summary>
        struct QueueSubmittingEventArgs : public EventArgs {
        private:
            const Enumerable<SharedPtr<const ICommandBuffer>> m_commandBuffers;

        public:
            QueueSubmittingEventArgs(const Enumerable<SharedPtr<const ICommandBuffer>>& commandBuffers) :
                EventArgs(), m_commandBuffers(commandBuffers) { }
            QueueSubmittingEventArgs(const QueueSubmittingEventArgs&) = default;
            QueueSubmittingEventArgs(QueueSubmittingEventArgs&&) = default;
            virtual ~QueueSubmittingEventArgs() noexcept = default;

        public:
            QueueSubmittingEventArgs& operator=(const QueueSubmittingEventArgs&) = default;
            QueueSubmittingEventArgs& operator=(QueueSubmittingEventArgs&&) = default;

        public:
            /// <summary>
            /// Gets the command buffers that are about to be submitted to the queue.
            /// </summary>
            /// <returns>An array containing the command buffers that are about to be submitted to the queue.</returns>
            inline const Enumerable<SharedPtr<const ICommandBuffer>>& commandBuffers() const noexcept {
                return m_commandBuffers;
            }
        };

        /// <summary>
        /// Event arguments for a <see cref="ICommandQueue::submitted" /> event.
        /// </summary>
        struct QueueSubmittedEventArgs : public EventArgs {
        private:
            UInt64 m_fence;

        public:
            QueueSubmittedEventArgs(UInt64 fence) :
                EventArgs(), m_fence(fence) { }
            QueueSubmittedEventArgs(const QueueSubmittedEventArgs&) = default;
            QueueSubmittedEventArgs(QueueSubmittedEventArgs&&) = default;
            virtual ~QueueSubmittedEventArgs() noexcept = default;

        public:
            QueueSubmittedEventArgs& operator=(const QueueSubmittedEventArgs&) = default;
            QueueSubmittedEventArgs& operator=(QueueSubmittedEventArgs&&) = default;

        public:
            /// <summary>
            /// Gets the fence that is triggered, if the command buffers have been executed.
            /// </summary>
            /// <returns>The fence that is triggered, if the command buffers have been executed.</returns>
            inline UInt64 fence() const noexcept {
                return m_fence;
            }
        };

    public:
        virtual ~ICommandQueue() noexcept = default;

    public:
        /// <summary>
        /// Returns the priority of the queue.
        /// </summary>
        /// <returns>The priority of the queue.</returns>
        virtual QueuePriority priority() const noexcept = 0;

        /// <summary>
        /// Returns the type of the queue.
        /// </summary>
        /// <returns>The type of the queue.</returns>
        virtual QueueType type() const noexcept = 0;

    public:
        /// <summary>
        /// Starts a new debug region.
        /// </summary>
        /// <remarks>
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        /// <param name="label">The name of the debug region.</param>
        /// <param name="color">The color of the debug region.</param>
        virtual void beginDebugRegion(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept { };
        
        /// <summary>
        /// Ends the current debug region.
        /// </summary>
        /// <remarks>
        /// This is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        virtual void endDebugRegion() const noexcept { };

        /// <summary>
        /// Inserts a debug marker.
        /// </summary>
        /// <remarks>
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        /// <param name="label">The name of the debug marker.</param>
        /// <param name="color">The color of the debug marker.</param>
        virtual void setDebugMarker(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept { };

    public:
        /// <summary>
        /// Invoked, when one or more command buffers are submitted to the queue.
        /// </summary>
        mutable Event<QueueSubmittingEventArgs> submitting;

        /// <summary>
        /// Invoked, after one or more command buffers have been submitted to the queue.
        /// </summary>
        mutable Event<QueueSubmittedEventArgs> submitted;

        /// <summary>
        /// Creates a command buffer that can be used to allocate commands on the queue.
        /// </summary>
        /// <remarks>
        /// Specifying <paramref name="secondary" /> allows to create secondary command buffers (aka. bundles). Those are intended to be used as efficient pre-recorded command buffers
        /// that are re-used multiple times. Using such a command buffer allows drivers to pre-apply optimizations, which causes a one-time cost during setup, but reduces cost when re-
        /// applying the command buffer multiple times. Ideally they are used as small chunks of re-occurring workloads.
        /// 
        /// A secondary command buffer must not be submitted to a queue, but rather to a primary command buffer by calling <see cref="ICommandBuffer::execute" />.
        /// </remarks>
        /// <param name="beginRecording">If set to <c>true</c>, the command buffer will be initialized in recording state and can receive commands straight away.</param>
        /// <param name="secondary">If set to `true`, the method will create a secondary command buffer/bundle.</param>
        /// <returns>The instance of the command buffer.</returns>
        inline SharedPtr<ICommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const {
            return this->getCommandBuffer(beginRecording, secondary);
        }

        /// <summary>
        /// Submits a single command buffer with shared ownership and inserts a fence to wait for it.
        /// </summary>
        /// <remarks>
        /// By calling this method, the queue takes shared ownership over the <paramref name="commandBuffer" /> until the fence is passed. The reference will be released
        /// during a <see cref="waitFor" />, if the awaited fence is inserted after the associated one.
        /// 
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer to submit to the command queue.</param>
        /// <returns>The value of the fence, inserted after the command buffer.</returns>
        /// <seealso cref="waitFor" />
        inline UInt64 submit(SharedPtr<const ICommandBuffer> commandBuffer) const {
            return this->submitCommandBuffer(commandBuffer);
        }

        /// <summary>
        /// Submits a single command buffer with shared ownership and inserts a fence to wait for it.
        /// </summary>
        /// <remarks>
        /// By calling this method, the queue takes shared ownership over the <paramref name="commandBuffer" /> until the fence is passed. The reference will be released
        /// during a <see cref="waitFor" />, if the awaited fence is inserted after the associated one.
        /// 
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer to submit to the command queue.</param>
        /// <returns>The value of the fence, inserted after the command buffer.</returns>
        /// <seealso cref="waitFor" />
        inline UInt64 submit(SharedPtr<ICommandBuffer> commandBuffer) const {
            return this->submitCommandBuffer(commandBuffer);
        }

        /// <summary>
        /// Submits a set of command buffers with shared ownership and inserts a fence to wait for them.
        /// </summary>
        /// <remarks>
        /// By calling this method, the queue takes shared ownership over the <paramref name="commandBuffers" /> until the fence is passed. The reference will be released
        /// during a <see cref="waitFor" />, if the awaited fence is inserted after the associated one.
        /// 
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        /// </remarks>
        /// <param name="commandBuffers">The command buffers to submit to the command queue.</param>
        /// <returns>The value of the fence, inserted after the command buffers.</returns>
        /// <seealso cref="waitFor" />
        inline UInt64 submit(const Enumerable<SharedPtr<const ICommandBuffer>>& commandBuffers) const {
            return this->submitCommandBuffers(commandBuffers);
        }

        /// <summary>
        /// Submits a set of command buffers with shared ownership and inserts a fence to wait for them.
        /// </summary>
        /// <remarks>
        /// By calling this method, the queue takes shared ownership over the <paramref name="commandBuffers" /> until the fence is passed. The reference will be released
        /// during a <see cref="waitFor" />, if the awaited fence is inserted after the associated one.
        /// 
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        /// </remarks>
        /// <param name="commandBuffers">The command buffers to submit to the command queue.</param>
        /// <returns>The value of the fence, inserted after the command buffers.</returns>
        /// <seealso cref="waitFor" />
        inline UInt64 submit(const Enumerable<SharedPtr<ICommandBuffer>>& commandBuffers) const {
            return this->submitCommandBuffers(commandBuffers | std::ranges::to<Enumerable<SharedPtr<const ICommandBuffer>>>());
        }

        /// <summary>
        /// Lets the CPU wait for a certain fence value to complete on the command queue.
        /// </summary>
        /// <remarks>
        /// This overload performs a CPU-side wait, i.e., the CPU blocks until the current queue has passed the fence value provided by the <paramref name="fence" /> parameter.
        /// </remarks>
        /// <param name="fence">The value of the fence to wait for.</param>
        /// <seealso cref="submit" />
        virtual void waitFor(UInt64 fence) const noexcept = 0;

        /// <summary>
        /// Lets the command queue wait for a certain fence value to complete on another queue.
        /// </summary>
        /// <remarks>
        /// This overload performs a GPU-side wait, i.e., the current command queue waits until <paramref name="queue" /> has passed the fence value provided by the <paramref name="fence" />
        /// parameter. This overload does return immediately and does not block the CPU.
        /// </remarks>
        /// <param name="queue">The queue to wait upon.</param>
        /// <param name="fence">The value of the fence to wait upon on the other queue.</param>
        inline void waitFor(const ICommandQueue& queue, UInt64 fence) const {
            this->waitForQueue(queue, fence);
        }

        /// <summary>
        /// Returns the value of the latest fence inserted into the queue.
        /// </summary>
        /// <returns>The value of the latest fence inserted into the queue.</returns>
        /// <seealso cref="waitFor" />
        virtual UInt64 currentFence() const noexcept = 0;

    private:
        virtual SharedPtr<ICommandBuffer> getCommandBuffer(bool beginRecording, bool secondary) const = 0;
        virtual UInt64 submitCommandBuffer(SharedPtr<const ICommandBuffer> commandBuffer) const = 0;
        virtual UInt64 submitCommandBuffers(const Enumerable<SharedPtr<const ICommandBuffer>>& commandBuffers) const = 0;
        virtual void waitForQueue(const ICommandQueue& queue, UInt64 fence) const = 0;
        
    protected:
        inline void releaseSharedState(const ICommandBuffer& commandBuffer) const {
            commandBuffer.releaseSharedState();
        }
    };

    /// <summary>
    /// The interface for a graphics factory.
    /// </summary>
    class LITEFX_RENDERING_API IGraphicsFactory {
    public:
        virtual ~IGraphicsFactory() noexcept = default;

    public:
        /// <summary>
        /// Creates a buffer of type <paramref name="type" />.
        /// </summary>
        /// <param name="type">The type of the buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements = 1, bool allowWrite = false) const {
            return this->getBuffer(type, usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, BufferUsage usage, UInt32 elements = 1, bool allowWrite = false) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), usage, descriptor.elementSize(), elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, BufferUsage usage, UInt32 elementSize, UInt32 elements, bool allowWrite = false) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, BufferUsage usage, UInt32 elementSize, UInt32 elements, bool allowWrite = false) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, BufferUsage usage, UInt32 elements = 1, bool allowWrite = false) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, usage, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer of type <paramref name="type" />.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="type">The type of the buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite = false) const {
            return this->getBuffer(name, type, usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, BufferUsage usage, UInt32 elements = 1, bool allowWrite = false) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), usage, descriptor.elementSize(), elements, allowWrite);
        };
        
        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite = false) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, BufferUsage usage, UInt32 elements = 1, bool allowWrite = false) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, usage, elements, allowWrite);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, BufferUsage usage, size_t elementSize, UInt32 elements = 1, bool allowWrite = false) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, usage, elementSize, elements, allowWrite);
        };

        /// <summary>
        /// Creates a vertex buffer, based on the <paramref name="layout" />
        /// </summary>
        /// <remarks>
        /// A vertex buffer can be used by different <see cref="RenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="layout">The layout of the vertex buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
        /// <returns>The instance of the vertex buffer.</returns>
        inline UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout& layout, BufferUsage usage, UInt32 elements = 1) const {
            return this->getVertexBuffer(layout, usage, elements);
        }

        /// <summary>
        /// Creates a vertex buffer, based on the <paramref name="layout" />
        /// </summary>
        /// <remarks>
        /// A vertex buffer can be used by different <see cref="RenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="layout">The layout of the vertex buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
        /// <returns>The instance of the vertex buffer.</returns>
        inline UniquePtr<IVertexBuffer> createVertexBuffer(const String& name, const IVertexBufferLayout& layout, BufferUsage usage, UInt32 elements = 1) const {
            return this->getVertexBuffer(name, layout, usage, elements);
        }

        /// <summary>
        /// Creates an index buffer, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// An index buffer can be used by different <see cref="RenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="layout">The layout of the index buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
        /// <returns>The instance of the index buffer.</returns>
        inline UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const {
            return this->getIndexBuffer(layout, usage, elements);
        }

        /// <summary>
        /// Creates an index buffer, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// An index buffer can be used by different <see cref="RenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="layout">The layout of the index buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
        /// <returns>The instance of the index buffer.</returns>
        inline UniquePtr<IIndexBuffer> createIndexBuffer(const String& name, const IIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const {
            return this->getIndexBuffer(name, layout, usage, elements);
        }

        /// <summary>
        /// Creates an image that is used as render target attachment.
        /// </summary>
        /// <param name="format">The format of the image.</param>
        /// <param name="size">The extent of the image.</param>
        /// <param name="samples">The number of samples, the image should be sampled with.</param>
        /// <returns>The instance of the attachment image.</returns>
        inline UniquePtr<IImage> createAttachment(Format format, const Size2d& size, MultiSamplingLevel samples = MultiSamplingLevel::x1) const {
            return this->getAttachment(format, size, samples);
        }

        /// <summary>
        /// Creates an image that is used as render target attachment.
        /// </summary>
        /// <param name="name">The name of the image.</param>
        /// <param name="format">The format of the image.</param>
        /// <param name="size">The extent of the image.</param>
        /// <param name="samples">The number of samples, the image should be sampled with.</param>
        /// <returns>The instance of the attachment image.</returns>
        inline UniquePtr<IImage> createAttachment(const String& name, Format format, const Size2d& size, MultiSamplingLevel samples = MultiSamplingLevel::x1) const {
            return this->getAttachment(name, format, size, samples);
        }

        /// <summary>
        /// Creates a texture, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// A texture in LiteFX is always backed by GPU-only visible memory and thus can only be transferred to/from. Thus you typically have to create a buffer using 
        /// <see cref="createBuffer" /> first that holds the actual image bytes. You than can transfer/copy the contents into the texture.
        /// </remarks>
        /// <param name="format">The format of the texture image.</param>
        /// <param name="size">The dimensions of the texture.</param>
        /// <param name="dimension">The dimensionality of the texture.</param>
        /// <param name="layers">The number of layers (slices) in this texture.</param>
        /// <param name="levels">The number of mip map levels of the texture.</param>
        /// <param name="samples">The number of samples, the texture should be sampled with.</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the texture.</returns>
        /// <seealso cref="createTextures" />
        inline UniquePtr<IImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool allowWrite = false) const {
            return this->getTexture(format, size, dimension, levels, layers, samples, allowWrite);
        }

        /// <summary>
        /// Creates a texture, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// A texture in LiteFX is always backed by GPU-only visible memory and thus can only be transferred to/from. Thus you typically have to create a buffer using 
        /// <see cref="createBuffer" /> first that holds the actual image bytes. You than can transfer/copy the contents into the texture.
        /// </remarks>
        /// <param name="name">The name of the texture image.</param>
        /// <param name="format">The format of the texture image.</param>
        /// <param name="size">The dimensions of the texture.</param>
        /// <param name="dimension">The dimensionality of the texture.</param>
        /// <param name="layers">The number of layers (slices) in this texture.</param>
        /// <param name="levels">The number of mip map levels of the texture.</param>
        /// <param name="samples">The number of samples, the texture should be sampled with.</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the texture.</returns>
        /// <seealso cref="createTextures" />
        inline UniquePtr<IImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool allowWrite = false) const {
            return this->getTexture(name, format, size, dimension, levels, layers, samples, allowWrite);
        }

        /// <summary>
        /// Creates an array of textures, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="layout">The layout of the textures.</param>
        /// <param name="elements">The number of textures to create.</param>
        /// <param name="format">The format of the texture images.</param>
        /// <param name="size">The dimensions of the textures.</param>
        /// <param name="layers">The number of layers (slices) in this texture.</param>
        /// <param name="levels">The number of mip map levels of the textures.</param>
        /// <param name="samples">The number of samples, the textures should be sampled with.</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>An array of texture instances.</returns>
        /// <seealso cref="createTexture" />
        inline Enumerable<UniquePtr<IImage>> createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 layers = 1, UInt32 levels = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool allowWrite = false) const {
            return this->getTextures(elements, format, size, dimension, layers, levels, samples, allowWrite);
        }

        /// <summary>
        /// Creates a texture sampler, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="magFilter">The filter operation used for magnifying.</param>
        /// <param name="minFilter">The filter operation used for minifying.</param>
        /// <param name="borderU">The border mode along the U-axis.</param>
        /// <param name="borderV">The border mode along the V-axis.</param>
        /// <param name="borderW">The border mode along the W-axis.</param>
        /// <param name="mipMapMode">The mip map mode.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="maxLod">The maximum level of detail value.</param>
        /// <param name="minLod">The minimum level of detail value.</param>
        /// <param name="anisotropy">The level of anisotropic filtering.</param>
        /// <returns>The instance of the sampler.</returns>
        /// <seealso cref="createSamplers" />
        inline UniquePtr<ISampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        /// <summary>
        /// Creates a texture sampler, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="name">The name of the sampler.</param>
        /// <param name="magFilter">The filter operation used for magnifying.</param>
        /// <param name="minFilter">The filter operation used for minifying.</param>
        /// <param name="borderU">The border mode along the U-axis.</param>
        /// <param name="borderV">The border mode along the V-axis.</param>
        /// <param name="borderW">The border mode along the W-axis.</param>
        /// <param name="mipMapMode">The mip map mode.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="maxLod">The maximum level of detail value.</param>
        /// <param name="minLod">The minimum level of detail value.</param>
        /// <param name="anisotropy">The level of anisotropic filtering.</param>
        /// <returns>The instance of the sampler.</returns>
        /// <seealso cref="createSamplers" />
        inline UniquePtr<ISampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSampler(name, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        /// <summary>
        /// Creates an array of texture samplers, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="elements">The number of samplers to create.</param>
        /// <param name="magFilter">The filter operation used for magnifying.</param>
        /// <param name="minFilter">The filter operation used for minifying.</param>
        /// <param name="borderU">The border mode along the U-axis.</param>
        /// <param name="borderV">The border mode along the V-axis.</param>
        /// <param name="borderW">The border mode along the W-axis.</param>
        /// <param name="mipMapMode">The mip map mode.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="maxLod">The maximum level of detail value.</param>
        /// <param name="minLod">The minimum level of detail value.</param>
        /// <param name="anisotropy">The level of anisotropic filtering.</param>
        /// <returns>An array of sampler instances.</returns>
        /// <seealso cref="createSampler" />
        inline Enumerable<UniquePtr<ISampler>> createSamplers(UInt32 elements, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSamplers(elements, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

    private:
        virtual UniquePtr<IBuffer> getBuffer(BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const = 0;
        virtual UniquePtr<IBuffer> getBuffer(const String& name, BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const = 0;
        virtual UniquePtr<IImage> getAttachment(Format format, const Size2d& size, MultiSamplingLevel samples) const = 0;
        virtual UniquePtr<IImage> getAttachment(const String& name, Format format, const Size2d& size, MultiSamplingLevel samples) const = 0;
        virtual UniquePtr<IImage> getTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const = 0;
        virtual UniquePtr<IImage> getTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const = 0;
        virtual Enumerable<UniquePtr<IImage>> getTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 layers, UInt32 levels, MultiSamplingLevel samples, bool allowWrite) const = 0;
        virtual UniquePtr<ISampler> getSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual UniquePtr<ISampler> getSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual Enumerable<UniquePtr<ISampler>> getSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
    };

    /// <summary>
    /// The interface for a graphics device that.
    /// </summary>
    class LITEFX_RENDERING_API IGraphicsDevice {
    public:
        virtual ~IGraphicsDevice() noexcept = default;

    public:
        /// <summary>
        /// Returns the device state that can be used to manage resources.
        /// </summary>
        /// <returns>A reference of the device state.</returns>
        virtual DeviceState& state() const noexcept = 0;

        /// <summary>
        /// Returns the surface, the device draws to.
        /// </summary>
        /// <returns>A reference of the surface, the device draws to.</returns>
        virtual const ISurface& surface() const noexcept = 0;

        /// <summary>
        /// Returns the graphics adapter, the device uses for drawing.
        /// </summary>
        /// <returns>A reference of the graphics adapter, the device uses for drawing.</returns>
        virtual const IGraphicsAdapter& adapter() const noexcept = 0;

        /// <summary>
        /// Returns the swap chain, that contains the back and front buffers used for presentation.
        /// </summary>
        /// <returns>The swap chain, that contains the back and front buffers used for presentation.</returns>
        virtual const ISwapChain& swapChain() const noexcept = 0;

        /// <summary>
        /// Returns the swap chain, that contains the back and front buffers used for presentation.
        /// </summary>
        /// <returns>The swap chain, that contains the back and front buffers used for presentation.</returns>
        virtual ISwapChain& swapChain() noexcept = 0;

        /// <summary>
        /// Returns the factory instance, used to create instances from the device.
        /// </summary>
        /// <returns>The factory instance, used to create instances from the device.</returns>
        virtual const IGraphicsFactory& factory() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the default <see cref="ICommandQueue" /> that supports the combination of queue types specified by the <paramref name="type" /> parameter.
        /// </summary>
        /// <remarks>
        /// When the device is created, it attempts to create a queue for each singular queue type. Each GPU is expected to provide at least one queue that is capable of supporting all
        /// queue types. This queue is used as a fallback queue, if no dedicated queue for a certain type is supported. For example, if no dedicated <see cref="QueueType::Transfer" />
        /// queue can be created, calling this method for the default transfer queue will return the same queue instance as the default graphics queue, which implicitly always supports
        /// transfer operations. The same is true for compute queues. This default graphics queue is ensured to support presentation and is also created with the highest queue priority.
        /// </remarks>
        /// <param name="type">The type or a combination of types that specifies the operation the queue should support.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if no default queue for the combination of queue types specified with the <paramref name="type" /> parameter has been created.</exception>
        /// <returns>The instance of the queue, used to process draw calls.</returns>
        /// <seealso cref="createQueue" />
        inline const ICommandQueue& defaultQueue(QueueType type) const {
            return this->getDefaultQueue(type);
        }

        /// <summary>
        /// Attempts to create a new queue that supports the combination of queue types specified by the <paramref name="type" /> parameter.
        /// </summary>
        /// <remarks>
        /// Note that a queue is not guaranteed to represent an *actual* hardware queue that runs in parallel to other hardware queues. Backends might create *virtual* queues, that map
        /// the same hardware queue. In this case, creating a new queue is always possible but might not yield performance benefits. As a good practice, it is advised to create only as
        /// few queues as required.
        /// 
        /// If this method is not able to create a new queue (i.e., it returns `nullptr`), you can either fall back to the default queue (<see cref="defaultQueue" />) or use any queue
        /// that you created earlier instead.
        /// 
        /// The <paramref name="priority" /> parameter can be specified to request a queue with a certain priority. However, the backend is not required to return a queue with that 
        /// actual priority. The default queues are always prioritized highest.
        /// </remarks>
        /// <param name="type">The type of the queue or a combination of capabilities the queue is required to support.</param>
        /// <param name="priority">The preferred priority of the queue.</param>
        /// <returns>A pointer to the newly created queue, or `nullptr`, if no queue could be created.</returns>
        /// <seealso cref="defaultQueue" />
        inline const ICommandQueue* createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) noexcept {
            return this->getNewQueue(type, priority);
        }

        /// <summary>
        /// Creates a memory barrier instance.
        /// </summary>
        /// <param name="syncBefore">The pipeline stage(s) all previous commands have to finish before the barrier is executed.</param>
        /// <param name="syncAfter">The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.</param>
        /// <returns>The instance of the memory barrier.</returns>
        inline UniquePtr<IBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept {
            return this->getNewBarrier(syncBefore, syncAfter);
        }

        /// <summary>
        /// Queries the device for the maximum supported number of multi-sampling levels.
        /// </summary>
        /// <remarks>
        /// This method returns the maximum supported multi-sampling level for a certain format. Typically you want to pass a back-buffer format for your swap-chain here. All lower 
        /// multi-sampling levels are implicitly supported for this format.
        /// </remarks>
        /// <param name="format">The target (i.e. back-buffer) format.</param>
        /// <returns>The maximum multi-sampling level.</returns>
        virtual MultiSamplingLevel maximumMultiSamplingLevel(Format format) const noexcept = 0;

        /// <summary>
        /// Returns the number of GPU ticks per milliseconds.
        /// </summary>
        /// <returns>The number of GPU ticks per milliseconds.</returns>
        /// <seealso cref="TimingEvent" />
        virtual double ticksPerMillisecond() const noexcept = 0;

    public:
        /// <summary>
        /// Waits until all queues allocated from the device have finished the work issued prior to this point.
        /// </summary>
        /// <remarks>
        /// Note that you must synchronize calls to this method, i.e., you have to ensure no other thread is submitting work on any queue while waiting. Calling this method only 
        /// guarantees that all *prior* work is finished after returning. If any other thread submits work to any queue after calling this method, this workload is not waited on.
        /// </remarks>
        virtual void wait() const = 0;

    private:
        virtual UniquePtr<IBarrier> getNewBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept = 0;
        virtual const ICommandQueue& getDefaultQueue(QueueType type) const = 0;
        virtual const ICommandQueue* getNewQueue(QueueType type, QueuePriority priority) noexcept = 0;
    };

    /// <summary>
    /// The interface to access a render backend.
    /// </summary>
    class LITEFX_RENDERING_API IRenderBackend : public IBackend {
    public:
        virtual ~IRenderBackend() noexcept = default;

    public:
        /// <summary>
        /// Lists all available graphics adapters.
        /// </summary>
        /// <returns>An array of pointers to all available graphics adapters.</returns>
        inline Enumerable<const IGraphicsAdapter*> listAdapters() const {
            return this->getAdapters();
        }

        /// <summary>
        /// Finds an adapter using its unique ID.
        /// </summary>
        /// <remarks>
        /// Note that the adapter ID is optional, which allows the back-end to return a default adapter instance. Which adapter is used as <i>default</i> adapter, depends on
        /// the actual back-end implementation. The interface does not make any constraints on the default adapter to choose. A naive implementation might simply return the 
        /// first available adapter.
        /// </remarks>
        /// <param name="adapterId">The unique ID of the adapter, or <c>std::nullopt</c> to find the default adapter.</param>
        /// <returns>A pointer to a graphics adapter, or <c>nullptr</c>, if no adapter could be found.</returns>
        /// <seealso cref="IGraphicsAdapter" />
        virtual const IGraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const = 0;

        /// <summary>
        /// Looks up a device and returns a pointer to it, or <c>nullptr</c>, if no device with the provided <paramref name="name" /> could be found.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        /// <returns>A pointer to the device or <c>nullptr</c>, if no device could be found.</returns>
        virtual IGraphicsDevice* device(const String& name) noexcept = 0;

        /// <summary>
        /// Looks up a device and returns a pointer to it, or <c>nullptr</c>, if no device with the provided <paramref name="name" /> could be found.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        /// <returns>A pointer to the device or <c>nullptr</c>, if no device could be found.</returns>
        virtual const IGraphicsDevice* device(const String& name) const noexcept = 0;

        /// <summary>
        /// Looks up a device and returns a pointer to it, or <c>nullptr</c>, if no device with the provided <paramref name="name" /> could be found.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        /// <returns>A pointer to the device or <c>nullptr</c>, if no device could be found.</returns>
        virtual inline const IGraphicsDevice* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// <summary>
        /// Looks up a device and returns a pointer to it, or <c>nullptr</c>, if no device with the provided <paramref name="name" /> could be found.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        /// <returns>A pointer to the device or <c>nullptr</c>, if no device could be found.</returns>
        virtual inline IGraphicsDevice* operator[](const String& name) noexcept {
            return this->device(name);
        };

    private:
        virtual Enumerable<const IGraphicsAdapter*> getAdapters() const = 0;
    };
}