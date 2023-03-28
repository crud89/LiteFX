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
    /// Describes the number of samples with which a <see cref="IImage" /> is sampled.
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
        /// The resource is used as a read-only buffer that can be bound to all read-only descriptor types.
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
        ///         <term>`D3D12_RESOURCE_STATE_GENERIC_READ`</term>
        ///         <term>`VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT`</term>
        ///         <term>`VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`</term>
        ///     </item>
        /// </list>
        /// 
        /// Note that this resource type is required for resources that are created with <see cref="BufferUsage::Dynamic" />.
        /// </remarks>
        GenericRead = 0x00000006,

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
        ReadWrite = 0x00000007,

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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by <see cref="RenderPass" />es.
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
        virtual const String& name() const noexcept override;
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
        explicit RenderTarget(const String& name, const UInt32& location, const RenderTargetType& type, const Format& format, const bool& clearBuffer, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const bool& clearStencil = true, const bool& isVolatile = false, const BlendState& blendState = {});
        RenderTarget(const RenderTarget&) noexcept;
        RenderTarget(RenderTarget&&) noexcept;
        virtual ~RenderTarget() noexcept;

    public:
        inline RenderTarget& operator=(const RenderTarget&) noexcept;
        inline RenderTarget& operator=(RenderTarget&&) noexcept;

    public:
        /// <inheritdoc />
        virtual const String& name() const noexcept override;

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
        virtual void setMinDepth(const float& depth) const noexcept = 0;

        /// <summary>
        /// Gets the maximum depth of the viewport.
        /// </summary>
        /// <returns>The maximum depth of the viewport.</returns>
        virtual float getMaxDepth() const noexcept = 0;

        /// <summary>
        /// Sets the maximum depth of the viewport.
        /// </summary>
        /// <param name="depth">The maximum depth of the viewport.</param>
        virtual void setMaxDepth(const float& depth) const noexcept = 0;
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
        explicit Viewport(const RectF& clientRect = { }, const Float& minDepth = 0.f, const Float& maxDepth = 1.f);

        Viewport(Viewport&&) noexcept = delete;
        Viewport(const Viewport&) noexcept = delete;
        virtual ~Viewport() noexcept;

    public:
        /// <inheritdoc />
        virtual RectF getRectangle() const noexcept override;

        /// <inheritdoc />
        virtual void setRectangle(const RectF& rectangle) noexcept override;

        /// <inheritdoc />
        virtual Float getMinDepth() const noexcept override;

        /// <inheritdoc />
        virtual void setMinDepth(const Float& depth) const noexcept override;

        /// <inheritdoc />
        virtual Float getMaxDepth() const noexcept override;

        /// <inheritdoc />
        virtual void setMaxDepth(const Float& depth) const noexcept override;
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
        virtual RectF getRectangle() const noexcept override;

        /// <inheritdoc />
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
        virtual const UInt32& binding() const noexcept = 0;

        /// <summary>
        /// Returns the buffer type of the buffer.
        /// </summary>
        /// <returns>The buffer type of the buffer.</returns>
        virtual const BufferType& type() const noexcept = 0;
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
        virtual Array<const BufferAttribute*> attributes() const noexcept = 0;
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
        virtual const IndexType& indexType() const noexcept = 0;
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
        virtual const DescriptorType& descriptorType() const noexcept = 0;

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
        virtual const UInt32& descriptors() const noexcept = 0;

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
        virtual void map(const void* const data, const size_t& size, const UInt32& element = 0) = 0;

        /// <summary>
        /// Maps the memory blocks within <paramref name="data" /> to the internal memory of an array.
        /// </summary>
        /// <param name="data">The data blocks to map.</param>
        /// <param name="size">The size of each data block within <paramref name="data" />.</param>
        /// <param name="firsElement">The first element of the array to map.</param>
        virtual void map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement = 0) = 0;
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
        virtual const UInt32& elements() const noexcept = 0;

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
        virtual const bool& writable() const noexcept = 0;

        /// <summary>
        /// Returns the current state of the resource.
        /// </summary>
        /// <param name="subresource">The index of the sub-resource for which the state is requested.</param>
        /// <returns>The current state of the resource.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the specified sub-resource is not an element of the resource.</exception>
        virtual const ResourceState& state(const UInt32& subresource = 0) const = 0;

        /// <summary>
        /// Returns a reference of the current state of the resource.
        /// </summary>
        /// <remarks>
        /// This overload can be used to change the internal resource state. It exists, to support external resource transitions in certain scenarios, where 
        /// automatic resource state tracking is not supported. For example, there might be implicit state transitions in some scenarios. Usually those scenarios
        /// do not require you to transition the resource into another state, however if you have to, the internal state of the resource does not match the 
        /// actual state. In order for the barrier to be well-formed, you have to set the proper state first.
        /// 
        /// In most cases, however, use a <see cref="Barrier" /> to transition between resource states.
        /// </remarks>
        /// <param name="subresource">The index of the sub-resource for which the state is requested.</param>
        /// <returns>A reference of the current state of the resource.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the specified sub-resource is not an element of the resource.</exception>
        virtual ResourceState& state(const UInt32& subresource = 0) = 0;
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
        virtual const BufferType& type() const noexcept = 0;
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
        virtual size_t size(const UInt32& level) const noexcept = 0;

        /// <summary>
        /// Gets the extent of the image at a certain mip-map level.
        /// </summary>
        /// <remarks>
        /// Not all components of the extent are actually used. Check the <see cref="dimensions" /> to see, which components are required. The extent will be 0 for invalid 
        /// mip-map levels and 1 or more for valid mip map levels.
        /// </remarks>
        /// <returns>The extent of the image at a certain mip-map level.</returns>
        /// <seealso cref="dimensions" />
        virtual Size3d extent(const UInt32& level = 0) const noexcept = 0;

        /// <summary>
        /// Gets the internal format of the image.
        /// </summary>
        /// <returns>The internal format of the image.</returns>
        virtual const Format& format() const noexcept = 0;

        /// <summary>
        /// Gets the images dimensionality.
        /// </summary>
        /// <remarks>
        /// The dimensions imply various things, most importantly, which components of the <see cref="extent" /> are used. Note that cube maps behave like 2D images when the 
        /// extent is used.
        /// </remarks>
        /// <returns>The images dimensionality.</returns>
        virtual const ImageDimensions& dimensions() const noexcept = 0;

        /// <summary>
        /// Gets the number of mip-map levels of the image.
        /// </summary>
        /// <returns>The number of mip-map levels of the image.</returns>
        virtual const UInt32& levels() const noexcept = 0;

        /// <summary>
        /// Gets the number of layers (slices) of the image.
        /// </summary>
        /// <returns>The number of layers (slices) of the image.</returns>
        virtual const UInt32& layers() const noexcept = 0;

        /// <summary>
        /// Returns the number of planes of the image resource.
        /// </summary>
        /// <remarks>
        /// The number of planes is dictated by the image format.
        /// </remarks>
        /// <returns>The number of planes of the image resource.</returns>
        /// <seealso cref="format" />
        virtual const UInt32& planes() const noexcept = 0;

        /// <summary>
        /// Gets the number of samples of the texture.
        /// </summary>
        /// <returns>The number of samples of the texture.</returns>
        virtual const MultiSamplingLevel& samples() const noexcept = 0;

        // TODO: getSampler() for combined samplers?

    public:
        inline virtual UInt32 subresourceId(const UInt32& level, const UInt32& layer, const UInt32& plane) const noexcept
        {
            return level + (layer * this->levels()) + (plane * this->levels() * this->layers());
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
        virtual const FilterMode& getMinifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the filtering mode that is used for magnifying lookups.
        /// </summary>
        /// <returns>The filtering mode that is used for magnifying lookups.</returns>
        virtual const FilterMode& getMagnifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the horizontal border.
        /// </summary>
        /// <returns>The addressing mode at the horizontal border.</returns>
        virtual const BorderMode& getBorderModeU() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the vertical border.
        /// </summary>
        /// <returns>The addressing mode at the vertical border.</returns>
        virtual const BorderMode& getBorderModeV() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the depth border.
        /// </summary>
        /// <returns>The addressing mode at the depth border.</returns>
        virtual const BorderMode& getBorderModeW() const noexcept = 0;

        /// <summary>
        /// Gets the anisotropy value used when sampling this texture.
        /// </summary>
        /// <remarks>
        /// Anisotropy will be disabled, if this value is set to <c>0.0</c>.
        /// </remarks>
        /// <returns>The anisotropy value used when sampling this texture.</returns>
        virtual const Float& getAnisotropy() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map selection mode.
        /// </summary>
        /// <returns>The mip-map selection mode.</returns>
        virtual const MipMapMode& getMipMapMode() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map level of detail bias.
        /// </summary>
        /// <returns>The mip-map level of detail bias.</returns>
        virtual const Float& getMipMapBias() const noexcept = 0;

        /// <summary>
        /// Gets the maximum texture level of detail.
        /// </summary>
        /// <returns>The maximum texture level of detail.</returns>
        virtual const Float& getMaxLOD() const noexcept = 0;

        /// <summary>
        /// Gets the minimum texture level of detail.
        /// </summary>
        /// <returns>The minimum texture level of detail.</returns>
        virtual const Float& getMinLOD() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a memory barrier.
    /// </summary>
    class LITEFX_RENDERING_API IBarrier {
    public:
        virtual ~IBarrier() noexcept = default;

    public:
        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="buffer"/> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        void transition(IBuffer& buffer, const ResourceState& targetState) {
            this->doTransition(buffer, targetState);
        };

        /// <summary>
        /// Inserts a transition for the sub-resource <paramref name="element" /> of <paramref name="buffer" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="element">The element of the resource to transition.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        void transition(IBuffer& buffer, const UInt32& element, const ResourceState& targetState) {
            this->doTransition(buffer, element, targetState);
        }

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="buffer"/> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the resource from.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        void transition(IBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) {
            this->doTransition(buffer, sourceState, targetState);
        }

        /// <summary>
        /// Inserts a transition for the sub-resource <paramref name="element" /> of <paramref name="buffer" /> from <paramref name="sourceState" /> into 
        /// <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the sub-resource from.</param>
        /// <param name="element">The element of the resource to transition.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        void transition(IBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) {
            this->doTransition(buffer, sourceState, element, targetState);
        }

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="image"/> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        void transition(IImage& image, const ResourceState& targetState) {
            this->doTransition(image, targetState);
        }

        /// <summary>
        /// Inserts a transition for a sub-resource of <paramref name="image" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="level">The mip-map level of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        void transition(IImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) {
            this->doTransition(image, level, layer, plane, targetState);
        }

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="image"/> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the resource from.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        void transition(IImage& image, const ResourceState& sourceState, const ResourceState& targetState) {
            this->doTransition(image, sourceState, targetState);
        }

        /// <summary>
        /// Inserts a transition for a sub-resource of <paramref name="image" /> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the sub-resource from.</param>
        /// <param name="level">The mip-map level of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        void transition(IImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) {
            this->doTransition(image, sourceState, level, layer, plane, targetState);
        }

        /// <summary>
        /// Inserts a barrier that waits for all read/write accesses to <paramref name="buffer" /> to be finished before continuing.
        /// </summary>
        /// <remarks>
        /// This translates to a UAV barrier in DirectX 12 and an execution + memory barrier with no layout transition in Vulkan.
        /// </remarks>
        /// <typeparam name="buffer">The buffer to wait for.</typeparam>
        void waitFor(const IBuffer& buffer) {
            this->doWaitFor(buffer);
        }

        /// <summary>
        /// Inserts a barrier that waits for all read/write accesses to <paramref name="image" /> to be finished before continuing.
        /// </summary>
        /// <remarks>
        /// This translates to a UAV barrier in DirectX 12 and an execution + memory barrier with no layout transition in Vulkan.
        /// </remarks>
        /// <typeparam name="image">The image to wait for.</typeparam>
        void waitFor(const IImage& image) {
            this->doWaitFor(image);
        }

    private:
        virtual void doTransition(IBuffer& buffer, const ResourceState& targetState) = 0;
        virtual void doTransition(IBuffer& buffer, const UInt32& element, const ResourceState& targetState) = 0;
        virtual void doTransition(IBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) = 0;
        virtual void doTransition(IBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) = 0;
        virtual void doTransition(IImage& image, const ResourceState& targetState) = 0;
        virtual void doTransition(IImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;
        virtual void doTransition(IImage& image, const ResourceState& sourceState, const ResourceState& targetState) = 0;
        virtual void doTransition(IImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;
        virtual void doWaitFor(const IBuffer& buffer) = 0;
        virtual void doWaitFor(const IImage& image) = 0;
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
        void update(const UInt32& binding, const IBuffer& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 0, const UInt32& firstDescriptor = 0) const {
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
        void update(const UInt32& binding, const IImage& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const {
            this->doUpdate(binding, texture, descriptor, firstLevel, levels, firstLayer, layers);
        }

        /// <summary>
        /// Updates a sampler within the current descriptor set.
        /// </summary>
        /// <param name="binding">The sampler binding point.</param>
        /// <param name="sampler">The sampler to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the sampler to.</param>
        void update(const UInt32& binding, const ISampler& sampler, const UInt32& descriptor = 0) const {
            this->doUpdate(binding, sampler, descriptor);
        }

        /// <summary>
        /// Attaches an image as an input attachment to a descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="image">The image to bind to the input attachment descriptor.</param>
        void attach(const UInt32& binding, const IImage& image) const {
            this->doAttach(binding, image);
        }

    private:
        virtual void doUpdate(const UInt32& binding, const IBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const = 0;
        virtual void doUpdate(const UInt32& binding, const IImage& texture, const UInt32& descriptor, const UInt32& firstLevel, const UInt32& levels, const UInt32& firstLayer, const UInt32& layers) const = 0;
        virtual void doUpdate(const UInt32& binding, const ISampler& sampler, const UInt32& descriptor) const = 0;
        virtual void doAttach(const UInt32& binding, const IImage& image) const = 0;
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
        Array<const IDescriptorLayout*> descriptors() const noexcept {
            return this->getDescriptors();
        }

        /// <summary>
        /// Returns the descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the requested descriptor layout.</param>
        /// <returns>The descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.</returns>
        virtual const IDescriptorLayout& descriptor(const UInt32& binding) const = 0;

        /// <summary>
        /// Returns the space index of the descriptor set.
        /// </summary>
        /// <remarks>
        /// The descriptor set space maps to the space index in HLSL and the set index in GLSL.
        /// </remarks>
        /// <returns>The space index of the descriptor set.</returns>
        virtual const UInt32& space() const noexcept = 0;

        /// <summary>
        /// Returns the shader stages, the descriptor set is used in.
        /// </summary>
        /// <returns>The shader stages, the descriptor set is used in.</returns>
        virtual const ShaderStage& shaderStages() const noexcept = 0;

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
        UniquePtr<IDescriptorSet> allocate(const Array<DescriptorBinding>& bindings = { }) const {
            return this->allocate(0, bindings);
        }

        /// <summary>
        /// Allocates a new descriptor set or returns an instance of an unused descriptor set.
        /// </summary>
        /// <param name="descriptors">The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.</param>
        /// <param name="bindings">Optional default bindings for descriptors in the descriptor set.</param>
        /// <returns>The instance of the descriptor set.</returns>
        /// <seealso cref="IDescriptorLayout" />
        UniquePtr<IDescriptorSet> allocate(const UInt32& descriptors, const Array<DescriptorBinding>& bindings = { }) const {
            return this->getDescriptorSet(descriptors, bindings);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="bindings">Optional default bindings for descriptors in each descriptor set.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        Array<UniquePtr<IDescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const Array<Array<DescriptorBinding>>& bindings = { }) const {
            return this->allocateMultiple(descriptorSets, 0, bindings);
        }

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <param name="bindingFactory">A factory function that is called for each descriptor set in order to provide the default bindings.</param>
        /// <returns>The array of descriptor set instances.</returns>
        /// <seealso cref="allocate" />
        Array<UniquePtr<IDescriptorSet>> allocateMultiple(const UInt32& descriptorSets, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const {
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
        Array<UniquePtr<IDescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, const Array<Array<DescriptorBinding>>& bindings = { }) const {
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
        Array<UniquePtr<IDescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const {
            return this->getDescriptorSets(descriptorSets, descriptors, bindingFactory);
        }

        /// <summary>
        /// Marks a descriptor set as unused, so that it can be handed out again instead of allocating a new one.
        /// </summary>
        /// <seealso cref="allocate" />
        void free(const IDescriptorSet& descriptorSet) const noexcept {
            this->releaseDescriptorSet(descriptorSet);
        }

    private:
        virtual Array<const IDescriptorLayout*> getDescriptors() const noexcept = 0;
        virtual UniquePtr<IDescriptorSet> getDescriptorSet(const UInt32& descriptors, const Array<DescriptorBinding>& bindings = { }) const = 0;
        virtual Array<UniquePtr<IDescriptorSet>> getDescriptorSets(const UInt32& descriptorSets, const UInt32& descriptors, const Array<Array<DescriptorBinding>>& bindings = { }) const = 0;
        virtual Array<UniquePtr<IDescriptorSet>> getDescriptorSets(const UInt32& descriptorSets, const UInt32& descriptors, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const = 0;
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
        virtual const UInt32& space() const noexcept = 0;

        /// <summary>
        /// Returns the binding point or register, the push constants are made available at.
        /// </summary>
        /// <returns>The binding point or register, the push constants are made available at.</returns>
        virtual const UInt32& binding() const noexcept = 0;

        /// <summary>
        /// Returns the offset from the push constants backing memory block, the range starts at.
        /// </summary>
        /// <returns>The offset from the push constants backing memory block, the range starts at.</returns>
        /// <seealso cref="size" />
        virtual const UInt32& offset() const noexcept = 0;

        /// <summary>
        /// Returns the size (in bytes) of the range.
        /// </summary>
        /// <returns>The size (in bytes) of the range.</returns>
        /// <seealso cref="offset" />
        virtual const UInt32& size() const noexcept = 0;

        /// <summary>
        /// Returns the shader stage(s), the range is accessible from.
        /// </summary>
        /// <returns>The shader stage(s), the range is accessible from.</returns>
        virtual const ShaderStage& stage() const noexcept = 0;
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
        virtual const UInt32& size() const noexcept = 0;

        /// <summary>
        /// Returns the push constant range associated with the shader stage provided in <paramref name="stage" />.
        /// </summary>
        /// <param name="stage">The shader stage to request the associated push constant range for. Specifying multiple stages is not supported and will raise an exception.</param>
        /// <returns>The push constant range associated with the provided shader stage.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if no range is mapped to the provided shader stage.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="stage" /> contains multiple shader stages.</exception>
        /// <seealso cref="ranges" />
        virtual const IPushConstantsRange& range(const ShaderStage& stage) const = 0;

        /// <summary>
        /// Returns all push constant ranges.
        /// </summary>
        /// <returns>All push constant ranges.</returns>
        /// <seealso cref="range" />
        Array<const IPushConstantsRange*> ranges() const noexcept {
            return this->getRanges();
        }

    private:
        virtual Array<const IPushConstantsRange*> getRanges() const noexcept = 0;
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
        Array<const IShaderModule*> modules() const noexcept {
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
        SharedPtr<IPipelineLayout> reflectPipelineLayout() const {
            return this->parsePipelineLayout();
        };

    private:
        virtual Array<const IShaderModule*> getModules() const noexcept = 0;
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
        virtual const IDescriptorSetLayout& descriptorSet(const UInt32& space) const = 0;

        /// <summary>
        /// Returns all descriptor set layouts, the pipeline has been initialized with.
        /// </summary>
        /// <returns>All descriptor set layouts, the pipeline has been initialized with.</returns>
        Array<const IDescriptorSetLayout*> descriptorSets() const noexcept {
            return this->getDescriptorSets();
        }

        /// <summary>
        /// Returns the push constants layout, or <c>nullptr</c>, if the pipeline does not use any push constants.
        /// </summary>
        /// <returns>The push constants layout, or <c>nullptr</c>, if the pipeline does not use any push constants.</returns>
        virtual const IPushConstantsLayout* pushConstants() const noexcept = 0;

    private:
        virtual Array<const IDescriptorSetLayout*> getDescriptorSets() const noexcept = 0;
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
        Array<const IVertexBufferLayout*> vertexBufferLayouts() const noexcept {
            return this->getVertexBufferLayouts();
        }

        /// <summary>
        /// Returns the vertex buffer layout for binding provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the vertex buffer layout.</param>
        /// <returns>The vertex buffer layout for binding provided with <paramref name="binding" />.</returns>
        virtual const IVertexBufferLayout& vertexBufferLayout(const UInt32& binding) const = 0;

        /// <summary>
        /// Returns the index buffer layout.
        /// </summary>
        /// <returns>The index buffer layout.</returns>
        virtual const IIndexBufferLayout& indexBufferLayout() const = 0;

        /// <summary>
        /// Returns the primitive topology.
        /// </summary>
        /// <returns>The primitive topology.</returns>
        virtual const PrimitiveTopology& topology() const noexcept = 0;

    private:
        virtual Array<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept = 0;
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
        SharedPtr<const IShaderProgram> program() const noexcept {
            return this->getProgram();
        }

        /// <summary>
        /// Returns the layout of the render pipeline.
        /// </summary>
        /// <returns>The layout of the render pipeline.</returns>
        SharedPtr<const IPipelineLayout> layout() const noexcept {
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
        /// <param name="invert">If set to <c>true</c>, the barrier will perform a transition back to the original resource states.</param>
        void barrier(const IBarrier& barrier, const bool& invert = false) const noexcept {
            this->cmdBarrier(barrier, invert);
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
        void generateMipMaps(IImage& image) noexcept {
            this->cmdGenerateMipMaps(image);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="targetElement">The index of the first element in the target buffer to copy to.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target buffer.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(const IBuffer& source, const IBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const {
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
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="targetElement">The index of the first element in the target buffer to copy to.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target buffer.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(SharedPtr<const IBuffer> source, const IBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const {
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
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(const IBuffer& source, const IImage& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const {
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
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(SharedPtr<const IBuffer> source, const IImage& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// <summary>
        /// Performs an image-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetSubresource">The image of the first sub-resource in the target image to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy between the images.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(const IImage& source, const IImage& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const {
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
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetSubresource">The image of the first sub-resource in the target image to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy between the images.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(SharedPtr<const IImage> source, const IImage& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const {
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
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetElement">The index of the first target element to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(const IImage& source, const IBuffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const {
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
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetElement">The index of the first target element to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        void transfer(SharedPtr<const IImage> source, const IBuffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        void use(const IPipeline& pipeline) const noexcept {
            this->cmdUse(pipeline);
        }

        // TODO: Allow bind to last used pipeline (throw, if no pipeline is in use).
        //void bind(const IDescriptorSet& descriptorSet) const;

        /// <summary>
        /// Binds the provided descriptor set to the provided pipeline.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        void bind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept {
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
        void bind(const IVertexBuffer& buffer) const noexcept {
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
        void bind(const IIndexBuffer& buffer) const noexcept {
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
        virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const noexcept = 0;

        /// <summary>
        /// Draws the currently bound vertex buffer with a set of indices from the currently bound index buffer.
        /// </summary>
        /// <param name="indices">The number of indices to draw.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const noexcept = 0;

        /// <summary>
        /// Pushes a block of memory into the push constants backing memory.
        /// </summary>
        /// <param name="layout">The layout of the push constants to update.</param>
        /// <param name="memory">A pointer to the source memory.</param>
        void pushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept {
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
        void draw(const IVertexBuffer& vertexBuffer, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const {
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
        void drawIndexed(const IIndexBuffer& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
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
        void drawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
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
        virtual void setStencilRef(const UInt32& stencilRef) const noexcept = 0;

    private:
        virtual void cmdBarrier(const IBarrier& barrier, const bool& invert) const noexcept = 0;
        virtual void cmdGenerateMipMaps(IImage& image) noexcept = 0;
        virtual void cmdTransfer(const IBuffer& source, const IBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const = 0;
        virtual void cmdTransfer(const IBuffer& source, const IImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const = 0;
        virtual void cmdTransfer(const IImage& source, const IImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const = 0;
        virtual void cmdTransfer(const IImage& source, const IBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<const IBuffer> source, const IBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const = 0;
        virtual void cmdTransfer(SharedPtr<const IBuffer> source, const IImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const = 0;
        virtual void cmdTransfer(SharedPtr<const IImage> source, const IImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<const IImage> source, const IBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const = 0;
        virtual void cmdUse(const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IVertexBuffer& buffer) const noexcept = 0;
        virtual void cmdBind(const IIndexBuffer& buffer) const noexcept = 0;
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept = 0;
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const = 0;
        
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
        SharedPtr<IInputAssembler> inputAssembler() const noexcept {
            return this->getInputAssembler();
        }

        /// <summary>
        /// Returns the rasterizer state used by the render pipeline.
        /// </summary>
        /// <returns>The rasterizer state used by the render pipeline.</returns>
        SharedPtr<IRasterizer> rasterizer() const noexcept {
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
        virtual const bool& alphaToCoverage() const noexcept = 0;

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
        virtual const UInt32& bufferIndex() const noexcept = 0;

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
        Array<SharedPtr<const ICommandBuffer>> commandBuffers() const noexcept {
            return this->getCommandBuffers();
        }

        /// <summary>
        /// Returns a command buffer that records draw commands for the frame buffer.
        /// </summary>
        /// <param name="index">The index of the command buffer.</param>
        /// <returns>A command buffer that records draw commands for the frame buffer</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the frame buffer does not store a command buffer at <paramref name="index" />.</exception>
        /// <seealso cref="commandBuffers" />
        SharedPtr<const ICommandBuffer> commandBuffer(const UInt32& index) const {
            return this->getCommandBuffer(index);
        }

        /// <summary>
        /// Returns the images that store the output attachments for the render targets of the <see cref="RenderPass" />.
        /// </summary>
        /// <returns>The images that store the output attachments for the render targets of the <see cref="RenderPass" />.</returns>
        Array<const IImage*> images() const noexcept {
            return this->getImages();
        }

        /// <summary>
        /// Returns the image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.
        /// </summary>
        /// <returns>The image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.</returns>
        virtual const IImage& image(const UInt32& location) const = 0;

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
        virtual SharedPtr<const ICommandBuffer> getCommandBuffer(const UInt32& index) const noexcept = 0;
        virtual Array<SharedPtr<const ICommandBuffer>> getCommandBuffers() const noexcept = 0;
        virtual Array<const IImage*> getImages() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a render pass.
    /// </summary>
    class LITEFX_RENDERING_API IRenderPass : public virtual IStateResource {
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
        /// Returns a list of all frame buffers.
        /// </summary>
        /// <returns>A list of all frame buffers. </returns>
        Array<const IFrameBuffer*> frameBuffers() const noexcept {
            return this->getFrameBuffers();
        }

        /// <summary>
        /// Returns an array of all render pipelines, owned by the render pass.
        /// </summary>
        /// <returns>An array of all render pipelines, owned by the render pass.</returns>
        /// <seealso cref="IRenderPipeline" />
        Array<const IRenderPipeline*> pipelines() const noexcept {
            return this->getPipelines();
        }

        /// <summary>
        /// Returns the render target mapped to the location provided by <paramref name="location" />.
        /// </summary>
        /// <param name="location">The location to return the render target for.</param>
        /// <returns>The render target mapped to the location provided by <paramref name="location" />.</returns>
        virtual const RenderTarget& renderTarget(const UInt32& location) const = 0;

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
        virtual const MultiSamplingLevel& multiSamplingLevel() const noexcept = 0;

    public:
        /// <summary>
        /// Begins the render pass.
        /// </summary>
        /// <param name="buffer">The back buffer to use. Typically this is the same as the value returned from <see cref="ISwapChain::swapBackBuffer" />.</param>
        virtual void begin(const UInt32& buffer) = 0;

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
        virtual void changeMultiSamplingLevel(const MultiSamplingLevel& samples) = 0;

        /// <summary>
        /// Resolves the input attachments mapped to the render pass and updates them on the descriptor set provided with <see cref="descriptorSet" />.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to update the input attachments on.</param>
        void updateAttachments(const IDescriptorSet& descriptorSet) const {
            this->setAttachments(descriptorSet);
        }

    private:
        virtual Array<const IFrameBuffer*> getFrameBuffers() const noexcept = 0;
        virtual Array<const IRenderPipeline*> getPipelines() const noexcept = 0;
        virtual void setAttachments(const IDescriptorSet& descriptorSet) const = 0;
    };

    /// <summary>
    /// Interface for a swap chain.
    /// </summary>
    class LITEFX_RENDERING_API ISwapChain {
    public:
        virtual ~ISwapChain() noexcept = default;

    public:
        /// <summary>
        /// Returns the swap chain image format.
        /// </summary>
        /// <returns>The swap chain image format.</returns>
        virtual const Format& surfaceFormat() const noexcept = 0;

        /// <summary>
        /// Returns the number of images in the swap chain.
        /// </summary>
        /// <returns>The number of images in the swap chain.</returns>
        virtual const UInt32& buffers() const noexcept = 0;

        /// <summary>
        /// Returns the size of the render area.
        /// </summary>
        /// <returns>The size of the render area.</returns>
        virtual const Size2d& renderArea() const noexcept = 0;

        /// <summary>
        /// Returns an array of the swap chain present images.
        /// </summary>
        /// <returns>Returns an array of the swap chain present images.</returns>
        Array<const IImage*> images() const noexcept {
            return this->getImages();
        };

        /// <summary>
        /// Queues a present that gets executed after <paramref name="frameBuffer" /> signals its readiness.
        /// </summary>
        /// <param name="frameBuffer">The frame buffer for which the present should wait.</param>
        virtual void present(const IFrameBuffer& frameBuffer) const = 0;

    public:
        /// <summary>
        /// Returns an array of supported formats, that can be drawn to the surface.
        /// </summary>
        /// <returns>An array of supported formats, that can be drawn to the surface.</returns>
        /// <see cref="surface" />
        /// <seealso cref="ISurface" />
        virtual Array<Format> getSurfaceFormats() const noexcept = 0;

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
        virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) = 0;

        /// <summary>
        /// Swaps the front buffer with the next back buffer in order.
        /// </summary>
        /// <returns>A reference of the front buffer after the buffer swap.</returns>
        [[nodiscard]] virtual UInt32 swapBackBuffer() const = 0;

    private:
        virtual Array<const IImage*> getImages() const noexcept = 0;
    };

    /// <summary>
    /// The interface for a command queue.
    /// </summary>
    class LITEFX_RENDERING_API ICommandQueue {
    public:
        virtual ~ICommandQueue() noexcept = default;

    public:
        /// <summary>
        /// Returns <c>true</c>, if the command queue is bound on the parent device.
        /// </summary>
        /// <remarks>
        /// Before a command queue can receive commands, it needs to be bound to a device. This ensures, that the queue is actually able to allocate commands. A 
        /// command queue starts in unbound state until <see cref="bind" /> gets called. Destroying the queue also releases it by calling <see cref="release" />.
        /// </remarks>
        /// <seealso cref="bind" />
        /// <seealso cref="release" />
        /// <returns><c>true</c>, if the command queue is bound on a device.</returns>
        virtual bool isBound() const noexcept = 0;

        /// <summary>
        /// Returns the priority of the queue.
        /// </summary>
        /// <returns>The priority of the queue.</returns>
        virtual const QueuePriority& priority() const noexcept = 0;

        /// <summary>
        /// Returns the type of the queue.
        /// </summary>
        /// <returns>The type of the queue.</returns>
        virtual const QueueType& type() const noexcept = 0;

    public:
        /// <summary>
        /// Starts a new debug region.
        /// </summary>
        /// <remarks>
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        /// <param name="label">The name of the debug region.</param>
        /// <param name="color">The color of the debug region.</param>
        virtual void BeginDebugRegion(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept { };
        
        /// <summary>
        /// Ends the current debug region.
        /// </summary>
        /// <remarks>
        /// This is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        virtual void EndDebugRegion() const noexcept { };

        /// <summary>
        /// Inserts a debug marker.
        /// </summary>
        /// <remarks>
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        /// </remarks>
        /// <param name="label">The name of the debug marker.</param>
        /// <param name="color">The color of the debug marker.</param>
        virtual void SetDebugMarker(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept { };

    public:
        /// <summary>
        /// Binds the queue on the parent device.
        /// </summary>
        /// <seealso cref="isBound" />
        virtual void bind() = 0;

        /// <summary>
        /// Releases the queue from the parent device.
        /// </summary>
        virtual void release() = 0;

        /// <summary>
        /// Creates a command buffer that can be used to allocate commands on the queue.
        /// </summary>
        /// <param name="beginRecording">If set to <c>true</c>, the command buffer will be initialized in recording state and can receive commands straight away.</param>
        /// <returns>The instance of the command buffer.</returns>
        SharedPtr<ICommandBuffer> createCommandBuffer(const bool& beginRecording = false) const {
            return this->getCommandBuffer(beginRecording);
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
        UInt64 submit(SharedPtr<const ICommandBuffer> commandBuffer) const {
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
        UInt64 submit(const Array<SharedPtr<const ICommandBuffer>>& commandBuffers) const {
            return this->submitCommandBuffers(commandBuffers);
        }

        /// <summary>
        /// Waits for a certain fence value to complete on the command queue.
        /// </summary>
        /// <remarks>
        /// Each time one or more command buffers are submitted to the queue, a fence is inserted and its value will be returned. By calling this method, it is possible to
        /// wait for this fence. A fence value is guaranteed to be larger than earlier fences, so the method returns, if the latest signaled fence value is larger or equal
        /// to the value specified in <paramref name="fence" />. 
        /// 
        /// Note that this behavior can cause overflows when performing *excessive* fencing! Take for example a scenario, where each frame requires 80 fences to be signaled
        /// and an application that runs at 60 frames per second in average. In this case, each second 4.800 fences are inserted into the queue. Given the limit of an 64
        /// bit unsigned integer fence value, the application can run ~2.9 billion years before overflowing. Drop me an e-mail or open an issue, if you ever happen to run 
        /// into such a situation.
        /// </remarks>
        /// <param name="fence">The value of the fence to wait for.</param>
        /// <seealso cref="submit" />
        virtual void waitFor(const UInt64& fence) const noexcept = 0;

        /// <summary>
        /// Returns the value of the latest fence inserted into the queue.
        /// </summary>
        /// <returns>The value of the latest fence inserted into the queue.</returns>
        /// <seealso cref="waitFor" />
        virtual UInt64 currentFence() const noexcept = 0;

    private:
        virtual SharedPtr<ICommandBuffer> getCommandBuffer(const bool& beginRecording) const = 0;
        virtual UInt64 submitCommandBuffer(SharedPtr<const ICommandBuffer> commandBuffer) const = 0;
        virtual UInt64 submitCommandBuffers(const Array<SharedPtr<const ICommandBuffer>>& commandBuffers) const = 0;
        
    protected:
        void releaseSharedState(const ICommandBuffer& commandBuffer) const {
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
        UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1, const bool& allowWrite = false) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), usage, descriptor.elementSize(), elements, allowWrite);
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
        UniquePtr<IBuffer> createBuffer(const IPipeline& pipeline, const UInt32& space, const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const String& name, const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, const UInt32& binding, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, const UInt32& space, const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1, const bool& allowWrite = false) const {
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
        UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, const UInt32& space, const UInt32& binding, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const {
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
        UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const {
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
        UniquePtr<IVertexBuffer> createVertexBuffer(const String& name, const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const {
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
        UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const {
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
        UniquePtr<IIndexBuffer> createIndexBuffer(const String& name, const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const {
            return this->getIndexBuffer(name, layout, usage, elements);
        }

        /// <summary>
        /// Creates an image that is used as render target attachment.
        /// </summary>
        /// <param name="format">The format of the image.</param>
        /// <param name="size">The extent of the image.</param>
        /// <param name="samples">The number of samples, the image should be sampled with.</param>
        /// <returns>The instance of the attachment image.</returns>
        UniquePtr<IImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const {
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
        UniquePtr<IImage> createAttachment(const String& name, const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const {
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
        UniquePtr<IImage> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const {
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
        UniquePtr<IImage> createTexture(const String& name, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const {
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
        Array<UniquePtr<IImage>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& layers = 1, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const {
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
        UniquePtr<ISampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const {
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
        UniquePtr<ISampler> createSampler(const String& name, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const {
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
        Array<UniquePtr<ISampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const {
            return this->getSamplers(elements, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

    private:
        virtual UniquePtr<IBuffer> getBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite) const = 0;
        virtual UniquePtr<IBuffer> getBuffer(const String& name, const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;
        virtual UniquePtr<IImage> getAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const = 0;
        virtual UniquePtr<IImage> getAttachment(const String& name, const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const = 0;
        virtual UniquePtr<IImage> getTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& allowWrite) const = 0;
        virtual UniquePtr<IImage> getTexture(const String& name, const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& allowWrite) const = 0;
        virtual Array<UniquePtr<IImage>> getTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& layers, const UInt32& levels, const MultiSamplingLevel& samples, const bool& allowWrite) const = 0;
        virtual UniquePtr<ISampler> getSampler(const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const = 0;
        virtual UniquePtr<ISampler> getSampler(const String& name, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const = 0;
        virtual Array<UniquePtr<ISampler>> getSamplers(const UInt32& elements, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const = 0;
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
        /// Returns the instance of the queue, used to process draw calls.
        /// </summary>
        /// <returns>The instance of the queue, used to process draw calls.</returns>
        virtual const ICommandQueue& graphicsQueue() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue used for device-device transfers (e.g. between render-passes).
        /// </summary>
        /// <remarks>
        /// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
        /// </remarks>
        /// <returns>The instance of the queue used for device-device transfers (e.g. between render-passes).</returns>
        virtual const ICommandQueue& transferQueue() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue used for host-device transfers.
        /// </summary>
        /// <remarks>
        /// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
        /// </remarks>
        /// <returns>The instance of the queue used for host-device transfers.</returns>
        virtual const ICommandQueue& bufferQueue() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue used for compute calls.
        /// </summary>
        /// <remarks>
        /// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated compute queues are supported on the device.
        /// </remarks>
        /// <returns>The instance of the queue used for compute calls.</returns>
        virtual const ICommandQueue& computeQueue() const noexcept = 0;

        /// <summary>
        /// Creates a memory barrier instance.
        /// </summary>
        /// <returns>The instance of the memory barrier.</returns>
        UniquePtr<IBarrier> makeBarrier() const noexcept {
            return this->getNewBarrier();
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
        virtual MultiSamplingLevel maximumMultiSamplingLevel(const Format& format) const noexcept = 0;

    public:
        /// <summary>
        /// Waits until the device is idle.
        /// </summary>
        /// <remarks>
        /// The complexity of this operation may depend on the graphics API that implements this method. Calling this method guarantees, that the device resources are in an unused state and 
        /// may safely be released.
        /// </remarks>
        virtual void wait() const = 0;

    private:
        virtual UniquePtr<IBarrier> getNewBarrier() const noexcept = 0;
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
        Array<const IGraphicsAdapter*> listAdapters() const {
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
        virtual const IGraphicsDevice* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// <summary>
        /// Looks up a device and returns a pointer to it, or <c>nullptr</c>, if no device with the provided <paramref name="name" /> could be found.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        /// <returns>A pointer to the device or <c>nullptr</c>, if no device could be found.</returns>
        virtual IGraphicsDevice* operator[](const String& name) noexcept {
            return this->device(name);
        };

    private:
        virtual Array<const IGraphicsAdapter*> getAdapters() const = 0;
    };
}