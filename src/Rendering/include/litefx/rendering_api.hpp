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
    class IAccelerationStructure;
    class IBottomLevelAccelerationStructure;
    class ITopLevelAccelerationStructure;
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
    class IRayTracingPipeline;
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
        /// Do not use this queue priority when creating queues, as it is reserved for the default (built-in) queues.
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

        /// <summary>
        /// Represents a ray-tracing acceleration structure.
        /// </summary>
        AccelerationStructure = 0x00000008
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
        /// Describes an acceleration structure buffer.
        /// </summary>
        /// <seealso cref="ICommandBuffer::buildAccelerationStructure" />
        /// <seealso cref="IBottomLevelAccelerationStructure" />
        /// <seealso cref="ITopLevelAccelerationStructure" />
        AccelerationStructure = 0x00000006,

        /// <summary>
        /// Describes a shader binding table for ray-tracing.
        /// </summary>
        ShaderBindingTable = 0x00000007,

        /// <summary>
        /// Describes another type of buffer, such as samplers or images.
        /// </summary>
        /// <remarks>
        /// Buffers of this type must not be bound to any descriptor, but can be used as copy/transfer targets and sources.
        /// </remarks>
        Other = 0x7FFFFFFF
    };

    /// <summary>
    /// Defines where a resource (buffer or image) memory is located and from where it can be accessed.
    /// </summary>
    /// <remarks>
    /// There are three common memory usage scenarios that are supported by the engine:
    ///
    /// <list type="number">
    /// <item>
    /// <description>
    /// <strong>Static resources</strong>: such as vertex/index/constant buffers, textures or other infrequently updated buffers. In this case, the most efficient 
    /// approach is to create a buffer using <see cref="ResourceHeap::Staging" /> and map it from the CPU. Create a second buffer using 
    /// <see cref="ResourceHeap::Resource" /> and transfer the staging buffer into it.
    /// </description>
    /// </item>
    /// <item>
    /// <description>
    /// <strong>Dynamic resources</strong>: such as deformable meshes or buffers that need to be updated every frame. For such buffers use the
    /// <see cref="ResourceHeap::Dynamic" /> mode to prevent regular transfer overhead.
    /// </description>
    /// </item>
    /// <item>
    /// <description>
    /// <strong>Readbacks</strong>: or resources that are written on the GPU and read by the CPU. The usage mode <see cref="ResourceHeap::Readback" /> is designed to 
    /// provide the best performance for this special case.
    /// </description>
    /// </item>
    /// </list>
    /// 
    /// Note that image resources cannot be created on heaps different to <see cref="ResourceHeap::Resource" />. For this very reason, the graphics factory does not 
    /// allow to specify the resource heap when creating images or attachments.
    /// </remarks>
    /// <seealso cref="IGraphicsFactory" />
    /// <seealso cref="IBuffer" />
    enum class LITEFX_RENDERING_API ResourceHeap {
        /// <summary>
        /// Creates a resource that can be mapped from the CPU in order to be transferred to the GPU later.
        /// </summary>
        /// <remarks>
        /// The memory for the resource will be allocated in the DRAM (CPU or host memory). It can be optimally accessed by the CPU in order to be written. However,
        /// reading it from the GPU is not supported. This usage mode should be used to create a staging buffer, i.e. a buffer that is written infrequently and
        /// then transferred to another buffer, that uses <see cref="ResourceHeap::Resource" />.
        /// </remarks>
        Staging = 0x00000001,

        /// <summary>
        /// Creates a resource that can be read by the GPU.
        /// </summary>
        /// <remarks>
        /// The memory for the resource will be allocated on the VRAM (GPU or device memory). It can be optimally accessed by the GPU in order to be read frequently.
        /// It can be written by a transfer call, but is inaccessible from the CPU.
        /// </remarks>
        Resource = 0x00000002,

        /// <summary>
        /// Creates a resource that can be mapped from the CPU and read by the GPU.
        /// </summary>
        /// <remarks>
        /// Dynamic buffers are used when the content is expected to be changed every frame. They do not require transfer calls, but may not be read as efficiently
        /// as <see cref="ResourceHeap::Resource" /> buffers.
        /// </remarks>
        Dynamic = 0x00000010,

        /// <summary>
        /// Creates a buffer that can be written on the GPU and read by the CPU.
        /// </summary>
        Readback = 0x00000100
    };

    /// <summary>
    /// Describes the intended usage for a resource.
    /// </summary>
    /// <seealso cref="IGraphicsFactory" />
    /// <seealso cref="IBuffer" />
    /// <seealso cref="IImage" />
    enum class LITEFX_RENDERING_API ResourceUsage {
        /// <summary>
        /// The resource is created without any special usage settings.
        /// </summary>
        None = 0x0000,

        /// <summary>
        /// Allows the resource to be written to.
        /// </summary>
        /// <remarks>
        /// This flag is not allowed for vertex buffers (<see cref="BufferType::Vertex" />), index buffers (<see cref="BufferType::Index" />) and uniform buffers (<see cref="BufferType::Uniform" />).
        /// </remarks>
        /// <seealso cref="IDeviceMemory::writable" />
        AllowWrite = 0x0001,

        /// <summary>
        /// Allows the resource data to be copied into another resource.
        /// </summary>
        /// <remarks>
        /// This flag is implicitly set for resources created with <see cref="ResourceHeap::Staging" /> and for render target images (attachments).
        /// </remarks>
        TransferSource = 0x0010,

        /// <summary>
        /// Allows the resource data to be copied from another resource.
        /// </summary>
        /// <remarks>
        /// This flag is implicitly set for resources created with <see cref="ResourceHeap::Readback" /> and for render target images (attachments).
        /// </remarks>
        TransferDestination = 0x0020,

        /// <summary>
        /// Allows the resource to be used as a render target.
        /// </summary>
        RenderTarget = 0x0040,

        /// <summary>
        /// Allows the resource to be used to build acceleration structures.
        /// </summary>
        /// <remarks>
        /// This flag is not allowed for images and other acceleration structures (<see cref="BufferType::AccelerationStructure" />).
        /// </remarks>
        /// <seealso cref="IAccelerationStructure" />
        AccelerationStructureBuildInput = 0x0100,

        /// <summary>
        /// Shortcut for commonly used `TransferSource | TransferDestination` combination.
        /// </summary>
        Default = TransferSource | TransferDestination,

        /// <summary>
        /// Default usage for frame buffer images.
        /// </summary>
        /// <seealso cref="IFrameBuffer" />
        FrameBufferImage = TransferSource | RenderTarget,
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
        /// Represents the geometry shader stage.
        /// </summary>
        /// <remarks>
        /// Note that geometry shaders come with a performance penalty and might not be supported on all platforms. If you can, avoid using them.
        /// </remarks>
        Geometry = 0x00000008,

        /// <summary>
        /// Represents the fragment or pixel shader stage.
        /// </summary>
        Fragment = 0x00000010,

        /// <summary>
        /// Represents the compute shader stage.
        /// </summary>
        Compute = 0x00000020,

        /// <summary>
        /// Represents the task or amplification shader stage.
        /// </summary>
        Task = 0x00000040,

        /// <summary>
        /// Represents the mesh shader stage.
        /// </summary>
        Mesh = 0x00000080,

        /// <summary>
        /// Represents the ray generation shader stage.
        /// </summary>
        RayGeneration = 0x00000100,

        /// <summary>
        /// Represents the any-hit shader stage.
        /// </summary>
        AnyHit = 0x00000200,

        /// <summary>
        /// Represents the closest-hit shader stage.
        /// </summary>
        ClosestHit = 0x00000400,

        /// <summary>
        /// Represents the miss shader stage.
        /// </summary>
        Miss = 0x00000800,

        /// <summary>
        /// Represents the intersection shader stage.
        /// </summary>
        Intersection = 0x00001000,

        /// <summary>
        /// Represents the callable shader stage.
        /// </summary>
        Callable = 0x00002000,

        /// <summary>
        /// Represents the complete rasterization pipeline.
        /// </summary>
        RasterizationPipeline = Vertex | Geometry | TessellationControl | TessellationEvaluation | Fragment,

        /// <summary>
        /// Represents the complete mesh shading pipeline.
        /// </summary>
        MeshPipeline = Task | Mesh | Fragment,

        /// <summary>
        /// Represents the complete ray-tracing pipeline.
        /// </summary>
        RayTracingPipeline = RayGeneration | AnyHit | ClosestHit | Miss | Intersection | Callable,

        /// <summary>
        /// Enables all supported shader stages.
        /// </summary>
        Any = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment | Compute | Task | Mesh | RayGeneration | AnyHit | ClosestHit | Miss | Intersection | Callable,

        /// <summary>
        /// Represents an unknown shader stage.
        /// </summary>
        Other = 0x7FFFFFFF
    };

    /// <summary>
    /// Describes the type of a shader module record within a shader collection or shader binting table.
    /// </summary>
    /// <seealso cref="IShaderRecord" />
    enum class LITEFX_RENDERING_API ShaderRecordType {
        /// <summary>
        /// Represents a ray generation shader record.
        /// </summary>
        RayGeneration = 0x01,

        /// <summary>
        /// Represents a hit group shader record.
        /// </summary>
        HitGroup = 0x02,

        /// <summary>
        /// Represents an intersection shader record.
        /// </summary>
        Intersection = 0x03,

        /// <summary>
        /// Represents a miss shader record.
        /// </summary>
        Miss = 0x04,

        /// <summary>
        /// Represents a callable shader record.
        /// </summary>
        Callable = 0x05,

        /// <summary>
        /// Represents a shader record that contains a module of an unsupported shader stage.
        /// </summary>
        Invalid = 0x7FFFFFFF
    };

    /// <summary>
    /// Describes a group or combination of groups of a shader binding table.
    /// </summary>
    /// <seealso cref="IRayTracingPipeline::allocateShaderBindingTable" />
    enum class LITEFX_RENDERING_API ShaderBindingGroup : UInt32 {
        /// <summary>
        /// Refers to the group of the shader binding table that stores the ray generation shader.
        /// </summary>
        RayGeneration = 0x01,

        /// <summary>
        /// Refers to the group of the shader binding table that stores the geometry hit shaders.
        /// </summary>
        HitGroup = 0x02,

        /// <summary>
        /// Refers to the group of the shader binding table that stores the miss shaders.
        /// </summary>
        Miss = 0x04,

        /// <summary>
        /// Refers to the group of the shader binding table that stores the callable shaders.
        /// </summary>
        Callable = 0x08,

        /// <summary>
        /// Refers to a combination of all possible groups that can be stored in a shader binding table.
        /// </summary>
        All = RayGeneration | HitGroup | Miss | Callable
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
        /// Polygons are only drawn as wire-frames.
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
        /// The rasterizer will discard back-facing polygons.
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
    /// Describes the behavior of render targets.
    /// </summary>
    enum class LITEFX_RENDERING_API RenderTargetFlags {
        /// <summary>
        /// No flags are enabled.
        /// </summary>
        None = 0x00,

        /// <summary>
        /// If enabled, color or depth (depending on the render target type) are cleared when starting a render pass that renders to the render target.
        /// </summary>
        Clear = 0x01,

        /// <summary>
        /// If enabled and the render target format supports stencil storage, the stencil part is cleared when the render pass that renders to the render target is started.
        /// </summary>
        ClearStencil = 0x02,

        /// <summary>
        /// If enabled, the render target is discarded after ending the render pass.
        /// </summary>
        /// <remarks>
        /// When this flag is set, the render target storage is freed after the render pass has finished. The main use of this is to have depth/stencil targets on a render 
        /// pass that are only required during this render pass. It is not valid to attempt accessing the render target before or after the render pass.
        /// </remarks>
        Volatile = 0x04
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
        /// The default number of samples. Multi-sampling will be deactivated, if this sampling level is used.
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
        /// The test succeeds, if the current value is greater or equal to the stencil ref or previous depth value.
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
        Resolve = 0x00001000,

        /// <summary>
        /// Waits for previous commands to finish the building stage for an acceleration structure, or blocks the following commands until the building has finished.
        /// </summary>
        /// <remarks>
        /// This flag is only supported, if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE` in DirectX 12 ❎.
        /// </remarks>
        /// <seealso cref="IAccelerationStructure" />
        AccelerationStructureBuild = 0x00010000,

        /// <summary>
        /// Waits for previous commands to finish the copying stage for an acceleration structure, or blocks the following commands until the copying has finished.
        /// </summary>
        /// <remarks>
        /// This flag is only supported, if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE` in DirectX 12 ❎.
        /// </remarks>
        /// <seealso cref="IAccelerationStructure" />
        AccelerationStructureCopy = 0x00020000,

        /// <summary>
        /// Waits for the previous commands to finish ray-tracing shader stages, or blocks the following commands until ray-tracing has finished.
        /// </summary>
        /// <remarks>
        /// This flag is only supported if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RAYTRACING` in DirectX 12 ❎.
        /// </remarks>
        Raytracing = 0x00040000,
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
        Common = 0x00002000,

        /// <summary>
        /// Indicates that a resources is accessed to read an acceleration structure.
        /// </summary>
        /// <remarks>
        /// This access mode flags is only supported if ray-tracing support is enabled. It translates `D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ` in the DirectX 12 ❎ backend and `VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR` in the Vulkan 🌋 backend.
        /// </remarks>
        AccelerationStructureRead = 0x00010000,

        /// <summary>
        /// Indicates that a resources is accessed to write an acceleration structure.
        /// </summary>
        /// <remarks>
        /// This access mode flags is only supported if ray-tracing support is enabled. It translates `D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE` in the DirectX 12 ❎ backend and `VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR` in the Vulkan 🌋 backend.
        /// </remarks>
        AccelerationStructureWrite = 0x00020000,
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
        /// 
        /// When using this layout as a source layout, the contents of the image may be discarded.
        /// </remarks>
        Undefined = 0x7FFFFFFF
    };

    /// <summary>
    /// Controls how a geometry that is part of a bottom-level acceleration structure (BLAS) behaves during ray-tracing.
    /// </summary>
    /// <seealso cref="IBottomLevelAccelerationStructure" />
    enum class LITEFX_RENDERING_API GeometryFlags {
        /// <summary>
        /// Implies no restrictions on the geometry.
        /// </summary>
        None = 0x00,

        /// <summary>
        /// If this flag is set, the any-hit shader for this geometry is never invoked, even if it is present within the hit group.
        /// </summary>
        Opaque = 0x01,

        /// <summary>
        /// If this flag is set, the any-hit shader for this geometry is only invoked once for each primitive of the geometry, even if it could be invoked multiple times during ray tracing.
        /// </summary>
        OneShotAnyHit = 0x02
    };

    /// <summary>
    /// Controls how an acceleration structure should be built.
    /// </summary>
    /// <seealso cref="IBottomLevelAccelerationStructure" />
    /// <seealso cref="ITopLevelAccelerationStructure" />
    enum class LITEFX_RENDERING_API AccelerationStructureFlags {
        /// <summary>
        /// Use default options for building the acceleration structure.
        /// </summary>
        None = 0x0000,

        /// <summary>
        /// Allow the acceleration structure to be updated.
        /// </summary>
        AllowUpdate = 0x0001,

        /// <summary>
        /// Allow the acceleration structure to be compacted.
        /// </summary>
        AllowCompaction = 0x0002,

        /// <summary>
        /// Prefer building a better performing acceleration structure, that possibly takes longer to build.
        /// </summary>
        /// <remarks>
        /// This flag cannot be combined with <see cref="PreferFastBuild" />.
        /// </remarks>
        PreferFastTrace = 0x0004,

        /// <summary>
        /// Prefer fast build times for the acceleration structure, but sacrifice ray-tracing performance.
        /// </summary>
        /// <remarks>
        /// This flag cannot be combined with <see cref="PreferFastTrace" />.
        /// </remarks>
        PreferFastBuild = 0x0008,

        /// <summary>
        /// Prefer to minimize the memory footprint of the acceleration structure, but at the cost of ray-tracing performance and build times.
        /// </summary>
        MinimizeMemory = 0x0010
    };

    /// <summary>
    /// Controls how an instance within a <see cref="ITopLevelAccelerationStructure" /> behaves during ray-tracing.
    /// </summary>
    enum class LITEFX_RENDERING_API InstanceFlags {
        /// <summary>
        /// The instance uses default behavior.
        /// </summary>
        None = 0x00,

        /// <summary>
        /// If this flag is set front- and backface culling is disabled for the instance.
        /// </summary>
        DisableCull = 0x01,

        /// <summary>
        /// If this flag is set, front- and backfaces flip their default cull order.
        /// </summary>
        FlipWinding = 0x02,

        /// <summary>
        /// If this flag is set, no geometry of the instance invokes the any-hit shader. This overwrites per-geometry flags.
        /// </summary>
        /// <remarks>
        /// This flag must not be set in combination with <see cref="ForceNonOpaque" />.
        /// </remarks>
        /// <seealso cref="GeometryFlags::Opaque" />
        ForceOpaque = 0x04,

        /// <summary>
        /// If this flag is set, each geometry of the instance will ignore the <seealso cref="GeometryFlags::Opaque" /> setting.
        /// </summary>
        /// <remarks>
        /// This flag must not be set in combination with <see cref="ForceOpaque" />.
        /// </remarks>
        /// <seealso cref="GeometryFlags::Opaque" />
        ForceNonOpaque = 0x08
    };

#pragma endregion

#pragma region "Flags"

    LITEFX_DEFINE_FLAGS(QueueType);
    LITEFX_DEFINE_FLAGS(ShaderStage);
    LITEFX_DEFINE_FLAGS(PipelineStage);
    LITEFX_DEFINE_FLAGS(ResourceAccess);
    LITEFX_DEFINE_FLAGS(BufferFormat);
    LITEFX_DEFINE_FLAGS(WriteMask);
    LITEFX_DEFINE_FLAGS(RenderTargetFlags);
    LITEFX_DEFINE_FLAGS(GeometryFlags);
    LITEFX_DEFINE_FLAGS(ResourceUsage);
    LITEFX_DEFINE_FLAGS(AccelerationStructureFlags);
    LITEFX_DEFINE_FLAGS(InstanceFlags);
    LITEFX_DEFINE_FLAGS(ShaderBindingGroup);

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
        /// Adds a new frame buffer to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="frameBuffer">The render pass to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another frame buffer with the same identifier has already been added.</exception>
        void add(UniquePtr<IFrameBuffer>&& frameBuffer);

        /// <summary>
        /// Adds a new frame buffer to the device state.
        /// </summary>
        /// <param name="id">The identifier for the frame buffer.</param>
        /// <param name="renderPass">The frame buffer to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another frame buffer with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IFrameBuffer>&& frameBuffer);

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
        /// Adds a new acceleration structure to the device state and uses its name as identifier.
        /// </summary>
        /// <param name="accelerationStructure">The acceleration structure to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another acceleration structure with the same <paramref name="id" /> has already been added.</exception>
        void add(UniquePtr<IAccelerationStructure>&& accelerationStructure);

        /// <summary>
        /// Adds a new acceleration structure to the device state.
        /// </summary>
        /// <param name="id">The identifier for the acceleration structure.</param>
        /// <param name="accelerationStructure">The acceleration structure to add to the device state.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another acceleration structure with the same <paramref name="id" /> has already been added.</exception>
        void add(const String& id, UniquePtr<IAccelerationStructure>&& accelerationStructure);
        
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
        /// Returns a frame buffer from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the frame buffer.</param>
        /// <returns>A reference of the frame buffer.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no frame buffer has been added for the provided <paramref name="id" />.</exception>
        IFrameBuffer& frameBuffer(const String& id) const;

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
        /// Returns a sampler from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the sampler.</param>
        /// <returns>A reference of the sampler.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no sampler has been added for the provided <paramref name="id" />.</exception>
        ISampler& sampler(const String& id) const;

        /// <summary>
        /// Returns an acceleration structure from the device state.
        /// </summary>
        /// <param name="id">The identifier associated with the acceleration structure.</param>
        /// <returns>A reference of the acceleration structure.</returns>
        /// <exception cref="InvalidArgumentExceptoin">Thrown, if no acceleration structure has been added for the provided <paramref name="id" />.</exception>
        IAccelerationStructure& accelerationStructure(const String& id) const;

        /// <summary>
        /// Returns a descriptor set from the device state.
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
        /// Releases a frame buffer.
        /// </summary>
        /// <param name="renderPass">The frame buffer to release.</param>
        /// <returns><c>true</c>, if the frame buffer was properly released, <c>false</c> otherwise.</returns>
        bool release(const IFrameBuffer& frameBuffer);

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
    /// Describes a single descriptor binding point within a <see cref="IShaderModule" />.
    /// </summary>
    struct LITEFX_RENDERING_API DescriptorBindingPoint final {
    public:
        /// <summary>
        /// Stores the register index of the binding point.
        /// </summary>
        UInt32 Register { 0 };

        /// <summary>
        /// Stores the descriptor space (or set index) of the binding point.
        /// </summary>
        UInt32 Space { 0 };
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

        /// <summary>
        /// For ray-tracing shader modules returns the binding point for the descriptor that receives shader-local data.
        /// </summary>
        /// <remarks>
        /// Ray-tracing shaders, especially hit and intersection shaders may rely on local per-invocation data to handle ray intersections. One prominent example of such data is a custom 
        /// index that identifies the geometry within the instance that has been hit, which can then be used to index into bindless arrays to acquire additional data, such as material 
        /// properties or texture maps. This data is placed alongside the shader binding table created from a <see cref="ShaderRecordCollection" /> and passed to the shader when it is 
        /// invoked. However, when building the <see cref="IPipelineLayout" /> for a ray-tracing pipeline, the device needs to know which descriptors bind globally and which descriptor
        /// binds locally. This information currently cannot be reliably acquired by shader reflection and must thus be specified on a per-module basis. 
        /// 
        /// Note that it is only possible for one descriptor to bind to local data. However, this descriptor can bind a constant/uniform buffer that contains multiple variables. Whilst 
        /// it is possible to bind buffer references (using <see cref="IDeviceMemory::virtualAddress" />), support for it is differs depending on the shader language. To keep shaders 
        /// portable, it is recommended to use descriptor indexing to bind buffers and textures and only pass constant values into local descriptor bindings.
        /// 
        /// For shader modules of types other than ray-tracing, this setting is ignored.
        /// </remarks>
        /// <returns>Returns the binding point for the descriptor that receives shader-local data.</returns>
        /// <seealso cref="ShaderRecord{{typename TLocalData}}" />
        virtual const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept = 0;
    };

    /// <summary>
    /// Represents a render target, i.e. an abstract view of the output of an <see cref="RenderPass" />.
    /// </summary>
    /// <remarks>
    /// A render target represents one output of a render pass, stored within an <see cref="IImage" />. It is contained by a <see cref="RenderPass" />, that maps it to an image resource on
    /// the <see cref="FrameBuffer" /> the render pass operates on. The <see cref="IRenderTarget::identifier" /> is used to associate an image within a frame buffer to a render target.
    /// 
    /// When using a <see cref="IRenderPipeline" /> during rendering, a similar lookup is performed to bind frame buffer images to input attachments.
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
        /// A unique identifier for the render target.
        /// </summary>
        /// <returns>The unique identifier for the render target.</returns>
        virtual UInt64 identifier() const noexcept = 0;

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
        /// Returns the flags that control the behavior of the render target.
        /// </summary>
        /// <returns>The flags that control the behavior of the render target.</returns>
        virtual RenderTargetFlags flags() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the render target should be cleared, when the render pass is started. If the <see cref="format" /> is set to a depth format, this clears the
        /// depth buffer. Otherwise it clears the color buffer.
        /// </summary>
        /// <returns><c>true</c>, if the render target should be cleared, when the render pass is started</returns>
        /// <seealso cref="clearStencil" />
        /// <seealso cref="clearValues" />
        /// <seealso cref="flags" />
        /// <seealso cref="RenderTargetFlags" />
        virtual bool clearBuffer() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the render target stencil should be cleared, when the render pass is started. If the <see cref="format" /> is does not contain a stencil channel,
        /// this has no effect.
        /// </summary>
        /// <returns><c>true</c>, if the render target stencil should be cleared, when the render pass is started</returns>
        /// <seealso cref="clearStencil" />
        /// <seealso cref="clearValues" />
        /// <seealso cref="flags" />
        /// <seealso cref="RenderTargetFlags" />
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
        /// <seealso cref="flags" />
        /// <seealso cref="RenderTargetFlags" />
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
        /// <summary>
        /// Initializes the render target.
        /// </summary>
        /// <param name="uid">A unique identifier for the render target.</param>
        /// <param name="location">The location of the render target output attachment.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The format of the render target.</param>
        /// <param name="flags">The flags that control the behavior of the render target.</param>
        /// <param name="clearValues">The values with which the render target gets cleared.</param>
        /// <param name="blendState">The render target blend state.</param>
        explicit RenderTarget(UInt64 uid, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const BlendState& blendState = {});

        /// <summary>
        /// Initializes the render target.
        /// </summary>
        /// <remarks>
        /// This overload uses the <paramname ref="name" /> parameter to compute the <see cref="identifier" />.
        /// </remarks>
        /// <param name="name">The unique name of the render target.</param>
        /// <param name="location">The location of the render target output attachment.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The format of the render target.</param>
        /// <param name="flags">The flags that control the behavior of the render target.</param>
        /// <param name="clearValues">The values with which the render target gets cleared.</param>
        /// <param name="blendState">The render target blend state.</param>
        explicit RenderTarget(StringView name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const BlendState& blendState = {});
        
        /// <summary>
        /// Creates a copy of a render target.
        /// </summary>
        /// <param name="_other">The render target instance to copy.</param>
        RenderTarget(const RenderTarget& _other) noexcept;

        /// <summary>
        /// Takes over another instance of a render target.
        /// </summary>
        /// <param name="_other">The render target instance to take over.</param>
        RenderTarget(RenderTarget&& _other) noexcept;
        
        /// <summary>
        /// Releases the render target instance.
        /// </summary>
        virtual ~RenderTarget() noexcept;

    public:
        /// <summary>
        /// Assigns a render target by copying it.
        /// </summary>
        /// <param name="_other">The render target instance to copy.</param>
        /// <returns>A reference to the current render target instance.</returns>
        inline RenderTarget& operator=(const RenderTarget& _other) noexcept;

        /// <summary>
        /// Assigns a render target by taking it over.
        /// </summary>
        /// <param name="_other">The render target to take over.</param>
        /// <returns>A reference to the current render target instance.</returns>
        inline RenderTarget& operator=(RenderTarget&& _other) noexcept;

    public:
        /// <inheritdoc />
        UInt64 identifier() const noexcept override;

        /// <inheritdoc />
        const String& name() const noexcept override;

        /// <inheritdoc />
        UInt32 location() const noexcept override;

        /// <inheritdoc />
        RenderTargetType type() const noexcept override;

        /// <inheritdoc />
        Format format() const noexcept override;

        /// <inheritdoc />
        RenderTargetFlags flags() const noexcept override;

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
    /// Represents a mapping between a set of <see cref="RenderTarget" /> instances and the input attachments of a <see cref="IRenderPass" />.
    /// </summary>
    class LITEFX_RENDERING_API RenderPassDependency final {
        LITEFX_IMPLEMENTATION(RenderPassDependencyImpl);

    public:
        /// <summary>
        /// Creates a new render target dependency.
        /// </summary>
        /// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
        /// <param name="descriptorBinding">The binding point to bind the input attachment to.</param>
        RenderPassDependency(const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) noexcept;

        /// <summary>
        /// Creates a new render target dependency.
        /// </summary>
        /// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
        /// <param name="bindingRegister">The register to bind the input attachment to.</param>
        /// <param name="space">The space to bind the input attachment to.</param>
        RenderPassDependency(const RenderTarget& renderTarget, UInt32 bindingRegister, UInt32 space) noexcept;

        /// <summary>
        /// Creates a copy of another render pass dependency.
        /// </summary>
        /// <param name="_other">The render pass dependency to copy.</param>
        RenderPassDependency(const RenderPassDependency& _other) noexcept;

        /// <summary>
        /// Takes over another render pass dependency instance.
        /// </summary>
        /// <param name="_other">The render pass dependency instance to take over.</param>
        RenderPassDependency(RenderPassDependency&& _other) noexcept;

        /// <summary>
        /// Releases the current render pass dependency instance.
        /// </summary>
        ~RenderPassDependency() noexcept;

    public:
        /// <summary>
        /// Assigns another render pass dependency instance by copying it.
        /// </summary>
        /// <param name="_other">The render pass dependency to copy.</param>
        /// <returns>A reference of the current render pass dependency instance.</returns>
        RenderPassDependency& operator=(const RenderPassDependency& _other) noexcept;

        /// <summary>
        /// Assigns another render pass dependency by taking it over.
        /// </summary>
        /// <param name="_other">The render pass dependency to take over.</param>
        /// <returns>A reference of the current render pass dependency instance.</returns>
        RenderPassDependency& operator=(RenderPassDependency&& _other) noexcept;

    public:
        /// <summary>
        /// Returns a reference of the render target that is mapped to the input attachment.
        /// </summary>
        /// <returns>A reference of the render target that is mapped to the input attachment.</returns>
        const RenderTarget& renderTarget() const noexcept;

        /// <summary>
        /// Returns the binding point for the input attachment binding.
        /// </summary>
        /// <returns>The binding point for the input attachment binding.</returns>
        const DescriptorBindingPoint& binding() const noexcept;
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
    /// Describes the offsets and sizes of a shader group within a shader binding table buffer.
    /// </summary>
    /// <remarks>
    /// If a group is not present within a shader binding table, the offset for this group is set to the maximum possible value and the size is set to `0`.
    /// </remarks>
    /// <seealso cref="IRayTracingPipeline::allocateShaderBindingTable" /> 
    struct LITEFX_RENDERING_API ShaderBindingTableOffsets {
        /// <summary>
        /// The offset to the beginning of the ray generation group within the shader binding table.
        /// </summary>
        UInt64 RayGenerationGroupOffset { std::numeric_limits<UInt64>::max() };

        /// <summary>
        /// The size of the ray generation group within the shader binding table.
        /// </summary>
        UInt64 RayGenerationGroupSize { 0 };

        /// <summary>
        /// The stride between individual ray generation group records in the shader binding table.
        /// </summary>
        UInt64 RayGenerationGroupStride { 0 };

        /// <summary>
        /// The offset to the beginning of the hit group within the shader binding table.
        /// </summary>
        UInt64 HitGroupOffset { std::numeric_limits<UInt64>::max() };

        /// <summary>
        /// The size of the hit group within the shader binding table.
        /// </summary>
        UInt64 HitGroupSize { 0 };

        /// <summary>
        /// The stride between individual hit group records in the shader binding table.
        /// </summary>
        UInt64 HitGroupStride { 0 };

        /// <summary>
        /// The offset to the beginning of the miss group within the shader binding table.
        /// </summary>
        UInt64 MissGroupOffset{ std::numeric_limits<UInt64>::max() };

        /// <summary>
        /// The size of the miss group within the shader binding table.
        /// </summary>
        UInt64 MissGroupSize { 0 };

        /// <summary>
        /// The stride between individual miss group records in the shader binding table.
        /// </summary>
        UInt64 MissGroupStride { 0 };

        /// <summary>
        /// The offset to the beginning of the callable group within the shader binding table.
        /// </summary>
        UInt64 CallableGroupOffset { std::numeric_limits<UInt64>::max() };

        /// <summary>
        /// The size of the callable group within the shader binding table.
        /// </summary>
        UInt64 CallableGroupSize { 0 };

        /// <summary>
        /// The stride between individual callable group records in the shader binding table.
        /// </summary>
        UInt64 CallableGroupStride { 0 };
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
        /// <param name="firstElement">The first element of the array to map.</param>
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
        /// <param name="firstElement">The first element of the array to map.</param>
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
        /// Returns the usage flags for the resource.
        /// </summary>
        /// <returns>The usage flags for the resource.</returns>
        virtual ResourceUsage usage() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the resource can be bound to a read/write descriptor.
        /// </summary>
        /// <remarks>
        /// If the resource is not writable, attempting to bind it to a writable descriptor will result in an exception.
        /// </remarks>
        /// <returns><c>true</c>, if the resource can be bound to a read/write descriptor.</returns>
        inline bool writable() const noexcept {
            return LITEFX_FLAG_IS_SET(this->usage(), ResourceUsage::AllowWrite);
        }

        /// <summary>
        /// Gets the address of the resource in GPU memory.
        /// </summary>
        /// <remarks>
        /// Note that this may not be supported for all resource types in all backends. For example, Vulkan does not support obtaining virtual addresses of image resources.
        /// </remarks>
        /// <returns>The address of the resource in GPU memory.</returns>
        virtual UInt64 virtualAddress() const noexcept = 0;
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
    /// Base interface for a ray tracing acceleration structure.
    /// </summary>
    /// <seealso cref="IBottomLevelAccelerationStructure" />
    /// <seealso cref="ITopLevelAccelerationStructure" />
    class LITEFX_RENDERING_API IAccelerationStructure : public virtual IStateResource {
    public:
        virtual ~IAccelerationStructure() noexcept = default;

    public:
        /// <summary>
        /// Returns the flags that control how the acceleration structure should be built.
        /// </summary>
        /// <returns>The flags that control how the acceleration structure should be built.</returns>
        virtual AccelerationStructureFlags flags() const noexcept = 0;

        /// <summary>
        /// Performs a complete build of the acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method builds or rebuilds the entire acceleration structure. If called without any further arguments beside <paramref name="commandBuffer" />, a new buffer and scratch buffer will be allocated
        /// from the <see cref="IGraphicsDevice" /> that created the command buffer. Alternatively, it is possible to provide a pre-allocated buffer in the <paramref name="buffer" /> parameter. This allows to
        /// re-use memory from another acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling <see cref="buffer" />) and 
        /// destroy it afterwards, which enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory 
        /// consumption. This is done by also providing the <paramref name="offset" /> and <paramref name="maxSize" /> parameters to address a range within the buffer itself, the acceleration structure may be 
        /// written into. Note that the pointer passed to the <see cref="buffer" /> parameter must have been initialized with the <see cref="BufferType::AccelerationStructure" /> buffer type and must be 
        /// writable (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// By providing a <see cref="scratchBuffer" />, it is possible to re-use temporary memory while building. This can lower memory consumption when building multiple acceleration structures. However, this
        /// also requires proper barriers to be executed between two build commands, as they are not allowed to access the same scratch memory simultaneously. Note that the pointer passed to the
        /// <see cref="scratchBuffer" /> parameter must have been initialized on the <see cref="ResourceHeap::Resource" /> heap and must be writable (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// After a successful build, the buffer pointer is stored by the acceleration structure and can be accessed by calling <see cref="buffer" /> on it.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to record the acceleration structure build commands.</param>
        /// <param name="scratchBuffer">The scratch buffer used during the acceleration structure build, or `nullptr` if a temporary buffer should be created.</param>
        /// <param name="buffer">The buffer that stores the acceleration structure after building, or `nullptr` if a new buffer should be created.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure should be stored. Must be a multiple of 256. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <param name="maxSize">The maximum available size within <paramref name="buffer" /> at <paramref name="offset" />. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="scratchBuffer" /> is not `nullptr` and does not contain enough scratch memory to build the acceleration structure.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="offset" /> is not aligned to 256 bytes.</exception>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if <paramref name="buffer" /> is not `nullptr` and the range provided by <paramref name="offset" /> and <paramref name="maxSize" /> is not fully contained by the buffer.</exception>
        /// <seealso cref="update" />
        inline void build(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer = nullptr, SharedPtr<const IBuffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0) {
            this->doBuild(commandBuffer, scratchBuffer, buffer, offset, maxSize);
        }

        /// <summary>
        /// Performs an update on the acceleration structure.
        /// </summary>
        /// <remarks>
        /// Updating an acceleration structure works similar to performing a build, but may be faster compared to a full re-build. Note that in order to support updates, the acceleration structure must have been
        /// created with the <see cref=AccelerationStructureFlags::AllowUpdate" /> flag provided. Note that this flag may cause the acceleration structure build times and memory consumption to increase and may 
        /// lower the ray-tracing performance.
        /// 
        /// If no arguments beside <paramref name="commandBuffer" /> are provided, the acceleration structure may re-use the same backing memory used for building, if the buffer holds enough space to contain it.
        /// Otherwise, a new buffer will be allocated. Alternatively, it is possible to provide a pre-allocated buffer in the <paramref name="buffer" /> parameter. This allows to re-use memory from another 
        /// acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling <see cref="buffer" />) and destroy it afterwards, which 
        /// enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory consumption. This is done by also 
        /// providing the <paramref name="offset" /> and <paramref name="maxSize" /> parameters to address a range within the buffer itself, the acceleration structure may be written into. Note that the pointer 
        /// passed to the <see cref="buffer" /> parameter must have been initialized with the <see cref="BufferType::AccelerationStructure" /> buffer type and must be writable 
        /// (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// By providing a <see cref="scratchBuffer" />, it is possible to re-use temporary memory while building. This can lower memory consumption when building multiple acceleration structures. However, this
        /// also requires proper barriers to be executed between two build commands, as they are not allowed to access the same scratch memory simultaneously. Note that the pointer passed to the
        /// <see cref="scratchBuffer" /> parameter must have been initialized on the <see cref="ResourceHeap::Resource" /> heap and must be writable (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// After a successful update, the buffer pointer is stored by the acceleration structure and can be accessed by calling <see cref="buffer" /> on it.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to record the acceleration structure build commands.</param>
        /// <param name="scratchBuffer">The scratch buffer used during the acceleration structure build, or `nullptr` if a temporary buffer should be created.</param>
        /// <param name="buffer">The buffer that stores the acceleration structure after updating, or `nullptr` if a new buffer should be created.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure should be stored. Must be a multiple of 256. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <param name="maxSize">The maximum available size within <paramref name="buffer" /> at <paramref name="offset" />. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure backing buffer is not initialized, indicating the acceleration structure has not yet been built.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="scratchBuffer" /> is not `nullptr` and does not contain enough scratch memory to build the acceleration structure.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="offset" /> is not aligned to 256 bytes.</exception>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if <paramref name="buffer" /> is not `nullptr` and the range provided by <paramref name="offset" /> and <paramref name="maxSize" /> is not fully contained by the buffer.</exception>
        /// <seealso cref="build" />
        inline void update(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer = nullptr, SharedPtr<const IBuffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0) {
            this->doUpdate(commandBuffer, scratchBuffer, buffer, offset, maxSize);
        }

        /// <summary>
        /// Returns the acceleration structure backing buffer, that stores its last build.
        /// </summary>
        /// <returns>The acceleration structure backing buffer, that stores its last build.</returns>
        /// <seealso cref="offset" />
        /// <seealso cref="size" />
        inline SharedPtr<const IBuffer> buffer() const noexcept {
            return this->getBuffer();
        }

        /// <summary>
        /// Returns the offset into <see cref="buffer" /> at which the acceleration structure is stored.
        /// </summary>
        /// <returns>The offset into <see cref="buffer" /> at which the acceleration structure is stored.</returns>
        /// <seealso cref="buffer" />
        virtual UInt64 offset() const noexcept = 0;

        /// <summary>
        /// Returns the amount of memory in bytes inside <see cref="buffer" /> that store the acceleration structure.
        /// </summary>
        /// <remarks>
        /// Note that this may be different to the value specified during build, as the actual size may be smaller (but can never be larger) after building. If you want to reduce the memory footprint, you can use 
        /// this amount of memory for a compacted buffer and copy the acceleration structure using a copy command. In order to acquire the actual size required by the acceleration structure, the system needs to
        /// wait for the last build or update process to finish. Before that, this property will return the memory requirements as pre-computed by the device. To make sure that the build has finished, you need to
        /// manually wait for the fence acquired by submitting the command buffer that builds or updates the acceleration structure. Afterwards this method will return the actual size required to store the 
        /// acceleration structure.
        ///  
        /// If the acceleration structure has not yet been built or is invalidated, this property returns `0`.
        /// </remarks>
        /// <returns>The amount of memory in bytes inside <see cref="buffer" /> that store the acceleration structure.</returns>
        /// <seealso cref="buffer" />
        virtual UInt64 size() const noexcept = 0;

    private:
        virtual SharedPtr<const IBuffer> getBuffer() const noexcept = 0;
        virtual void doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) = 0;
        virtual void doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) = 0;
    };

    /// <summary>
    /// A structure that holds a singular entity of geometry for hardware ray-tracing.
    /// </summary>
    /// <remarks>
    /// Bottom-level acceleration structures describe actual pieces of geometry (sets of triangular meshes or axis-aligned bounding boxes for procedural geometry). They can 
    /// best be thought of entities in terms of a scene graph, whilst <see cref="ITopLevelAccelerationStructure" />s represent their respective *instances*. For example, a 
    /// top-level acceleration structure (TLAS) would store the world transform of the object itself, which can be placed multiple times in the scene with different transforms 
    /// each time. Each TLAS points to a bottom-level acceleration structure (BLAS), that contains the actual geometry, consisting of multiple meshes that are all transformed 
    /// relative to the TLAS transform.
    /// 
    /// Note that a bottom-level acceleration structure can only contain either triangle meshes or bounding boxes, but never both in the same structure.
    /// </remarks>
    /// <seealso cref="TriangleMesh" />
    /// <seealso cref="AxisAlignedBoundingBox" />
    /// <seealso cref="ITopLevelAccelerationStructure" />
    class LITEFX_RENDERING_API IBottomLevelAccelerationStructure : public IAccelerationStructure {
    public:
        /// <summary>
        /// Represents a triangle mesh.
        /// </summary>
        struct TriangleMesh final {
        public:
            /// <summary>
            /// Initializes a new triangle mesh.
            /// </summary>
            /// <param name="vertexBuffer">The vertex buffer that stores the mesh vertices.</param>
            /// <param name="indexBuffer">The index buffer that stores the mesh indices.</param>
            /// <param name="transformBuffer">A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.</param>
            /// <param name="flags">The flags that control how the primitives in the geometry behaves during ray-tracing.</param>
            TriangleMesh(SharedPtr<const IVertexBuffer> vertexBuffer, SharedPtr<const IIndexBuffer> indexBuffer = nullptr, SharedPtr<const IBuffer> transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) :
                VertexBuffer(vertexBuffer), IndexBuffer(indexBuffer), TransformBuffer(transformBuffer), Flags(flags) { 
                if (vertexBuffer == nullptr) [[unlikely]]
                    throw ArgumentNotInitializedException("vertexBuffer", "The vertex buffer must be initialized.");
            }

            /// <summary>
            /// Initializes a new triangle mesh by copying another one.
            /// </summary>
            /// <param name="other">The triangle mesh to copy.</param>
            TriangleMesh(const TriangleMesh& other) noexcept :
                VertexBuffer(other.VertexBuffer), IndexBuffer(other.IndexBuffer), TransformBuffer(other.TransformBuffer), Flags(other.Flags) { }

            /// <summary>
            /// Initializes a new triangle mesh by taking over another one.
            /// </summary>
            /// <param name="other">The triangle mesh to take over.</param>
            TriangleMesh(TriangleMesh&& other) noexcept :
                VertexBuffer(std::move(other.VertexBuffer)), IndexBuffer(std::move(other.IndexBuffer)), TransformBuffer(std::move(other.TransformBuffer)), Flags(std::move(other.Flags)) { }

            /// <summary>
            /// Releases the triangle mesh.
            /// </summary>
            ~TriangleMesh() noexcept = default;

            /// <summary>
            /// Copies another triangle mesh.
            /// </summary>
            /// <param name="other">The triangle mesh to copy.</param>
            /// <returns>A reference to the current triangle mesh instance.</returns>
            TriangleMesh& operator=(const TriangleMesh& other) {
                this->VertexBuffer = other.VertexBuffer;
                this->IndexBuffer = other.IndexBuffer;
                this->TransformBuffer = other.TransformBuffer;
                this->Flags = other.Flags;
                return *this;
            }

            /// <summary>
            /// Takes over another triangle mesh.
            /// </summary>
            /// <param name="other">The triangle mesh to take over.</param>
            /// <returns>A reference to the current triangle mesh instance.</returns>
            TriangleMesh& operator=(TriangleMesh&& other) {
                this->VertexBuffer = std::move(other.VertexBuffer);
                this->IndexBuffer = std::move(other.IndexBuffer);
                this->TransformBuffer = std::move(other.TransformBuffer);
                this->Flags = std::move(other.Flags);
                return *this;
            }

        public:
            /// <summary>
            /// The vertex buffer that stores the mesh vertices.
            /// </summary>
            SharedPtr<const IVertexBuffer> VertexBuffer;

            /// <summary>
            /// The index buffer that stores the mesh indices.
            /// </summary>
            SharedPtr<const IIndexBuffer> IndexBuffer;

            /// <summary>
            /// A buffer that stores a row-major 3x4 transformation matrix applied to the vertex buffer when building the BLAS.
            /// </summary>
            /// <remarks>
            /// If the transform is not set, the vertices are not further transformed, which can improve building performance.
            /// </remarks>
            SharedPtr<const IBuffer> TransformBuffer;

            /// <summary>
            /// The flags that control how the primitives in the geometry behaves during ray-tracing.
            /// </summary>
            GeometryFlags Flags;
        };

        /// <summary>
        /// Stores a buffer that contains axis-aligned bounding boxes.
        /// </summary>
        /// <remarks>
        /// You may think of this structure as a set containing voxels for procedural geometry.
        /// </remarks>
        struct BoundingBoxes final {
            /// <summary>
            /// A buffer containing the bounding box definitions.
            /// </summary>
            /// <remarks>
            /// Each element of the buffer must contain a bounding box at the start of the buffer, where a bounding box takes up 6 single-precision floating point values, with the first triplet 
            /// describing the lower corner of the bounding box and the second triplet describing the upper corner of the bounding box, as shown in the following definition:
            /// 
            /// <code>
            /// struct alignas(16) AABB {
            ///     Float minimum[3];
            ///     Float maximum[3];
            /// }
            /// </code>
            /// 
            /// The rest of the bounding box elements memory can be filled with arbitrary data, that can be read by shaders.
            /// </remarks>
            SharedPtr<const IBuffer> Buffer;

            /// <summary>
            /// The flags that control how the primitives in the geometry behaves during ray-tracing.
            /// </summary>
            GeometryFlags Flags;
        };

    public:
        virtual ~IBottomLevelAccelerationStructure() noexcept = default;

    public:
        /// <summary>
        /// Returns an array of triangle meshes contained by the BLAS.
        /// </summary>
        /// <returns>The array of triangle meshes contained by the BLAS.</returns>
        virtual const Array<TriangleMesh>& triangleMeshes() const noexcept = 0;

        /// <summary>
        /// Adds a triangle mesh to the BLAS.
        /// </summary>
        /// <param name="mesh">The triangle mesh to add to the BLAS.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains bounding boxes.</exception>
        virtual void addTriangleMesh(const TriangleMesh& mesh) = 0;

        /// <summary>
        /// Adds a triangle mesh to the BLAS.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer that stores the mesh vertices.</param>
        /// <param name="indexBuffer">The index buffer that stores the mesh indices.</param>
        /// <param name="transformBuffer">A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.</param>
        /// <param name="flags">The flags that control how the primitives in the geometry behaves during ray-tracing.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains bounding boxes.</exception>
        inline void addTriangleMesh(SharedPtr<const IVertexBuffer> vertexBuffer, SharedPtr<const IIndexBuffer> indexBuffer = nullptr, SharedPtr<const IBuffer> transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) {
            this->addTriangleMesh(TriangleMesh(vertexBuffer, indexBuffer, transformBuffer, flags));
        }

        /// <summary>
        /// Returns an array of buffers, each containing axis-aligned bounding boxes stored in the BLAS.
        /// </summary>
        /// <returns>The array of axis-aligned bounding boxes contained by the BLAS.</returns>
        virtual const Array<BoundingBoxes>& boundingBoxes() const noexcept = 0;

        /// <summary>
        /// Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        /// </summary>
        /// <param name="aabbs">The bounding boxes to add to the BLAS.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains triangle meshes.</exception>
        virtual void addBoundingBox(const BoundingBoxes& aabbs) = 0;

        /// <summary>
        /// Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        /// </summary>
        /// <param name="buffer">A buffer containing the bounding box definitions.</param>
        /// <param name="flags">The flags that control how the primitives in the geometry behaves during ray-tracing.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains triangle meshes.</exception>
        inline void addBoundingBox(SharedPtr<const IBuffer> buffer, GeometryFlags flags = GeometryFlags::None) {
            this->addBoundingBox(BoundingBoxes { .Buffer = buffer, .Flags = flags });
        }

        /// <summary>
        /// Clears all bounding boxes and triangle meshes from the acceleration structure.
        /// </summary>
        virtual void clear() noexcept = 0;

        /// <summary>
        /// Removes a triangle mesh from the acceleration structure.
        /// </summary>
        /// <param name="mesh">The triangle mesh to remove from the acceleration structure.</param>
        /// <returns>`true`, if the triangle mesh was removed, otherwise `false`.</returns>
        virtual bool remove(const TriangleMesh& mesh) noexcept = 0;

        /// <summary>
        /// Removes a bounding box set from the acceleration structure.
        /// </summary>
        /// <param name="aabb">The bounding box set to remove from the acceleration structure.</param>
        /// <returns>`true`, if the bounding box set was removed, otherwise `false`.</returns>
        virtual bool remove(const BoundingBoxes& aabb) noexcept = 0;

        /// <summary>
        /// Copies the acceleration structure into the acceleration structure provided by <paramref name="destination" />.
        /// </summary>
        /// <remarks>
        /// This method copies the acceleration structure into another one, which is especially useful for compression. If called without any arguments besides <paramref name="commandBuffer" /> and
        /// <paramref name="destination" />, the method will create a clone of the current acceleration structure, including any build info (i.e., triangle mesh or bounding box data). If the destination
        /// acceleration structure already contains a buffer and the buffer contains enough memory to store the copy, it will be re-used and its contents will be overwritten. Otherwise, a new buffer with enough
        /// memory to store the copy will be allocated.
        /// 
        /// If the <paramref name="compress" /> option is set to `true`, the copy will be compressed. Note that this is only possible, if the acceleration structure was created with the 
        /// <see cref="AccelerationStructureFlags::AllowCompaction" /> flag enabled. Note that compression requires a query for the size of the compressed data, which can only be determined *after* the 
        /// acceleration structure was built or updated. This implies that a copy command that is used for compression is not valid on the same command buffer that did also record the build or update commands
        /// for it. You have to use a fence to wait for the build to finish before attempting a compression.
        /// 
        /// It is possible to provide a buffer for the destination acceleration structure to use after copying. This buffer can be set by providing the <paramref name="buffer" /> parameter. This allows to
        /// re-use memory from another acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling <see cref="buffer" />) and 
        /// destroy it afterwards, which enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory 
        /// consumption. This is done by also providing the <paramref name="offset" /> parameter to address where the copy should be stored. Note that the pointer passed to the <see cref="buffer" /> parameter 
        /// must have been initialized with the <see cref="BufferType::AccelerationStructure" /> buffer type and must be writable (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// To reduce memory consumption, the build info (i.e., triangle mesh and bounding box data) is not copied to the destination acceleration structure by default. However, this also implies that further
        /// updates to it are inconvenient, requiring to manually copy the data in an additional pass. To also include build data in the copy, the <paramref name="copyBuildInfo" /> setting can be set to `true`.
        /// 
        /// After a successful copy, the buffer pointer is stored by the acceleration structure <paramref name="destination" /> and can be accessed by calling <see cref="buffer" /> on it.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to record the acceleration structure copy commands.</param>
        /// <param name="destination">The acceleration structure to copy the current one into.</param>
        /// <param name="compress">If `true`, the acceleration structure data will be compressed.</param>
        /// <param name="buffer">If not `nullptr`, the destination acceleration structure will be written into the provided buffer. Otherwise a new buffer is allocated, or the existing one is used depending on the available size.</param>
        /// <param name="offset">The offset at which to store the copy within <paramref name="buffer" />. Must be a multiple of 256. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <param name="copyBuildInfo">If `true`, the mesh data or bounding box data is copied into the acceleration structure.</param>
        /// <excetpion cref="InvalidArgumentException">Thrown, if <paramref name="compress" /> is set to `true`, but the current acceleration structure has not been created with the <see cref="AccelerationStructureFlags::AllowCompaction" /> flag.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="offset" /> is not aligned to 256 bytes.</exception>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if <paramref name="buffer" /> is not `nullptr` and does not fully contain the required memory to store the copy, starting at <paramref name="offset" />.</exception>
        inline void copy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress = false, SharedPtr<const IBuffer> buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const {
            this->doCopy(commandBuffer, destination, compress, buffer, offset, copyBuildInfo);
        }

    public:
        /// <summary>
        /// Adds a triangle mesh to the BLAS.
        /// </summary>
        /// <param name="mesh">The triangle mesh to add to the BLAS.</param>
        /// <returns>A reference to the current BLAS.</returns>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains bounding boxes.</exception>
        template <typename TSelf>
        inline auto withTriangleMesh(this TSelf&& self, const TriangleMesh& mesh) -> TSelf&& {
            self.addTriangleMesh(mesh);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds a triangle mesh to the BLAS.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer that stores the mesh vertices.</param>
        /// <param name="indexBuffer">The index buffer that stores the mesh indices.</param>
        /// <param name="transformBuffer">A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.</param>
        /// <param name="flags">The flags that control how the primitives in the geometry behaves during ray-tracing.</param>
        /// <returns>A reference to the current BLAS.</returns>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains bounding boxes.</exception>
        template <typename TSelf>
        inline auto withTriangleMesh(this TSelf&& self, SharedPtr<const IVertexBuffer> vertexBuffer, SharedPtr<const IIndexBuffer> indexBuffer = nullptr, SharedPtr<const IBuffer> transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) -> TSelf&& {
            return self.withTriangleMesh(TriangleMesh(vertexBuffer, indexBuffer, transformBuffer, flags));
        }

        /// <summary>
        /// Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        /// </summary>
        /// <param name="aabb">The bounding box buffer to add to the BLAS.</param>
        /// <returns>A reference to the current BLAS.</returns>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains triangle meshes.</exception>
        template <typename TSelf>
        inline auto withBoundingBox(this TSelf&& self, const BoundingBoxes& aabb) -> TSelf&& {
            self.addBoundingBox(aabb);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        /// </summary>
        /// <param name="buffer">A buffer containing the bounding box definitions.</param>
        /// <param name="flags">The flags that control how the primitives in the geometry behaves during ray-tracing.</param>
        /// <returns>A reference to the current BLAS.</returns>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure already contains triangle meshes.</exception>
        template <typename TSelf>
        inline auto withBoundingBox(this TSelf&& self, SharedPtr<const IBuffer> buffer, GeometryFlags flags = GeometryFlags::None) -> TSelf&& {
            return self.withBoundingBox(BoundingBoxes { .Buffer = buffer, .Flags = flags });
        }

    private:
        virtual void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const = 0;
    };

    /// <summary>
    /// A structure that stores the instance data for a <see cref="IBottomLevelAccelerationStructure" />.
    /// </summary>
    /// <seealso cref="IBottomLevelAccelerationStructure" />
    class LITEFX_RENDERING_API ITopLevelAccelerationStructure : public IAccelerationStructure {
    public:
        /// <summary>
        /// Represents an instance of an <see cref="IBottomLevelAccelerationStructure" />.
        /// </summary>
        struct alignas(16) Instance final {
            /// <summary>
            /// The bottom-level acceleration structure that contains the geometries of this instance.
            /// </summary>
            SharedPtr<const IBottomLevelAccelerationStructure> BottomLevelAccelerationStructure;

            /// <summary>
            /// The transformation matrix for the instance.
            /// </summary>
            TMatrix3x4<Float> Transform = TMatrix3x4<Float>::identity();

            /// <summary>
            /// The instance ID used in shaders to identify the instance.
            /// </summary>
            UInt32 Id : 24 = 0;

            /// <summary>
            /// A user-defined mask value that is matched with another mask value during ray-tracing to include or discard the instance.
            /// </summary>
            Byte Mask : 8 = 0xFF;

            /// <summary>
            /// An offset added to the address of the shader-local data of the shader record that is invoked for the instance, *after* the <see cref="IBottomLevelAccelerationStructure" /> indexing
            /// rules have been applied.
            /// </summary>
            /// <remarks>
            /// Shader-local data is a piece of constant data that is available to the shader during invocation. During a ray hit/miss event, the shader record is selected based on geometry 
            /// (<see cref="IBottomLevelAccelerationStructure" />), instance (<see cref="ITopLevelAccelerationStructure" /> and an implementation-specific offset. The selected record is then used 
            /// to load the shader and pass the shader local data to it.
            /// 
            /// The first part of the address is determined from the geometry index within the TLAS and a user-defined multiplier and base index specified in the shader when calling `TraceRay`. After 
            /// this index is calculated the value of this property is added to it. The result is an offset into the shader-local data for the selected shader record.
            /// </remarks>
            /// <seealso cref="https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#addressing-calculations-within-shader-tables" />
            /// <seealso cref="https://docs.vulkan.org/spec/latest/chapters/raytracing.html#shader-binding-table-indexing-rules" />
            UInt32 HitGroupOffset : 24 = 0;

            /// <summary>
            /// The flags that control the behavior of this instance.
            /// </summary>
            InstanceFlags Flags : 8 = InstanceFlags::None;
        };
    public:
        virtual ~ITopLevelAccelerationStructure() noexcept = default;

    public:
        /// <summary>
        /// Returns an array of instances in the TLAS.
        /// </summary>
        /// <returns>The array of instances in the TLAS.</returns>
        virtual const Array<Instance>& instances() const noexcept = 0;

        /// <summary>
        /// Adds an instance to the TLAS.
        /// </summary>
        /// <param name="instance">The instance to add to the TLAS.</param>
        /// <exception cref="RuntimeException">Thrown, if the acceleration structure buffers have already been allocated.</exception>
        virtual void addInstance(const Instance& instance) = 0;

        /// <summary>
        /// Adds an instance to the TLAS.
        /// </summary>
        /// <param name="blas">The bottom-level acceleration structure that contains the geometries of the instance.</param>
        /// <param name="id">The instance ID used in shaders to identify the instance.</param>
        /// <param name="hitGroupOffset">An offset added to the shader-local data for a hit-group shader record.</param>
        /// <param name="mask">A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.</param>
        /// <param name="flags">The flags that control the behavior of the instance.</param>
        inline void addInstance(SharedPtr<const IBottomLevelAccelerationStructure> blas, UInt32 id, UInt32 hitGroupOffset = 0, Byte mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept {
            this->addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
        }
        
        /// <summary>
        /// Adds an instance to the TLAS.
        /// </summary>
        /// <param name="blas">The bottom-level acceleration structure that contains the geometries of the instance.</param>
        /// <param name="transform">The transformation matrix applied to the instance geometry.</param>
        /// <param name="id">The instance ID used in shaders to identify the instance.</param>
        /// <param name="hitGroupOffset">An offset added to the shader-local data for a hit-group shader record.</param>
        /// <param name="mask">A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.</param>
        /// <param name="flags">The flags that control the behavior of the instance.</param>
        inline void addInstance(SharedPtr<const IBottomLevelAccelerationStructure> blas, const TMatrix3x4<Float>& transform, UInt32 id, UInt32 hitGroupOffset = 0, Byte mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept {
            this->addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Transform = transform, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
        }

        /// <summary>
        /// Clears all instances from the acceleration structure.
        /// </summary>
        virtual void clear() noexcept = 0;

        /// <summary>
        /// Removes an instance from the acceleration structure.
        /// </summary>
        /// <param name="instance">The instance to remove from the acceleration structure.</param>
        /// <returns>`true`, if the instance has been removed, otherwise `false`.</returns>
        virtual bool remove(const Instance& instance) noexcept = 0;

        /// <summary>
        /// Copies the acceleration structure into the acceleration structure provided by <paramref name="destination" />.
        /// </summary>
        /// <remarks>
        /// This method copies the acceleration structure into another one, which is especially useful for compression. If called without any arguments besides <paramref name="commandBuffer" /> and
        /// <paramref name="destination" />, the method will create a clone of the current acceleration structure, including any build info (i.e., triangle mesh or bounding box data). If the destination
        /// acceleration structure already contains a buffer and the buffer contains enough memory to store the copy, it will be re-used and its contents will be overwritten. Otherwise, a new buffer with enough
        /// memory to store the copy will be allocated.
        /// 
        /// If the <paramref name="compress" /> option is set to `true`, the copy will be compressed. Note that this is only possible, if the acceleration structure was created with the 
        /// <see cref="AccelerationStructureFlags::AllowCompaction" /> flag enabled. Note that compression requires a query for the size of the compressed data, which can only be determined *after* the 
        /// acceleration structure was built or updated. This implies that a copy command that is used for compression is not valid on the same command buffer that did also record the build or update commands
        /// for it. You have to use a fence to wait for the build to finish before attempting a compression.
        /// 
        /// It is possible to provide a buffer for the destination acceleration structure to use after copying. This buffer can be set by providing the <paramref name="buffer" /> parameter. This allows to
        /// re-use memory from another acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling <see cref="buffer" />) and 
        /// destroy it afterwards, which enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory 
        /// consumption. This is done by also providing the <paramref name="offset" /> parameter to address where the copy should be stored. Note that the pointer passed to the <see cref="buffer" /> parameter 
        /// must have been initialized with the <see cref="BufferType::AccelerationStructure" /> buffer type and must be writable (<see cref="ResourceUsage::AllowWrite" />).
        /// 
        /// To reduce memory consumption, the build info (i.e., triangle mesh and bounding box data) is not copied to the destination acceleration structure by default. However, this also implies that further
        /// updates to it are inconvenient, requiring to manually copy the data in an additional pass. To also include build data in the copy, the <paramref name="copyBuildInfo" /> setting can be set to `true`.
        /// 
        /// After a successful copy, the buffer pointer is stored by the acceleration structure <paramref name="destination" /> and can be accessed by calling <see cref="buffer" /> on it.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to record the acceleration structure copy commands.</param>
        /// <param name="destination">The acceleration structure to copy the current one into.</param>
        /// <param name="compress">If `true`, the acceleration structure data will be compressed.</param>
        /// <param name="buffer">If not `nullptr`, the destination acceleration structure will be written into the provided buffer. Otherwise a new buffer is allocated, or the existing one is used depending on the available size.</param>
        /// <param name="offset">The offset at which to store the copy within <paramref name="buffer" />. Must be a multiple of 256. Ignored if <paramref name="buffer" /> is `nullptr`.</param>
        /// <param name="copyBuildInfo">If `true`, the mesh data or bounding box data is copied into the acceleration structure.</param>
        /// <excetpion cref="InvalidArgumentException">Thrown, if <paramref name="compress" /> is set to `true`, but the current acceleration structure has not been created with the <see cref="AccelerationStructureFlags::AllowCompaction" /> flag.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="offset" /> is not aligned to 256 bytes.</exception>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if <paramref name="buffer" /> is not `nullptr` and does not fully contain the required memory to store the copy, starting at <paramref name="offset" />.</exception>
        inline void copy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress = false, SharedPtr<const IBuffer> buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const {
            this->doCopy(commandBuffer, destination, compress, buffer, offset, copyBuildInfo);
        }

    public:
        /// <summary>
        /// Adds an instance to the current TLAS.
        /// </summary>
        /// <param name="instance">The instance to add to the TLAS.</param>
        /// <returns>A reference to the current TLAS.</returns>
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, const Instance& instance) noexcept -> TSelf&& {
            self.addInstance(instance);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an instance to the current TLAS.
        /// </summary>
        /// <param name="blas">The bottom-level acceleration structure that contains the geometries of the instance.</param>
        /// <param name="id">The instance ID used in shaders to identify the instance.</param>
        /// <param name="hitGroupOffset">An offset added to the shader-local data for a hit-group shader record.</param>
        /// <param name="mask">A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.</param>
        /// <param name="flags">The flags that control the behavior of the instance.</param>
        /// <returns>A reference to the current TLAS.</returns>
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, SharedPtr<const IBottomLevelAccelerationStructure> blas, UInt32 id, UInt32 hitGroupOffset = 0, Byte mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept -> TSelf&& {
            self.addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an instance to the current TLAS.
        /// </summary>
        /// <param name="blas">The bottom-level acceleration structure that contains the geometries of the instance.</param>
        /// <param name="transform">The transformation matrix applied to the instance geometry.</param>
        /// <param name="id">The instance ID used in shaders to identify the instance.</param>
        /// <param name="hitGroupOffset">An offset added to the shader-local data for a hit-group shader record.</param>
        /// <param name="mask">A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.</param>
        /// <param name="flags">The flags that control the behavior of the instance.</param>
        /// <returns>A reference to the current TLAS.</returns>
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, SharedPtr<const IBottomLevelAccelerationStructure> blas, const TMatrix3x4<Float>& transform, UInt32 id, UInt32 hitGroupOffset = 0, Byte mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept -> TSelf&& {
            self.addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Transform = transform, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
            return std::forward<TSelf>(self);
        }

    private:
        virtual void doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const = 0;
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
        constexpr inline void transition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) {
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
        constexpr inline void transition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) {
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
        constexpr inline void transition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
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
        constexpr inline void transition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
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
        constexpr inline void transition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
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
        constexpr inline void transition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
            this->doTransition(image, level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
        }

    private:
        constexpr inline virtual void doTransition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr inline virtual void doTransition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr inline virtual void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr inline virtual void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
        constexpr inline virtual void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr inline virtual void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
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
        /// Updates an acceleration structure within the current descriptor set.
        /// </summary>
        /// <param name="binding">The acceleration structure binding point.</param>
        /// <param name="accelerationStructure">The acceleration structure to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the acceleration structure to.</param>
        void update(UInt32 binding, const IAccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const {
            this->doUpdate(binding, accelerationStructure, descriptor);
        }

    private:
        virtual void doUpdate(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const = 0;
        virtual void doUpdate(UInt32 binding, const IImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const = 0;
        virtual void doUpdate(UInt32 binding, const ISampler& sampler, UInt32 descriptor) const = 0;
        virtual void doUpdate(UInt32 binding, const IAccelerationStructure& accelerationStructure, UInt32 descriptor) const = 0;
    };

    /// <summary>
    /// Describes a resource binding to a descriptor or descriptor set.
    /// </summary>
    /// <seealso cref="IDescriptorSet" />
    /// <seealso cref="IDescriptorSetLayout" />
    struct LITEFX_RENDERING_API DescriptorBinding {
    public:
        using resource_container = Variant<std::monostate, Ref<const IBuffer>, Ref<const IImage>, Ref<const ISampler>, Ref<const IAccelerationStructure>>;
        
    public:
        /// <summary>
        /// The binding point to bind the resource at. If not provided (i.e., `std::nullopt`), the index within the collection of `DescriptorBindings` is used.
        /// </summary>
        Optional<UInt32> binding = std::nullopt;

        /// <summary>
        /// The resource to bind or `std::monostate` if no resource should be bound.
        /// </summary>
        /// <remarks>
        /// Note that not providing any resource does not perform any binding, in which case a resource needs to be manually bound to the descriptor set later 
        /// (<see cref="IDescriptorSet::update" />). This is useful in situations where you frequently update the resource bound to a descriptor set or where you do no have
        /// access to the resource at the time the descriptor set is allocated.
        /// </remarks>
        /// <seealso cref="IBuffer" />
        /// <seealso cref="IImage" />
        /// <seealso cref="ISampler" />
        resource_container resource = {};

        /// <summary>
        /// The index of the descriptor in a descriptor array at which binding the resource arrays starts.
        /// </summary>
        /// <remarks>
        /// If the resource contains an array, the individual elements (*layers* for images) will be bound, starting at this descriptor. The first element/layer to be
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
    /// Describes a record within a shader binding table.
    /// </summary>
    /// <remarks>
    /// A shader record contains a shader group, that is either a single shader of type <see cref="ShaderStage::RayGeneration" />, <see cref="ShaderStage::Intersection" />,
    /// <see cref="ShaderStage::Miss" /> or <see cref="ShaderStage::Callable" />, or a pair of types <see cref="ShaderStage::ClosestHit" /> and <see cref="ShaderStage::AnyHit" />,
    /// where at least one of them needs to be set.
    /// 
    /// Typically you do not want to implement this interface itself. Prefer using the <see cref="ShaderRecord" /> template to create shader records instead.
    /// </remarks>
    /// <seealso cref="ShaderRecord" />
    /// <seealso cref="IShaderProgram::buildShaderBindingTable" />
    struct LITEFX_RENDERING_API IShaderRecord {
    public:
        /// <summary>
        /// Describes a hit group for a triangle mesh geometry.
        /// </summary>
        /// <remarks>
        /// Note that when using this structure, at least one of the contained shaders must be set. A shader record containing a mesh geometry hit group must 
        /// only be ever called by triangle mesh bottom-level acceleration structures.
        /// </remarks>
        /// <seealso cref="IBottomLevelAccelerationStructure" />
        struct MeshGeometryHitGroup {
            /// <summary>
            /// The closest hit shader for the triangle mesh.
            /// </summary>
            const IShaderModule* ClosestHitShader;

            /// <summary>
            /// The any hit shader for the triangle mesh.
            /// </summary>
            const IShaderModule* AnyHitShader;
        };

        /// <summary>
        /// Defines the type that stores the shaders of the shader group.
        /// </summary>
        using shader_group_type = Variant<const IShaderModule*, MeshGeometryHitGroup>;

    public:
        /// <summary>
        /// Returns the type of the shader record.
        /// </summary>
        /// <returns>The type of the shader record.</returns>
        inline ShaderRecordType type() const noexcept {
            const auto& group = this->shaderGroup();

            if (std::holds_alternative<MeshGeometryHitGroup>(group))
            {
                return ShaderRecordType::HitGroup;
            }
            else if (std::holds_alternative<const IShaderModule*>(group))
            {
                switch (std::get<const IShaderModule*>(group)->type())
                {
                case ShaderStage::RayGeneration: return ShaderRecordType::RayGeneration;
                case ShaderStage::Miss: return ShaderRecordType::Miss;
                case ShaderStage::Callable: return ShaderRecordType::Callable;
                case ShaderStage::Intersection: return ShaderRecordType::Intersection;
                default: return ShaderRecordType::Invalid;
                }
            }

            std::unreachable();
        }

    public:
        /// <summary>
        /// Returns the shader group containing the modules for this record.
        /// </summary>
        virtual const shader_group_type& shaderGroup() const noexcept = 0;

        /// <summary>
        /// Returns a pointer to the shader-local data of the record.
        /// </summary>
        /// <remarks>
        /// Shader-local data is a piece of constant data that is available to the shader during invocation. During a ray hit/miss event, the shader record is selected
        /// based on geometry (<see cref="IBottomLevelAccelerationStructure" />), instance (<see cref="ITopLevelAccelerationStructure" /> and an implementation-specific
        /// offset. The selected record is then used to load the shader and pass the shader local data to it.
        /// </remarks>
        /// <returns>A pointer to the shader-local data of the record.</returns>
        /// <seealso cref="localDataSize" />
        virtual const void* localData() const noexcept = 0;

        /// <summary>
        /// Returns the size of the shader-local data of the record.
        /// </summary>
        /// <returns>The size of the shader-local data of the record.</returns>
        /// <seealso cref="localData" />
        virtual UInt64 localDataSize() const noexcept = 0;

    public:
        virtual ~IShaderRecord() noexcept = default;
    };

    /// <summary>
    /// Defines a generic shader record.
    /// </summary>
    /// <seealso cref="ShaderRecord{{}}" />
    /// <seealso cref="ShaderRecord{{typename TLocalData}}" />
    template <typename... TLocalData>
    struct ShaderRecord;

    /// <summary>
    /// Denotes a shader record containing shader-local data.
    /// </summary>
    /// <remarks>
    /// The <typeparamref name="TLocalData" /> defines the data that is passed to a shader's local resource bindings upon invocation. Two types of elements are 
    /// allowed: buffer references and constants. Buffer references can be obtained by calling <see cref="IBuffer::virtualAddress" /> and are always 8 bytes
    /// long. Constants do not strictly need to follow 8 byte alignment rules, but rather can also be smaller, in which case they should be defined as an
    /// aligned array, aligned to 8 bytes within the shader-local data.
    /// </remarks>
    /// <seealso cref="https://github.com/crud89/LiteFX/wiki/Raytracing#local-resource-bindings" />
    template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
    struct ShaderRecord<TLocalData> final : public IShaderRecord {
    public:
        using shader_group_type = IShaderRecord::shader_group_type;

    private:
        /// <summary>
        /// Stores the shader-local data of the shader record, that gets passed to the shader local data.
        /// </summary>
        TLocalData m_payload;
        
        /// <summary>
        /// Stores the shader group.
        /// </summary>
        shader_group_type m_shaderGroup;

    public:
        /// <inheritdoc />
        const shader_group_type& shaderGroup() const noexcept override {
            return m_shaderGroup;
        }

        /// <inheritdoc />
        const void* localData() const noexcept override {
            return reinterpret_cast<const void*>(&m_payload);
        }

        /// <inheritdoc />
        UInt64 localDataSize() const noexcept override {
            return sizeof(TLocalData);
        }

    public:
        ShaderRecord() = delete;
        virtual ~ShaderRecord() noexcept = default;

        /// <summary>
        /// Initializes a shader record.
        /// </summary>
        /// <param name="group">The shader group containing the modules to invoke.</param>
        /// <param name="payload">The shader-local data to pass to the shader's local resource bindings.</param>
        ShaderRecord(const shader_group_type& group, TLocalData payload) noexcept :
            m_shaderGroup(group), m_payload(payload) { }

        /// <summary>
        /// Copies another shader record.
        /// </summary>
        /// <param name="_other">The shader record to copy.</param>
        ShaderRecord(const ShaderRecord& _other) :
            m_shaderGroup(_other.m_shaderGroup), m_payload(_other.m_payload) { }

        /// <summary>
        /// Takes over another shader record.
        /// </summary>
        /// <param name="_other">The shader record to take over.</param>
        ShaderRecord(ShaderRecord&& _other) :
            m_shaderGroup(std::move(_other.m_shaderGroup)), m_payload(std::move(_other.m_payload)) { }

        /// <summary>
        /// Copies another shader record.
        /// </summary>
        /// <param name="_other">The shader record to copy.</param>
        /// <returns>A reference to the current shader record.</returns>
        auto& operator=(const ShaderRecord& _other) {
            m_shaderGroup = _other.m_shaderGroup;
            m_payload = _other.m_payload;
            return *this;
        }

        /// <summary>
        /// Takes over another shader record.
        /// </summary>
        /// <param name="_other">The shader record to take over.</param>
        /// <returns>A reference to the current shader record.</returns>
        auto& operator=(ShaderRecord&& _other) {
            m_shaderGroup = std::move(_other.m_shaderGroup);
            m_payload = std::move(_other.m_payload);
            return *this;
        }
    };

    /// <summary>
    /// Denotes a shader record containing no shader-local data.
    /// </summary>
    template <>
    struct ShaderRecord<> final : public IShaderRecord {
    public:
        using shader_group_type = IShaderRecord::shader_group_type;

    private:
        /// <summary>
        /// Stores the shader group.
        /// </summary>
        shader_group_type m_shaderGroup;

    public:
        /// <inheritdoc />
        const shader_group_type& shaderGroup() const noexcept override {
            return m_shaderGroup;
        }

        /// <inheritdoc />
        const void* localData() const noexcept override {
            return nullptr;
        }

        /// <inheritdoc />
        UInt64 localDataSize() const noexcept override {
            return 0_ui64;
        }

    public:
        ShaderRecord() = delete;
        virtual ~ShaderRecord() noexcept = default;

        /// <summary>
        /// Initializes a shader record.
        /// </summary>
        /// <param name="group">The shader group containing the modules to invoke.</param>
        ShaderRecord(const shader_group_type& group) noexcept :
            m_shaderGroup(group) { }

        /// <summary>
        /// Copies another shader record.
        /// </summary>
        /// <param name="_other">The shader record to copy.</param>
        ShaderRecord(const ShaderRecord& _other) :
            m_shaderGroup(_other.m_shaderGroup) { }

        /// <summary>
        /// Takes over another shader record.
        /// </summary>
        /// <param name="_other">The shader record to take over.</param>
        ShaderRecord(ShaderRecord&& _other) :
            m_shaderGroup(std::move(_other.m_shaderGroup)) { }

        /// <summary>
        /// Copies another shader record.
        /// </summary>
        /// <param name="_other">The shader record to copy.</param>
        /// <returns>A reference to the current shader record.</returns>
        auto& operator=(const ShaderRecord& _other) {
            m_shaderGroup = _other.m_shaderGroup;
            return *this;
        }

        /// <summary>
        /// Takes over another shader record.
        /// </summary>
        /// <param name="_other">The shader record to take over.</param>
        /// <returns>A reference to the current shader record.</returns>
        auto& operator=(ShaderRecord&& _other) {
            m_shaderGroup = std::move(_other.m_shaderGroup);
            return *this;
        }
    };

    /// <summary>
    /// Stores a set of <see cref="IShaderRecord" />s in that later form a shader binding table used for ray-tracing.
    /// </summary>
    class LITEFX_RENDERING_API ShaderRecordCollection final {
        friend class IShaderProgram;

    private:
        SharedPtr<const IShaderProgram> m_program;
        Array<UniquePtr<const IShaderRecord>> m_records;

        /// <summary>
        /// Initializes a new shader record collection.
        /// </summary>
        /// <param name="shaderProgram">The shader program that contains the shader modules</param>
        ShaderRecordCollection(SharedPtr<const IShaderProgram> shaderProgram) noexcept : 
            m_program(shaderProgram) 
        {
            // This can only be built from a shader program, which passes the pointer to itself, which must not be nullptr. If more factory methods are added,
            // we must validate the program pointer here.
        }

    public:
        ShaderRecordCollection() = delete;
        ShaderRecordCollection(const ShaderRecordCollection&) = delete;
        ShaderRecordCollection(ShaderRecordCollection&&) = default;
        ~ShaderRecordCollection() noexcept = default;

    private:
        /// <summary>
        /// Finds a shader module in the parent shader program.
        /// </summary>
        /// <param name="name">The case-sensitive name of the shader module to find.</param>
        /// <returns>A pointer to the shader module, or `nullptr`, if no module with the specified name was found in the parent program.</returns>
        const IShaderModule* findShaderModule(StringView name) const noexcept;

    public:
        /// <summary>
        /// Returns the parent shader program of the collection.
        /// </summary>
        /// <returns>The parent shader program of the collection.</returns>
        inline SharedPtr<const IShaderProgram> program() const noexcept {
            return m_program;
        }

        /// <summary>
        /// Returns an array of all shader records within the shader record collection.
        /// </summary>
        /// <returns>The array containing all shader records within the shader record collection.</returns>
        const Array<UniquePtr<const IShaderRecord>>& shaderRecords() const noexcept;

        /// <summary>
        /// Adds a new shader record to the shader record collection.
        /// </summary>
        /// <param name="record">The shader record to add to the shader record collection.</param>
        /// <exception cref="ArgumentNotInitializedException">Thrown, if the shader record was not initialized.</exception>
        /// <exception cref="InvalidArgumentException">Thrown, if the shader module(s) within the shader record are of invalid type, or the parent shader program does not contain the shader module(s).</exception>
        void addShaderRecord(UniquePtr<const IShaderRecord>&& record);

    public:
        /// <summary>
        /// Adds a new shader record based on the name of a shader module in the parent shader program.
        /// </summary>
        /// <remarks>
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an 
        /// any and closest hit shader, use <see cref="addMeshGeometryShaderHitGroupRecord" /> instead.
        /// </remarks>
        /// <param name="shaderName">The name of the shader module.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if no shader module with the provided name was found in the parent shader program.</exception>
        inline void addShaderRecord(StringView shaderName) {
            auto shaderModule = this->findShaderModule(shaderName);

            if (shaderModule == nullptr) [[unlikely]]
                throw InvalidArgumentException("shaderName", "The parent shader program does not contain a shader named \"{}\".", shaderName);

            if (shaderModule->type() == ShaderStage::AnyHit)
                this->addShaderRecord(makeUnique<ShaderRecord<>>(IShaderRecord::MeshGeometryHitGroup{ .AnyHitShader = shaderModule }));
            else if (shaderModule->type() == ShaderStage::ClosestHit)
                this->addShaderRecord(makeUnique<ShaderRecord<>>(IShaderRecord::MeshGeometryHitGroup{ .ClosestHitShader = shaderModule }));
            else
                this->addShaderRecord(makeUnique<ShaderRecord<>>(shaderModule));
        }

        /// <summary>
        /// Adds a new shader record based on the name of a shader module in the parent shader program.
        /// </summary>
        /// <remarks>
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an 
        /// any and closest hit shader, use <see cref="addMeshGeometryShaderHitGroupRecord" /> instead.
        /// </remarks>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="shaderName">The name of the shader module.</param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if no shader module with the provided name was found in the parent shader program.</exception>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline void addShaderRecord(StringView shaderName, TLocalData payload) {
            auto shaderModule = this->findShaderModule(shaderName);

            if (shaderModule == nullptr) [[unlikely]]
                throw InvalidArgumentException("shaderName", "The parent shader program does not contain a shader named \"{}\".", shaderName);
                
            if (shaderModule->type() == ShaderStage::AnyHit)
                this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(IShaderRecord::MeshGeometryHitGroup{ .AnyHitShader = shaderModule }, payload));
            else if (shaderModule->type() == ShaderStage::ClosestHit)
                this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(IShaderRecord::MeshGeometryHitGroup{ .ClosestHitShader = shaderModule }, payload));
            else
                this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(shaderModule, payload));
        }

        /// <summary>
        /// Adds a new mesh geometry hit group record based on names of the shader modules.
        /// </summary>
        /// <param name="anyHitShaderName">The name of the any hit shader module.</param>
        /// <param name="closestHitShaderName">The name of the closest hit shader module.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if both provided shader names are empty or not found, the shaders are not of the right type or do not belong to the parent shader program.</exception>
        inline void addMeshGeometryShaderHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName) {
            IShaderRecord::MeshGeometryHitGroup hitGroup = { 
                .ClosestHitShader = closestHitShaderName.has_value() ? this->findShaderModule(closestHitShaderName.value()) : nullptr,
                .AnyHitShader = anyHitShaderName.has_value() ? this->findShaderModule(anyHitShaderName.value()) : nullptr
            };

            this->addShaderRecord(makeUnique<ShaderRecord<>>(hitGroup));
        }

        /// <summary>
        /// Adds a new mesh geometry hit group record based on names of the shader modules.
        /// </summary>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="anyHitShaderName">The name of the any hit shader module.</param>
        /// <param name="closestHitShaderName">The name of the closest hit shader module.</param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if both provided shader names are empty or not found, the shaders are not of the right type or do not belong to the parent shader program.</exception>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline void addMeshGeometryShaderHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName, TLocalData payload) {
            IShaderRecord::MeshGeometryHitGroup hitGroup = { 
                .ClosestHitShader = closestHitShaderName.has_value() ? this->findShaderModule(closestHitShaderName.value()) : nullptr,
                .AnyHitShader = anyHitShaderName.has_value() ? this->findShaderModule(anyHitShaderName.value()) : nullptr
            };

            this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(hitGroup, payload));
        }

        /// <summary>
        /// Adds a new shader record to the shader record collection.
        /// </summary>
        /// <param name="shaderGroup">The shader module or hit group.</param>
        inline void addShaderRecord(const ShaderRecord<>::shader_group_type& shaderGroup) {
            this->addShaderRecord(makeUnique<ShaderRecord<>>(shaderGroup));
        }

        /// <summary>
        /// Adds a new shader record to the shader record collection.
        /// </summary>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="shaderGroup">The shader module or hit group.</param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline void addShaderRecord(ShaderRecord<TLocalData>::shader_group_type shaderGroup, TLocalData payload) {
            this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(shaderGroup, payload));
        }

        /// <summary>
        /// Adds a new shader record based on the name of a shader module in the parent shader program.
        /// </summary>
        /// <remarks>
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an 
        /// any and closest hit shader, use <see cref="withMeshGeometryShaderHitGroupRecord" /> instead.
        /// </remarks>
        /// <param name="shaderName">The name of the shader module.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        inline ShaderRecordCollection&& withShaderRecord(StringView shaderName) {
            this->addShaderRecord(shaderName);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// <summary>
        /// Adds a new shader record based on the name of a shader module in the parent shader program.
        /// </summary>
        /// <remarks>
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an 
        /// any and closest hit shader, use <see cref="withMeshGeometryShaderHitGroupRecord" /> instead.
        /// </remarks>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="shaderName"></param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline ShaderRecordCollection&& withShaderRecord(StringView shaderName, TLocalData payload) {
            this->addShaderRecord(shaderName, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// <summary>
        /// Adds a new mesh geometry hit group record based on names of the shader modules.
        /// </summary>
        /// <param name="anyHitShaderName">The name of the any hit shader module.</param>
        /// <param name="closestHitShaderName">The name of the closest hit shader module.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        inline ShaderRecordCollection&& withMeshGeometryHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName) {
            this->addMeshGeometryShaderHitGroupRecord(anyHitShaderName, closestHitShaderName);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// <summary>
        /// Adds a new mesh geometry hit group record based on names of the shader modules.
        /// </summary>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="anyHitShaderName">The name of the any hit shader module.</param>
        /// <param name="closestHitShaderName">The name of the closest hit shader module.</param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline ShaderRecordCollection&& withMeshGeometryHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName, TLocalData payload) {
            this->addMeshGeometryShaderHitGroupRecord(anyHitShaderName, closestHitShaderName, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// <summary>
        /// Adds a new shader record to the shader record collection.
        /// </summary>
        /// <param name="shaderGroup">The shader module or hit group.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        inline ShaderRecordCollection&& withShaderRecord(ShaderRecord<>::shader_group_type shaderGroup) {
            this->addShaderRecord(shaderGroup);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// <summary>
        /// Adds a new shader record to the shader record collection.
        /// </summary>
        /// <typeparam name="TLocalData">The type of the shader record local data.</typeparam>
        /// <param name="shaderGroup">The shader module or hit group.</param>
        /// <param name="payload">The shader-local data of the shader record.</param>
        /// <returns>A reference to the current shader record collection.</returns>
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == 8)
        inline ShaderRecordCollection&& withShaderRecord(ShaderRecord<TLocalData>::shader_group_type shaderGroup, TLocalData payload) {
            this->addShaderRecord(shaderGroup, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }
    };

    /// <summary>
    /// The interface for a shader program.
    /// </summary>
    /// <remarks>
    /// A shader program differs in it's functionality as well as the contained shader modules, depending on the pipeline type it gets assigned to. A shader program can be
    /// of any of the following types:
    /// 
    /// <list type="bullet">
    /// <item>
    /// <description>
    /// **Rasterization:** A rasterization pipeline is a traditional pipeline, that can contain at maximum one module of the following stages: *Vertex*, *Tessellation Control*, 
    /// *Tessellation Evaluation*, *Geometry*, *Fragment*. A vertex and fragment shader are required for rasterization programs.
    /// </description>
    /// </item>
    /// <item>
    /// <description>
    /// **Mesh shading:** If mesh shader support is enabled (through the device feature <see cref="GraphicsDeviceFeatures::MeshShaders" />), a mesh shading program can contain 
    /// at maximum one module of the following stages: *Task*, *Mesh*, *Fragment*. A mesh and fragment shader are required for a mesh shading program.
    /// </description>
    /// </item>
    /// <item>
    /// <description>
    /// **Compute:** A compute shader program must only contain a single module for the *Compute* stage.
    /// </description>
    /// </item>
    /// <item>
    /// <description>
    /// **Ray-tracing:** If ray tracing support is enabled (through the device feature <see cref="GraphicsDeviceFeatures::RayTracing" />), a ray tracing program can contain 
    /// modules of the following stages: *Ray Generation*, *Any Hit*, *Closest Hit*, *Intersection*, *Miss*, *Callable*. There must be exactly one *Ray Generation* module. All 
    /// other modules can occur multiple times. To build a ray tracing pipeline, all shaders should be added to a single shader program, which is then passed to the pipeline 
    /// during creation.
    /// </description>
    /// </item>
    /// </list>
    /// 
    /// Shaders from different program types must not be mixed. For example, it is not valid to add a compute module to a rasterization program. The only exception to this
    /// is the <see cref="ShaderStage::Fragment" /> module, which can be added to a mesh pipeline, as well as a rasterization pipeline.
    /// </remarks>
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_RENDERING_API IShaderProgram : public std::enable_shared_from_this<IShaderProgram> {
    public:
        virtual ~IShaderProgram() noexcept = default;

    public:
        /// <summary>
        /// Returns a pointer with shared ownership to the current instance.
        /// </summary>
        /// <returns>A pointer with shared ownership to the current instance.</returns>
        inline std::shared_ptr<const IShaderProgram> getptr() const {
            return shared_from_this();
        }

        /// <summary>
        /// Returns a pointer with shared ownership to the current instance.
        /// </summary>
        /// <returns>A pointer with shared ownership to the current instance.</returns>
        inline std::shared_ptr<IShaderProgram> getptr() {
            return shared_from_this();
        }

    public:
        /// <summary>
        /// Returns a pointer to shader module based on its (case-sensitive) name.
        /// </summary>
        /// <param name="name">The name or file name of the shader module.</param>
        /// <returns>A pointer to the shader module, or `nullptr`, if it was not found.</returns>
        inline const IShaderModule* operator[](StringView name) const noexcept {
            auto modules = this->getModules();

            if (auto match = std::ranges::find_if(modules, [name](auto module) { return std::strcmp(module->fileName().c_str(), name.data()) == 0; }); match != modules.end())
                return *match;

            return nullptr;
        }

        /// <summary>
        /// Returns `true`, if the program contains a shader module with the provided name or file name and `false` otherwise.
        /// </summary>
        /// <param name="name">The case-sensitive name or file name of the shader module to look up.</param>
        /// <returns>`true`, if the program contains a shader module with the provided name or file name and `false` otherwise.</returns>
        inline bool contains(StringView name) const noexcept {
            auto modules = this->getModules();
            return std::ranges::find_if(modules, [name](auto module) { return std::strcmp(module->fileName().c_str(), name.data()) == 0; }) != modules.end();
        };

        /// <summary>
        /// Returns `true`, if the program contains the provided shader module and `false` otherwise.
        /// </summary>
        /// <param name="module">The module to look up in the shader program.</param>
        /// <returns>`true`, if the program contains the provided shader module and `false` otherwise.</returns>
        inline bool contains(const IShaderModule& module) const noexcept {
            auto modules = this->getModules();
            return std::ranges::find_if(modules, [&module](auto m) { return m == &module; }) != modules.end();
        };

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

        /// <summary>
        /// Builds a shader record collection based on the current shader program.
        /// </summary>
        /// <returns>The shader record collection instance.</returns>
        inline [[nodiscard]] ShaderRecordCollection buildShaderRecordCollection() const noexcept {
            return ShaderRecordCollection(this->getptr());
        }

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
        /// Returns a pointer the vertex buffer layout for binding provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the vertex buffer layout.</param>
        /// <returns>The vertex buffer layout for binding provided with <paramref name="binding" />.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if no vertex buffer layout is bound to <paramref name="binding" />.</exception>
        virtual const IVertexBufferLayout* vertexBufferLayout(UInt32 binding) const = 0;

        /// <summary>
        /// Returns a pointer to the index buffer layout, or `nullptr` if the input assembler does not handle indices.
        /// </summary>
        /// <returns>The index buffer layout, or `nullptr` if the input assembler does not handle indices.</returns>
        virtual const IIndexBufferLayout* indexBufferLayout() const noexcept = 0;

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
    /// <seealso cref="IComputePipeline" />
    /// <seealso cref="IRenderPipeline" />
    /// <seealso cref="IRayTracingPipeline" />
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
        /// Gets a reference to the command queue that this command buffer was allocated from.
        /// </summary>
        /// <returns>A reference to the command queue that this command buffer was allocated from.</returns>
        virtual const ICommandQueue& queue() const noexcept = 0;

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
        /// Creates a new barrier instance.
        /// </summary>
        /// <param name="syncBefore">The pipeline stage(s) all previous commands have to finish before the barrier is executed.</param>
        /// <param name="syncAfter">The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.</param>
        /// <returns>The instance of the barrier.</returns>
        inline [[nodiscard]] UniquePtr<IBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept {
            return this->getBarrier(syncBefore, syncAfter);
        }

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
        inline void transfer(const IBuffer& source, const IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
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
        inline void transfer(SharedPtr<const IBuffer> source, const IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, targetElement, elements);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from a temporary buffer into <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method creates a temporary buffer and maps <paramref name="data" /> into it, before transferring it into <paramref name="target" />. A reference of the temporary buffer is stored 
        /// until the parent command queue finished using the command buffer. At this point, the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this
        /// is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been executed. 
        /// </remarks>
        /// <param name="data">The address that marks the beginning of the data to map.</param>
        /// <param name="size">The number of bytes to map.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="targetElement">The array element to map the data to.</param>
        /// <param name="elements">The number of elements to copy.</param>
        inline void transfer(const void* const data, size_t size, const IBuffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(data, size, target, targetElement, elements);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from a temporary buffer into <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method creates a temporary buffer and maps <paramref name="data" /> into it, before transferring it into <paramref name="target" />. A reference of the temporary buffer is stored 
        /// until the parent command queue finished using the command buffer. At this point, the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this
        /// is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been executed. 
        /// </remarks>
        /// <param name="data">The addresses that mark the beginning of the element data to map.</param>
        /// <param name="elementSize">The number of bytes to map for each element.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="targetElement">The first array element to transfer the data to.</param>
        inline void transfer(Span<const void* const> data, size_t elementSize, const IBuffer& target, UInt32 targetElement = 0) const {
            this->cmdTransfer(data, elementSize, target, targetElement);
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
        inline void transfer(const IBuffer& source, const IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
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
        inline void transfer(SharedPtr<const IBuffer> source, const IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from a temporary buffer into <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method creates a temporary buffer and maps <paramref name="data" /> into it, before transferring it into <paramref name="target" />. A reference of the temporary buffer is stored 
        /// until the parent command queue finished using the command buffer. At this point, the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this
        /// is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been executed. 
        /// </remarks>
        /// <param name="data">The address that marks the beginning of the data to map.</param>
        /// <param name="size">The number of bytes to map.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        inline void transfer(const void* const data, size_t size, const IImage& target, UInt32 subresource = 0) const {
            this->cmdTransfer(data, size, target, subresource);
        }

        /// <summary>
        /// Performs a buffer-to-buffer transfer from a temporary buffer into <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// This method creates a temporary buffer and maps <paramref name="data" /> into it, before transferring it into <paramref name="target" />. A reference of the temporary buffer is stored 
        /// until the parent command queue finished using the command buffer. At this point, the command queue calls <see cref="releaseSharedState" /> to release all shared references. Note that this
        /// is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been executed. 
        /// </remarks>
        /// <param name="data">The addresses that mark the beginning of the element data to map.</param>
        /// <param name="elementSize">The number of bytes to map for each element.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        inline void transfer(Span<const void* const> data, size_t elementSize, const IImage& target, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(data, elementSize, target, firstSubresource, elements);
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
        inline void transfer(const IImage& source, const IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
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
        inline void transfer(SharedPtr<const IImage> source, const IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
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
        inline void transfer(const IImage& source, const IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
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
        inline void transfer(SharedPtr<const IImage> source, const IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        inline void use(const IPipeline& pipeline) const noexcept {
            this->cmdUse(pipeline);
        }

        /// <summary>
        /// Binds the provided descriptor to the last pipeline that was used by the command buffer.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to bind.</param>
        /// <exception cref="RuntimeException">Thrown, if no pipeline has been used before attempting to bind the descriptor set.</exception>
        /// <seealso cref="use" />
        inline void bind(const IDescriptorSet& descriptorSet) const {
            this->cmdBind(descriptorSet);
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <typeparam name="T">The type of the descriptor sets.</typeparam>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        /// <exception cref="RuntimeException">Thrown, if no pipeline has been used before attempting to bind the descriptor set.</exception>
        template <typename TSelf, typename T>
        inline void bind(this const TSelf& self, std::initializer_list<const T*> descriptorSets) requires
            std::derived_from<T, IDescriptorSet>
        {
            // NOTE: In the future we might be able to remove this method, if P2447R4 is added to the language.
            Array<const T*> sets = descriptorSets;
            self.bind(Span<const T*>(sets));
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        /// <exception cref="RuntimeException">Thrown, if no pipeline has been used before attempting to bind the descriptor set.</exception>
        template <typename TSelf>
        inline void bind(this const TSelf& self, std::ranges::input_range auto&& descriptorSets) requires 
            std::derived_from<std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>, IDescriptorSet>
        {
            using descriptor_set_type = std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>;
            auto sets = descriptorSets | std::ranges::to<Array<const descriptor_set_type*>>();
            self.bind(Span<const descriptor_set_type*>(sets));
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        inline void bind(Span<const IDescriptorSet*> descriptorSets) const noexcept {
            this->cmdBind(descriptorSets);
        }

        /// <summary>
        /// Binds the provided descriptor set to the provided pipeline.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        inline void bind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept {
            this->cmdBind(descriptorSet, pipeline);
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <typeparam name="T">The type of the descriptor sets.</typeparam>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        /// <exception cref="RuntimeException">Thrown, if no pipeline has been used before attempting to bind the descriptor set.</exception>
        template <typename TSelf, typename T>
        inline void bind(this const TSelf& self, std::initializer_list<const T*> descriptorSets, const typename TSelf::pipeline_type& pipeline) noexcept
        {
            // NOTE: In the future we might be able to remove this method, if P2447R4 is added to the language.
            Array<const T*> sets = descriptorSets;
            self.bind(Span<const T*>(sets), pipeline);
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the provided pipeline.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        template <typename TSelf>
        inline void bind(this const TSelf& self, std::ranges::input_range auto&& descriptorSets, const typename TSelf::pipeline_type& pipeline) noexcept requires 
            std::derived_from<std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>, IDescriptorSet>
        {
            using descriptor_set_type = std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>;
            auto sets = descriptorSets | std::ranges::to<Array<const descriptor_set_type*>>();
            self.bind(Span<const descriptor_set_type*>(sets), pipeline);
        }

        /// <summary>
        /// Binds an arbitrary input range of descriptor sets to the provided pipeline.
        /// </summary>
        /// <remarks>
        /// Note that if an element of <paramref name="descriptorSets" /> is `nullptr`, it will be ignored.
        /// </remarks>
        /// <param name="descriptorSets">The pointers to the descriptor sets to bind.</param>
        /// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
        inline void bind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const noexcept {
            this->cmdBind(descriptorSets, pipeline);
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
        /// <param name="threadCount">The number of threads per dimension.</param>
        virtual void dispatch(const Vector3u& threadCount) const noexcept = 0;

        /// <summary>
        /// Executes a compute shader.
        /// </summary>
        /// <param name="x">The number of threads along the x dimension.</param>
        /// <param name="y">The number of threads along the y dimension.</param>
        /// <param name="z">The number of threads along the z dimension.</param>
        inline void dispatch(UInt32 x, UInt32 y, UInt32 z) const noexcept {
            this->dispatch({ x,y, z });
        }
        
        /// <summary>
        /// Executes a mesh shader pipeline.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::MeshShaders" /> feature is enabled.
        /// </remarks>
        /// <param name="threadCount">The number of threads per dimension.</param>
        virtual void dispatchMesh(const Vector3u& threadCount) const noexcept = 0;

        /// <summary>
        /// Executes a mesh shader pipeline.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::MeshShaders" /> feature is enabled.
        /// </remarks>
        /// <param name="x">The number of threads along the x dimension.</param>
        /// <param name="y">The number of threads along the y dimension.</param>
        /// <param name="z">The number of threads along the z dimension.</param>
        inline void dispatchMesh(UInt32 x, UInt32 y, UInt32 z) const noexcept {
            this->dispatchMesh({ x, y, z });
        }

        /// <summary>
        /// Executes a query on a ray-tracing pipeline.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="width">The width of the ray-tracing query.</param>
        /// <param name="height">The height of the ray-tracing query.</param>
        /// <param name="depth">The depth of the ray-tracing query.</param>
        /// <param name="offsets">The offsets, sizes and strides for each shader binding table.</param>
        /// <param name="rayGenerationShaderBindingTable">The shader binding table that contains the ray generation shader.</param>
        /// <param name="missShaderBindingTable">The shader binding table that contains the miss shaders.</param>
        /// <param name="hitShaderBindingTable">The shader binding table that contains the hit shaders.</param>
        /// <param name="callableShaderBindingTable">The shader binding table that contains the callable shaders.</param>
        inline void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable = nullptr, const IBuffer* hitShaderBindingTable = nullptr, const IBuffer* callableShaderBindingTable = nullptr) const noexcept {
            this->cmdTraceRays(width, height, depth, offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

        /// <summary>
        /// Executes a query on a ray-tracing pipeline.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="dimensions">The dimensions of the ray-tracing query.</param>
        /// <param name="offsets">The offsets, sizes and strides for each shader binding table.</param>
        /// <param name="rayGenerationShaderBindingTable">The shader binding table that contains the ray generation shader.</param>
        /// <param name="missShaderBindingTable">The shader binding table that contains the miss shaders.</param>
        /// <param name="hitShaderBindingTable">The shader binding table that contains the hit shaders.</param>
        /// <param name="callableShaderBindingTable">The shader binding table that contains the callable shaders.</param>
        inline void traceRays(const Vector3u& dimensions, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable = nullptr, const IBuffer* hitShaderBindingTable = nullptr, const IBuffer* callableShaderBindingTable = nullptr) const noexcept {
            this->traceRays(dimensions.x(), dimensions.y(), dimensions.z(), offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

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

        /// <summary>
        /// Builds a bottom-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="blas">The bottom-level acceleration structure to build.</param>
        /// <param name="scratchBuffer">The scratch buffer to use for building the acceleration structure.</param>
        /// <param name="buffer">The buffer that contains the acceleration structure after the build.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure gets stored after the build.</param>
        /// <exception cref="ArgumentNotInitializedException">Thrown, if the provided <paramref name="scratchBuffer" /> is not initialized.</exception>
        /// <seealso cref="IAccelerationStructure::build" />
        inline void buildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdBuildAccelerationStructure(blas, scratchBuffer, buffer, offset);
        }

        /// <summary>
        /// Builds a top-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="tlas">The top-level acceleration structure to build.</param>
        /// <param name="scratchBuffer">The scratch buffer to use for building the acceleration structure.</param>
        /// <param name="buffer">The buffer that contains the acceleration structure after the build.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure gets stored after the build.</param>
        /// <exception cref="ArgumentNotInitializedException">Thrown, if the provided <paramref name="scratchBuffer" /> is not initialized.</exception>
        /// <seealso cref="IAccelerationStructure::build" />
        inline void buildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdBuildAccelerationStructure(tlas, scratchBuffer, buffer, offset);
        }

        /// <summary>
        /// Updates a bottom-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="blas">The bottom-level acceleration structure to build.</param>
        /// <param name="scratchBuffer">The scratch buffer to use for building the acceleration structure.</param>
        /// <param name="buffer">The buffer that contains the acceleration structure after the build.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure gets stored after the build.</param>
        /// <exception cref="ArgumentNotInitializedException">Thrown, if the provided <paramref name="scratchBuffer" /> is not initialized.</exception>
        /// <seealso cref="IAccelerationStructure::build" />
        inline void updateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdUpdateAccelerationStructure(blas, scratchBuffer, buffer, offset);
        }

        /// <summary>
        /// Updates a top-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="tlas">The top-level acceleration structure to build.</param>
        /// <param name="scratchBuffer">The scratch buffer to use for building the acceleration structure.</param>
        /// <param name="buffer">The buffer that contains the acceleration structure after the build.</param>
        /// <param name="offset">The offset into <paramref name="buffer" /> at which the acceleration structure gets stored after the build.</param>
        /// <exception cref="ArgumentNotInitializedException">Thrown, if the provided <paramref name="scratchBuffer" /> is not initialized.</exception>
        /// <seealso cref="IAccelerationStructure::build" />
        inline void updateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdUpdateAccelerationStructure(tlas, scratchBuffer, buffer, offset);
        }

        /// <summary>
        /// Copies the acceleration structure <paramref name="from" /> into the acceleration structure <paramref name="to" />.
        /// </summary>
        /// <remarks>
        /// Prefer calling <see cref="IBottomLevelAccelerationStructure::copy" /> over directly issuing copy commands on a command buffer, as this will make sure that the destination buffer will 
        /// be properly allocated and contains enough memory to store the copy. Only issue copies on the command buffer directly, if you want to retain the destination buffer and know for certain,
        /// that it contains a sufficient amount of memory.
        /// 
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="from">The source acceleration structure to copy from.</param>
        /// <param name="to">The destination acceleration structure to copy to.</param>
        /// <param name="compress">If set to `true`, the acceleration structure will be compressed.</param>
        inline void copyAccelerationStructure(const IBottomLevelAccelerationStructure& from, const IBottomLevelAccelerationStructure& to, bool compress = false) const noexcept {
            this->cmdCopyAccelerationStructure(from, to, compress);
        }

        /// <summary>
        /// Copies the acceleration structure <paramref name="from" /> into the acceleration structure <paramref name="to" />.
        /// </summary>
        /// <remarks>
        /// Prefer calling <see cref="ITopLevelAccelerationStructure::copy" /> over directly issuing copy commands on a command buffer, as this will make sure that the destination buffer will be 
        /// properly allocated and contains enough memory to store the copy. Only issue copies on the command buffer directly, if you want to retain the destination buffer and know for certain,
        /// that it contains a sufficient amount of memory.
        /// 
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="from">The source acceleration structure to copy from.</param>
        /// <param name="to">The destination acceleration structure to copy to.</param>
        /// <param name="compress">If set to `true`, the acceleration structure will be compressed.</param>
        inline void copyAccelerationStructure(const ITopLevelAccelerationStructure& from, const ITopLevelAccelerationStructure& to, bool compress = false) const noexcept {
            this->cmdCopyAccelerationStructure(from, to, compress);
        }

    protected:
        /// <summary>
        /// Called by the parent command queue to signal that the command buffer should release it's shared state.
        /// </summary>
        virtual void releaseSharedState() const = 0;

    private:
        virtual UniquePtr<IBarrier> getBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept = 0;
        virtual void cmdBarrier(const IBarrier& barrier) const noexcept = 0;
        virtual void cmdGenerateMipMaps(IImage& image) noexcept = 0;
        virtual void cmdTransfer(const IBuffer& source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(const IBuffer& source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(const IImage& source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const IImage& source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<const IBuffer> source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(SharedPtr<const IBuffer> source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(SharedPtr<const IImage> source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(SharedPtr<const IImage> source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const void* const data, size_t size, const IBuffer& target, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(Span<const void* const> data, size_t elementSize, const IBuffer& target, UInt32 targetElement) const = 0;
        virtual void cmdTransfer(const void* const data, size_t size, const IImage& target, UInt32 subresource) const = 0;
        virtual void cmdTransfer(Span<const void* const> data, size_t elementSize, const IImage& target, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdUse(const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet) const = 0;
        virtual void cmdBind(Span<const IDescriptorSet*> descriptorSets) const = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IVertexBuffer& buffer) const noexcept = 0;
        virtual void cmdBind(const IIndexBuffer& buffer) const noexcept = 0;
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept = 0;
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdExecute(SharedPtr<const ICommandBuffer> commandBuffer) const = 0;
        virtual void cmdExecute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffer) const = 0;
        virtual void cmdBuildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdBuildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdUpdateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdUpdateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdCopyAccelerationStructure(const IBottomLevelAccelerationStructure& from, const IBottomLevelAccelerationStructure& to, bool compress) const noexcept = 0;
        virtual void cmdCopyAccelerationStructure(const ITopLevelAccelerationStructure& from, const ITopLevelAccelerationStructure& to, bool compress) const noexcept = 0;
        virtual void cmdTraceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable, const IBuffer* hitShaderBindingTable, const IBuffer* callableShaderBindingTable) const noexcept = 0;
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

        /// <summary>
        /// Returns the multi-sampling level of the pipeline.
        /// </summary>
        /// <remarks>
        /// When using the pipeline, the multi-sampling level must match the level of the render target images.
        /// </remarks>
        /// <returns>The multi-sampling level of the pipeline.</returns>
        /// <seealso cref="updateSamples" />
        virtual MultiSamplingLevel samples() const noexcept = 0;

        /// <summary>
        /// Changes the multi-sampling level of the pipeline.
        /// </summary>
        /// <remarks>
        /// Changing the multi-sampling level of a pipeline causes it to be re-created, which is considered an expensive operation. Don't use this method to change 
        /// samples frequently, for example when binding frame buffers with different sample levels. Instead, use multiple pipelines for this purpose.
        /// </remarks>
        virtual void updateSamples(MultiSamplingLevel samples) = 0;

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
    /// The interface for a ray tracing pipeline.
    /// </summary>
    class LITEFX_RENDERING_API IRayTracingPipeline : public virtual IPipeline {
    public:
        virtual ~IRayTracingPipeline() noexcept = default;

    public:
        /// <summary>
        /// Returns the shader record collection of the ray tracing pipeline.
        /// </summary>
        /// <returns>The shader record collection of the ray tracing pipeline.</returns>
        virtual const ShaderRecordCollection& shaderRecords() const noexcept = 0;

        /// <summary>
        /// Returns the maximum number of ray bounces.
        /// </summary>
        /// <returns>The shader record collection of the ray tracing pipeline.</returns>
        virtual UInt32 maxRecursionDepth() const noexcept = 0;

        /// <summary>
        /// Returns the maximum size of a single ray payload.
        /// </summary>
        /// <remarks>
        /// A ray payload is the data that is passed down the `TraceRay` function call chain. It can be zero, if 
        /// [Ray Payload Qualifiers](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#payload-access-qualifiers) are used. Otherwise it must be set to the largest ray
        /// payload size used in the ray-tracing pipeline. 
        /// 
        /// This property can currently not be queried from reflection.
        /// </remarks>
        /// <returns>The maximum size of a single ray payload.</returns>
        virtual UInt32 maxPayloadSize() const noexcept = 0;

        /// <summary>
        /// Returns the maximum size of a single ray attribute.
        /// </summary>
        /// A ray attribute is the data that is passed to a hit shader for a specific event. Different to ray payloads, it only contains the data that describe the event (such as 
        /// the hit coordinates, etc.).
        /// 
        /// This property can currently not be queried from reflection.
        /// </remarks>
        /// <returns></returns>
        virtual UInt32 maxAttributeSize() const noexcept = 0;

        /// <summary>
        /// Allocates a buffer that contains the shader binding table containing the shader groups specified by the <paramref name="groups" /> parameter.
        /// </summary>
        /// <remarks>
        /// The shader binding table consists out of individual shader records, where each record refers to a shader record plus its local data, as specified in the shader record 
        /// collection that was passed to the ray-tracing pipeline during creation. The size of a record within the shader binding table is determined by the largest local data
        /// size of all records of the groups to be included. It makes sense to pack multiple records into the same buffer for efficiency, however it may generally be a good
        /// idea to separate groups that require large amount of local shader data into their own buffers to keep the other buffers smaller.
        /// 
        /// The shader binding table is created on the default resource heap (<see cref="ResourceHeap::Dynamic" />). However, for best performance, consider transferring it to a
        /// buffer on the GPU resource heap (<see cref="ResourceHeap::Resource" />) afterwards.
        /// </remarks>
        /// <param name="offsets">A reference to a structure that receives the offsets and sizes to the groups within the shader binding table.</param>
        /// <param name="groups">The groups to include into the shader binding table.</param>
        /// <returns>The buffer that stores the shader binding table.</returns>
        inline UniquePtr<IBuffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const noexcept {
            return this->getShaderBindingTable(offsets, groups);
        }

    private:
        virtual UniquePtr<IBuffer> getShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const noexcept = 0;
    };

    /// <summary>
    /// The interface for a frame buffer.
    /// </summary>
    /// <remarks>
    /// A frame buffer is a set of images of equal size, that are used by render targets and/or input attachments in a <see cref="IRenderPass" />. When creating a new frame buffer,
    /// it is empty by default and needs images to be added into it. When beginning a render pass during rendering, a frame buffer instance needs to be passed to it. The render 
    /// pass then tries to obtain an image for each render target from the frame buffer. It does this by resolving it's render targets (<see cref="IRenderPass::renderTargets" />).
    /// A render target stores a unique identifier (<see cref="IRenderTarget::identifier" />), that is used to obtain the image. Before this resolution process can be successful, 
    /// the render targets must first be mapped to the images in the frame buffer by calling <see cref="IFrameBuffer::mapRenderTarget" />. Calling this method multiple times will
    /// overwrite the mapping. It is also possible to remove a render target mapping by calling <see cref="IFrameBuffer::unmapRenderTarget" />. This will result in future attempts
    /// to resolve this render target using the frame buffer instance to fail.
    /// </remarks>
    class LITEFX_RENDERING_API IFrameBuffer : public virtual IStateResource {
    public:
        /// <summary>
        /// Event arguments that are published to subscribers when a frame buffer gets resized.
        /// </summary>
        /// <seealso cref="IFrameBuffer::resize" />
        /// <seealso cref="IFrameBuffer::resized" />
        struct ResizeEventArgs : public EventArgs {
        private:
            const Size2d& m_newSize;

        public:
            ResizeEventArgs(const Size2d& newSize) : 
                EventArgs(), m_newSize(newSize) { }
            ResizeEventArgs(const ResizeEventArgs&) = default;
            ResizeEventArgs(ResizeEventArgs&&) = default;
            virtual ~ResizeEventArgs() noexcept = default;

        public:
            ResizeEventArgs& operator=(const ResizeEventArgs&) = default;
            ResizeEventArgs& operator=(ResizeEventArgs&&) = default;

        public:
            /// <summary>
            /// Returns the new size of the frame buffer.
            /// </summary>
            /// <returns>The new size of the frame buffer.</returns>
            inline const Size2d& newSize() const noexcept {
                return m_newSize;
            }
        };

        /// <summary>
        /// Event arguments that are published to subscribers when a frame buffer gets released.
        /// </summary>
        /// <seealso cref="IFrameBuffer::~IFrameBuffer" />
        /// <seealso cref="IFrameBuffer::released" />
        struct ReleasedEventArgs : public EventArgs {
        public:
            ReleasedEventArgs() : 
                EventArgs() { }
            ReleasedEventArgs(const ReleasedEventArgs&) = default;
            ReleasedEventArgs(ReleasedEventArgs&&) = default;
            virtual ~ReleasedEventArgs() noexcept = default;

        public:
            ReleasedEventArgs& operator=(const ReleasedEventArgs&) = default;
            ReleasedEventArgs& operator=(ReleasedEventArgs&&) = default;
        };

    public:
        /// <summary>
        /// Releases the frame buffer.
        /// </summary>
        virtual inline ~IFrameBuffer() noexcept {
            released.invoke(this, { });
        }

    public:
        /// <summary>
        /// Invoked when the frame buffer gets resized.
        /// </summary>
        /// <seealso cref="resize" />
        mutable Event<ResizeEventArgs> resized;

        /// <summary>
        /// Invoked when the frame buffer gets released.
        /// </summary>
        /// <remarks>
        /// Note that it is no longer valid to access the frame buffer when receiving this event. The only thing that can be assumed to still be valid is the pointer to the frame 
        /// buffer. The intent of this event is to release any resources that depend on the frame buffer instance. Internally, render passes and pipelines use this event to release
        /// cached frame buffer states they hold, such as descriptor sets for input attachment bindings or command buffers associated with the frame buffer.
        /// </remarks>
        /// <seealso cref="~IFrameBuffer" />
        mutable Event<ReleasedEventArgs> released;

    public:
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
        /// Maps a render target to a frame buffer image.
        /// </summary>
        /// <remarks>
        /// When calling <see cref="IRenderPass::begin" />, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be 
        /// successful, a mapping first needs to be established between the render target and the image. This method establishes this mapping.
        /// 
        /// Calling this method multiple times will overwrite the mapped index.
        /// </remarks>
        /// <param name="renderTarget">The render target to map the image to.</param>
        /// <param name="index">The index of the image to map to the render target.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if <paramref name="index" /> does not address an image in the frame buffer.</exception>
        /// <seealso cref="unmapRenderTarget" />
        virtual void mapRenderTarget(const RenderTarget& renderTarget, UInt32 index) = 0;

        /// <summary>
        /// Maps a render target to a frame buffer image.
        /// </summary>
        /// <remarks>
        /// When calling <see cref="IRenderPass::begin" />, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be 
        /// successful, a mapping first needs to be established between the render target and the image. This method establishes this mapping.
        /// 
        /// Calling this method multiple times will overwrite the mapped index.
        /// </remarks>
        /// <param name="renderTarget">The render target to map the image to.</param>
        /// <param name="imageName">The name of the image the render target maps to.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if the frame buffer does not contain an image with the name specified in <paramref name="imageName" />.</exception>
        /// <seealso cref="unmapRenderTarget" />
        virtual void mapRenderTarget(const RenderTarget& renderTarget, StringView imageName) = 0;

        /// <summary>
        /// Maps a render target to a frame buffer image using the render targets name to look up the image.
        /// </summary>
        /// <remarks>
        /// When calling <see cref="IRenderPass::begin" />, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be 
        /// successful, a mapping first needs to be established between the render target and the image. This method establishes this mapping.
        /// 
        /// Calling this method multiple times will overwrite the mapped index.
        /// </remarks>
        /// <param name="renderTarget">The render target to map the image to.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if the frame buffer does not contain an image with the same name as the render target.</exception>
        /// <seealso cref="unmapRenderTarget" />
        inline void mapRenderTarget(const RenderTarget& renderTarget) {
            this->mapRenderTarget(renderTarget, renderTarget.name());
        }

        /// <summary>
        /// Maps a set of render targets to the frame buffer images, using the names of the render targets to look up the images.
        /// </summary>
        /// <param name="renderTargets">The render targets to map to the frame buffer.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if the frame buffer cannot map the name of one or more render targets to images.</exception>
        /// <seealso cref="mapRenderTarget" />
        /// <seealso cref="unmapRenderTarget" />
        inline void mapRenderTargets(Span<const RenderTarget> renderTargets) {
            std::ranges::for_each(renderTargets, [this](auto& renderTarget) { this->mapRenderTarget(renderTarget); });
        }

        /// <summary>
        /// Removes a mapping between a render target and an image in the frame buffer.
        /// </summary>
        /// <remarks>
        /// If no image in the frame buffer is currently mapped to <paramref name="renderTarget" />, calling this method will have no effect.
        /// </remarks>
        /// <param name="renderTarget">The render target to remove the mapping for.</param>
        /// <seealso cref="mapRenderTarget" />
        virtual void unmapRenderTarget(const RenderTarget& renderTarget) noexcept = 0;

        /// <summary>
        /// Returns all images contained by the frame buffer.
        /// </summary>
        /// <returns>A set of pointers to the images contained by the frame buffer.</returns>
        inline Enumerable<const IImage*> images() const noexcept {
            return this->getImages();
        }

        /// <summary>
        /// Returns an image from the frame buffer.
        /// </summary>
        /// <param name="index">The index of the image.</param>
        /// <returns>The image from the frame buffer with the index <paramref name="index" />.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the <paramref name="index" /> does not address an image in the frame buffer.</exception>
        virtual const IImage& operator[](UInt32 index) const = 0;

        /// <summary>
        /// Returns an image from the frame buffer.
        /// </summary>
        /// <param name="index">The index of the image.</param>
        /// <returns>The image from the frame buffer with the index <paramref name="index" />.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the <paramref name="index" /> does not address an image in the frame buffer.</exception>
        virtual const IImage& image(UInt32 index) const = 0;

        /// <summary>
        /// Resolves a render target and returns the image mapped to it.
        /// </summary>
        /// <param name="renderTarget">The render target to resolve.</param>
        /// <returns>The image mapped to the render target.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="renderTarget" /> is not mapped to an image in the frame buffer.</exception>
        virtual const IImage& operator[](const RenderTarget& renderTarget) const = 0;

        /// <summary>
        /// Resolves a render target and returns the image mapped to it.
        /// </summary>
        /// <param name="renderTarget">The render target to resolve.</param>
        /// <returns>The image mapped to the render target.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="renderTarget" /> is not mapped to an image in the frame buffer.</exception>
        virtual const IImage& image(const RenderTarget& renderTarget) const = 0;

        /// <summary>
        /// Resolves a render target name and returns the image mapped to it.
        /// </summary>
        /// <param name="renderTargetName">The render target name to resolve.</param>
        /// <returns>The image mapped to the render target.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="renderTargetName" /> is not mapped to an image in the frame buffer.</exception>
        virtual const IImage& operator[](StringView renderTargetName) const = 0;

        /// <summary>
        /// Resolves a render target name and returns the image mapped to it.
        /// </summary>
        /// <param name="renderTargetName">The render target name to resolve.</param>
        /// <returns>The image mapped to the render target.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="renderTargetName" /> is not mapped to an image in the frame buffer.</exception>
        virtual const IImage& image(StringView renderTargetName) const = 0;

        /// <summary>
        /// Resolves a render target name hash and returns the image mapped to it.
        /// </summary>
        /// <param name="hash">The render target name hash to resolve.</param>
        /// <returns>The image mapped to the render target.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if <paramref name="hash" /> is not mapped to an image in the frame buffer.</exception>
        /// <seealso cref="image" />
        virtual const IImage& resolveImage(UInt64 hash) const = 0;

        /// <summary>
        /// Adds an image to the frame buffer.
        /// </summary>
        /// <param name="format">The format of the image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(format, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an image to the frame buffer.
        /// </summary>
        /// <param name="format">The format of the image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        inline void addImage(Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) {
            this->addImage("", format, samples, usage);
        }

        /// <summary>
        /// Adds an image to the frame buffer.
        /// </summary>
        /// <param name="name">The name of the image.</param>
        /// <param name="format">The format of the image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, StringView name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(name, format, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an image to the frame buffer.
        /// </summary>
        /// <param name="name">The name of the image.</param>
        /// <param name="format">The format of the image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if another image with the same name as provided in <paramref name="name" /> has already been added to the frame buffer.</exception>
        virtual void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) = 0;

        /// <summary>
        /// Adds an image for a render target to the frame buffer.
        /// </summary>
        /// <param name="renderTarget">The render target for which to add an image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(renderTarget, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an image for a render target to the frame buffer.
        /// </summary>
        /// <param name="renderTarget">The render target for which to add an image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        inline void addImage(const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) {
            this->addImage(renderTarget.name(), renderTarget, samples, usage);
        }

        /// <summary>
        /// Adds an image for a render target to the frame buffer.
        /// </summary>
        /// <param name="name">The name of the image.</param>
        /// <param name="renderTarget">The render target for which to add an image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, StringView name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(name, renderTarget, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Adds an image for a render target to the frame buffer.
        /// </summary>
        /// <param name="name">The name of the image.</param>
        /// <param name="renderTarget">The render target for which to add an image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        virtual void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) = 0;

        /// <summary>
        /// Adds multiple images for a set of render targets to the frame buffer.
        /// </summary>
        /// <remarks>
        /// Note that the names of the images are built from the render target names.
        /// </remarks>
        /// <param name="renderTargets">The render targets for which to add an image.</param>
        /// <param name="samples">The number of samples of the image.</param>
        /// <param name="usage">The desired resource usage flags for the image.</param>
        template <typename TSelf>
        inline auto addImages(this TSelf&& self, Span<const RenderTarget> renderTargets, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            std::ranges::for_each(renderTargets, [&](auto& renderTarget) { self.addImage(renderTarget.name(), renderTarget, samples, usage); });
            return std::forward<TSelf>(self);
        }

        /// <summary>
        /// Causes the frame buffer to be invalidated and recreated with a new size.
        /// </summary>
        /// <param name="renderArea">The new dimensions of the frame buffer.</param>
        virtual void resize(const Size2d& renderArea) = 0;

    private:
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
        struct BeginEventArgs : public EventArgs {
        private:
            const IFrameBuffer& m_frameBuffer;

        public:
            BeginEventArgs(const IFrameBuffer& frameBuffer) : 
                EventArgs(), m_frameBuffer(frameBuffer) { }
            BeginEventArgs(const BeginEventArgs&) = default;
            BeginEventArgs(BeginEventArgs&&) = default;
            virtual ~BeginEventArgs() noexcept = default;

        public:
            BeginEventArgs& operator=(const BeginEventArgs&) = default;
            BeginEventArgs& operator=(BeginEventArgs&&) = default;

        public:
            /// <summary>
            /// Gets the frame buffer on which the render pass is executing.
            /// </summary>
            /// <returns>The buffer on which the render pass is executing.</returns>
            inline const IFrameBuffer& frameBuffer() const noexcept {
                return m_frameBuffer;
            }
        };

    public:
        virtual ~IRenderPass() noexcept = default;

    public:
        /// <summary>
        /// Invoked, when the render pass is beginning.
        /// </summary>
        /// <seealso cref="begin" />
        mutable Event<BeginEventArgs> beginning;

        /// <summary>
        /// Invoked, when the render pass is ending.
        /// </summary>
        /// <seealso cref="end" />
        mutable Event<EventArgs> ending;

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
        /// <exception cref="RuntimeException">Thrown, if the render pass has not started.</exception>
        /// <seealso cref="begin" />
        virtual const IFrameBuffer& activeFrameBuffer() const = 0;

        /// <summary>
        /// Returns the command queue, the render pass is executing on.
        /// </summary>
        /// <returns>A reference of the command queue, the render pass is executing on.</returns>
        virtual const ICommandQueue& commandQueue() const noexcept = 0;

        /// <summary>
        /// Returns an array of all render pipelines, owned by the render pass.
        /// </summary>
        /// <returns>An array of all render pipelines, owned by the render pass.</returns>
        /// <seealso cref="IRenderPipeline" />
        inline Enumerable<const IRenderPipeline*> pipelines() const noexcept {
            return this->getPipelines();
        }

        /// <summary>
        /// Returns all command buffers, that can be currently used for recording multi-threaded commands in the render pass.
        /// </summary>
        /// <returns>
        /// All command buffers, that can be currently used for recording multi-threaded commands in the render pass, or an empty set, if the render pass has not been 
        /// initialized with additional command buffers, or the render pass is currently not active.
        /// </returns>
        /// <seealso cref="commandBuffer" />
        inline Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers() const noexcept {
            return this->getCommandBuffers();
        }

        /// <summary>
        /// Returns a command buffer that can be currently used for recording multi-threaded commands in the render pass.
        /// </summary>
        /// <param name="index">The index of the command buffer.</param>
        /// <returns>A command buffer that can be currently used for recording multi-threaded commands in the render pass.</returns>
        /// <exception cref="RuntimeException">Thrown, if the render pass has not been begun.</exception>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the frame buffer does not store a command buffer at <paramref name="index" />.</exception>
        /// <seealso cref="commandBuffers" />
        inline SharedPtr<const ICommandBuffer> commandBuffer(UInt32 index) const {
            return this->getCommandBuffer(index);
        }

        /// <summary>
        /// Returns the number of secondary command buffers the render pass stores for multi-threaded command recording.
        /// </summary>
        /// <returns>The number of secondary command buffers the render pass stores for multi-threaded command recording.</returns>
        virtual UInt32 secondaryCommandBuffers() const noexcept = 0;

        /// <summary>
        /// Returns the list of render targets, the render pass renders into.
        /// </summary>
        /// <remarks>
        /// Note that the actual render target image resources are stored within the individual <see cref="FrameBuffer" />s of the render pass.
        /// </remarks>
        /// <returns>A list of render targets, the render pass renders into.</returns>
        /// <seealso cref="IFrameBuffer" />
        virtual const Array<RenderTarget>& renderTargets() const noexcept = 0;

        /// <summary>
        /// Returns the render target mapped to the location provided by <paramref name="location" />.
        /// </summary>
        /// <param name="location">The location to return the render target for.</param>
        /// <returns>The render target mapped to the location provided by <paramref name="location" />.</returns>
        virtual const RenderTarget& renderTarget(UInt32 location) const = 0;

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
        virtual const Array<RenderPassDependency>& inputAttachments() const noexcept = 0;

        /// <summary>
        /// Returns the input attachment at a <paramref name="location" />.
        /// </summary>
        /// <returns>The input attachment at a <paramref name="location" />.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if no input attachment is defined at the specified <paramref name="location" />.</exception>
        virtual const RenderPassDependency& inputAttachment(UInt32 location) const = 0;

        /// <summary>
        /// Returns the binding point for input attachment samplers.
        /// </summary>
        /// <remarks>
        /// Note that in Vulkan this is ignored, as render pass inputs are mapped to sub-pass inputs directly, which do not need to be sampled.
        /// </remarks>
        /// <returns>The binding point for input attachment samplers.</returns>
        virtual const Optional<DescriptorBindingPoint>& inputAttachmentSamplerBinding() const noexcept = 0;

        /// <summary>
        /// Begins the render pass.
        /// </summary>
        /// <param name="frameBuffer">The frame buffer to obtain input attachments and render targets from.</param>
        inline void begin(const IFrameBuffer& frameBuffer) const {
            this->beginRenderPass(frameBuffer);
        };

        /// <summary>
        /// Ends the render pass.
        /// </summary>
        /// <remarks>
        /// If the frame buffer has a present render target, this causes the render pass to synchronize with the swap chain and issue a present command.
        /// </remarks>
        /// <returns>The value of the fence that indicates the end of the render pass.</returns>
        virtual UInt64 end() const = 0;

    private:
        virtual void beginRenderPass(const IFrameBuffer& frameBuffer) const = 0;
        virtual SharedPtr<const ICommandBuffer> getCommandBuffer(UInt32 index) const noexcept = 0;
        virtual Enumerable<SharedPtr<const ICommandBuffer>> getCommandBuffers() const noexcept = 0;
        virtual Enumerable<const IRenderPipeline*> getPipelines() const noexcept = 0;
    };

    /// <summary>
    /// Interface for a swap chain.
    /// </summary>
    class LITEFX_RENDERING_API ISwapChain {
    public:
        /// <summary>
        /// Event arguments for a <see cref="ISwapChain::reseted" /> event.
        /// </summary>
        struct ResetEventArgs : public EventArgs {
        private:
            Format m_surfaceFormat;
            const Size2d& m_renderArea;
            UInt32 m_buffers;

        public:
            ResetEventArgs(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers) :
                EventArgs(), m_surfaceFormat(surfaceFormat), m_renderArea(renderArea), m_buffers(buffers) { }
            ResetEventArgs(const ResetEventArgs&) = default;
            ResetEventArgs(ResetEventArgs&&) = default;
            virtual ~ResetEventArgs() noexcept = default;

        public:
            ResetEventArgs& operator=(const ResetEventArgs&) = default;
            ResetEventArgs& operator=(ResetEventArgs&&) = default;

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
        virtual IImage* image(UInt32 backBuffer) const = 0;

        /// <summary>
        /// Returns the current swap chain back buffer image.
        /// </summary>
        /// <returns>A reference of the current swap chain back buffer image.</returns>
        virtual const IImage& image() const noexcept = 0;

        /// <summary>
        /// Returns an array of the swap chain present images.
        /// </summary>
        /// <returns>Returns an array of the swap chain present images.</returns>
        inline Enumerable<IImage*> images() const noexcept {
            return this->getImages();
        };

        /// <summary>
        /// Queues a present that gets executed after <paramref name="fence" /> has been signaled on the default graphics queue.
        /// </summary>
        /// <remarks>
        /// You can use this overload in situations where you do not have an <see cref="IRenderPass" /> or <see cref="IFrameBuffer" /> to render into before presenting. Instead, you typically
        /// copy into the swap chain back buffer images directly (<see cref="image" />). This copy is done in a command buffer that must be submitted to the default graphics queue. The swap
        /// chain can then wait for the copy to finish before presenting it. Example scenarios where this is useful are, where you want to write to the back buffer from a compute shader, that
        /// does not have an equivalent to render passes.
        /// </remarks>
        /// <param name="fence">The fence to pass on the default graphics queue after which the present is executed.</param>
        virtual void present(UInt64 fence) const = 0;

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
       mutable Event<ResetEventArgs> reseted;

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
        virtual Enumerable<IImage*> getImages() const noexcept = 0;
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
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getBuffer(type, heap, elementSize, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), heap, descriptor.elementSize(), elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), heap, elementSize, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, heap, elements, usage);
        };

        /// <summary>
        /// Creates a buffer of type <paramref name="type" />.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="type">The type of the buffer.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getBuffer(name, type, heap, elementSize, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), heap, descriptor.elementSize(), elements, usage);
        };
        
        /// <summary>
        /// Creates a buffer that can be bound to a specific descriptor.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="descriptorSet">The layout of the descriptors parent descriptor set.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), heap, elementSize, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, heap, elements, usage);
        };

        /// <summary>
        /// Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        /// </summary>
        /// <param name="name">The name of the buffer.</param>
        /// <param name="pipeline">The pipeline that provides the descriptor set.</param>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        /// <param name="binding">The binding point of the descriptor within the parent descriptor set.</param>
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the buffer.</returns>
        inline UniquePtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage);
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
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the vertex buffer.</returns>
        inline UniquePtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getVertexBuffer(layout, heap, elements, usage);
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
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the vertex buffer.</returns>
        inline UniquePtr<IVertexBuffer> createVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getVertexBuffer(name, layout, heap, elements, usage);
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
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the index buffer.</returns>
        inline UniquePtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getIndexBuffer(layout, heap, elements, usage);
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
        /// <param name="heap">The heap to allocate the buffer on.</param>
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
        /// <returns>The instance of the index buffer.</returns>
        inline UniquePtr<IIndexBuffer> createIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getIndexBuffer(name, layout, heap, elements, usage);
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
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the texture.</returns>
        /// <seealso cref="createTextures" />
        inline UniquePtr<IImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getTexture(format, size, dimension, levels, layers, samples, usage);
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
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>The instance of the texture.</returns>
        /// <seealso cref="createTextures" />
        inline UniquePtr<IImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getTexture(name, format, size, dimension, levels, layers, samples, usage);
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
        /// <param name="usage">The intended usage for the buffer.</param>
        /// <returns>An array of texture instances.</returns>
        /// <seealso cref="createTexture" />
        inline Enumerable<UniquePtr<IImage>> createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 layers = 1, UInt32 levels = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const {
            return this->getTextures(elements, format, size, dimension, layers, levels, samples, usage);
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

        /// <summary>
        /// Creates a bottom-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <returns>The bottom-level acceleration structure instance.</returns>
        /// <seealso cref="IBottomLevelAccelerationStructure" />
        inline UniquePtr<IBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->createBottomLevelAccelerationStructure("", flags);
        }

        /// <summary>
        /// Creates a bottom-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <returns>The bottom-level acceleration structure instance.</returns>
        /// <seealso cref="IBottomLevelAccelerationStructure" />
        inline UniquePtr<IBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->getBlas(name, flags);
        }

        /// <summary>
        /// Creates a top-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <returns>The top-level acceleration structure instance.</returns>
        /// <seealso cref="ITopLevelAccelerationStructure" />
        inline UniquePtr<ITopLevelAccelerationStructure> createTopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->createTopLevelAccelerationStructure("", flags);
        }

        /// <summary>
        /// Creates a top-level acceleration structure.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <returns>The top-level acceleration structure instance.</returns>
        /// <seealso cref="ITopLevelAccelerationStructure" />
        inline UniquePtr<ITopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->getTlas(name, flags);
        }

    private:
        virtual UniquePtr<IBuffer> getBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IBuffer> getBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const = 0;
        virtual UniquePtr<IImage> getTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const = 0;
        virtual UniquePtr<IImage> getTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const = 0;
        virtual Enumerable<UniquePtr<IImage>> getTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 layers, UInt32 levels, MultiSamplingLevel samples, ResourceUsage usage) const = 0;
        virtual UniquePtr<ISampler> getSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual UniquePtr<ISampler> getSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual Enumerable<UniquePtr<ISampler>> getSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual UniquePtr<IBottomLevelAccelerationStructure> getBlas(StringView name, AccelerationStructureFlags flags) const = 0;
        virtual UniquePtr<ITopLevelAccelerationStructure> getTlas(StringView name, AccelerationStructureFlags flags) const = 0;
    };

    /// <summary>
    /// Describes optional features that can be supported by a device.
    /// </summary>
    /// <remarks>
    /// Device features are evaluated when creating a <see cref="IGraphicsDevice" />. If a feature is not supported by the device, an exception is raised.
    /// 
    /// Note that feature support is not strictly enforced by the engine. For example, if you are calling any feature-related API, the call may succeed even if the feature is not enabled, if the GPU 
    /// supports it. Graphics API validation may warn about it and the same program may fail on GPUs that do not support this feature. Enabling a feature through the settings in this structure makes
    /// the device check for support creation, resulting in a clear fail path, if a required extension is not supported by the system hardware.
    /// </remarks>
    struct LITEFX_RENDERING_API GraphicsDeviceFeatures {
    public:
        /// <summary>
        /// Enables or disables mesh shader support.
        /// </summary>
        bool MeshShaders { false };

        /// <summary>
        /// Enables or disables ray-tracing support.
        /// </summary>
        bool RayTracing { false };

        /// <summary>
        /// Enables or disables ray query and inline ray-tracing support.
        /// </summary>
        bool RayQueries { false };
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
        inline [[nodiscard]] UniquePtr<IBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept {
            return this->getNewBarrier(syncBefore, syncAfter);
        }

        /// <summary>
        /// Creates a new frame buffer instance.
        /// </summary>
        /// <param name="renderArea">The initial render area of the frame buffer.</param>
        /// <returns>The instance of the frame buffer.</returns>
        inline [[nodiscard]] UniquePtr<IFrameBuffer> makeFrameBuffer(const Size2d& renderArea) const noexcept {
            return this->makeFrameBuffer("", renderArea);
        }

        /// <summary>
        /// Creates a new frame buffer instance.
        /// </summary>
        /// <param name="name">The name of the frame buffer.</param>
        /// <param name="renderArea">The initial render area of the frame buffer.</param>
        /// <returns>The instance of the frame buffer.</returns>
        inline [[nodiscard]] UniquePtr<IFrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const noexcept {
            return this->getNewFrameBuffer(name, renderArea);
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

        /// <summary>
        /// Computes the required amount of device memory for an <see cref="IBottomLevelAccelerationStructure" />.
        /// </summary>
        /// <remarks>
        /// Acceleration structures are built on the GPU, which requires additional memory called *scratch memory*. When creating an acceleration structure (AS), you have to 
        /// provide a temporary buffer containing the scratch memory, alongside the actual buffer that stores the AS itself. This method can be used to pre-compute the buffer
        /// sizes for both buffers.
        /// 
        /// This method is only supported, if the <see cref="GraphicsDeviceFeatures::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="blas">The bottom-level acceleration structure to compute the memory requirements for.</param>
        /// <param name="bufferSize">The size of the acceleration structure buffer.</param>
        /// <param name="scratchSize">The size of the scratch memory buffer.</param>
        /// <param name="forUpdate">If set to `true`, <paramref name="scratchSize" /> will contain the amount of scratch memory required for an update.</param>
        inline void computeAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const {
            this->getAccelerationStructureSizes(blas, bufferSize, scratchSize, forUpdate);
        }

        /// <summary>
        /// Computes the required amount of device memory for an <see cref="ITopLevelAccelerationStructure" />.
        /// </summary>
        /// <remarks>
        /// Acceleration structures are built on the GPU, which requires additional memory called *scratch memory*. When creating an acceleration structure (AS), you have to 
        /// provide a temporary buffer containing the scratch memory, alongside the actual buffer that stores the AS itself. This method can be used to pre-compute the buffer
        /// sizes for both buffers.
        /// 
        /// This method is only supported, if the <see cref="GraphicsDeviceFeatures::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="tlas">The top-level acceleration structure to compute the memory requirements for.</param>
        /// <param name="bufferSize">The size of the acceleration structure buffer.</param>
        /// <param name="scratchSize">The size of the scratch memory buffer.</param>
        /// <param name="forUpdate">If set to `true`, <paramref name="scratchSize" /> will contain the amount of scratch memory required for an update.</param>
        inline void computeAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const {
            this->getAccelerationStructureSizes(tlas, bufferSize, scratchSize, forUpdate);
        }

    private:
        virtual void getAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const = 0;
        virtual void getAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const = 0;

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
        virtual UniquePtr<IFrameBuffer> getNewFrameBuffer(StringView name, const Size2d& renderArea) const noexcept = 0;
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