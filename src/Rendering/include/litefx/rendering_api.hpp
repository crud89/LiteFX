#pragma once

#include <litefx/config.h>
#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering_export.h>

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
    class VirtualAllocator;

#pragma region "Enumerations"

    // NOLINTBEGIN(performance-enum-size)

    /// @brief Defines different types of graphics adapters.
    enum class GraphicsAdapterType {
        /// @brief The adapter is not a valid graphics adapter.
        None = 0,

        /// @brief The adapter type is not captured by this enum. This value is used internally to mark invalid adapters and should not be used.
        Other = 1,

        /// @brief The adapter is a software driver.
        Software = 2,

        /// @brief The adapter is a GPU integrated into the CPU.
        CPU = 3,
        
        /// @brief The adapter is a dedicated or external GPU.
        GPU = 4,
    };

    /// @brief Provides a preference setting when selecting an adapter.
    enum class GpuPreference {
        /// @brief Returns the first adapter in the list without any preference.
        None = 0,

        /// @brief Returns the preferred adapter for minimum power consumption.
        Power = 1,

        /// @brief Returns the preferred adapter for maximum performance.
        Performance = 2
    };

    /// @brief Represents the type of a @ref CommandQueue.
    ///
    /// There are three major queue types: `Graphics`, `Compute` and `Transfer`. Each queue type has a larger subset of commands it is allowed to execute. For example, a graphics queue can be used to execute
    /// transfer or compute commands, whilst a compute queue may not execute graphics commands (such as *draw*).
    ///
    /// You should always aim to use a queue that is dedicated for the workload you want to submit. For example, if you want to upload resources to the GPU, use the a dedicated transfer queue and synchronize
    /// access to the resource by waiting for the queue to finish at the point of time you need to access the resource. You can, however, also use the graphics queue for transfers. This can be more efficient,
    /// if you have resources that require to be updated with each frame. The performance impact of synchronizing two queues may be larger than simply using the graphics queue to begin with.
    ///
    /// The advantage of using dedicated queues is, that they do not necessarily block execution. For example, when performing a compute or transfer workload on a graphics queue, you do not need to
    /// synchronize in order to wait for the result, however this also means that no rendering can take place until the workloads have finished.
    enum class QueueType : UInt32 {
        /// @brief Describes an unspecified command queue. It is not valid to create a queue instance with this type.
        None = 0x00000000,

        /// @brief Represents a queue that can execute graphics, compute and transfer workloads.
        Graphics = 0x00000001,

        /// @brief Represents a queue that can execute compute and transfer workloads.
        Compute = 0x00000002,

        /// @brief Represents a queue that can execute only transfer workloads.
        Transfer = 0x00000004,

        /// @brief Represents a queue that can perform hardware video decoding.
        ///
        /// Video encoding/decoding is currently not a supported feature, but knowing all the capabilities of a queue is useful to select the best queue family for a particular task.
        VideoDecode = 0x00000010,

        /// @brief Represents a queue that can perform hardware video encoding.
        ///
        /// Video encoding/decoding is currently a supported feature, but knowing all the capabilities of a queue is useful to select the best queue family for a particular task.
        VideoEncode = 0x00000020,

        /// @brief Represents an invalid queue type.
        Other = 0x7FFFFFFF
    };

    /// @brief Specifies the priority with which a queue is scheduled on the GPU.
    enum class QueuePriority {
        /// @brief The default queue priority.
        Normal = 33,

        /// @brief A high queue priority.
        High = 66,

        /// @brief The highest possible queue priority. Submitting work to this queue might block other queues.
        ///
        /// Do not use this queue priority when creating queues, as it is reserved for the default (built-in) queues.
        Realtime = 100
    };

    /// @brief Describes a texel format.
    enum class Format {
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

    /// @brief Describes a buffer attribute format.
    ///
    /// @see getBufferFormatChannels
    /// @see getBufferFormatChannelSize
    /// @see getBufferFormatType
    enum class BufferFormat : UInt32 {
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

    /// @brief Describes the semantic of a buffer attribute.
    ///
    /// @see BufferAttribute
    enum class AttributeSemantic : UInt32 {
        /// @brief The attribute contains a bi-normal vector.
        Binormal = 0x00000001,

        /// @brief The attribute contains blend indices.
        BlendIndices = 0x00000002,

        /// @brief The attribute contains blend weights.
        BlendWeight = 0x00000003,

        /// @brief The attribute contains a color value.
        Color = 0x00000004,

        /// @brief The attribute contains a normal vector.
        Normal = 0x00000005,

        /// @brief The attribute contains a position vector.
        Position = 0x00000006,

        /// @brief The attribute contains a pre-transformed position vector.
        TransformedPosition = 0x00000007,

        /// @brief The attribute contains a point size.
        PointSize = 0x00000008,

        /// @brief The attribute contains a tangent vector.
        Tangent = 0x00000009,

        /// @brief The attribute contains a texture coordinate.
        TextureCoordinate = 0x0000000A,

        /// @brief The attribute contains arbitrary data, that does not have any semantic associated with it.
        Arbitrary = 0x0000000B,

        /// @brief The attribute is a generic, unknown semantic.
        Unknown = 0x7FFFFFFF
    };

    /// @brief Describes the type of a @ref IDescriptor.
    ///
    /// Note that, while in theory you can declare a writable descriptor in any format, the rendering back-end might not necessarily support writing in a specific format.
    ///
    /// @see IDescriptorLayout
    /// @see https://docs.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads#supported-formats-and-api-calls
    enum class DescriptorType {
        /// @brief A uniform buffer in Vulkan. Maps to a constant buffer in DirectX.
        ///
        /// A uniform or constant buffer is read-only. In GLSL, use the `uniform` keyword to access a uniform buffer. In HLSL, use the `ConstantBuffer` keyword.
        ConstantBuffer = 0x00000001,

        /// @brief A shader storage buffer object in Vulkan. Maps to a structured buffer in DirectX.
        ///
        /// A storage buffer is read-only by default. If you want to create a writable storage buffer, use @ref WritableStorage instead.
        ///
        /// In GLSL, use the `buffer` keyword to access storage buffers. In HLSL, use the `StructuredBuffer` keyword.
        ///
        /// The difference between @ref Uniform and storage buffers is, that storage buffers can have variable length. However, they are typically less efficient.
        StructuredBuffer = 0x00000002,

        /// @brief A writable shader storage object in Vulkan. Maps to a read/write structured buffer in DirectX.
        ///
        /// In GLSL, use the `buffer` keyword to access storage buffers. In HLSL, use the `RWStructuredBuffer` keyword.
        ///
        /// When using shader reflection on `AppendStructuredBuffer`/`ConsumeStructuredBuffer`, this descriptor type will be deduced, but support for those constructs is not implemented. Instead, use a separate
        /// @ref RWByteAddressBuffer descriptor and call `InterlockedAdd` on it.
        RWStructuredBuffer = 0x00000012,

        /// @brief A read-only sampled image.
        ///
        /// Textures are read-only by default. If you want to create a writable texture, use the @ref WritableTexture instead.
        ///
        /// In GLSL, use the `uniform texture` keywords to access the texture. In HLSL, use the `Texture` keywords.
        ///
        /// Note, that textures are typically not be accessed directly, but instead are sampled using a @ref Sampler.
        Texture = 0x00000003,

        /// @brief A writable image.
        ///
        /// In GLSL, use the `uniform image` keywords to access the texture. In HLSL, use the `RWTexture` keywords.
        RWTexture = 0x00000013,
        
        /// @brief A sampler state of a texture or image.
        Sampler = 0x00000004,

        /// @brief The result of a render target from an earlier render pass. Maps to a `SubpassInput` in HLSL.
        InputAttachment = 0x00000005,

        /// @brief Represents a read-only texel buffer (uniform texel buffer).
        ///
        /// Use the `uniform imageBuffer` keyword in GLSL to access the buffer. In HLSL, use the `Buffer` keyword.
        Buffer = 0x00000006,

        /// @brief Represents a writable texel buffer (storage texel buffer).
        ///
        /// Use the `uniform imageBuffer` keyword in GLSL to access the buffer. In HLSL, use the `RWBuffer` keyword.
        RWBuffer = 0x00000016,

        /// @brief Represents an unformatted buffer.
        ///
        /// In GLSL, use the `buffer` keyword to access byte address buffers. In HLSL, use the `ByteAddressBuffer` keyword.
        ByteAddressBuffer = 0x00000007,

        /// @brief Represents an unformatted writable buffer.
        ///
        /// In GLSL, use the `buffer` keyword to access byte address buffers. In HLSL, use the `RWByteAddressBuffer` keyword.
        RWByteAddressBuffer = 0x00000017,

        /// @brief Represents a ray-tracing acceleration structure.
        AccelerationStructure = 0x00000008,

        /// @brief Special descriptor type, that can bind all resources besides constant buffers, acceleration structures and samplers, which then can be directly indexed from the global resource heap.
        ///
        /// This descriptor type does not directly map to an underlying resource type and instead denotes a descriptor binding, that accepts any resource descriptor besides constant buffers and acceleration
        /// structures. Samplers are also disallowed, as they need to be bound to a descriptor of the @ref GlobalSamplerHeap type. The existence of such a descriptor as a part of a descriptor set, indicates that
        /// the pipeline layout uses direct descriptor indexing (see @ref IPipelineLayout::directlyIndexResources). A descriptor set containing a descriptor of this type does not allocate any space on the
        /// respective global descriptor heap. Instead, it acts as a proxy set, that binds any of the aforementioned buffers. When binding to this descriptor, a single uncached descriptor address will be
        /// allocated for the resource on the global descriptor heap. On the shader side, this descriptor can be retrieved by calling the @ref IDescriptorSet::bindToHeap method.
        ///
        /// As there's no underlying descriptor set or binding when using this descriptor type, you should only ever have on descriptor of this type in a @ref IPipelineLayout. In the DirectX 12 backend, it is
        /// sufficient to have a descriptor of this type in a pipeline to access any descriptor on the global resource heap. In Vulkan, this descriptor type creates a descriptor set containing an unbounded
        /// runtime array to emulate this behavior. This array is special, as it uses the `VK_EXT_mutable_descriptor_type` extension to bind arbitrary resources to a descriptor. However, this is only allowed for
        /// descriptors created this way, so indexing only works within the range of the proxy descriptor set. It is therefore good practice not to use indices obtained outside a binding created from the proxy
        /// descriptor sets. Furthermore, the use of mutable descriptor types is considered less efficient than the traditional binding procedure, as it might prevent certain fast paths. For this reason, consider
        /// alternative approaches, like multiple unbounded descriptor arrays first. Directly indexing into the global descriptor heap this way can be beneficial, if it allows you to re-use the same pipeline
        /// state where you would otherwise have to switch between multiple states, however, especially in combination with indirect drawing.
        ///
        /// @see DescriptorType::GlobalSamplerHeap
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        ResourceDescriptorHeap = 0x00000009,

        /// @brief A special descriptor type that allows indexed access to the a portion of the global sampler heap.
        ///
        /// This descriptor type is equivalent to @ref DescriptorType::GlobalResourceHeap, except that it enables access to the global sampler heap instead. The same conceptual design as for the resource heap
        /// applies here, with the same limitations, listed in the remarks for the `GlobalResourceHeap` descriptor type.
        ///
        /// @see DescriptorType::GlobalResourceHeap
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        SamplerDescriptorHeap = 0x0000000A
    };

    /// @brief The target heap type for a descriptor.
    enum class DescriptorHeapType {
        /// @brief Indicates an invalid heap.
        None = 0x00,

        /// @brief Binds all non-sampler resource views.
        Resource = 0x01,

        /// @brief Binds all sampler states.
        Sampler = 0x02
    };

    /// @brief Describes the type of a @ref IBuffer.
    ///
    /// @see IBufferLayout
    enum class BufferType {
        /// @brief Describes a vertex buffer.
        Vertex = 0x00000001,
        
        /// @brief Describes an index buffer.
        Index = 0x00000002,

        /// @brief Describes an uniform buffer object (Vulkan) or constant buffer view (DirectX).
        ///
        /// Buffers of this type can be bound to `ConstantBuffer` descriptors.
        Uniform = 0x00000003,

        /// @brief Describes a shader storage buffer object (Vulkan) or unordered access view (DirectX).
        ///
        /// Buffers of this type can be bound to `StructuredBuffer`/`RWStructuredBuffer` or `ByteAddressBuffer`/`RWByteAddressBuffer` descriptors.
        Storage = 0x00000004,

        /// @brief Describes a shader texel storage buffer object (Vulkan) or unordered access view (DirectX).
        ///
        /// Buffers of this type can be bound to `Buffer`/`RWBuffer` descriptors.
        Texel = 0x00000005,

        /// @brief Describes an acceleration structure buffer.
        ///
        /// @see ICommandBuffer::buildAccelerationStructure
        /// @see IBottomLevelAccelerationStructure
        /// @see ITopLevelAccelerationStructure
        AccelerationStructure = 0x00000006,

        /// @brief Describes a shader binding table for ray-tracing.
        ShaderBindingTable = 0x00000007,

        /// @brief Describes a buffer that stores data to generate indirect draw calls.
        ///
        /// An indirect buffer refers to a buffer that contains a set of information used to generate dispatch or draw calls. A single dispatch or draw call in this context is referred to as a *batch*. An
        /// indirect buffer must only contain one type of batches, e.g., it is not allowed to mix indexed and non-indexed batches in a single indirect buffer. Batches need to be densely packed within the buffer,
        /// i.e., the stride between elements is always assumed to be `0`.
        ///
        /// Indirect buffers can be written from shaders, which enables use cases like GPU-culling, where a compute shader writes the batches in an indirect buffer, that is then passed to an indirect draw call.
        /// In such situations, the number of batches in the buffer is typically not known beforehand, so an additional buffer is used to store the number of draw calls in.
        ///
        /// Note that indirect drawing support is currently limited in how data can be passed to draw calls. This is due to Vulkan not providing an adequate interface for describing per-draw bindings in the
        /// indirect signature. In DirectX 12, it is possible for batches to provide different vertex and index buffers, as well as resource bindings for each draw call. Vulkan does only support draw calls that
        /// target already bound descriptors. Due to this limitation, it is currently best practice to use bind-less descriptor arrays to pass per-draw data to draws and use a vertex attribute to index into the
        /// descriptor array.
        Indirect = 0x00000008,

        /// @brief Describes another type of buffer, such as samplers or images.
        ///
        /// Buffers of this type must not be bound to any descriptor, but can be used as copy/transfer targets and sources.
        Other = 0x7FFFFFFF
    };

    /// @brief Defines where a resource (buffer or image) memory is located and from where it can be accessed.
    ///
    /// There are three common memory usage scenarios that are supported by the engine:
    ///
    /// 1. **Static resources**: such as vertex/index/constant buffers, textures or other infrequently updated buffers. In this case, the most efficient approach is to create a buffer using @ref ResourceHeap::Staging and map it from the CPU. Create a second buffer using @ref ResourceHeap::Resource and transfer the staging buffer into it.
    /// 2. **Dynamic resources**: such as deformable meshes or buffers that need to be updated every frame. For such buffers use the @ref ResourceHeap::Dynamic mode to prevent regular transfer overhead.
    /// 3. **Readbacks**: or resources that are written on the GPU and read by the CPU. The usage mode @ref ResourceHeap::Readback is designed to provide the best performance for this special case.
    ///
    /// Note that image resources cannot be created on heaps different to @ref ResourceHeap::Resource. For this very reason, the graphics factory does not allow to specify the resource heap when creating
    /// images or attachments.
    ///
    /// @see IGraphicsFactory
    /// @see IBuffer
    enum class ResourceHeap {
        /// @brief Creates a resource that can be mapped from the CPU in order to be transferred to the GPU later.
        ///
        /// The memory for the resource will be allocated in the DRAM (CPU or host memory). It can be optimally accessed by the CPU in order to be written. However, reading it from the GPU is not supported. This
        /// usage mode should be used to create a staging buffer, i.e. a buffer that is written infrequently and then transferred to another buffer, that uses @ref ResourceHeap::Resource.
        Staging = 0x00000001,

        /// @brief Creates a resource that can be read by the GPU.
        ///
        /// The memory for the resource will be allocated on the VRAM (GPU or device memory). It can be optimally accessed by the GPU in order to be read frequently. It can be written by a transfer call, but is
        /// inaccessible from the CPU.
        Resource = 0x00000002,

        /// @brief Creates a resource that can be mapped from the CPU and read by the GPU.
        ///
        /// Dynamic buffers are used when the content is expected to be changed every frame. They do not require transfer calls, but may not be read as efficiently as @ref ResourceHeap::Resource buffers.
        Dynamic = 0x00000010,

        /// @brief Creates a buffer that can be written on the GPU and read by the CPU.
        Readback = 0x00000100,

        /// @brief Creates a buffer that is directly allocated in GPU memory, but that can be efficiently written from the CPU.
        ///
        /// This heap uses the resizable base address register (ReBAR) of the GPU to create the buffer. However, this is only possible, if the GPU supports it. To check support for it, you can query @ref
        /// IGraphicsFactory::supportsResizableBaseAddressRegister. In case this feature is not supported, you may want to fall back to a @ref Dynamic resource.
        GPUUpload = 0x00001000
    };

    /// @brief Describes the intended usage for a resource.
    ///
    /// @see IGraphicsFactory
    /// @see IBuffer
    /// @see IImage
    enum class ResourceUsage {
        /// @brief The resource is created without any special usage settings.
        None = 0x0000,

        /// @brief Allows the resource to be written to.
        ///
        /// This flag is not allowed for vertex buffers (@ref BufferType::Vertex), index buffers (@ref BufferType::Index) and uniform buffers (@ref BufferType::Uniform).
        ///
        /// @see IDeviceMemory::writable
        AllowWrite = 0x0001,

        /// @brief Allows the resource data to be copied into another resource.
        ///
        /// This flag is implicitly set for resources created with @ref ResourceHeap::Staging and for render target images (attachments).
        TransferSource = 0x0010,

        /// @brief Allows the resource data to be copied from another resource.
        ///
        /// This flag is implicitly set for resources created with @ref ResourceHeap::Readback and for render target images (attachments).
        TransferDestination = 0x0020,

        /// @brief Allows the resource to be used as a render target.
        RenderTarget = 0x0040,

        /// @brief Allows the resource to be used to build acceleration structures.
        ///
        /// This flag is not allowed for images and other acceleration structures (@ref BufferType::AccelerationStructure).
        ///
        /// @see IAccelerationStructure
        AccelerationStructureBuildInput = 0x0100,

        /// @brief Shortcut for commonly used `TransferSource | TransferDestination` combination.
        Default = TransferSource | TransferDestination,

        /// @brief Default usage for frame buffer images.
        ///
        /// @see IFrameBuffer
        FrameBufferImage = TransferSource | RenderTarget,

        /// @brief Causes the contents of the resource to not be copied during defragmentation.
        ///
        /// @see IGraphicsFactory::beginDefragmentation
        /// @see IDeviceMemory::volatileMove
        /// @see IDeviceMemory::moving
        /// @see IDeviceMemory::moved
        Volatile = 0x1000
    };

    /// @brief Controls the allocation behavior of @ref IGraphicsFactory.
    enum class AllocationBehavior : UInt32 {
        /// @brief Represents the default behavior, which might fall back to slower memory types, if required.
        Default = 0x00,

        /// @brief Stays within heap budgets. If the desired resource heap is out of memory, allocation will fail. Use this behavior for resources that are not required to prevent them from being allocated in
        /// potentially slower memory heaps.
        StayWithinBudget = 0x01,

        /// @brief Does not resize heap cache, if no more pre-allocated memory is available and will fail, if available memory is exceeded. Use this in situations, where you can potentially delay an allocation to
        /// a less time-critical point.
        DontExpandCache = 0x02
    };

    /// @brief The allocation algorithm used by @ref VirtualAllocators.
    enum class AllocationAlgorithm : UInt32 {
        /// @brief The default algorithm without any constraints on the memory layout.
        Default = 0x01,

        /// @brief A linear allocation algorithm, that allocates memory blocks sequentially.
        ///
        /// @see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/custom_memory_pools.html#linear_algorithm
        /// @see https://gpuopen-librariesandsdks.github.io/D3D12MemoryAllocator/html/linear_algorithm.html
        Linear = 0x02
    };

    /// @brief The allocation strategy used by allocators (@ref IGraphicsFactory and @ref VirtualAllocator) when allocating new chunks of memory.
    enum class AllocationStrategy : UInt32 {
        /// @brief Prefers good packing over allocation time and reduces fragmentation.
        OptimizePacking = 0x01,

        /// @brief Prefers allocation time over packing.
        OptimizeTime = 0x02,
    };

    /// @brief The strategy to apply to a defragmentation pass.
    ///
    /// @see IGraphicsFactory::beginDefragmentation
    /// @see IGraphicsFactory::defragment
    enum class DefragmentationStrategy : UInt32 {
        /// @brief Provides fast fragment computation, but potentially suboptimal packing.
        Fast = 0x01,

        /// @brief Provides a balance between fragment computation time and packing efficiency.
        Balanced = 0x02,

        /// @brief Provides optimal packing at the cost of potentially more copies.
        Full = 0x03
    };

    /// @brief Describes the element type of an index buffer.
    enum class IndexType : UInt32 {
        /// @brief Indices are stored as 2 byte unsigned integers.
        UInt16 = 0x00000010,

        /// @brief Indices are stored as 4 byte unsigned integers.
        UInt32 = 0x00000020
    };

    /// @brief The rate at which a vertex buffer of a certain @ref IVertexBufferLayout is made available for vertex shaders.
    ///
    /// @see IVertexBufferLayout
    enum class VertexBufferInputRate : UInt32 {
        /// @brief The vertex buffer layout describes data that is made available per individual vertex.
        Vertex = 0,

        /// @brief The vertex buffer layout describes data that is made available per instance.
        Instance = 1
    };

    /// @brief Describes the topology of a mesh primitive.
    enum class PrimitiveTopology {
        /// @brief A list of points where each vertex refers to an individual point.
        PointList = 0x00010001,

        /// @brief A list of lines where each vertex pair refers to the start and end points of a line.
        ///
        /// @see LineStrip
        LineList = 0x00020001,

        /// @brief A list of lines where each vertex pair refers to the start and end points of a line and where vertices also store adjacency data.
        ///
        /// @see LineList
        LineListWithAdjacency = 0x00020011,

        /// @brief A list of triangles, where each triplet of vertices refers to a whole triangle.
        ///
        /// @see TriangleStrip
        TriangleList = 0x00040001,

        /// @brief A list of triangles, where each triplet of vertices refers to a whole triangle and where vertices also store adjacency data.
        ///
        /// @see TriangleList
        TriangleListWithAdjacency = 0x00040011,

        /// @brief A strip of lines where each vertex (except the first one) refers to the end point for the next line segment.
        ///
        /// @see LineList
        LineStrip = 0x00020002,

        /// @brief A strip of lines where each vertex (except the first one) refers to the end point for the next line segment.
        ///
        /// @see LineStrip
        LineStripWithAdjacency = 0x00020012,
        
        /// @brief A strip of triangles, where each vertex (except the first two) refers to the third vertex of the next triangle segment and where vertices also store adjacency data..
        ///
        /// @see TriangleList
        TriangleStrip = 0x00040002,

        /// @brief A strip of triangles, where each vertex (except the first two) refers to the third vertex of the next triangle segment and where vertices also store adjacency data..
        ///
        /// @see TriangleStrip
        TriangleStripWithAdjacency = 0x00040012,

        /// @brief A list of patches used in combination with tessellation.
        PatchList = 0x00080001
    };

    /// @brief Describes the valid shader stages of a graphics pipeline.
    enum class ShaderStage : UInt32 {
        /// @brief Represents the vertex shader stage.
        Vertex = 0x00000001,

        /// @brief Represents the tessellation control or hull shader stage.
        TessellationControl = 0x00000002,

        /// @brief Represents the tessellation evaluation or domain shader stage.
        TessellationEvaluation = 0x00000004,

        /// @brief Represents the geometry shader stage.
        ///
        /// Note that geometry shaders come with a performance penalty and might not be supported on all platforms. If you can, avoid using them.
        Geometry = 0x00000008,

        /// @brief Represents the fragment or pixel shader stage.
        Fragment = 0x00000010,

        /// @brief Represents the compute shader stage.
        Compute = 0x00000020,

        /// @brief Represents the task or amplification shader stage.
        Task = 0x00000040,

        /// @brief Represents the mesh shader stage.
        Mesh = 0x00000080,

        /// @brief Represents the ray generation shader stage.
        RayGeneration = 0x00000100,

        /// @brief Represents the any-hit shader stage.
        AnyHit = 0x00000200,

        /// @brief Represents the closest-hit shader stage.
        ClosestHit = 0x00000400,

        /// @brief Represents the miss shader stage.
        Miss = 0x00000800,

        /// @brief Represents the intersection shader stage.
        Intersection = 0x00001000,

        /// @brief Represents the callable shader stage.
        Callable = 0x00002000,

        /// @brief Represents the complete rasterization pipeline.
        RasterizationPipeline = Vertex | Geometry | TessellationControl | TessellationEvaluation | Fragment,

        /// @brief Represents the complete mesh shading pipeline.
        MeshPipeline = Task | Mesh | Fragment,

        /// @brief Represents the complete ray-tracing pipeline.
        RayTracingPipeline = RayGeneration | AnyHit | ClosestHit | Miss | Intersection | Callable,

        /// @brief Enables all supported shader stages.
        Any = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment | Compute | Task | Mesh | RayGeneration | AnyHit | ClosestHit | Miss | Intersection | Callable,

        /// @brief Represents an unknown shader stage.
        Other = 0x7FFFFFFF
    };

    /// @brief Describes the type of a shader module record within a shader collection or shader binting table.
    ///
    /// @see IShaderRecord
    enum class ShaderRecordType {
        /// @brief Represents a ray generation shader record.
        RayGeneration = 0x01,

        /// @brief Represents a hit group shader record.
        HitGroup = 0x02,

        /// @brief Represents an intersection shader record.
        Intersection = 0x03,

        /// @brief Represents a miss shader record.
        Miss = 0x04,

        /// @brief Represents a callable shader record.
        Callable = 0x05,

        /// @brief Represents a shader record that contains a module of an unsupported shader stage.
        Invalid = 0x7FFFFFFF
    };

    /// @brief Describes a group or combination of groups of a shader binding table.
    ///
    /// @see IRayTracingPipeline::allocateShaderBindingTable
    enum class ShaderBindingGroup : UInt32 {
        /// @brief Refers to the group of the shader binding table that stores the ray generation shader.
        RayGeneration = 0x01,

        /// @brief Refers to the group of the shader binding table that stores the geometry hit shaders.
        HitGroup = 0x02,

        /// @brief Refers to the group of the shader binding table that stores the miss shaders.
        Miss = 0x04,

        /// @brief Refers to the group of the shader binding table that stores the callable shaders.
        Callable = 0x08,

        /// @brief Refers to a combination of all possible groups that can be stored in a shader binding table.
        All = RayGeneration | HitGroup | Miss | Callable
    };

    /// @brief Describes the draw mode for polygons.
    ///
    /// @see InputAssembler
    enum class PolygonMode {
        /// @brief Polygons are drawn as solid surfaces.
        Solid = 0x00000001,

        /// @brief Polygons are only drawn as wire-frames.
        Wireframe = 0x00000002,

        /// @brief Polygons are drawn as points at the vertex positions.
        Point = 0x00000004
    };

    /// @brief Describes which faces are culled by the @ref Rasterizer stage.
    ///
    /// @see Rasterizer
    enum class CullMode {
        /// @brief The rasterizer will discard front-facing polygons.
        FrontFaces = 0x00000001,

        /// @brief The rasterizer will discard back-facing polygons.
        BackFaces = 0x00000002,

        /// @brief The rasterizer will discard front and back-facing polygons.
        Both = 0x00000004,

        /// @brief The rasterizer will not discard any polygons.
        Disabled = 0x0000000F
    };

    /// @brief Describes the order or vertex winding, that is used to determine, whether a polygon is facing towards or away from the camera.
    ///
    /// @see CullMode
    /// @see Rasterizer
    enum class CullOrder {
        /// @brief Vertices are evaluated in a clock-wise manner.
        ClockWise = 0x00000001,

        /// @brief Vertices are evaluated in a counter clock-wise manner.
        CounterClockWise = 0x00000002
    };

    /// @brief Describes the type of a render target.
    enum class RenderTargetType {
        /// @brief Represents a color target.
        Color = 0x00000001,

        /// @brief Represents a depth/stencil target.
        DepthStencil = 0x00000002,

        /// @brief Represents a color target that should be presented.
        ///
        /// This is similar to @ref RenderTargetType::Color, but is used to optimize the memory layout of the target for it to be pushed to a swap chain.
        Present = 0x00000004
    };

    /// @brief Describes the behavior of render targets.
    enum class RenderTargetFlags {
        /// @brief No flags are enabled.
        None = 0x00,

        /// @brief If enabled, color or depth (depending on the render target type) are cleared when starting a render pass that renders to the render target.
        Clear = 0x01,

        /// @brief If enabled and the render target format supports stencil storage, the stencil part is cleared when the render pass that renders to the render target is started.
        ClearStencil = 0x02,

        /// @brief If enabled, the render target is discarded after ending the render pass.
        ///
        /// When this flag is set, the render target storage is freed after the render pass has finished. The main use of this is to have depth/stencil targets on a render pass that are only required during this
        /// render pass. It is not valid to attempt accessing the render target before or after the render pass.
        Volatile = 0x04
    };

    /// @brief Describes the dimensions of a image resource, i.e. the dimensions that are required to access a texel or describe the image extent.
    ///
    /// @see IImage
    enum class ImageDimensions {
        /// @brief Represents a 1D image.
        DIM_1 = 0x01,

        /// @brief Represents a 2D image.
        DIM_2 = 0x02,

        /// @brief Represents a 3D image.
        DIM_3 = 0x03,

        /// @brief Represents a set of six 2D images that are used to build a cube map.
        CUBE = 0x04
    };

    /// @brief Describes the number of samples with which a @ref IImage is sampled.
    enum class MultiSamplingLevel : UInt32 {
        /// @brief The default number of samples. Multi-sampling will be deactivated, if this sampling level is used.
        x1 = 0x00000001,

        /// @brief Use 2 samples per pixel.
        x2 = 0x00000002,

        /// @brief Use 4 samples per pixel.
        x4 = 0x00000004,

        /// @brief Use 8 samples per pixel.
        x8 = 0x00000008,

        /// @brief Use 16 samples per pixel.
        x16 = 0x00000010,

        /// @brief Use 32 samples per pixel.
        x32 = 0x00000020,

        /// @brief Use 64 samples per pixel.
        x64 = 0x00000040
    };

    /// @brief Describes the filter operation when accessing a pixel from a texture coordinate.
    ///
    /// @see IImage
    enum class FilterMode {
        /// @brief Take the nearest texel with respect to the texture coordinate.
        Nearest = 0x00000001,

        /// @brief Linearly interpolate between the two closest texels with respect to the texture coordinate.
        Linear = 0x00000002
    };

    /// @brief Describes the filter operation between two mip-map levels.
    ///
    /// @see IImage
    /// @see FilterMode
    enum class MipMapMode {
        /// @brief Take the texel from the mip-map level that is closest to the actual depth.
        Nearest = 0x00000001,

        /// @brief Linearly interpolate between the texels of the two neighboring mip-map levels.
        Linear = 0x00000002
    };

    /// @brief Describes how to treat texture coordinates that are outside the domain `[0..1]`.
    enum class BorderMode {
        /// @brief Repeat the texture.
        Repeat = 0x00000001,

        /// @brief Mirror the texture.
        RepeatMirrored = 0x00010001,

        /// @brief Take the closest edge texel.
        ClampToEdge = 0x00000002,

        /// @brief Take the closest edge texel from the opposite site.
        ClampToEdgeMirrored = 0x00010002,

        /// @brief Return a pre-specified border color.
        ClampToBorder = 0x00000003,
    };

    /// @brief Describes the operation used to compare depth or stencil values during depth/stencil tests.
    ///
    /// @see DepthStencilState
    enum class CompareOperation {
        /// @brief The test always fails.
        Never = 0x00000000,

        /// @brief The test succeeds, if the current value is less than the stencil ref or previous depth value.
        Less = 0x00000001,

        /// @brief The test succeeds, if the current value is greater than the stencil ref or previous depth value.
        Greater = 0x0000002,

        /// @brief The test succeeds, if the current value is equal to the stencil ref or previous depth value.
        Equal = 0x00000003,

        /// @brief The test succeeds, if the current value is less or equal to the stencil ref or previous depth value.
        LessEqual = 0x00000004,

        /// @brief The test succeeds, if the current value is greater or equal to the stencil ref or previous depth value.
        GreaterEqual = 0x00000005,

        /// @brief The test succeeds, if the current value is not equal to the stencil ref or previous depth value.
        NotEqual = 0x00000006,

        /// @brief The test always succeeds.
        Always = 0x00000007
    };

    /// @brief An operation that is applied to the stencil buffer.
    ///
    /// @see DepthStencilState
    enum class StencilOperation {
        /// @brief Keep the current stencil value.
        Keep = 0x00000000,
        
        /// @brief Set the stencil value to `0`.
        Zero = 0x00000001,

        /// @brief Replace the current stencil value with the stencil ref.
        Replace = 0x00000002,

        /// @brief Increment the current stencil value.
        IncrementClamp = 0x00000003,

        /// @brief Decrement the current stencil value.
        DecrementClamp = 0x00000004,

        /// @brief Bitwise invert the current stencil value.
        Invert = 0x00000005,

        /// @brief Increment the current stencil value and wrap it, if it goes out of bounds.
        IncrementWrap = 0x00000006,

        /// @brief Decrement the current stencil value and wrap it, if it goes out of bounds.
        DecrementWrap = 0x00000007
    };

    /// @brief Specifies a blend factor.
    ///
    /// @see DepthStencilState
    enum class BlendFactor {
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

    /// @brief Specifies a write mask for a color buffer.
    ///
    /// @see RenderTargetType
    enum class WriteMask {
        /// @brief Write into the red channel.
        R = 0x01,

        /// @brief Write into the green channel.
        G = 0x02,

        /// @brief Write into the blue channel.
        B = 0x04,

        /// @brief Write into the alpha channel.
        A = 0x08
    };

    /// @brief Specifies a blend operation.
    ///
    /// @see DepthStencilState
    enum class BlendOperation {
        Add = 0x01,
        Subtract = 0x02,
        ReverseSubtract = 0x03,
        Minimum = 0x04,
        Maximum = 0x05
    };

    /// @brief Defines pipeline stages as points where synchronization may occur.
    ///
    /// @see IBarrier
    /// @see ResourceAccess
    /// @see ImageLayout
    enum class PipelineStage {
        /// @brief Represents no-blocking behavior.
        ///
        /// Translates to `VK_PIPELINE_STAGE_NONE` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_NONE` in DirectX 12 ❎.
        ///
        /// This stage flag is special, as it cannot be combined with other stage flags.
        None = 0x00000000,

        /// @brief Waits for all previous commands to be finished, or blocks all following commands until the barrier is executed.
        ///
        /// Translates to `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_ALL` in DirectX 12 ❎.
        ///
        /// This stage flag is special, as it cannot be combined with other stage flags.
        All = 0x00000001,

        /// @brief Waits for previous commands to finish all graphics stages, or blocks following commands until the graphics stages has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_DRAW` in DirectX 12 ❎.
        ///
        /// This stage flag is special, as it cannot be combined with other stage flags.
        Draw = 0x00000002,

        /// @brief Waits for previous commands to finish the input assembly stage, or blocks following commands until the input assembly stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_VERTEX_INPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_INDEX_INPUT` in DirectX 12 ❎.
        InputAssembly = 0x00000004,

        /// @brief Waits for previous commands to finish the vertex shader stage, or blocks following commands until the vertex shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_VERTEX_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        Vertex = 0x00000006,

        /// @brief Waits for previous commands to finish the tessellation control/hull shader stage, or blocks following commands until the tessellation control/hull shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        TessellationControl = 0x00000008,

        /// @brief Waits for previous commands to finish the tessellation evaluation/domain shader stage, or blocks following commands until the tessellation evaluation/domain shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        TessellationEvaluation = 0x00000010,

        /// @brief Waits for previous commands to finish the geometry shader stage, or blocks following commands until the geometry shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_VERTEX_SHADING` in DirectX 12 ❎.
        Geometry = 0x00000020,

        /// @brief Waits for previous commands to finish the fragment/pixel shader stage, or blocks following commands until the fragment/pixel shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_PIXEL_SHADING` in DirectX 12 ❎.
        Fragment = 0x00000040,

        /// @brief Waits for previous commands to finish the depth/stencil stage, or blocks following commands until the depth/stencil stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_DEPTH_STENCIL` in DirectX 12 ❎.
        DepthStencil = 0x00000080,

        /// @brief Waits for previous commands to finish the draw indirect stage, or blocks following commands until the draw indirect stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_EXECUTE_INDIRECT` in DirectX 12 ❎.
        Indirect = 0x00000100,

        /// @brief Waits for previous commands to finish the output merger stage, or blocks following commands until the output merger stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RENDER_TARGET` in DirectX 12 ❎.
        RenderTarget = 0x00000200,

        /// @brief Waits for previous commands to finish the compute shader stage, or blocks following commands until the compute shader stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_COMPUTE_SHADING` in DirectX 12 ❎.
        ///
        /// This stage flag is special, as it cannot be combined with other stage flags.
        Compute = 0x00000400,

        /// @brief Waits for previous commands to finish the transfer stage, or blocks following commands until the transfer stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_TRANSFER_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_COPY` in DirectX 12 ❎.
        Transfer = 0x00000800,

        /// @brief Waits for previous commands to finish the multi-sampling resolution stage, or blocks following commands until the multi-sampling resolution stage has finished.
        ///
        /// Translates to `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RESOLVE` in DirectX 12 ❎.
        Resolve = 0x00001000,

        /// @brief Waits for previous commands to finish the building stage for an acceleration structure, or blocks the following commands until the building has finished.
        ///
        /// This flag is only supported, if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR` in Vulkan 🌋 and
        /// `D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE` in DirectX 12 ❎.
        ///
        /// @see IAccelerationStructure
        AccelerationStructureBuild = 0x00010000,

        /// @brief Waits for previous commands to finish the copying stage for an acceleration structure, or blocks the following commands until the copying has finished.
        ///
        /// This flag is only supported, if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR` in Vulkan 🌋 and
        /// `D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE` in DirectX 12 ❎.
        ///
        /// @see IAccelerationStructure
        AccelerationStructureCopy = 0x00020000,

        /// @brief Waits for the previous commands to finish ray-tracing shader stages, or blocks the following commands until ray-tracing has finished.
        ///
        /// This flag is only supported if ray-tracing support is enabled. It translates to `VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR` in Vulkan 🌋 and `D3D12_BARRIER_SYNC_RAYTRACING` in DirectX 12 ❎.
        Raytracing = 0x00040000,
    };

    /// @brief Defines how a @ref IBuffer or @ref IImage resource is accessed.
    ///
    /// @see IBarrier
    /// @see IImage
    /// @see IBuffer
    /// @see PipelineStage
    /// @see ImageLayout
    enum class ResourceAccess {
        /// @brief Indicates that a resource is not accessed.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_NO_ACCESS` in the DirectX 12 ❎ backend and `VK_ACCESS_NONE` in the Vulkan 🌋 backend.
        ///
        /// This access flag is special, as it cannot be combined with other access flags.
        None = 0x7FFFFFFF,

        /// @brief Indicates that a resource is accessed as a vertex buffer.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_VERTEX_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT` in the Vulkan 🌋 backend.
        VertexBuffer = 0x00000001,

        /// @brief Indicates that a resource is accessed as an index buffer.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_INDEX_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_INDEX_READ_BIT` in the Vulkan 🌋 backend.
        IndexBuffer = 0x00000002,

        /// @brief Indicates that a resource is accessed as an uniform/constant buffer.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_CONSTANT_BUFFER` in the DirectX 12 ❎ backend and `VK_ACCESS_UNIFORM_READ_BIT` in the Vulkan 🌋 backend.
        UniformBuffer = 0x00000004,

        /// @brief Indicates that a resource is accessed as a render target.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RENDER_TARGET` in the DirectX 12 ❎ backend and `VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT` in the Vulkan 🌋
        /// backend.
        RenderTarget = 0x00000008,

        /// @brief Indicates that a resource is accessed as to read depth/stencil values.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ` in the DirectX 12 ❎ backend and `VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT` in the Vulkan 🌋 backend.
        DepthStencilRead = 0x00000010,

        /// @brief Indicates that a resource is accessed as to write depth/stencil values.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE` in the DirectX 12 ❎ backend and `VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT` in the Vulkan 🌋 backend.
        DepthStencilWrite = 0x00000020,

        /// @brief Indicates that a resource is accessed as a read-only shader resource.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_SHADER_RESOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_SHADER_READ_BIT` in the Vulkan 🌋 backend.
        ShaderRead = 0x00000040,

        /// @brief Indicates that a resource is accessed as a read-write shader resource.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_UNORDERED_ACCESS` in the DirectX 12 ❎ backend and `VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT` in the Vulkan 🌋 backend.
        ShaderReadWrite = 0x00000080,

        /// @brief Indicates that a resource is accessed as to read indirect draw commands.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT` in the DirectX 12 ❎ backend and `VK_ACCESS_INDIRECT_COMMAND_READ_BIT` in the Vulkan 🌋 backend.
        Indirect = 0x00000100,

        /// @brief Indicates that a resource is accessed as to read during a transfer operation.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COPY_SOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_TRANSFER_READ_BIT` in the Vulkan 🌋 backend.
        TransferRead = 0x00000200,

        /// @brief Indicates that a resource is accessed as to write during a transfer operation.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COPY_DEST` in the DirectX 12 ❎ backend and `VK_ACCESS_TRANSFER_WRITE_BIT` in the Vulkan 🌋 backend.
        TransferWrite = 0x00000400,

        /// @brief Indicates that a resource is accessed as to read during a resolve operation.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RESOLVE_SOURCE` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_READ_BIT` in the Vulkan 🌋 backend.
        ResolveRead = 0x00000800,

        /// @brief Indicates that a resource is accessed as to write during a resolve operation.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_RESOLVE_DEST` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_WRITE_BIT` in the Vulkan 🌋 backend.
        ResolveWrite = 0x00001000,
        
        /// @brief Indicates that a resource can be accessed in any way, compatible to the layout.
        ///
        /// Note that you have to ensure that you do not access the resource in an incompatible way manually.
        ///
        /// This access mode translates to `D3D12_BARRIER_ACCESS_COMMON` in the DirectX 12 ❎ backend and `VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT` in the Vulkan 🌋 backend.
        Common = 0x00002000,

        /// @brief Indicates that a resources is accessed to read an acceleration structure.
        ///
        /// This access mode flags is only supported if ray-tracing support is enabled. It translates `D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ` in the DirectX 12 ❎ backend and
        /// `VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR` in the Vulkan 🌋 backend.
        AccelerationStructureRead = 0x00010000,

        /// @brief Indicates that a resources is accessed to write an acceleration structure.
        ///
        /// This access mode flags is only supported if ray-tracing support is enabled. It translates `D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE` in the DirectX 12 ❎ backend and
        /// `VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR` in the Vulkan 🌋 backend.
        AccelerationStructureWrite = 0x00020000,
    };

    /// @brief Specifies the layout of an @ref IImage resource.
    ///
    /// @see IImage
    /// @see IBarrier
    /// @see ResourceAccess
    /// @see PipelineStage
    enum class ImageLayout {
        /// @brief A common image layout that allows for all types of access (shader resource, transfer destination, transfer source).
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COMMON` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_GENERAL` in the Vulkan 🌋 backend.
        Common = 0x00000001,

        /// @brief Indicates that the image is used as a read-only storage or texel buffer.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_SHADER_RESOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` in the Vulkan 🌋 backend.
        ShaderResource = 0x00000002,

        /// @brief Indicates that the image is used as a read-write storage or texel buffer.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_GENERAL` in the Vulkan 🌋 backend.
        ReadWrite = 0x00000003,

        /// @brief Allows the image to be used as a source for transfer operations.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COPY_SOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL` in the Vulkan 🌋 backend.
        CopySource = 0x00000010,

        /// @brief Allows the image to be used as a destination for transfer operations.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_COPY_DEST` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL` in the Vulkan 🌋 backend.
        CopyDestination = 0x00000011,

        /// @brief Indicates that the image is used as a render target.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RENDER_TARGET` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        RenderTarget = 0x00000020,

        /// @brief Indicates that image is used as a read-only depth/stencil target.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL` in the Vulkan 🌋 backend.
        DepthRead = 0x00000021,

        /// @brief Indicates that the image is used as a write-only depth/stencil target.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        DepthWrite = 0x00000022,

        /// @brief Indicates that the image is presented on a swap chain.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_PRESENT` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` in the Vulkan 🌋 backend.
        ///
        /// Typically you do not want to manually transition a resource into this state. Render target transitions are automatically managed by @ref RenderPasses.
        Present = 0x00000023,

        /// @brief Indicates that the image is resolved from a multi-sampled image.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        ResolveSource = 0x00000024,

        /// @brief Indicates that the image is a render-target that a multi-sampled image is resolved into.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_RESOLVE_DEST` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` in the Vulkan 🌋 backend.
        ResolveDestination = 0x00000025,

        /// @brief The layout of the image is not known by the engine.
        ///
        /// Indicates that an image's layout is not known, which typically happens after creating image resources. It is not valid to transition any resource into this state.
        ///
        /// This image layout translates to `D3D12_BARRIER_LAYOUT_UNDEFINED` in the DirectX 12 ❎ backend and `VK_IMAGE_LAYOUT_UNDEFINED` in the Vulkan 🌋 backend.
        ///
        /// When using this layout as a source layout, the contents of the image may be discarded.
        Undefined = 0x7FFFFFFF
    };

    /// @brief Controls how a geometry that is part of a bottom-level acceleration structure (BLAS) behaves during ray-tracing.
    ///
    /// @see IBottomLevelAccelerationStructure
    enum class GeometryFlags {
        /// @brief Implies no restrictions on the geometry.
        None = 0x00,

        /// @brief If this flag is set, the any-hit shader for this geometry is never invoked, even if it is present within the hit group.
        Opaque = 0x01,

        /// @brief If this flag is set, the any-hit shader for this geometry is only invoked once for each primitive of the geometry, even if it could be invoked multiple times during ray tracing.
        OneShotAnyHit = 0x02
    };

    /// @brief Controls how an acceleration structure should be built.
    ///
    /// @see IBottomLevelAccelerationStructure
    /// @see ITopLevelAccelerationStructure
    enum class AccelerationStructureFlags {
        /// @brief Use default options for building the acceleration structure.
        None = 0x0000,

        /// @brief Allow the acceleration structure to be updated.
        AllowUpdate = 0x0001,

        /// @brief Allow the acceleration structure to be compacted.
        AllowCompaction = 0x0002,

        /// @brief Prefer building a better performing acceleration structure, that possibly takes longer to build.
        ///
        /// This flag cannot be combined with @ref PreferFastBuild.
        PreferFastTrace = 0x0004,

        /// @brief Prefer fast build times for the acceleration structure, but sacrifice ray-tracing performance.
        ///
        /// This flag cannot be combined with @ref PreferFastTrace.
        PreferFastBuild = 0x0008,

        /// @brief Prefer to minimize the memory footprint of the acceleration structure, but at the cost of ray-tracing performance and build times.
        MinimizeMemory = 0x0010
    };

    /// @brief Controls how an instance within a @ref ITopLevelAccelerationStructure behaves during ray-tracing.
    enum class InstanceFlags {
        /// @brief The instance uses default behavior.
        None = 0x00,

        /// @brief If this flag is set front- and backface culling is disabled for the instance.
        DisableCull = 0x01,

        /// @brief If this flag is set, front- and backfaces flip their default cull order.
        FlipWinding = 0x02,

        /// @brief If this flag is set, no geometry of the instance invokes the any-hit shader. This overwrites per-geometry flags.
        ///
        /// This flag must not be set in combination with @ref ForceNonOpaque.
        ///
        /// @see GeometryFlags::Opaque
        ForceOpaque = 0x04,

        /// @brief If this flag is set, each geometry of the instance will ignore the @ref GeometryFlags::Opaque setting.
        ///
        /// This flag must not be set in combination with @ref ForceOpaque.
        ///
        /// @see GeometryFlags::Opaque
        ForceNonOpaque = 0x08
    };

    // NOLINTEND(performance-enum-size)

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

    /// @brief Returns the number of channels for a buffer format.
    ///
    /// @see BufferFormat
    constexpr UInt32 getBufferFormatChannels(BufferFormat format) {
        return static_cast<UInt32>(format) & 0x000000FF; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }

    /// @brief Returns the number of bytes used by a channel of a buffer format.
    ///
    /// @see BufferFormat
    constexpr UInt32 getBufferFormatChannelSize(BufferFormat format) {
        return (static_cast<UInt32>(format) & 0xFF000000) >> 24; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }

    /// @brief Returns the underlying data type of a buffer format.
    ///
    /// @see BufferFormat
    constexpr UInt32 getBufferFormatType(BufferFormat format) {
        return (static_cast<UInt32>(format) & 0x0000FF00) >> 8; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }

    /// @brief Returns the size of an element of a specified format.
    constexpr size_t getSize(Format format) {
        switch (format) {
            using enum Format;
        case None:
            return 0; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R4G4_UNORM:
        case R8_UNORM:
        case R8_SNORM:
        case R8_USCALED:
        case R8_SSCALED:
        case R8_UINT:
        case R8_SINT:
        case R8_SRGB:
        case S8_UINT:
            return 1; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R4G4B4A4_UNORM:
        case B4G4R4A4_UNORM:
        case R5G6B5_UNORM:
        case B5G6R5_UNORM:
        case R5G5B5A1_UNORM:
        case B5G5R5A1_UNORM:
        case A1R5G5B5_UNORM:
        case R8G8_UNORM:
        case R8G8_SNORM:
        case R8G8_USCALED:
        case R8G8_SSCALED:
        case R8G8_UINT:
        case R8G8_SINT:
        case R8G8_SRGB:
        case R16_UNORM:
        case R16_SNORM:
        case R16_USCALED:
        case R16_SSCALED:
        case R16_UINT:
        case R16_SINT:
        case R16_SFLOAT:
        case D16_UNORM:
            return 2; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R8G8B8_UNORM:
        case R8G8B8_SNORM:
        case R8G8B8_USCALED:
        case R8G8B8_SSCALED:
        case R8G8B8_UINT:
        case R8G8B8_SINT:
        case R8G8B8_SRGB:
        case B8G8R8_UNORM:
        case B8G8R8_SNORM:
        case B8G8R8_USCALED:
        case B8G8R8_SSCALED:
        case B8G8R8_UINT:
        case B8G8R8_SINT:
        case B8G8R8_SRGB:
        case D16_UNORM_S8_UINT:
            return 3; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R8G8B8A8_UNORM:
        case R8G8B8A8_SNORM:
        case R8G8B8A8_USCALED:
        case R8G8B8A8_SSCALED:
        case R8G8B8A8_UINT:
        case R8G8B8A8_SINT:
        case R8G8B8A8_SRGB:
        case B8G8R8A8_UNORM:
        case B8G8R8A8_SNORM:
        case B8G8R8A8_USCALED:
        case B8G8R8A8_SSCALED:
        case B8G8R8A8_UINT:
        case B8G8R8A8_SINT:
        case B8G8R8A8_SRGB:
        case A8B8G8R8_UNORM:
        case A8B8G8R8_SNORM:
        case A8B8G8R8_USCALED:
        case A8B8G8R8_SSCALED:
        case A8B8G8R8_UINT:
        case A8B8G8R8_SINT:
        case A8B8G8R8_SRGB:
        case A2R10G10B10_UNORM:
        case A2R10G10B10_SNORM:
        case A2R10G10B10_USCALED:
        case A2R10G10B10_SSCALED:
        case A2R10G10B10_UINT:
        case A2R10G10B10_SINT:
        case A2B10G10R10_UNORM:
        case A2B10G10R10_SNORM:
        case A2B10G10R10_USCALED:
        case A2B10G10R10_SSCALED:
        case A2B10G10R10_UINT:
        case A2B10G10R10_SINT:
        case R16G16_UNORM:
        case R16G16_SNORM:
        case R16G16_USCALED:
        case R16G16_SSCALED:
        case R16G16_UINT:
        case R16G16_SINT:
        case R16G16_SFLOAT:
        case R32_UINT:
        case R32_SINT:
        case R32_SFLOAT:
        case B10G11R11_UFLOAT:
        case E5B9G9R9_UFLOAT:
        case X8_D24_UNORM:
        case D32_SFLOAT:
        case D24_UNORM_S8_UINT:
            return 4; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R16G16B16_UNORM:
        case R16G16B16_SNORM:
        case R16G16B16_USCALED:
        case R16G16B16_SSCALED:
        case R16G16B16_UINT:
        case R16G16B16_SINT:
        case R16G16B16_SFLOAT:
            return 6; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R16G16B16A16_UNORM:
        case R16G16B16A16_SNORM:
        case R16G16B16A16_USCALED:
        case R16G16B16A16_SSCALED:
        case R16G16B16A16_UINT:
        case R16G16B16A16_SINT:
        case R16G16B16A16_SFLOAT:
        case R32G32_UINT:
        case R32G32_SINT:
        case R32G32_SFLOAT:
        case R64_UINT:
        case R64_SINT:
        case R64_SFLOAT:
        case D32_SFLOAT_S8_UINT:	// NOTE: This may be wrong here - spec states, however, that 24 bits are unused.
        case BC1_RGB_UNORM:
        case BC1_RGB_SRGB:
        case BC1_RGBA_UNORM:
        case BC1_RGBA_SRGB:
        case BC4_UNORM:
        case BC4_SNORM:
            return 8; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R32G32B32_UINT:
        case R32G32B32_SINT:
        case R32G32B32_SFLOAT:
            return 12; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R32G32B32A32_UINT:
        case R32G32B32A32_SINT:
        case R32G32B32A32_SFLOAT:
        case R64G64_UINT:
        case R64G64_SINT:
        case R64G64_SFLOAT:
        case BC2_UNORM:
        case BC2_SRGB:
        case BC3_UNORM:
        case BC3_SRGB:
        case BC5_UNORM:
        case BC5_SNORM:
        case BC6H_UFLOAT:
        case BC6H_SFLOAT:
        case BC7_UNORM:
        case BC7_SRGB:
            return 16; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R64G64B64_UINT:
        case R64G64B64_SINT:
        case R64G64B64_SFLOAT:
            return 24; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        case R64G64B64A64_UINT:
        case R64G64B64A64_SINT:
        case R64G64B64A64_SFLOAT:
            return 32; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        default:
            throw InvalidArgumentException("format", "Unsupported format: {0}.", std::to_underlying(format)); // An invalid format will not have a formatter anyway, but it also would be illegal to use it here.
        }
    }

    /// @brief Returns `true`, if the format contains a depth channel.
    ///
    /// @see DepthStencilState
    constexpr bool hasDepth(Format format) noexcept {
        constexpr std::array<Format, 6> depthFormats {
            Format::D16_UNORM,
            Format::D32_SFLOAT,
            Format::X8_D24_UNORM,
            Format::D16_UNORM_S8_UINT,
            Format::D24_UNORM_S8_UINT,
            Format::D32_SFLOAT_S8_UINT
        };

        return std::any_of(std::begin(depthFormats), std::end(depthFormats), [&](Format f) { return f == format; });
    }

    /// @brief Returns `true`, if the format contains a stencil channel.
    ///
    /// @see DepthStencilState
    constexpr bool hasStencil(Format format) noexcept {
        constexpr std::array<Format, 4> stencilFormats {
            Format::D16_UNORM_S8_UINT,
            Format::D24_UNORM_S8_UINT,
            Format::D32_SFLOAT_S8_UINT,
            Format::S8_UINT
        };

        return std::any_of(std::begin(stencilFormats), std::end(stencilFormats), [&](Format f) { return f == format; });
    }

#pragma endregion

#pragma region "Data Types"

    /// @brief An indirect batch used to execute an standard draw call.
    ///
    /// @see IndirectDispatchBatch
    /// @see IndirectIndexedBatch
    struct LITEFX_RENDERING_API alignas(16) IndirectBatch { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        /// @brief The number of vertices of the mesh.
        ///
        /// @see FirstVertex
        UInt32 VertexCount{ };

        /// @brief The number of instances to draw of this mesh.
        ///
        /// @see FirstInstance
        UInt32 InstanceCount{ };

        /// @brief The index of the first vertex of the mesh in the vertex buffer.
        ///
        /// @see VertexCount
        UInt32 FirstVertex{ };

        /// @brief The index of the first index to draw. This value is added to each instance index before obtaining per-instance data from the vertex buffer.
        ///
        /// @see InstanceCount
        UInt32 FirstInstance{ };
    };

#pragma warning(push)
#pragma warning(disable: 4324) // Structure was padded due to alignment specifier
    /// @brief An indirect batch used to execute an indexed draw call.
    ///
    /// @see IndirectDispatchBatch
    /// @see IndirectBatch
    struct LITEFX_RENDERING_API alignas(16) IndirectIndexedBatch { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        /// @brief The number of indices in the mesh index buffer.
        ///
        /// @see FirstIndex
        UInt32 IndexCount{ };

        /// @brief The number of instances to draw of this mesh.
        ///
        /// @see FirstInstance
        UInt32 InstanceCount{ };

        /// @brief The first index in the index buffer used to draw the mesh.
        ///
        /// @see IndexCount
        UInt32 FirstIndex{ };

        /// @brief An offset added to each index to obtain a vertex.
        Int32 VertexOffset{ };

        /// @brief The index of the first index to draw. This value is added to each instance index before obtaining per-instance data from the vertex buffer.
        ///
        /// @see InstanceCount
        UInt32 FirstInstance{ };
    };

    /// @brief An indirect batch used to dispatch a compute shader kernel.
    ///
    /// @see IndirectIndexedBatch
    /// @see IndirectBatch
    struct LITEFX_RENDERING_API alignas(16) IndirectDispatchBatch { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        /// @brief The number of threads into x-direction.
        UInt32 X{ 1 };

        /// @brief The number of threads into y-direction.
        UInt32 Y{ 1 };

        /// @brief The number of threads into z-direction.
        UInt32 Z{ 1 };
    };
#pragma warning(pop)

    /// @brief Contains the parameters for a resource allocation.
    ///
    /// @see IGraphicsFactory
    /// @see ResourceAllocationResult
    struct LITEFX_RENDERING_API ResourceAllocationInfo final {
    public:
        /// @brief Stores information about a buffer resource allocation.
        struct BufferInfo 
        {
            /// @brief Stores the buffer type.
            BufferType Type{ BufferType::Uniform };

            /// @brief Stores the size of a single element within the buffer.
            size_t ElementSize{ 0u };

            /// @brief Stores the number of elements in the buffer.
            UInt32 Elements{ 1u };
            
            /// @brief Stores the resource heap on which to allocate the resource.
            ResourceHeap Heap{ ResourceHeap::Dynamic };

            /// @brief The layout of a vertex buffer.
            ///
            /// If @ref Type is @ref BufferType::Vertex, setting this property creates a @ref IVertexBuffer, otherwise a usual @ref IBuffer is created. If @ref Type is not @ref BufferType::Vertex, this property is
            /// ignored.
            SharedPtr<const IVertexBufferLayout> VertexBufferLayout{ nullptr };

            /// @brief The layout of a index buffer.
            ///
            /// If @ref Type is @ref BufferType::Index, setting this property creates a @ref IIndexBuffer, otherwise a usual @ref IBuffer is created. If @ref Type is not @ref BufferType::Index, this property is
            /// ignored.
            SharedPtr<const IIndexBufferLayout> IndexBufferLayout{ nullptr };
        };

        /// @brief Stores information about an image resource allocation.
        struct ImageInfo 
        {
            /// @brief Stores the desired format of the image.
            Format Format{ Format::R8G8B8A8_SRGB };

            /// @brief Stores the dimensions of the image.
            ImageDimensions Dimensions{ ImageDimensions::DIM_2 };

            /// @brief Stores the size of the image.
            Size3d Size{ };

            /// @brief Stores the number of mip-map levels in the image.
            UInt32 Levels{ 1u };

            /// @brief Stores the number of layers in the image.
            UInt32 Layers{ 1u };

            /// @brief Stores the number of multi-samples in the image.
            MultiSamplingLevel Samples{ MultiSamplingLevel::x1 };
        };

    public:
        /// @brief Stores the buffer or image info associated with the allocation info.
        Variant<BufferInfo, ImageInfo> ResourceInfo{};

        /// @brief Stores the resource usage flags for the allocation info.
        ResourceUsage Usage{ ResourceUsage::Default };

        /// @brief Stores the desired name of the allocated resource.
        String Name{ };

        /// @brief An optional offset that is used to place the resource in a block of allocated memory when allocating overlapping resources.
        ///
        /// @see IGraphicsFactory::allocate
        size_t AliasingOffset{ 0u };

    public:
        /// @brief Creates a new resource allocation info instance.
        ResourceAllocationInfo() = default;

        /// @brief Creates a new resource allocation info instance for a buffer resource.
        ///
        /// @param bufferInfo The details about the buffer.
        /// @param usage The usage flags for the buffer.
        /// @param name The name of the buffer resource.
        /// @param aliasingOffset An optional offset that is used to place the resource in a block of allocated memory when allocating overlapping resources.
        explicit ResourceAllocationInfo(const BufferInfo& bufferInfo, ResourceUsage usage = ResourceUsage::Default, String name = "", size_t aliasingOffset = 0u) :
            ResourceInfo(bufferInfo), Usage(usage), Name(std::move(name)), AliasingOffset(aliasingOffset) { }

        /// @brief Creates a new resource allocation info instance for an image resource.
        ///
        /// @param imageInfo The details about the image.
        /// @param usage The usage flags for the image.
        /// @param name The name of the image resource.
        /// @param aliasingOffset An optional offset that is used to place the resource in a block of allocated memory when allocating overlapping resources.
        explicit ResourceAllocationInfo(const ImageInfo& imageInfo, ResourceUsage usage = ResourceUsage::Default, String name = "", size_t aliasingOffset = 0u) :
            ResourceInfo(imageInfo), Usage(usage), Name(std::move(name)), AliasingOffset(aliasingOffset) { }
        
        ResourceAllocationInfo(const ResourceAllocationInfo&) = default;
        ResourceAllocationInfo(ResourceAllocationInfo&&) noexcept = default;
        ResourceAllocationInfo& operator=(const ResourceAllocationInfo&) = default;
        ResourceAllocationInfo& operator=(ResourceAllocationInfo&&) noexcept = default;
        ~ResourceAllocationInfo() noexcept = default;
    };

    /// @brief Stores the result of a resource allocation.
    ///
    /// @see IGraphicsFactory
    /// @see ResourceAllocationInfo
    struct LITEFX_RENDERING_API ResourceAllocationResult final {
    private:
        Variant<SharedPtr<IImage>, SharedPtr<IBuffer>> m_resource;

    public:
        /// @brief Initializes an allocation result for an image resource.
        ///
        /// @param image The allocate image resource.
        ResourceAllocationResult(SharedPtr<IImage>&& image) noexcept :
            m_resource(std::move(image)) { }
        
        /// @brief Initializes an allocation result for a buffer resource.
        ///
        /// @param buffer The allocated buffer resource.
        ResourceAllocationResult(SharedPtr<IBuffer>&& buffer) noexcept :
            m_resource(std::move(buffer)) { }

        ResourceAllocationResult() = delete;
        ResourceAllocationResult(const ResourceAllocationResult&) = delete;
        ResourceAllocationResult(ResourceAllocationResult&&) noexcept = default;
        ResourceAllocationResult& operator=(const ResourceAllocationResult&) = delete;
        ResourceAllocationResult& operator=(ResourceAllocationResult&&) noexcept = default;
        ~ResourceAllocationResult() noexcept = default;

    public:
        /// @brief Returns the allocated image resource, or raises an exception if the allocation does not contain an image resource, or the image resource is not of @p TImage.
        ///
        /// @tparam TImage The type of the image.
        /// @return The pointer to the image resource.
        /// @throws RuntimeException Thrown, if the allocated resource is not an image, or if the allocated image does not implement @p TImage.
        template <std::derived_from<IImage> TImage>
        SharedPtr<TImage> image() const {
            if (!std::holds_alternative<SharedPtr<IImage>>(m_resource)) [[unlikely]]
                throw RuntimeException("The allocation result does not contain an image.");
            
            auto image = std::dynamic_pointer_cast<TImage>(std::get<SharedPtr<IImage>>(m_resource));

            if (image == nullptr)
                throw RuntimeException("The allocated image resource is not of the requested image type.");
            
            return image;
        }

        /// @brief Returns the allocated buffer resource, or raises an exception if the allocation does not contain a buffer resource, or the buffer resource is not of @p TBuffer.
        ///
        /// @tparam TBuffer The type of the buffer.
        /// @return The pointer to the buffer resource.
        /// @throws RuntimeException Thrown, if the allocated resource is not a buffer, or if the allocated buffer does not implement @p TBuffer.
        template <std::derived_from<IBuffer> TBuffer>
        SharedPtr<TBuffer> buffer() const {
            if (!std::holds_alternative<SharedPtr<IBuffer>>(m_resource)) [[unlikely]]
                throw RuntimeException("The allocation result does not contain a buffer.");

            auto buffer = std::dynamic_pointer_cast<TBuffer>(std::get<SharedPtr<IBuffer>>(m_resource));

            if (buffer == nullptr)
                throw RuntimeException("The allocated buffer resource is not of the requested buffer type.");

            return buffer;
        }
    };
#pragma endregion

    /// @brief The interface for a state resource.
    class LITEFX_RENDERING_API IStateResource {
    protected:
        IStateResource() noexcept = default;
        IStateResource(const IStateResource&) = delete;
        IStateResource(IStateResource&&) noexcept = default;
        IStateResource& operator=(const IStateResource&) = delete;
        IStateResource& operator=(IStateResource&&) noexcept = default;

    public:
        /// @brief Releases the state resource instance.
        virtual ~IStateResource() noexcept = default;

    public:
        /// @brief Returns the name of the resource.
        ///
        /// @return The name of the resource.
        virtual const String& name() const noexcept = 0;
    };

    /// @brief Base class for a resource that can be identified by a name string within a @ref DeviceState.
    class LITEFX_RENDERING_API StateResource : public virtual IStateResource {
        LITEFX_IMPLEMENTATION(StateResourceImpl);

    protected:
        /// @brief Initializes a new state resource instance with a default name.
        StateResource();

        /// @brief Initializes a new state resource instance.
        ///
        /// @param name The name of the resource.
        explicit StateResource(StringView name);

        StateResource(StateResource&&) noexcept;
        StateResource& operator=(StateResource&&) noexcept;

        StateResource(const StateResource&) = delete;
        StateResource& operator=(const StateResource&) = delete;

    public:
        /// @brief Releases the state resource instance.
        ~StateResource() noexcept override;

    protected:
        /// @copydoc name() const
        String& name() noexcept;

    public:
        /// @copydoc IStateResource::name
        const String& name() const noexcept override;
    };

    /// @brief A class that can be used to manage the state of a @ref IGraphicsDevice.
    ///
    /// The device state makes managing resources created by a device easier, since you do not have to worry about storage and release order. Note, however, that this is not free. Requesting a resource
    /// requires a lookup within a hash-map. Also device states are not specialized for the concrete device, so you can only work with interfaces. This implies potentially inefficient upcasting of the state
    /// resource when its passed to another object. You have to decide if or to which degree you want to rely on storing resources in a device state.
    ///
    /// @see StateResource
    /// @see IGraphicsDevice
    class LITEFX_RENDERING_API DeviceState {
        LITEFX_IMPLEMENTATION(DeviceStateImpl);
        friend class IGraphicsDevice;

    public:
        /// @brief Creates a new device state instance.
        explicit DeviceState();

        /// @brief Takes over another instance of a device state.
        ///
        /// @param _other The device state instance to take over.
        DeviceState(DeviceState&& _other) noexcept;

        /// @brief Assigns a device state by taking it over.
        ///
        /// @param _other The device state to take over.
        /// @return A reference to the current device state instance.
        DeviceState& operator=(DeviceState&& _other) noexcept;

        /// @brief Releases the device state instance.
        virtual ~DeviceState() noexcept;

        DeviceState(const DeviceState&) = delete;
        DeviceState& operator=(const DeviceState&) = delete;

    public:
        /// @brief Release all resources managed by the device state.
        void clear();

        /// @brief Adds a new render pass to the device state and uses its name as identifier.
        ///
        /// @param renderPass The render pass to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another render pass with the same identifier has already been added.
        void add(SharedPtr<IRenderPass>&& renderPass);

        /// @brief Adds a new render pass to the device state.
        ///
        /// @param id The identifier for the render pass.
        /// @param renderPass The render pass to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another render pass with the same @p id has already been added.
        void add(const String& id, SharedPtr<IRenderPass>&& renderPass);

        /// @brief Adds a new frame buffer to the device state and uses its name as identifier.
        ///
        /// @param frameBuffer The render pass to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another frame buffer with the same identifier has already been added.
        void add(SharedPtr<IFrameBuffer>&& frameBuffer);

        /// @brief Adds a new frame buffer to the device state.
        ///
        /// @param id The identifier for the frame buffer.
        /// @param renderPass The frame buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another frame buffer with the same @p id has already been added.
        void add(const String& id, SharedPtr<IFrameBuffer>&& frameBuffer);

        /// @brief Adds a new pipeline to the device state and uses its name as identifier.
        ///
        /// @param pipeline The pipeline to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another pipeline with the same identifier has already been added.
        void add(UniquePtr<IPipeline>&& pipeline);

        /// @brief Adds a new pipeline to the device state.
        ///
        /// @param id The identifier for the pipeline.
        /// @param pipeline The pipeline to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another pipeline with the same @p id has already been added.
        void add(const String& id, UniquePtr<IPipeline>&& pipeline);

        /// @brief Adds a new buffer to the device state and uses its name as identifier.
        ///
        /// @param buffer The buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another buffer with the same identifier has already been added.
        void add(SharedPtr<IBuffer>&& buffer);

        /// @brief Adds a new buffer to the device state.
        ///
        /// @param id The identifier for the buffer.
        /// @param buffer The buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another buffer with the same @p id has already been added.
        void add(const String& id, SharedPtr<IBuffer>&& buffer);

        /// @brief Adds a new vertex buffer to the device state and uses its name as identifier.
        ///
        /// @param vertexBuffer The vertex buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another vertex buffer with the same identifier has already been added.
        void add(SharedPtr<IVertexBuffer>&& vertexBuffer);

        /// @brief Adds a new vertex buffer to the device state.
        ///
        /// @param id The identifier for the vertex buffer.
        /// @param vertexBuffer The vertex buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another vertex buffer with the same @p id has already been added.
        void add(const String& id, SharedPtr<IVertexBuffer>&& vertexBuffer);

        /// @brief Adds a new index buffer to the device state and uses its name as identifier.
        ///
        /// @param indexBuffer The index buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another index buffer with the same identifier has already been added.
        void add(SharedPtr<IIndexBuffer>&& indexBuffer);

        /// @brief Adds a new index buffer to the device state.
        ///
        /// @param id The identifier for the index buffer.
        /// @param indexBuffer The index buffer to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another index buffer with the same @p id has already been added.
        void add(const String& id, SharedPtr<IIndexBuffer>&& indexBuffer);

        /// @brief Adds a new image to the device state and uses its name as identifier.
        ///
        /// @param image The image to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another image with the same identifier has already been added.
        void add(SharedPtr<IImage>&& image);

        /// @brief Adds a new image to the device state.
        ///
        /// @param id The identifier for the image.
        /// @param image The image to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another image with the same @p id has already been added.
        void add(const String& id, SharedPtr<IImage>&& image);

        /// @brief Adds a new sampler to the device state and uses its name as identifier.
        ///
        /// @param sampler The sampler to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another sampler with the same identifier has already been added.
        void add(SharedPtr<ISampler>&& sampler);

        /// @brief Adds a new sampler to the device state.
        ///
        /// @param id The identifier for the sampler.
        /// @param sampler The sampler to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another sampler with the same @p id has already been added.
        void add(const String& id, SharedPtr<ISampler>&& sampler);

        /// @brief Adds a new acceleration structure to the device state and uses its name as identifier.
        ///
        /// @param accelerationStructure The acceleration structure to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another acceleration structure with the same @p id has already been added.
        void add(UniquePtr<IAccelerationStructure>&& accelerationStructure);

        /// @brief Adds a new acceleration structure to the device state.
        ///
        /// @param id The identifier for the acceleration structure.
        /// @param accelerationStructure The acceleration structure to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another acceleration structure with the same @p id has already been added.
        void add(const String& id, UniquePtr<IAccelerationStructure>&& accelerationStructure);
        
        /// @brief Adds a new descriptor set to the device state.
        ///
        /// @param id The identifier for the descriptor set.
        /// @param sampler The descriptor set to add to the device state.
        /// @throws InvalidArgumentException Thrown, if another descriptor set with the same @p id has already been added.
        void add(const String& id, UniquePtr<IDescriptorSet>&& descriptorSet);

        /// @brief Returns a render pass from the device state.
        ///
        /// @param id The identifier associated with the render pass.
        /// @return A reference of the render pass.
        /// @throws InvalidArgumentExceptoin Thrown, if no render pass has been added for the provided @p id.
        IRenderPass& renderPass(const String& id) const;

        /// @brief Returns a frame buffer from the device state.
        ///
        /// @param id The identifier associated with the frame buffer.
        /// @return A reference of the frame buffer.
        /// @throws InvalidArgumentExceptoin Thrown, if no frame buffer has been added for the provided @p id.
        IFrameBuffer& frameBuffer(const String& id) const;

        /// @brief Returns a pipeline from the device state.
        ///
        /// @param id The identifier associated with the pipeline.
        /// @return A reference of the pipeline.
        /// @throws InvalidArgumentExceptoin Thrown, if no pipeline has been added for the provided @p id.
        IPipeline& pipeline(const String& id) const;

        /// @brief Returns a buffer from the device state.
        ///
        /// @param id The identifier associated with the buffer.
        /// @return A reference of the buffer.
        /// @throws InvalidArgumentExceptoin Thrown, if no buffer has been added for the provided @p id.
        IBuffer& buffer(const String& id) const;

        /// @brief Returns a vertex buffer from the device state.
        ///
        /// @param id The identifier associated with the vertex buffer.
        /// @return A reference of the vertex buffer.
        /// @throws InvalidArgumentExceptoin Thrown, if no vertex buffer has been added for the provided @p id.
        IVertexBuffer& vertexBuffer(const String& id) const;

        /// @brief Returns an index buffer from the device state.
        ///
        /// @param id The identifier associated with the index buffer.
        /// @return A reference of the index buffer.
        /// @throws InvalidArgumentExceptoin Thrown, if no index buffer has been added for the provided @p id.
        IIndexBuffer& indexBuffer(const String& id) const;

        /// @brief Returns an image from the device state.
        ///
        /// @param id The identifier associated with the image.
        /// @return A reference of the image.
        /// @throws InvalidArgumentExceptoin Thrown, if no image has been added for the provided @p id.
        IImage& image(const String& id) const;

        /// @brief Returns a sampler from the device state.
        ///
        /// @param id The identifier associated with the sampler.
        /// @return A reference of the sampler.
        /// @throws InvalidArgumentExceptoin Thrown, if no sampler has been added for the provided @p id.
        ISampler& sampler(const String& id) const;

        /// @brief Returns an acceleration structure from the device state.
        ///
        /// @param id The identifier associated with the acceleration structure.
        /// @return A reference of the acceleration structure.
        /// @throws InvalidArgumentExceptoin Thrown, if no acceleration structure has been added for the provided @p id.
        IAccelerationStructure& accelerationStructure(const String& id) const;

        /// @brief Returns a descriptor set from the device state.
        ///
        /// @param id The identifier associated with the descriptor set.
        /// @return A reference of the descriptor set.
        /// @throws InvalidArgumentExceptoin Thrown, if no descriptor set has been added for the provided @p id.
        IDescriptorSet& descriptorSet(const String& id) const;

        /// @brief Releases a render pass.
        ///
        /// Calling this method will destroy the render pass. Before calling it, the render pass must be requested using @ref renderPass. After this method has been executed, all references (including the @p
        /// renderPass parameter) will be invalid. If the render pass is not managed by the device state, this method will do nothing and return `false`.
        ///
        /// @param renderPass The render pass to release.
        /// @return `true`, if the render pass was properly released, `false` otherwise.
        bool release(const IRenderPass& renderPass);

        /// @brief Releases a frame buffer.
        ///
        /// @param renderPass The frame buffer to release.
        /// @return `true`, if the frame buffer was properly released, `false` otherwise.
        bool release(const IFrameBuffer& frameBuffer);

        /// @brief Releases a pipeline.
        ///
        /// @param pipeline The pipeline to release.
        /// @return `true`, if the pipeline was properly released, `false` otherwise.
        bool release(const IPipeline& pipeline);

        /// @brief Releases a buffer.
        ///
        /// @param buffer The buffer to release.
        /// @return `true`, if the buffer was properly released, `false` otherwise.
        bool release(const IBuffer& buffer);

        /// @brief Releases a vertex buffer.
        ///
        /// @param buffer The vertex buffer to release.
        /// @return `true`, if the vertex buffer was properly released, `false` otherwise.
        bool release(const IVertexBuffer& buffer);

        /// @brief Releases a index buffer.
        ///
        /// @param buffer The index buffer to release.
        /// @return `true`, if the index buffer was properly released, `false` otherwise.
        bool release(const IIndexBuffer& buffer);

        /// @brief Releases an image.
        ///
        /// @param image The image to release.
        /// @return `true`, if the image was properly released, `false` otherwise.
        bool release(const IImage& image);

        /// @brief Releases a sampler.
        ///
        /// @param sampler The sampler to release.
        /// @return `true`, if the sampler was properly released, `false` otherwise.
        bool release(const ISampler& sampler);

        /// @brief Releases a descriptor set.
        ///
        /// @param descriptorSet The descriptor set to release.
        /// @return `true`, if the descriptor set was properly released, `false` otherwise.
        bool release(const IDescriptorSet& descriptorSet);
    };

    /// @brief Represents a virtual allocator that manages memory distribution from a piece of raw memory.
    ///
    /// Note that the virtual allocator does not actually contain memory, but rather keeps track over a range of memory that is externally managed.
    class LITEFX_RENDERING_API VirtualAllocator final {
    public:
        /// @brief Represents an allocation within the memory managed by the virtual allocator.
        struct Allocation final {
            /// @brief The handle that identifies the allocation.
            UInt64 Handle { 0u };

            /// @brief The overall size of the allocation in bytes.
            UInt64 Size { 0u };

            /// @brief The offset to the start of the allocation within the memory block.
            UInt64 Offset { std::numeric_limits<UInt64>::max() };
        };

    private:
        /// @brief The interface for an allocator implementation.
        struct AllocatorImplBase {
        private:
            UInt64 m_size;
            AllocationAlgorithm m_algorithm;

        protected:
            /// @brief Creates a new allocator instance.
            ///
            /// @param overallMemory The overall size (in bytes) of memory available to the allocator.
            /// @param algorithm The algorithm used to find a suitable block in the allocator memory.
            AllocatorImplBase(UInt64 overallMemory, AllocationAlgorithm algorithm) :
                m_size(overallMemory), m_algorithm(algorithm)
            {
            }

            AllocatorImplBase(const AllocatorImplBase&) = delete;
            AllocatorImplBase(AllocatorImplBase&&) noexcept = delete;
            AllocatorImplBase& operator=(const AllocatorImplBase&) = delete;
            AllocatorImplBase& operator=(AllocatorImplBase&&) noexcept = delete;

        public:
            virtual ~AllocatorImplBase() noexcept = default;

        public:
            /// @brief Returns the size of the memory managed by the virtual allocator.
            ///
            /// @return The size (in bytes) of the memory managed by the virtual allocator.
            inline UInt64 size() const noexcept {
                return m_size;
            }

            /// @brief Returns the algorithm used by the allocator.
            ///
            /// @return The algorithm used by the allocator.
            inline AllocationAlgorithm algorithm() const noexcept {
                return m_algorithm;
            }

            /// @brief Allocates a piece of memory of @p size bytes, aligned to @p alignment.
            ///
            /// @param size The size (in bytes) of the resource to place in the allocation.
            /// @param alignment The alignment requirements of the resource.
            /// @param strategy The strategy to look for a place to put the allocation in.
            /// @param privateData A pointer to an object that should be internally associated with the allocation.
            /// @return An object that contains details about the allocation.
            [[nodiscard]] virtual Allocation allocate(UInt64 size, UInt32 alignment = 1u, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const = 0;

            /// @brief Attempts to allocate a piece of memory of @p size bytes, aligned to @p alignment. If the allocation fails `std::nullopt` is returned.
            ///
            /// @param size The size (in bytes) of the resource to place in the allocation.
            /// @param alignment The alignment requirements of the resource.
            /// @param strategy The strategy to look for a place to put the allocation in.
            /// @param privateData A pointer to an object that should be internally associated with the allocation.
            /// @return An object that contains details about the allocation, or `std::nullopt` if the allocation fails.
            [[nodiscard]] virtual Optional<Allocation> tryAllocate(UInt64 size, UInt32 alignment = 1u, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const = 0;

            /// @brief Releases an allocation from the allocator, so that its memory can be re-used later.
            ///
            /// @param allocation The allocation to release.
            virtual void free(Allocation&& allocation) const = 0;

            /// @brief Returns the private data associated with an allocation.
            ///
            /// @param allocation The allocation for which to obtain the private data.
            /// @return A pointer that references the private data associated with the allocation.
            virtual void* privateData(const Allocation& allocation) const = 0;
        };

        /// @brief Implements a specific allocator.
        ///
        /// @tparam TBackend The backend, for which the allocator is implemented
        template <typename TBackend>
        struct AllocatorImpl final : public AllocatorImplBase { 
            static_assert(false, "Attempting to use a non-specialized virtual allocator is invalid.");
        };

        /// @brief Stores the allocator implementation.
        UniquePtr<AllocatorImplBase> m_impl;

        /// @brief Creates a new virtual allocator instance.
        ///
        /// @param pImpl The pointer to the allocator implementation.
        VirtualAllocator(UniquePtr<AllocatorImplBase>&& pImpl) :
            m_impl(std::move(pImpl))
        {
        }

    public:
        VirtualAllocator(const VirtualAllocator&) = delete;
        VirtualAllocator(VirtualAllocator&&) noexcept = delete;
        VirtualAllocator& operator=(const VirtualAllocator&) = delete;
        VirtualAllocator& operator=(VirtualAllocator&&) noexcept = delete;
        ~VirtualAllocator() noexcept = default;

    public:
        /// @brief Creates a new virtual allocator instance.
        ///
        /// @param overallMemory The overall size (in bytes) of memory available to the allocator.
        /// @param algorithm The algorithm used to find a suitable block in the allocator memory.
        /// @return The instance of the virtual allocator.
        template <typename TBackend>
        [[nodiscard]] static inline VirtualAllocator create(UInt64 overallMemory, AllocationAlgorithm algorithm = AllocationAlgorithm::Default) {
            return VirtualAllocator(UniquePtr<AllocatorImplBase>(new AllocatorImpl<TBackend>(overallMemory, algorithm)));
        }

    public:
        /// @brief Returns the size of the memory managed by the virtual allocator.
        ///
        /// @return The size (in bytes) of the memory managed by the virtual allocator.
        inline UInt64 size() const noexcept {
            return m_impl->size();
        }

        /// @brief Returns the algorithm used by the allocator.
        ///
        /// @return The algorithm used by the allocator.
        inline AllocationAlgorithm algorithm() const noexcept {
            return m_impl->algorithm();
        }

        /// @brief Allocates a piece of memory of @p size bytes, aligned to @p alignment.
        ///
        /// @param size The size (in bytes) of the resource to place in the allocation.
        /// @param alignment The alignment requirements of the resource.
        /// @param strategy The strategy to look for a place to put the allocation in.
        /// @param privateData A pointer to an object that should be internally associated with the allocation.
        /// @return An object that contains details about the allocation.
        [[nodiscard]] inline Allocation allocate(UInt64 size, UInt32 alignment = 1u, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const {
            return m_impl->allocate(size, alignment, strategy, privateData);
        }

        /// @brief Attempts to allocate a piece of memory of @p size bytes, aligned to @p alignment. If the allocation fails `std::nullopt` is returned.
        ///
        /// @param size The size (in bytes) of the resource to place in the allocation.
        /// @param alignment The alignment requirements of the resource.
        /// @param strategy The strategy to look for a place to put the allocation in.
        /// @param privateData A pointer to an object that should be internally associated with the allocation.
        /// @return An object that contains details about the allocation, or `std::nullopt` if the allocation fails.
        [[nodiscard]] Optional<Allocation> tryAllocate(UInt64 size, UInt32 alignment = 1u, AllocationStrategy strategy = AllocationStrategy::OptimizePacking, void* privateData = nullptr) const {
            return m_impl->tryAllocate(size, alignment, strategy, privateData);
        }

        /// @brief Releases an allocation from the allocator, so that its memory can be re-used later.
        ///
        /// Releasing an allocation that was not allocated from the virtual allocator is undefined behavior.
        ///
        /// @param allocation The allocation to release.
        inline void free(Allocation&& allocation) const { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
            m_impl->free(std::forward<Allocation>(allocation));
        }

        /// @brief Returns the private data associated with an allocation.
        ///
        /// @param allocation The allocation for which to obtain the private data.
        /// @return A pointer that references the private data associated with the allocation.
        inline void* privateData(const Allocation& allocation) const {
            return m_impl->privateData(allocation);
        };
    };

    /// @brief Represents a physical graphics adapter.
    ///
    /// A graphics adapter can be seen as an actual physical device that can run graphics computations. Typically this resembles a GPU that is connected to the bus. However, it can also represent an emulated,
    /// virtual adapter, such as a software rasterizer.
    class LITEFX_RENDERING_API IGraphicsAdapter : public SharedObject {
    protected:
        IGraphicsAdapter() noexcept = default;
        IGraphicsAdapter(const IGraphicsAdapter&) = default;
        IGraphicsAdapter(IGraphicsAdapter&&) noexcept = default;
        IGraphicsAdapter& operator=(const IGraphicsAdapter&) = default;
        IGraphicsAdapter& operator=(IGraphicsAdapter&&) noexcept = default;

    public:
        ~IGraphicsAdapter() noexcept override= default;

    public:
        /// @brief Retrieves the name of the graphics adapter.
        ///
        /// @return The name of the graphics adapter.
        virtual String name() const = 0;

        /// @brief Returns a unique identifier, that identifies the device in the system.
        ///
        /// @return A unique identifier, that identifies the device in the system.
        virtual UInt64 uniqueId() const noexcept = 0;

        /// @brief Returns a unique identifier, that identifies the vendor of the graphics adapter.
        ///
        /// @return A unique identifier, that identifies the vendor of the graphics adapter.
        virtual UInt32 vendorId() const noexcept = 0;

        /// @brief Returns a unique identifier, that identifies the product.
        ///
        /// @return A unique identifier, that identifies the product.
        virtual UInt32 deviceId() const noexcept = 0;

        /// @brief Returns the type of the graphics adapter.
        ///
        /// @return The type of the graphics adapter.
        virtual GraphicsAdapterType type() const noexcept = 0;

        /// @brief Returns the graphics driver version.
        ///
        /// Note that this is a vendor and API specific identifier that can be used to compare against specific (known) versions. It is not recommended to parse this into a front-facing version number for users,
        /// as it differs between backends. For this, use vendor-supplied APIs instead.
        ///
        /// @return The graphics driver version.
        virtual UInt64 driverVersion() const noexcept = 0;

        /// @brief Returns the graphics API version.
        ///
        /// @return The graphics API version.
        virtual UInt32 apiVersion() const noexcept = 0;

        /// @brief Returns the amount of dedicated graphics memory (in bytes), this adapter can use.
        ///
        /// @return The amount of dedicated graphics memory (in bytes), this adapter can use.
        virtual UInt64 dedicatedMemory() const noexcept = 0;
    };

    /// @brief Represents a surface to render to.
    ///
    /// A surface can be seen as a window or area on the screen, the renderer can draw to. Note that the interface does not make any constraints on the surface to allow for portability. A surface
    /// implementation may provide access to the actual handle to use. Surface instances are responsible for owning the handle.
    class LITEFX_RENDERING_API ISurface {
    protected:
        ISurface() noexcept = default;
        ISurface(const ISurface&) = default;
        ISurface(ISurface&&) noexcept = default;
        ISurface& operator=(const ISurface&) = default;
        ISurface& operator=(ISurface&&) noexcept = default;

    public:
        virtual ~ISurface() noexcept = default;
    };
    
    /// @brief Describes a single descriptor binding point within a @ref IShaderModule.
    struct LITEFX_RENDERING_API DescriptorBindingPoint final {
    public:
        /// @brief Stores the register index of the binding point.
        UInt32 Register { 0 };

        /// @brief Stores the descriptor space (or set index) of the binding point.
        UInt32 Space { 0 };

    public:
        /// @brief Implements three-way comparison for descriptor binding points.
        ///
        /// @param other The other binding point to compare against.
        /// @return `less`, if the `Space` property of the instance is lower than the `Space` property of @p other, and `greater` if the opposite is true and they are not equal. If the `Space` properties are
        /// equal, the `Register` properties are compared accordingly. If both, `Space` and `Register` are equal, the operator returns `equal`.
        inline auto operator<=>(const DescriptorBindingPoint& other) const noexcept {
            // NOLINTBEGIN(bugprone-branch-clone)
            if (this->Space < other.Space)
                return std::strong_ordering::less;
            else if (this->Space > other.Space)
                return std::strong_ordering::greater;
            else if (this->Register < other.Register)
                return std::strong_ordering::less;
            else if(this->Register > other.Register)
                return std::strong_ordering::greater;
            else // Space and Register are equal.
                return std::strong_ordering::equal;
            // NOLINTEND(bugprone-branch-clone)
        }

        /// @brief Implements equality comparison for descriptor binding points.
        ///
        /// @param other The other binding point to compare against.
        /// @return `true`, if the `Space` and `Register` values for both binding points are equal, otherwise `false`.
        inline bool operator==(const DescriptorBindingPoint& other) const noexcept {
            return other.Space == this->Space && other.Register == this->Register;
        }
    };

    /// @brief Represents a single shader module, i.e. a part of a @ref IShaderProgram.
    ///
    /// A shader module corresponds to a single shader source file.
    ///
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_RENDERING_API IShaderModule {
    protected:
        IShaderModule() noexcept = default;
        IShaderModule(const IShaderModule&) = default;
        IShaderModule(IShaderModule&&) noexcept = default;
        IShaderModule& operator=(const IShaderModule&) = default;
        IShaderModule& operator=(IShaderModule&&) noexcept = default;

    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        /// @brief Returns the type of the shader module.
        ///
        /// @return The type of the shader module.
        virtual ShaderStage type() const noexcept = 0;

        /// @brief Returns the file name of the shader module.
        ///
        /// @return The file name of the shader module.
        virtual const String& fileName() const noexcept = 0;

        /// @brief Returns the name of the shader module entry point.
        ///
        /// @return The name of the shader module entry point.
        virtual const String& entryPoint() const noexcept = 0;

        /// @brief For ray-tracing shader modules returns the binding point for the descriptor that receives shader-local data.
        ///
        /// Ray-tracing shaders, especially hit and intersection shaders may rely on local per-invocation data to handle ray intersections. One prominent example of such data is a custom index that identifies the
        /// geometry within the instance that has been hit, which can then be used to index into bindless arrays to acquire additional data, such as material properties or texture maps. This data is placed
        /// alongside the shader binding table created from a @ref ShaderRecordCollection and passed to the shader when it is invoked. However, when building the @ref IPipelineLayout for a ray-tracing pipeline,
        /// the device needs to know which descriptors bind globally and which descriptor binds locally. This information currently cannot be reliably acquired by shader reflection and must thus be specified on a
        /// per-module basis.
        ///
        /// Note that it is only possible for one descriptor to bind to local data. However, this descriptor can bind a constant/uniform buffer that contains multiple variables. Whilst it is possible to bind
        /// buffer references (using @ref IDeviceMemory::virtualAddress), support for it is differs depending on the shader language. To keep shaders portable, it is recommended to use descriptor indexing to bind
        /// buffers and textures and only pass constant values into local descriptor bindings.
        ///
        /// For shader modules of types other than ray-tracing, this setting is ignored.
        ///
        /// @return Returns the binding point for the descriptor that receives shader-local data.
        /// @see ShaderRecord
        virtual const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept = 0;
    };

    /// @brief Represents a render target, i.e. an abstract view of the output of an @ref RenderPass.
    ///
    /// A render target represents one output of a render pass, stored within an @ref IImage. It is contained by a @ref RenderPass, that maps it to an image resource on the @ref FrameBuffer the render pass
    /// operates on. The @ref IRenderTarget::identifier is used to associate an image within a frame buffer to a render target.
    ///
    /// When using a @ref IRenderPipeline during rendering, a similar lookup is performed to bind frame buffer images to input attachments.
    ///
    /// @see RenderTarget
    /// @see RenderPass
    /// @see FrameBuffer
    /// @see IImage
    class LITEFX_RENDERING_API IRenderTarget {
    public:
        /// @brief Describes the blend state of the render target.
        struct BlendState {
        public:
            /// @brief Specifies, if the target should be blended (default: `false`).
            bool Enable{ false };

            /// @brief The blend factor for the source color channels (default: `BlendFactor::One`).
            BlendFactor SourceColor{ BlendFactor::One };

            /// @brief The blend factor for the destination color channels (default: `BlendFactor::Zero`).
            BlendFactor DestinationColor{ BlendFactor::Zero };

            /// @brief The blend factor for the source alpha channel (default: `BlendFactor::One`).
            BlendFactor SourceAlpha{ BlendFactor::One };

            /// @brief The blend factor for the destination alpha channels (default: `BlendFactor::Zero`).
            BlendFactor DestinationAlpha{ BlendFactor::Zero };

            /// @brief The blend operation for the color channels (default: `BlendOperation::Add`).
            BlendOperation ColorOperation{ BlendOperation::Add };

            /// @brief The blend operation for the alpha channel (default: `BlendOperation::Add`).
            BlendOperation AlphaOperation{ BlendOperation::Add };

            /// @brief The channel write mask, determining which channels are written to (default: `WriteMask::R | WriteMask::G | WriteMask::B | WriteMask::A`).
            WriteMask ChannelWriteMask{ WriteMask::R | WriteMask::G | WriteMask::B | WriteMask::A };
        };

    protected:
        IRenderTarget() noexcept = default;
        IRenderTarget(const IRenderTarget&) = default;
        IRenderTarget(IRenderTarget&&) noexcept = default;
        IRenderTarget& operator=(const IRenderTarget&) = default;
        IRenderTarget& operator=(IRenderTarget&&) noexcept = default;

    public:
        virtual ~IRenderTarget() noexcept = default;

    public:
        /// @brief A unique identifier for the render target.
        ///
        /// @return The unique identifier for the render target.
        virtual UInt64 identifier() const noexcept = 0;

        /// @brief Returns the name of the render target.
        ///
        /// @return The name of the render target.
        virtual const String& name() const noexcept = 0;

        /// @brief Returns the location of the render target output attachment within the fragment shader.
        ///
        /// The locations of all render targets of a frame buffer must be within a continuous domain, starting at `0`. A frame buffer validates the render target locations when it is initialized and will raise an
        /// exception, if a location is either not mapped or assigned multiple times.
        ///
        /// @return The location of the render target output attachment within the fragment shader
        virtual UInt32 location() const noexcept = 0;

        /// @brief Returns the type of the render target.
        ///
        /// @return The type of the render target.
        virtual RenderTargetType type() const noexcept = 0;

        /// @brief Returns the internal format of the render target.
        ///
        /// @return The internal format of the render target.
        virtual Format format() const noexcept = 0;

        /// @brief Returns the flags that control the behavior of the render target.
        ///
        /// @return The flags that control the behavior of the render target.
        virtual RenderTargetFlags flags() const noexcept = 0;

        /// @brief Returns `true`, if the render target should be cleared, when the render pass is started. If the @ref format is set to a depth format, this clears the depth buffer. Otherwise it clears the color
        /// buffer.
        ///
        /// @return `true`, if the render target should be cleared, when the render pass is started
        /// @see clearStencil
        /// @see clearValues
        /// @see flags
        /// @see RenderTargetFlags
        virtual bool clearBuffer() const noexcept = 0;

        /// @brief Returns `true`, if the render target stencil should be cleared, when the render pass is started. If the @ref format is does not contain a stencil channel, this has no effect.
        ///
        /// @return `true`, if the render target stencil should be cleared, when the render pass is started
        /// @see clearStencil
        /// @see clearValues
        /// @see flags
        /// @see RenderTargetFlags
        virtual bool clearStencil() const noexcept = 0;

        /// @brief Returns the value, the render target is cleared with, if @ref clearBuffer either or @ref clearStencil is specified.
        ///
        /// If the @ref format is a color format and @ref clearBuffer is specified, this contains the clear color. However, if the format is a depth/stencil format, the R and G channels contain the depth and
        /// stencil value to clear the buffer with. Note that the stencil buffer is only cleared, if @ref clearStencil is specified and vice versa.
        ///
        /// @return The value, the render target is cleared with, if @ref clearBuffer either or @ref clearStencil is specified.
        virtual const Vector4f& clearValues() const noexcept = 0;

        /// @brief Returns `true`, if the target should not be made persistent for access after the render pass has finished.
        ///
        /// A render target can be marked as volatile if it does not need to be accessed after the render pass has finished. This can be used to optimize away unnecessary GPU/CPU memory round-trips. For example a
        /// depth buffer may only be used as an input for the lighting stage of a deferred renderer, but is not required after this. So instead of reading it from the GPU after the lighting pass has finished and
        /// then discarding it anyway, it can be marked as volatile in order to prevent it from being read from the GPU memory again in the first place.
        ///
        /// @return `true`, if the target should not be made persistent for access after the render pass has finished.
        /// @see flags
        /// @see RenderTargetFlags
        virtual bool isVolatile() const noexcept = 0;

        /// @brief Returns the render targets blend state.
        ///
        /// @return The render targets blend state.
        virtual const BlendState& blendState() const noexcept = 0;
    };

    /// @brief Implements a render target.
    ///
    /// @ref IRenderTarget
    class LITEFX_RENDERING_API RenderTarget : public IRenderTarget {
        LITEFX_IMPLEMENTATION(RenderTargetImpl);

    public:
        /// @brief Initializes the render target.
        ///
        /// @param uid A unique identifier for the render target.
        /// @param location The location of the render target output attachment.
        /// @param type The type of the render target.
        /// @param format The format of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param clearValues The values with which the render target gets cleared.
        /// @param blendState The render target blend state.
        explicit RenderTarget(UInt64 uid, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const BlendState& blendState = {});

        /// @brief Initializes the render target.
        ///
        /// This overload uses the @p name parameter to compute the @ref identifier.
        ///
        /// @param name The unique name of the render target.
        /// @param location The location of the render target output attachment.
        /// @param type The type of the render target.
        /// @param format The format of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param clearValues The values with which the render target gets cleared.
        /// @param blendState The render target blend state.
        explicit RenderTarget(StringView name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.f , 0.f, 0.f, 0.f }, const BlendState& blendState = {});
        
        /// @brief Creates a copy of a render target.
        ///
        /// @param _other The render target instance to copy.
        RenderTarget(const RenderTarget& _other);

        /// @brief Takes over another instance of a render target.
        ///
        /// @param _other The render target instance to take over.
        RenderTarget(RenderTarget&& _other) noexcept;

        /// @brief Assigns a render target by copying it.
        ///
        /// @param _other The render target instance to copy.
        /// @return A reference to the current render target instance.
        RenderTarget& operator=(const RenderTarget& _other);

        /// @brief Assigns a render target by taking it over.
        ///
        /// @param _other The render target to take over.
        /// @return A reference to the current render target instance.
        RenderTarget& operator=(RenderTarget&& _other) noexcept;
        
        /// @brief Releases the render target instance.
        ~RenderTarget() noexcept override;

    public:
        /// @copydoc IRenderTarget::identifier
        UInt64 identifier() const noexcept override;

        /// @copydoc IRenderTarget::name
        const String& name() const noexcept override;

        /// @copydoc IRenderTarget::location
        UInt32 location() const noexcept override;

        /// @copydoc IRenderTarget::type
        RenderTargetType type() const noexcept override;

        /// @copydoc IRenderTarget::format
        Format format() const noexcept override;

        /// @copydoc IRenderTarget::flags
        RenderTargetFlags flags() const noexcept override;

        /// @copydoc IRenderTarget::clearBuffer
        bool clearBuffer() const noexcept override;

        /// @copydoc IRenderTarget::clearStencil
        bool clearStencil() const noexcept override;

        /// @copydoc IRenderTarget::clearValues
        const Vector4f& clearValues() const noexcept override;

        /// @copydoc IRenderTarget::isVolatile
        bool isVolatile() const noexcept override;

        /// @copydoc IRenderTarget::blendState
        const BlendState& blendState() const noexcept override;
    };

    /// @brief Represents a mapping between a set of @ref RenderTarget instances and the input attachments of a @ref IRenderPass.
    class LITEFX_RENDERING_API RenderPassDependency {
        LITEFX_IMPLEMENTATION(RenderPassDependencyImpl);

    public:
        /// @brief Creates a new render target dependency.
        ///
        /// @param renderTarget The render target of the @p renderPass that is used for the input attachment.
        /// @param descriptorBinding The binding point to bind the input attachment to.
        RenderPassDependency(const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) noexcept;

        /// @brief Creates a new render target dependency.
        ///
        /// @param renderTarget The render target of the @p renderPass that is used for the input attachment.
        /// @param bindingRegister The register to bind the input attachment to.
        /// @param space The space to bind the input attachment to.
        RenderPassDependency(const RenderTarget& renderTarget, UInt32 bindingRegister, UInt32 space) noexcept;

        /// @brief Creates a copy of another render pass dependency.
        ///
        /// @param _other The render pass dependency to copy.
        RenderPassDependency(const RenderPassDependency& _other);

        /// @brief Takes over another render pass dependency instance.
        ///
        /// @param _other The render pass dependency instance to take over.
        RenderPassDependency(RenderPassDependency&& _other) noexcept;

        /// @brief Assigns another render pass dependency instance by copying it.
        ///
        /// @param _other The render pass dependency to copy.
        /// @return A reference of the current render pass dependency instance.
        RenderPassDependency& operator=(const RenderPassDependency& _other);

        /// @brief Assigns another render pass dependency by taking it over.
        ///
        /// @param _other The render pass dependency to take over.
        /// @return A reference of the current render pass dependency instance.
        RenderPassDependency& operator=(RenderPassDependency&& _other) noexcept;

        /// @brief Releases the current render pass dependency instance.
        ~RenderPassDependency() noexcept;

    public:
        /// @brief Returns a reference of the render target that is mapped to the input attachment.
        ///
        /// @return A reference of the render target that is mapped to the input attachment.
        const RenderTarget& renderTarget() const noexcept;

        /// @brief Returns the binding point for the input attachment binding.
        ///
        /// @return The binding point for the input attachment binding.
        const DescriptorBindingPoint& binding() const noexcept;
    };

    /// @brief Stores the depth/stencil state of a see @ref IRasterizer.
    class LITEFX_RENDERING_API DepthStencilState final {
        LITEFX_IMPLEMENTATION(DepthStencilStateImpl);

    public:
        /// @brief Describes the rasterizer depth state.
        struct DepthState {
        public:
            /// @brief Specifies, if depth testing should be enabled (default: `true`).
            bool Enable{ true };

            /// @brief Specifies, if depth should be written (default: `true`).
            bool Write{ true };

            /// @brief The compare operation used to pass the depth test (default: `CompareOperation::Always`).
            CompareOperation Operation{ CompareOperation::Always };

            /// @brief Enables the depth bounds test.
            ///
            /// Enabling the depth bounds test allows to supply a depth range to the command buffer by calling @ref ICommandBuffer::setDepthBounds, which will cause an rasterization operation to exit early, if the
            /// resulting depth lies outside of the provided range. To use this test, the @ref GraphicsDeviceFeatures::DepthBoundsTest must be enabled on the device.
            ///
            /// @see GraphicsDeviceFeatures::DepthBoundsTest
            /// @see ICommandBuffer::setDepthBounds
            bool DepthBoundsTestEnable{ false };
        };

        /// @brief Describes the rasterizer depth bias.
        ///
        /// The depth bias can be used to alter the depth value function, i.e. how the values within the depth buffer are distributed. By default, the depth buffer uses an exponential function scale to increase
        /// precision for closer objects. The values provided with @ref depthBiasClamp, @ref depthBiasConstantFactor and @ref depthBiasSlopeFactor are used to change the domain clamping, offset and steepness of
        /// the depth value distribution.
        struct DepthBias {
        public:
            /// @brief Specifies, if depth bias should be used (default: `false`).
            bool Enable{ false };

            /// @brief Specifies the depth bias clamp (default: `0.0`).
            Float Clamp{ 0.f };

            /// @brief Specifies the depth bias slope factor (default: `0.0`).
            Float SlopeFactor{ 0.f };

            /// @brief Specifies the depth bias constant factor (default: `0.0`).
            Float ConstantFactor{ 0.f };
        };

        /// @brief Describes a stencil test for either front or back faces.
        struct StencilTest {
        public:
            /// @brief The operation to apply to the stencil buffer, if the stencil test fails (default: `StencilOperation::Keep`).
            StencilOperation StencilFailOp{ StencilOperation::Keep };

            /// @brief The operation to apply to the stencil buffer, if the stencil test passes (default: `StencilOperation::Keep`).
            StencilOperation StencilPassOp{ StencilOperation::Replace };

            /// @brief The operation to apply to the stencil buffer, if the depth test fails (default: `StencilOperation::Keep`).
            StencilOperation DepthFailOp{ StencilOperation::Keep };

            /// @brief The operation use for stencil testing (default: `CompareOperation::Never`).
            CompareOperation Operation{ CompareOperation::Never };
        };

        /// @brief Describes the rasterizer stencil state.
        struct StencilState {
        public:
            /// @brief Specifies, if stencil state should be used (default: `false`).
            bool Enable{ false };

            /// @brief Specifies the bits to write to the stencil state (default: `0xFF`).
            UInt8 WriteMask{ 0xFF }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            /// @brief Specifies the bits to read from the stencil state (default: `0xFF`).
            UInt8 ReadMask{ 0xFF }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            /// @brief Describes the stencil test for faces that point towards the camera.
            StencilTest FrontFace{};

            /// @brief Describes the stencil test for faces that point away from the camera.
            StencilTest BackFace{};
        };

    public:
        /// @brief Initializes a new rasterizer depth/stencil state.
        ///
        /// @param depthState The depth state of the rasterizer.
        /// @param depthBias The depth bias configuration of the rasterizer.
        /// @param stencilState The stencil state of the rasterizer.
        explicit DepthStencilState(const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) noexcept;

        /// @brief Initializes a new rasterizer depth/stencil state.
        DepthStencilState() noexcept;

        /// @brief Creates a copy of a depth/stencil state.
        DepthStencilState(const DepthStencilState&);

        /// @brief Moves a depth/stencil state.
        DepthStencilState(DepthStencilState&&) noexcept;

        /// @brief Copies a depth/stencil state.
        ///
        /// @return A reference to the current depth/stencil state instance.
        DepthStencilState& operator=(const DepthStencilState&);

        /// @brief Moves a depth/stencil state.
        ///
        /// @return A reference to the current depth/stencil state instance.
        DepthStencilState& operator=(DepthStencilState&&) noexcept;

        /// @brief Destroys a depth/stencil state.
        ~DepthStencilState() noexcept;

    public:
        /// @brief Returns the depth state.
        ///
        /// @return The depth state.
        DepthState& depthState() const noexcept;

        /// @brief Returns the depth bias.
        ///
        /// @return The depth bias.
        DepthBias& depthBias() const noexcept;

        /// @brief Returns the stencil state.
        ///
        /// @return The stencil state.
        StencilState& stencilState() const noexcept;
    };

    /// @brief Represents the rasterizer state of a @ref RenderPipeline.
    class LITEFX_RENDERING_API IRasterizer : public SharedObject {
    protected:
        IRasterizer() noexcept = default;
        IRasterizer(const IRasterizer&) = default;
        IRasterizer(IRasterizer&&) noexcept = default;
        IRasterizer& operator=(const IRasterizer&) = default;
        IRasterizer& operator=(IRasterizer&&) noexcept = default;

    public:
        ~IRasterizer() noexcept override = default;

    public:
        /// @brief Returns the polygon mode of the rasterizer state.
        ///
        /// @return The polygon mode of the rasterizer state.
        virtual PolygonMode polygonMode() const noexcept = 0;

        /// @brief Returns the cull mode of the rasterizer state.
        ///
        /// @return The cull mode of the rasterizer state.
        virtual CullMode cullMode() const noexcept = 0;

        /// @brief Returns the cull mode of the rasterizer state.
        ///
        /// @return The cull mode of the rasterizer state.
        virtual CullOrder cullOrder() const noexcept = 0;

        /// @brief Returns the line width of the rasterizer state.
        ///
        /// Note that line width is not supported in DirectX and is only emulated under Vulkan. Instead of forcing this value, it is recommended to use a custom shader for it.
        ///
        /// @return The line width of the rasterizer state.
        virtual Float lineWidth() const noexcept = 0;

        /// @brief Returns the depth/stencil state of the rasterizer.
        ///
        /// @return The depth/stencil state of the rasterizer.
        virtual const DepthStencilState& depthStencilState() const noexcept = 0;

        /// @brief Returns `true`, if z-clipping should be used during distance clipping.
        ///
        /// @return `true`, if z-clipping should be used during distance clipping and `false` otherwise.
        virtual bool depthClip() const noexcept = 0;

        /// @brief Returns `true`, if conservative rasterization is enabled and `false` otherwise.
        ///
        /// This setting requires the @ref GraphicsDeviceFeatures::ConservativeRasterization feature to be enabled.
        ///
        /// @return `true`, if conservative rasterization is enabled and `false` otherwise.
        virtual bool conservativeRasterization() const noexcept = 0;
    };

    /// @brief Implements a @ref IRasterizer.
    class LITEFX_RENDERING_API Rasterizer : public IRasterizer {
        LITEFX_IMPLEMENTATION(RasterizerImpl);

    protected:
        /// @brief Initializes a new rasterizer instance.
        ///
        /// @param polygonMode The polygon mode of the rasterizer state.
        /// @param cullMode The cull mode of the rasterizer state.
        /// @param cullOrder The cull order of the rasterizer state.
        /// @param lineWidth The line width of the rasterizer state.
        /// @param depthClip The depth clip toggle of the rasterizer state.
        /// @param depthStencilState The rasterizer depth/stencil state.
        /// @param conservativeRasterization Toggles the use of conservative rasterization in the rasterizer.
        explicit Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, bool depthClip = true, const DepthStencilState& depthStencilState = {}, bool conservativeRasterization = false) noexcept;

        /// @brief Creates a copy of a rasterizer.
        ///
        /// @param _other The rasterizer instance to copy.
        Rasterizer(const Rasterizer& _other);

        /// @brief Takes over another instance of a rasterizer.
        ///
        /// @param _other The rasterizer instance to take over.
        Rasterizer(Rasterizer&& _other) noexcept;

        /// @brief Assigns a rasterizer by copying it.
        ///
        /// @param _other The rasterizer instance to copy.
        /// @return A reference to the current rasterizer instance.
        Rasterizer& operator=(const Rasterizer& _other);

        /// @brief Assigns a rasterizer by taking it over.
        ///
        /// @param _other The rasterizer to take over.
        /// @return A reference to the current rasterizer instance.
        Rasterizer& operator=(Rasterizer&& _other) noexcept;

    public:
        /// @brief Releases the rasterizer instance.
        ~Rasterizer() noexcept override;

    public:
        /// @copydoc IRasterizer::polygonMode
        PolygonMode polygonMode() const noexcept override;

        /// @copydoc IRasterizer::cullMode
        CullMode cullMode() const noexcept override;

        /// @copydoc IRasterizer::cullOrder
        CullOrder cullOrder() const noexcept override;

        /// @copydoc IRasterizer::lineWidth
        Float lineWidth() const noexcept override;

        /// @copydoc IRasterizer::depthStencilState
        const DepthStencilState& depthStencilState() const noexcept override;

        /// @copydoc IRasterizer::depthClip
        bool depthClip() const noexcept override;

        /// @copydoc IRasterizer::conservativeRasterization
        bool conservativeRasterization() const noexcept override;

    protected:
        virtual PolygonMode& polygonMode() noexcept;
        virtual CullMode& cullMode() noexcept;
        virtual CullOrder& cullOrder() noexcept;
        virtual Float& lineWidth() noexcept;
        virtual bool& depthClip() noexcept;
        virtual DepthStencilState& depthStencilState() noexcept;
        virtual bool& conservativeRasterization() noexcept;
    };

    /// @brief Interface for a viewport.
    class LITEFX_RENDERING_API IViewport {
    protected:
        IViewport() noexcept = default;
        IViewport(const IViewport&) = default;
        IViewport(IViewport&&) noexcept = default;
        IViewport& operator=(const IViewport&) = default;
        IViewport& operator=(IViewport&&) noexcept = default;

    public:
        virtual ~IViewport() noexcept = default;

    public:
        /// @brief Gets the rectangle that defines the dimensions of the viewport.
        ///
        /// @return The rectangle that defines the dimensions of the viewport.
        virtual RectF getRectangle() const noexcept = 0;

        /// @brief Sets the rectangle that defines the dimensions of the viewport.
        ///
        /// @param rectangle The rectangle that defines the dimensions of the viewport.
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;

        /// @brief Gets the minimum depth of the viewport.
        ///
        /// @return The minimum depth of the viewport.
        virtual float getMinDepth() const noexcept = 0;

        /// @brief Sets the minimum depth of the viewport.
        ///
        /// @param depth The minimum depth of the viewport.
        virtual void setMinDepth(Float depth) const noexcept = 0;

        /// @brief Gets the maximum depth of the viewport.
        ///
        /// @return The maximum depth of the viewport.
        virtual float getMaxDepth() const noexcept = 0;

        /// @brief Sets the maximum depth of the viewport.
        ///
        /// @param depth The maximum depth of the viewport.
        virtual void setMaxDepth(Float depth) const noexcept = 0;
    };

    /// @brief Implements a viewport.
    class LITEFX_RENDERING_API Viewport : public IViewport {
        LITEFX_IMPLEMENTATION(ViewportImpl);

    public:
        /// @brief Initializes a new viewport.
        ///
        /// @param clientRect The rectangle that defines the dimensions of the viewport.
        /// @param minDepth The minimum depth of the viewport.
        /// @param maxDepth The maximum depth of the viewport.
        explicit Viewport(const RectF& clientRect = { }, Float minDepth = 0.f, Float maxDepth = 1.f) noexcept;

        /// @brief Creates a copy of a viewport.
        ///
        /// @param _other The viewport instance to copy.
        Viewport(const Viewport& _other);

        /// @brief Takes over another instance of a viewport.
        ///
        /// @param _other The viewport instance to take over.
        Viewport(Viewport&& _other) noexcept;

        /// @brief Assigns a viewport by copying it.
        ///
        /// @param _other The viewport instance to copy.
        /// @return A reference to the current viewport instance.
        Viewport& operator=(const Viewport& _other);

        /// @brief Assigns a viewport by taking it over.
        ///
        /// @param _other The viewport to take over.
        /// @return A reference to the current viewport instance.
        Viewport& operator=(Viewport&& _other) noexcept;

        /// @brief Releases the render target instance.
        ~Viewport() noexcept override;

    public:
        /// @copydoc IViewport::getRectangle
        RectF getRectangle() const noexcept override;

        /// @copydoc IViewport::setRectangle
        void setRectangle(const RectF& rectangle) noexcept override;

        /// @copydoc IViewport::getMinDepth
        Float getMinDepth() const noexcept override;

        /// @copydoc IViewport::setMinDepth
        void setMinDepth(Float depth) const noexcept override;

        /// @copydoc IViewport::getMaxDepth
        Float getMaxDepth() const noexcept override;

        /// @copydoc IViewport::setMaxDepth
        void setMaxDepth(Float depth) const noexcept override;
    };

    /// @brief The interface of a scissor.
    class LITEFX_RENDERING_API IScissor {
    protected:
        IScissor() noexcept = default;
        IScissor(IScissor&&) noexcept = default;
        IScissor(const IScissor&) = default;
        IScissor& operator=(IScissor&&) noexcept = default;
        IScissor& operator=(const IScissor&) = default;

    public:
        virtual ~IScissor() noexcept = default;

    public:
        /// @brief Gets the rectangle that defines the scissor region.
        ///
        /// @return The rectangle that defines the scissor region.
        virtual RectF getRectangle() const noexcept = 0;

        /// @brief Sets the rectangle that defines the scissor region.
        ///
        /// @param rectangle The rectangle that defines the scissor region.
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
    };

    /// @brief Implements a scissor.
    class LITEFX_RENDERING_API Scissor final : public IScissor {
        LITEFX_IMPLEMENTATION(ScissorImpl);

    public:
        /// @brief Initializes a new scissor.
        ///
        /// @param scissorRect The rectangle that defines the scissor region.
        explicit Scissor(const RectF& scissorRect = { }) noexcept;

        /// @brief Creates a copy of a scissor.
        ///
        /// @param _other The scissor instance to copy.
        Scissor(const Scissor& _other);

        /// @brief Takes over another instance of a scissor.
        ///
        /// @param _other The scissor instance to take over.
        Scissor(Scissor&& _other) noexcept;

        /// @brief Assigns a scissor by copying it.
        ///
        /// @param _other The scissor instance to copy.
        /// @return A reference to the current scissor instance.
        Scissor& operator=(const Scissor& _other);

        /// @brief Assigns a scissor by taking it over.
        ///
        /// @param _other The scissor to take over.
        /// @return A reference to the current scissor instance.
        Scissor& operator=(Scissor&& _other) noexcept;

        /// @brief Releases the scissor instance.
        ~Scissor() noexcept override;

    public:
        /// @copydoc IScissor::getRectangle
        RectF getRectangle() const noexcept override;

        /// @copydoc IScissor::setRectangle
        void setRectangle(const RectF& rectangle) noexcept override;
    };

    /// @brief Describes the offsets and sizes of a shader group within a shader binding table buffer.
    ///
    /// If a group is not present within a shader binding table, the offset for this group is set to the maximum possible value and the size is set to `0`.
    ///
    /// @see IRayTracingPipeline::allocateShaderBindingTable
    struct LITEFX_RENDERING_API ShaderBindingTableOffsets {
        /// @brief The offset to the beginning of the ray generation group within the shader binding table.
        UInt64 RayGenerationGroupOffset { std::numeric_limits<UInt64>::max() };

        /// @brief The size of the ray generation group within the shader binding table.
        UInt64 RayGenerationGroupSize { 0 };

        /// @brief The stride between individual ray generation group records in the shader binding table.
        UInt64 RayGenerationGroupStride { 0 };

        /// @brief The offset to the beginning of the hit group within the shader binding table.
        UInt64 HitGroupOffset { std::numeric_limits<UInt64>::max() };

        /// @brief The size of the hit group within the shader binding table.
        UInt64 HitGroupSize { 0 };

        /// @brief The stride between individual hit group records in the shader binding table.
        UInt64 HitGroupStride { 0 };

        /// @brief The offset to the beginning of the miss group within the shader binding table.
        UInt64 MissGroupOffset{ std::numeric_limits<UInt64>::max() };

        /// @brief The size of the miss group within the shader binding table.
        UInt64 MissGroupSize { 0 };

        /// @brief The stride between individual miss group records in the shader binding table.
        UInt64 MissGroupStride { 0 };

        /// @brief The offset to the beginning of the callable group within the shader binding table.
        UInt64 CallableGroupOffset { std::numeric_limits<UInt64>::max() };

        /// @brief The size of the callable group within the shader binding table.
        UInt64 CallableGroupSize { 0 };

        /// @brief The stride between individual callable group records in the shader binding table.
        UInt64 CallableGroupStride { 0 };
    };

    /// @brief An event that is used to measure timestamps in a command queue.
    ///
    /// Timing events are used to collect GPU time stamps asynchronously. A timing event can be inserted to a @ref ICommandBuffer by calling @ref ICommandBuffer::writeTimingEvent. This will cause the GPU to
    /// write the current time stamp when the command gets executed. Since command order is not preserved within command buffers, this is not guaranteed to be accurate an accurate point of time for when a
    /// certain command in order has been executed. However, since a time stamp is always written at the bottom of the pipe, the difference between two timestamps resembles the actual time the GPU was
    /// occupied with the commands between them.
    ///
    /// Timing events are asynchronous. They are set for a certain back buffer of a @ref ISwapChain. Reading the time stamp requires the back buffer to be ready (i.e., the frame in flight needs to have
    /// executed). The earliest point where this is guaranteed is, if the swap chain swaps to the back buffer again. This means that the time stamps issued in one frame can only be read the next time the
    /// frame's back buffer is used again.
    ///
    /// Note that timing events are only supported on graphics and compute @ref ICommandQueues.
    ///
    /// @see ISwapChain
    class LITEFX_RENDERING_API TimingEvent final : public SharedObject {
        LITEFX_IMPLEMENTATION(TimingEventImpl);
        friend class ISwapChain;
        friend struct SharedObject::Allocator<TimingEvent>;

    private:
        /// @brief Initializes a new timing event instance.
        ///
        /// @param swapChain The swap chain on which the timing event is registered.
        /// @param name The name of the timing event.
        explicit TimingEvent(const ISwapChain& swapChain, StringView name = "");

    public:
        /// @brief Releases the render target instance.
        ~TimingEvent() noexcept override;

        TimingEvent(TimingEvent&&) noexcept = delete;
        TimingEvent(const TimingEvent&) = delete;
        auto operator=(TimingEvent&&) noexcept = delete;
        auto operator=(const TimingEvent&) = delete;

    private:
        /// @brief Creates a new timing event instance.
        ///
        /// @param swapChain The swap chain on which the timing event is registered.
        /// @param name The name of the timing event.
        /// @return A shared pointer to the timing event instance.
        static inline auto create(const ISwapChain& swapChain, StringView name = "") {
            return SharedObject::create<TimingEvent>(swapChain, name);
        }

    public:
        /// @brief Gets the name of the timing event.
        ///
        /// @return The name of the timing event.
        StringView name() const noexcept;

        /// @brief Reads the current timestamp (as a tick count) of the event.
        ///
        /// In order to convert the number of ticks to (milli-)seconds, this value needs to be divided by @ref IGraphicsDevice::ticksPerMillisecond. To improve precision, calculate the difference between two time
        /// stamps in ticks first and only then convert them to seconds.
        ///
        /// @return The current time stamp of the event as a tick count.
        /// @throws RuntimeException Thrown, if the parent device instance is already released.
        /// @see ISwapChain::readTimingEvent
        UInt64 readTimestamp() const;

        /// @brief Returns the query ID for the timing event.
        ///
        /// @return The query ID for the timing event.
        /// @throws RuntimeException Thrown, if the parent device instance is already released.
        /// @see ISwapChain::resolveQueryId
        UInt32 queryId() const;
    };

    /// @brief Stores meta data about a buffer attribute, i.e. a member or field of a descriptor or buffer.
    class LITEFX_RENDERING_API BufferAttribute final {
        LITEFX_IMPLEMENTATION(BufferAttributeImpl);

    public:
        /// @brief Initializes an empty buffer attribute.
        BufferAttribute() noexcept;

        /// @brief Initializes a new buffer attribute.
        ///
        /// @param location The location the buffer attribute is bound to.
        /// @param offset The offset of the attribute relative to the buffer.
        /// @param format The format of the buffer attribute.
        /// @param semantic The semantic of the buffer attribute.
        /// @param semanticIndex The semantic index of the buffer attribute.
        BufferAttribute(UInt32 location, UInt32 offset, BufferFormat format, AttributeSemantic semantic, UInt32 semanticIndex = 0) noexcept;

        /// @brief Creates a copy of a buffer attribute.
        ///
        /// @param _other The buffer attribute instance to copy.
        BufferAttribute(const BufferAttribute& _other);

        /// @brief Takes over another instance of a buffer attribute.
        ///
        /// @param _other The buffer attribute instance to take over.
        BufferAttribute(BufferAttribute&& _other) noexcept;

        /// @brief Assigns a buffer attribute by copying it.
        ///
        /// @param _other The buffer attribute instance to copy.
        /// @return A reference to the current buffer attribute instance.
        BufferAttribute& operator=(const BufferAttribute& _other);

        /// @brief Assigns a buffer attribute by taking it over.
        ///
        /// @param _other The buffer attribute to take over.
        /// @return A reference to the current buffer attribute instance.
        BufferAttribute& operator=(BufferAttribute&& _other) noexcept;

        /// @brief Releases the buffer attribute instance.
        ~BufferAttribute() noexcept;

    public:
        /// @brief Returns the location of the buffer attribute.
        ///
        /// Locations can only be specified in Vulkan and are implicitly generated based on semantics for DirectX. However, it is a good practice to provide them anyway.
        ///
        /// @return The location of the buffer attribute.
        UInt32 location() const noexcept;

        /// @brief Returns the format of the buffer attribute.
        ///
        /// @return The format of the buffer attribute.
        BufferFormat format() const noexcept;

        /// @brief Returns the offset of the buffer attribute.
        ///
        /// @return The offset of the buffer attribute.
        UInt32 offset() const noexcept;

        /// @brief Returns the semantic of the buffer attribute.
        ///
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        ///
        /// @return The semantic of the buffer attribute.
        /// @see semanticIndex
        AttributeSemantic semantic() const noexcept;

        /// @brief Returns the semantic index of the buffer attribute.
        ///
        /// Semantics are only used in DirectX and HLSL, however it is a good practice to provide them anyway.
        ///
        /// @return The semantic index of the buffer attribute.
        /// @see semantic
        UInt32 semanticIndex() const noexcept;
    };

    /// @brief Describes a buffer layout.
    ///
    /// @see IVertexBufferLayout
    /// @see IIndexBufferLayout
    /// @see IDescriptorLayout
    class LITEFX_RENDERING_API IBufferLayout {
    protected:
        IBufferLayout() noexcept = default;
        IBufferLayout(IBufferLayout&&) noexcept = default;
        IBufferLayout(const IBufferLayout&) = default;
        IBufferLayout& operator=(IBufferLayout&&) noexcept = default;
        IBufferLayout& operator=(const IBufferLayout&) = default;

    public:
        virtual ~IBufferLayout() noexcept = default;

    public:
        /// @brief Returns the size of a single element within the buffer.
        ///
        /// @return The size of a single element within the buffer.
        virtual size_t elementSize() const noexcept = 0;

        /// @brief Returns the binding point, the buffer will be bound to.
        ///
        /// In GLSL, the binding point is identified by the `binding` keyword, whilst in HLSL the binding maps to a register.
        ///
        /// @return The binding point, the buffer will be bound to.
        virtual UInt32 binding() const noexcept = 0;

        /// @brief Returns the buffer type of the buffer.
        ///
        /// @return The buffer type of the buffer.
        virtual BufferType type() const noexcept = 0;
    };

    /// @brief Describes a vertex buffer layout.
    ///
    /// @see IVertexBuffer
    class LITEFX_RENDERING_API IVertexBufferLayout : public IBufferLayout, public SharedObject {
    protected:
        IVertexBufferLayout() noexcept = default;
        IVertexBufferLayout(IVertexBufferLayout&&) noexcept = default;
        IVertexBufferLayout(const IVertexBufferLayout&) = default;
        IVertexBufferLayout& operator=(IVertexBufferLayout&&) noexcept = default;
        IVertexBufferLayout& operator=(const IVertexBufferLayout&) = default;

    public:
        ~IVertexBufferLayout() noexcept override = default;

    public:
        /// @brief Returns the vertex buffer attributes.
        ///
        /// @return The vertex buffer attributes.
        virtual const Array<BufferAttribute>& attributes() const = 0;

        /// @brief Returns the vertex buffer input rate that describes how the data is made available to the vertex shader.
        ///
        /// @return The vertex buffer input rate setting.
        virtual VertexBufferInputRate inputRate() const noexcept = 0;
    };

    /// @brief Describes a index buffer layout.
    ///
    /// @see IIndexBuffer
    class LITEFX_RENDERING_API IIndexBufferLayout : public IBufferLayout, public SharedObject {
    protected:
        IIndexBufferLayout() noexcept = default;
        IIndexBufferLayout(IIndexBufferLayout&&) noexcept = default;
        IIndexBufferLayout(const IIndexBufferLayout&) = default;
        IIndexBufferLayout& operator=(IIndexBufferLayout&&) noexcept = default;
        IIndexBufferLayout& operator=(const IIndexBufferLayout&) = default;

    public:
        ~IIndexBufferLayout() noexcept override = default;

    public:
        /// @brief Returns the index type of the index buffer.
        ///
        /// @return The index type of the index buffer.
        virtual IndexType indexType() const noexcept = 0;
    };

    /// @brief Describes a the layout of a single descriptor within a @ref DescriptorSet.
    ///
    /// A common metaphor for a descriptor to think of it as a "pointer for the GPU". Basically, a descriptor points to a buffer in a shader. A descriptor can have different types and sizes. The types a
    /// descriptor can have are described by the @ref DescriptorType.
    ///
    /// If the descriptor is a sampler, it can either be a dynamic or static sampler. A dynamic sampler needs to be bound during runtime just like any other descriptor by calling @ref IDescriptorSet::update.
    /// A static sampler is defined alongside the descriptor layout and is automatically set when the pipeline that uses the descriptor layout gets bound. In this case, the descriptor must not be updated with
    /// another sampler. If a descriptor layout describes a static sampler, the @ref IDescriptorLayout::staticSampler returns a pointer to the static sampler state.
    ///
    /// Typically, a descriptor "points" to a singular buffer, i.e. a scalar. However, a descriptor can also resemble an array. In this case, @ref IDescriptorLayout::descriptors returns the number of elements
    /// in the array. If the size of the array is not known beforehand, the descriptor can be defined as unbounded, causing the @ref IDescriptorLayout::unbounded property to return `true`. In this case, the
    /// number of descriptors defines the upper limit for the actual descriptor count that can be allocated for the array when calling @ref IDescriptorSetLayout::allocate.
    ///
    /// @see DescriptorSetLayout
    class LITEFX_RENDERING_API IDescriptorLayout : public IBufferLayout {
    protected:
        IDescriptorLayout() noexcept = default;
        IDescriptorLayout(IDescriptorLayout&&) noexcept = default;
        IDescriptorLayout(const IDescriptorLayout&) = default;
        IDescriptorLayout& operator=(IDescriptorLayout&&) noexcept = default;
        IDescriptorLayout& operator=(const IDescriptorLayout&) = default;

    public:
        ~IDescriptorLayout() noexcept override = default;

    public:
        /// @brief Returns the type of the descriptor.
        ///
        /// @return The type of the descriptor.
        virtual DescriptorType descriptorType() const noexcept = 0;

        /// @brief Returns the number of descriptors in the descriptor array.
        ///
        /// If @ref unbounded is set to `true`, the descriptor count defines the upper limit for the number of descriptors that can be allocated for in the array.
        ///
        /// @return The number of descriptors in the descriptor array.
        /// @see IDescriptorLayout
        virtual UInt32 descriptors() const noexcept = 0;

        /// @brief Returns `true`, if the descriptor defines an unbounded descriptor array.
        ///
        /// @return `true`, if the descriptor defines an unbounded descriptor array, `false` otherwise.
        /// @see descriptors
        virtual bool unbounded() const noexcept = 0;

        /// @brief If the descriptor describes a static sampler, this method returns the state of the sampler. Otherwise, it returns `nullptr`.
        ///
        /// Static samplers are called immutable samplers in Vulkan and describe sampler states, that are defined along the pipeline layout. While they do occupy a descriptor, they must not be bound explicitly.
        /// Instead, static samplers are automatically bound if the pipeline gets used. If a static sampler is set, the @ref descriptorType must be set to @ref DescriptorType::Sampler.
        ///
        /// @return The state of the static sampler, or `nullptr`, if the descriptor is not a static sampler.
        virtual const ISampler* staticSampler() const noexcept = 0;
    };

    /// @brief Allows for data to be mapped into the object.
    class LITEFX_RENDERING_API IMappable {
    protected:
        IMappable() noexcept = default;
        IMappable(IMappable&&) noexcept = default;
        IMappable(const IMappable&) = default;
        IMappable& operator=(IMappable&&) noexcept = default;
        IMappable& operator=(const IMappable&) = default;

    public:
        virtual ~IMappable() noexcept = default;

    public:
        /// @brief Maps the memory at @p data to the internal memory of this object.
        ///
        /// @param data The address that marks the beginning of the data to map.
        /// @param size The number of bytes to map.
        /// @param element The array element to map the data to.
        virtual void map(const void* const data, size_t size, UInt32 element = 0) = 0;

        /// @brief Maps the memory blocks within @p data to the internal memory of an array.
        ///
        /// @param data The data blocks to map.
        /// @param size The size of each data block within @p data.
        /// @param firstElement The first element of the array to map.
        virtual void map(Span<const void* const> data, size_t elementSize, UInt32 firstElement = 0) = 0;

        /// @brief Maps the memory at @p data to the internal memory of this object.
        ///
        /// @param data The address that marks the beginning of the data to map.
        /// @param size The number of bytes to map.
        /// @param element The array element to map the data to.
        /// @param write If `true`, @p data is copied into the internal memory. If `false` the internal memory is copied into @p data.
        virtual void map(void* data, size_t size, UInt32 element = 0, bool write = true) = 0;

        /// @brief Maps the memory blocks within @p data to the internal memory of an array.
        ///
        /// @param data The data blocks to map.
        /// @param size The size of each data block within @p data.
        /// @param firstElement The first element of the array to map.
        /// @param write If `true`, @p data is copied into the internal memory. If `false` the internal memory is copied into @p data.
        virtual void map(Span<void*> data, size_t elementSize, UInt32 firstElement = 0, bool write = true) = 0;

        /// @brief Writes a span of memory in @p data into the internal memory of this object, starting at @p offset.
        ///
        /// @param data The span of bytes containing the data to write.
        /// @param size The size of the memory block at @p data.
        /// @param offset The offset at which to start writing.
        virtual void write(const void* const data, size_t size, size_t offset = 0) = 0;

        /// @brief Writes a span of memory in @p data into the internal memory of this object, starting at @p offset.
        ///
        /// @param data The span of bytes containing the data to write.
        /// @param size The size of the memory block at @p data.
        /// @param offset The offset at which to start writing.
        virtual void read(void* data, size_t size, size_t offset = 0) = 0;
    };

    /// @brief Describes a chunk of device memory.
    class LITEFX_RENDERING_API IDeviceMemory {
    public:
        /// @brief Stores a reference to a barrier that can be used to synchronize accesses to the resource with a move operation.
        ///
        /// @see IDeviceMemory::prepareMove
        struct PrepareMoveEventArgs final {
        private:
            /// @brief Stores a reference to the underlying barrier.
            IBarrier& m_barrier;

        public:
            PrepareMoveEventArgs() = delete;
            PrepareMoveEventArgs(const PrepareMoveEventArgs&) = delete;
            PrepareMoveEventArgs(PrepareMoveEventArgs&&) noexcept = delete;
            PrepareMoveEventArgs& operator=(const PrepareMoveEventArgs&) = delete;
            PrepareMoveEventArgs& operator=(PrepareMoveEventArgs&&) noexcept = delete;
            ~PrepareMoveEventArgs() noexcept = default;
            
            /// @brief Creates a new instance of the event arguments.
            ///
            /// @param barrier A reference to the underlying barrier, that is used to synchronize the move operation with other accesses to the resource.
            PrepareMoveEventArgs(IBarrier& barrier) :
                m_barrier(barrier) { 
            }

        public:
            /// @brief Returns a reference of the barrier that is used to synchronize the move operation with other accesses to the resource.
            ///
            /// @return A reference to the underlying barrier.
            IBarrier& barrier() const noexcept {
                return m_barrier;
            }
        };

        /// @brief Stores the fence and the command queue to wait on for the fence before a moved resource can be used.
        ///
        /// @see IDeviceMemory::moving
        struct ResourceMovingEventArgs final {
        private:
            SharedPtr<const ICommandQueue> m_queue{};
            UInt64 m_fence{};

        public:
            /// @brief Creates a new instance of the resource moving event arguments.
            ///
            /// @param queue The queue that executes the resource move.
            /// @param fence The fence value on @p queue after which the resource can be used.
            ResourceMovingEventArgs(SharedPtr<const ICommandQueue> queue, UInt64 fence) noexcept :
                m_queue(std::move(queue)), m_fence(fence)
            {
            }

            ResourceMovingEventArgs(const ResourceMovingEventArgs&) = default;
            ResourceMovingEventArgs(ResourceMovingEventArgs&&) noexcept = default;
            ResourceMovingEventArgs& operator=(const ResourceMovingEventArgs&) = default;
            ResourceMovingEventArgs& operator=(ResourceMovingEventArgs&&) noexcept = default;
            ~ResourceMovingEventArgs() noexcept = default;

        public:
            /// @brief Returns the queue that executes the resource move.
            ///
            /// @return A pointer to the queue that executes the resource move.
            SharedPtr<const ICommandQueue> queue() const noexcept {
                return m_queue;
            }

            /// @brief Returns the fence on @ref queue after which the resource can be used.
            ///
            /// @return
            UInt64 fence() const noexcept {
                return m_fence;
            }
        };

    protected:
        IDeviceMemory() noexcept = default;
        IDeviceMemory(IDeviceMemory&&) noexcept = default;
        IDeviceMemory(const IDeviceMemory&) = default;
        IDeviceMemory& operator=(IDeviceMemory&&) noexcept = default;
        IDeviceMemory& operator=(const IDeviceMemory&) = default;

    public:
        virtual ~IDeviceMemory() noexcept = default;

    public:
        /// @brief An event that gets invoked to prepare a resource for a move operation.
        ///
        /// The purpose of this event is to prepare a barrier that synchronizes the resource with other accesses. The event arguments of this event contain a reference to a barrier instance, that can be used to
        /// insert a barrier for the resource. Note that the transition must be supported on the underlying command queue that executes the move.
        ///
        /// Note that both, the DirectX 12 as well as the Vulkan backend expect images to be in @ref ImageLayout::Common layout before moving them.
        ///
        /// @see IGraphicsFactory::defragment
        /// @see https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#command-queue-layout-compatibility
        mutable Event<const PrepareMoveEventArgs&> prepareMove;

        /// @brief An event that gets invoked before a resource is copied during a move.
        ///
        /// This event gets invoked during defragmentation to inform any subscribers about the relocation. Moving a resource involves an asynchronous copy-command. This event is invoked after this command has
        /// been submitted, but before it has been executed. The @ref moved event executes after the resource has been copied to the new location.
        ///
        /// Note that this event is invoked on the thread that executes the defragmentation process, which means you potentially might want to synchronize the handler with other potential resource accesses.
        ///
        /// @see moved
        /// @see IGraphicsFactory::defragment
        mutable Event<ResourceMovingEventArgs> moving;

        /// @brief An event that gets invoked, after the resource has been moved to a different location, but before the old resource gets destroyed.
        ///
        /// This event gets invoked during defragmentation to inform any subscribers about a relocation. You might want to subscribe to this event is to update any descriptor bindings, as they become invalid
        /// after the previous resource gets removed. Additionally, you may want to insert barriers to transition the resource back into the desired layout.
        ///
        /// Note that this event is invoked on the thread that executes the defragmentation process, which means you potentially might want to synchronize the handler with other resource accesses.
        ///
        /// @see moving
        /// @see IGraphicsFactory::defragment
        mutable Event<EventArgs> moved;

    public:
        /// @brief Gets the number of sub-resources inside the memory chunk.
        ///
        /// For buffers, this equals the number of array elements. For images, this equals the product of layers, levels and planes. This number represents the number of states, that can be obtained by calling
        /// the @ref state method.
        ///
        /// @return The number of array elements inside the memory chunk.
        /// @see state
        virtual UInt32 elements() const noexcept = 0;

        /// @brief Gets the size (in bytes) of the aligned memory chunk.
        ///
        /// The size of the device memory block depends on different factors. The actual used memory of one element can be obtained by calling by the @ref elementSize. For different reasons, though, elements may
        /// be required to be aligned to a certain size. The size of one aligned element is returned by @ref alignedElementSize. The size of the memory block, the elements get aligned to is returned by @ref
        /// elementAlignment.
        ///
        /// @return The size (in bytes) of the memory chunk.
        /// @see elements
        /// @see elementSize
        /// @see elementAlignment
        /// @see alignedElementSize
        virtual size_t size() const noexcept = 0;

        /// @brief Returns the size of a single element within the buffer. If there is only one element, this is equal to @ref size.
        ///
        /// For images, this method will return a value that equals the result of @ref size.
        ///
        /// @return The size of a single element within the buffer
        /// @see elementAlignment
        /// @see alignedElementSize
        virtual size_t elementSize() const noexcept = 0;

        /// @brief Returns the alignment of a single element.
        ///
        /// @return The alignment of a single element.
        /// @see elementSize
        /// @see alignedElementSize
        virtual size_t elementAlignment() const noexcept = 0;

        /// @brief Returns the actual size of the element in device memory.
        ///
        /// For images, this method will return a value that equals the result of @ref size, aligned by @ref elementAlignment.
        ///
        /// @return The actual size of the element in device memory.
        /// @see elementAlignment
        /// @see elementSize
        virtual size_t alignedElementSize() const noexcept = 0;

        /// @brief Returns the usage flags for the resource.
        ///
        /// @return The usage flags for the resource.
        virtual ResourceUsage usage() const noexcept = 0;

        /// @brief Gets the address of the resource in GPU memory.
        ///
        /// Note that this may not be supported for all resource types in all backends. For example, Vulkan does not support obtaining virtual addresses of image resources.
        ///
        /// @return The address of the resource in GPU memory.
        virtual UInt64 virtualAddress() const noexcept = 0;

        /// @brief Returns `true`, if the resource can be bound to a read/write descriptor.
        ///
        /// If the resource is not writable, attempting to bind it to a writable descriptor will result in an exception.
        ///
        /// @return `true`, if the resource can be bound to a read/write descriptor.
        virtual inline bool writable() const noexcept {
            return LITEFX_FLAG_IS_SET(this->usage(), ResourceUsage::AllowWrite);
        }

        /// @brief Returns `true`, if the contents of the resource should not be copied during a move.
        ///
        /// To set this flag, include @ref ResourceUsage::Volatile in the resource usage flags.
        ///
        /// @return `true`, if the contents of the resource should not be copied during a move and `false` otherwise.
        /// @see moving
        /// @see usage
        /// @see ResourceUsage::Volatile
        virtual inline bool volatileMove() const noexcept {
            return LITEFX_FLAG_IS_SET(this->usage(), ResourceUsage::Volatile);
        }
    };

    /// @brief Base interface for buffer objects.
    class LITEFX_RENDERING_API IBuffer : public virtual IDeviceMemory, public virtual IMappable, public virtual IStateResource, public SharedObject {
    protected:
        IBuffer() noexcept = default;
        IBuffer(IBuffer&&) noexcept = default;
        IBuffer(const IBuffer&) = delete;
        IBuffer& operator=(IBuffer&&) noexcept = default;
        IBuffer& operator=(const IBuffer&) = delete;

    public:
        ~IBuffer() noexcept override = default;

    public:
        /// @brief Returns the type of the buffer.
        ///
        /// @return The type of the buffer.
        virtual BufferType type() const noexcept = 0;
    };

    /// @brief Describes a generic image.
    class LITEFX_RENDERING_API IImage : public virtual IDeviceMemory, public virtual IStateResource, public SharedObject {
    public:
        using IDeviceMemory::size;

    protected:
        IImage() noexcept = default;
        IImage(IImage&&) noexcept = default;
        IImage(const IImage&) = delete;
        IImage& operator=(IImage&&) noexcept = default;
        IImage& operator=(const IImage&) = delete;

    public:
        ~IImage() noexcept override = default;

    public:
        /// @brief Returns the size (in bytes) of an image at a specified mip map level. If the image does not contain the provided mip map level, the method returns `0`.
        ///
        /// Note that the size will only be returned for one layer. You have to multiply this value by the number of layers, if you want to receive the size of all layers of a certain mip-map level. This is
        /// especially important, if you use cube mapping, because this method will only return the size of one face.
        ///
        /// @param level The mip map level to return the size for.
        /// @return The size (in bytes) of an image at a specified mip map level.
        virtual size_t size(UInt32 level) const = 0;

        /// @brief Gets the extent of the image at a certain mip-map level.
        ///
        /// Not all components of the extent are actually used. Check the @ref dimensions to see, which components are required. The extent will be 0 for invalid mip-map levels and 1 or more for valid mip map
        /// levels.
        ///
        /// @return The extent of the image at a certain mip-map level.
        /// @see dimensions
        virtual Size3d extent(UInt32 level = 0) const noexcept = 0;

        /// @brief Gets the internal format of the image.
        ///
        /// @return The internal format of the image.
        virtual Format format() const noexcept = 0;

        /// @brief Gets the images dimensionality.
        ///
        /// The dimensions imply various things, most importantly, which components of the @ref extent are used. Note that cube maps behave like 2D images when the extent is used.
        ///
        /// @return The images dimensionality.
        virtual ImageDimensions dimensions() const noexcept = 0;

        /// @brief Gets the number of mip-map levels of the image.
        ///
        /// @return The number of mip-map levels of the image.
        virtual UInt32 levels() const noexcept = 0;

        /// @brief Gets the number of layers (slices) of the image.
        ///
        /// @return The number of layers (slices) of the image.
        virtual UInt32 layers() const noexcept = 0;

        /// @brief Returns the number of planes of the image resource.
        ///
        /// The number of planes is dictated by the image format.
        ///
        /// @return The number of planes of the image resource.
        /// @see format
        virtual UInt32 planes() const noexcept = 0;

        /// @brief Gets the number of samples of the texture.
        ///
        /// @return The number of samples of the texture.
        virtual MultiSamplingLevel samples() const noexcept = 0;

        // TODO: getSampler() for combined samplers?

    public:
        /// @brief Returns the sub-resource ID for a combination of mip-map @p level, array @p layer and @p plane.
        ///
        /// @param level The mip map level of the sub-resource.
        /// @param layer The array layer of the sub-resource.
        /// @param plane The plane of the sub-resource.
        /// @return The sub-resource ID for the sub-resource.
        /// @see resolveSubresource
        virtual inline UInt32 subresourceId(UInt32 level, UInt32 layer, UInt32 plane) const noexcept {
            return level + (layer * this->levels()) + (plane * this->levels() * this->layers());
        }

        /// @brief Returns the @p plane, @p layer and @p level for the provided @p subresource.
        ///
        /// @param subresource The sub-resource ID.
        /// @param plane The plane index of the sub-resource.
        /// @param layer The array layer of the sub-resource.
        /// @param level The mip-map level of the sub-resource.
        /// @see subresourceId
        virtual inline void resolveSubresource(UInt32 subresource, UInt32& plane, UInt32& layer, UInt32& level) const noexcept {
            const auto levels = this->levels();
            const UInt32 resourcesPerPlane = levels * this->layers();
            plane = subresource / resourcesPerPlane;
            layer = (subresource % resourcesPerPlane) / levels;
            level = subresource % levels;
        }
    };

    /// @brief Describes a texture sampler.
    class LITEFX_RENDERING_API ISampler : public virtual IStateResource, public SharedObject {
    protected:
        ISampler() noexcept = default;
        ISampler(ISampler&&) noexcept = default;
        ISampler(const ISampler&) = delete;
        ISampler& operator=(ISampler&&) noexcept = default;
        ISampler& operator=(const ISampler&) = delete;

    public:
        ~ISampler() noexcept override = default;

    public:
        /// @brief Gets the filtering mode that is used for minifying lookups.
        ///
        /// @return The filtering mode that is used for minifying lookups.
        virtual FilterMode getMinifyingFilter() const noexcept = 0;

        /// @brief Gets the filtering mode that is used for magnifying lookups.
        ///
        /// @return The filtering mode that is used for magnifying lookups.
        virtual FilterMode getMagnifyingFilter() const noexcept = 0;

        /// @brief Gets the addressing mode at the horizontal border.
        ///
        /// @return The addressing mode at the horizontal border.
        virtual BorderMode getBorderModeU() const noexcept = 0;

        /// @brief Gets the addressing mode at the vertical border.
        ///
        /// @return The addressing mode at the vertical border.
        virtual BorderMode getBorderModeV() const noexcept = 0;

        /// @brief Gets the addressing mode at the depth border.
        ///
        /// @return The addressing mode at the depth border.
        virtual BorderMode getBorderModeW() const noexcept = 0;

        /// @brief Gets the anisotropy value used when sampling this texture.
        ///
        /// Anisotropy will be disabled, if this value is set to `0.0`.
        ///
        /// @return The anisotropy value used when sampling this texture.
        virtual Float getAnisotropy() const noexcept = 0;

        /// @brief Gets the mip-map selection mode.
        ///
        /// @return The mip-map selection mode.
        virtual MipMapMode getMipMapMode() const noexcept = 0;

        /// @brief Gets the mip-map level of detail bias.
        ///
        /// @return The mip-map level of detail bias.
        virtual Float getMipMapBias() const noexcept = 0;

        /// @brief Gets the maximum texture level of detail.
        ///
        /// @return The maximum texture level of detail.
        virtual Float getMaxLOD() const noexcept = 0;

        /// @brief Gets the minimum texture level of detail.
        ///
        /// @return The minimum texture level of detail.
        virtual Float getMinLOD() const noexcept = 0;
    };

    /// @brief The interface for a vertex buffer.
    class LITEFX_RENDERING_API IVertexBuffer : public virtual IBuffer {
    protected:
        IVertexBuffer() noexcept = default;
        IVertexBuffer(const IVertexBuffer&) = delete;
        IVertexBuffer(IVertexBuffer&&) noexcept = default;
        IVertexBuffer& operator=(const IVertexBuffer&) = delete;
        IVertexBuffer& operator=(IVertexBuffer&&) noexcept = default;

    public:
        ~IVertexBuffer() noexcept override = default;

    public:
        /// @brief Gets the layout of the vertex buffer.
        ///
        /// @return The layout of the vertex buffer.
        virtual const IVertexBufferLayout& layout() const noexcept = 0;
    };

    /// @brief The interface for an index buffer.
    class LITEFX_RENDERING_API IIndexBuffer : public virtual IBuffer {
    protected:
        IIndexBuffer() noexcept = default;
        IIndexBuffer(const IIndexBuffer&) = delete;
        IIndexBuffer(IIndexBuffer&&) noexcept = default;
        IIndexBuffer& operator=(const IIndexBuffer&) = delete;
        IIndexBuffer& operator=(IIndexBuffer&&) noexcept = default;

    public:
        ~IIndexBuffer() noexcept override = default;

    public:
        /// @brief Gets the layout of the index buffer.
        ///
        /// @return The layout of the index buffer.
        virtual const IIndexBufferLayout& layout() const noexcept = 0;
    };

    /// @brief Base interface for a ray tracing acceleration structure.
    ///
    /// @see IBottomLevelAccelerationStructure
    /// @see ITopLevelAccelerationStructure
    class LITEFX_RENDERING_API IAccelerationStructure : public virtual IStateResource {
    protected:
        IAccelerationStructure() noexcept = default;
        IAccelerationStructure(IAccelerationStructure&&) noexcept = default;
        IAccelerationStructure(const IAccelerationStructure&) = delete;
        IAccelerationStructure& operator=(IAccelerationStructure&&) noexcept = default;
        IAccelerationStructure& operator=(const IAccelerationStructure&) = delete;

    public:
        ~IAccelerationStructure() noexcept override = default;

    public:
        /// @brief Returns the flags that control how the acceleration structure should be built.
        ///
        /// @return The flags that control how the acceleration structure should be built.
        virtual AccelerationStructureFlags flags() const noexcept = 0;

        /// @brief Performs a complete build of the acceleration structure.
        ///
        /// This method builds or rebuilds the entire acceleration structure. If called without any further arguments beside @p commandBuffer, a new buffer and scratch buffer will be allocated from the @ref
        /// IGraphicsDevice that created the command buffer. Alternatively, it is possible to provide a pre-allocated buffer in the @p buffer parameter. This allows to re-use memory from another acceleration
        /// structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling @ref buffer) and destroy it afterwards, which enables re-use scenarios
        /// for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory consumption. This is done by also providing the @p offset
        /// and @p maxSize parameters to address a range within the buffer itself, the acceleration structure may be written into. Note that the pointer passed to the @ref buffer parameter must have been
        /// initialized with the @ref BufferType::AccelerationStructure buffer type and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// By providing a @ref scratchBuffer, it is possible to re-use temporary memory while building. This can lower memory consumption when building multiple acceleration structures. However, this also
        /// requires proper barriers to be executed between two build commands, as they are not allowed to access the same scratch memory simultaneously. Note that the pointer passed to the @ref scratchBuffer
        /// parameter must have been initialized on the @ref ResourceHeap::Resource heap and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// After a successful build, the buffer pointer is stored by the acceleration structure and can be accessed by calling @ref buffer on it.
        ///
        /// @param commandBuffer The command buffer used to record the acceleration structure build commands.
        /// @param scratchBuffer The scratch buffer used during the acceleration structure build, or `nullptr` if a temporary buffer should be created.
        /// @param buffer The buffer that stores the acceleration structure after building, or `nullptr` if a new buffer should be created.
        /// @param offset The offset into @p buffer at which the acceleration structure should be stored. Must be a multiple of 256. Ignored if @p buffer is `nullptr`.
        /// @param maxSize The maximum available size within @p buffer at @p offset. Ignored if @p buffer is `nullptr`.
        /// @throws InvalidArgumentException Thrown, if @p scratchBuffer is not `nullptr` and does not contain enough scratch memory to build the acceleration structure.
        /// @throws InvalidArgumentException Thrown, if @p offset is not aligned to 256 bytes.
        /// @throws ArgumentOutOfRangeException Thrown, if @p buffer is not `nullptr` and the range provided by @p offset and @p maxSize is not fully contained by the buffer.
        /// @see update
        inline void build(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer = nullptr, const SharedPtr<const IBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0) {
            this->doBuild(commandBuffer, scratchBuffer, buffer, offset, maxSize);
        }

        /// @brief Performs an update on the acceleration structure.
        ///
        /// Updating an acceleration structure works similar to performing a build, but may be faster compared to a full re-build. Note that in order to support updates, the acceleration structure must have been
        /// created with the <see cref=AccelerationStructureFlags::AllowUpdate" /> flag provided. Note that this flag may cause the acceleration structure build times and memory consumption to increase and may
        /// lower the ray-tracing performance.
        ///
        /// If no arguments beside @p commandBuffer are provided, the acceleration structure may re-use the same backing memory used for building, if the buffer holds enough space to contain it. Otherwise, a new
        /// buffer will be allocated. Alternatively, it is possible to provide a pre-allocated buffer in the @p buffer parameter. This allows to re-use memory from another acceleration structure, that no longer
        /// uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling @ref buffer) and destroy it afterwards, which enables re-use scenarios for example for caching.
        /// Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory consumption. This is done by also providing the @p offset and @p maxSize parameters
        /// to address a range within the buffer itself, the acceleration structure may be written into. Note that the pointer passed to the @ref buffer parameter must have been initialized with the @ref
        /// BufferType::AccelerationStructure buffer type and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// By providing a @ref scratchBuffer, it is possible to re-use temporary memory while building. This can lower memory consumption when building multiple acceleration structures. However, this also
        /// requires proper barriers to be executed between two build commands, as they are not allowed to access the same scratch memory simultaneously. Note that the pointer passed to the @ref scratchBuffer
        /// parameter must have been initialized on the @ref ResourceHeap::Resource heap and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// After a successful update, the buffer pointer is stored by the acceleration structure and can be accessed by calling @ref buffer on it.
        ///
        /// @param commandBuffer The command buffer used to record the acceleration structure build commands.
        /// @param scratchBuffer The scratch buffer used during the acceleration structure build, or `nullptr` if a temporary buffer should be created.
        /// @param buffer The buffer that stores the acceleration structure after updating, or `nullptr` if a new buffer should be created.
        /// @param offset The offset into @p buffer at which the acceleration structure should be stored. Must be a multiple of 256. Ignored if @p buffer is `nullptr`.
        /// @param maxSize The maximum available size within @p buffer at @p offset. Ignored if @p buffer is `nullptr`.
        /// @throws RuntimeException Thrown, if the acceleration structure backing buffer is not initialized, indicating the acceleration structure has not yet been built.
        /// @throws InvalidArgumentException Thrown, if @p scratchBuffer is not `nullptr` and does not contain enough scratch memory to build the acceleration structure.
        /// @throws InvalidArgumentException Thrown, if @p offset is not aligned to 256 bytes.
        /// @throws ArgumentOutOfRangeException Thrown, if @p buffer is not `nullptr` and the range provided by @p offset and @p maxSize is not fully contained by the buffer.
        /// @see build
        inline void update(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer = nullptr, const SharedPtr<const IBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0) {
            this->doUpdate(commandBuffer, scratchBuffer, buffer, offset, maxSize);
        }

        /// @brief Returns the acceleration structure backing buffer, that stores its last build.
        ///
        /// @return The acceleration structure backing buffer, that stores its last build.
        /// @see offset
        /// @see size
        inline SharedPtr<const IBuffer> buffer() const noexcept {
            return this->getBuffer();
        }

        /// @brief Returns the offset into @ref buffer at which the acceleration structure is stored.
        ///
        /// @return The offset into @ref buffer at which the acceleration structure is stored.
        /// @see buffer
        virtual UInt64 offset() const noexcept = 0;

        /// @brief Returns the amount of memory in bytes inside @ref buffer that store the acceleration structure.
        ///
        /// Note that this may be different to the value specified during build, as the actual size may be smaller (but can never be larger) after building. If you want to reduce the memory footprint, you can use
        /// this amount of memory for a compacted buffer and copy the acceleration structure using a copy command. In order to acquire the actual size required by the acceleration structure, the system needs to
        /// wait for the last build or update process to finish. Before that, this property will return the memory requirements as pre-computed by the device. To make sure that the build has finished, you need to
        /// manually wait for the fence acquired by submitting the command buffer that builds or updates the acceleration structure. Afterwards this method will return the actual size required to store the
        /// acceleration structure.
        ///
        /// If the acceleration structure has not yet been built or is invalidated, this property returns `0`.
        ///
        /// @return The amount of memory in bytes inside @ref buffer that store the acceleration structure.
        /// @see buffer
        virtual UInt64 size() const noexcept = 0;

    private:
        virtual SharedPtr<const IBuffer> getBuffer() const noexcept = 0;
        virtual void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) = 0;
        virtual void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) = 0;
    };

    /// @brief A structure that holds a singular entity of geometry for hardware ray-tracing.
    ///
    /// Bottom-level acceleration structures describe actual pieces of geometry (sets of triangular meshes or axis-aligned bounding boxes for procedural geometry). They can best be thought of entities in
    /// terms of a scene graph, whilst @ref ITopLevelAccelerationStructures represent their respective *instances*. For example, a top-level acceleration structure (TLAS) would store the world transform of
    /// the object itself, which can be placed multiple times in the scene with different transforms each time. Each TLAS points to a bottom-level acceleration structure (BLAS), that contains the actual
    /// geometry, consisting of multiple meshes that are all transformed relative to the TLAS transform.
    ///
    /// Note that a bottom-level acceleration structure can only contain either triangle meshes or bounding boxes, but never both in the same structure.
    ///
    /// @see TriangleMesh
    /// @see AxisAlignedBoundingBox
    /// @see ITopLevelAccelerationStructure
    class LITEFX_RENDERING_API IBottomLevelAccelerationStructure : public virtual IAccelerationStructure {
    public:
        /// @brief Represents a triangle mesh.
        struct TriangleMesh final {
        public:
            /// @brief Initializes a new triangle mesh.
            ///
            /// @param vertexBuffer The vertex buffer that stores the mesh vertices.
            /// @param indexBuffer The index buffer that stores the mesh indices.
            /// @param transformBuffer A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.
            /// @param flags The flags that control how the primitives in the geometry behaves during ray-tracing.
            TriangleMesh(const SharedPtr<const IVertexBuffer>& vertexBuffer, const SharedPtr<const IIndexBuffer>& indexBuffer = nullptr, const SharedPtr<const IBuffer>& transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) :
                VertexBuffer(vertexBuffer), IndexBuffer(indexBuffer), TransformBuffer(transformBuffer), Flags(flags) { 
                if (vertexBuffer == nullptr) [[unlikely]]
                    throw ArgumentNotInitializedException("vertexBuffer", "The vertex buffer must be initialized.");
            }

            /// @brief Initializes a new triangle mesh by taking over another one.
            ///
            /// @param other The triangle mesh to take over.
            TriangleMesh(TriangleMesh&& other) noexcept = default;

            /// @brief Initializes a new triangle mesh by copying another one.
            ///
            /// @param other The triangle mesh to copy.
            TriangleMesh(const TriangleMesh& other) = default;

            /// @brief Takes over another triangle mesh.
            ///
            /// @param other The triangle mesh to take over.
            /// @return A reference to the current triangle mesh instance.
            TriangleMesh& operator=(TriangleMesh&& other) noexcept = default;

            /// @brief Copies another triangle mesh.
            ///
            /// @param other The triangle mesh to copy.
            /// @return A reference to the current triangle mesh instance.
            TriangleMesh& operator=(const TriangleMesh& other) = default;

            /// @brief Releases the triangle mesh.
            ~TriangleMesh() noexcept = default;

        public:
            /// @brief The vertex buffer that stores the mesh vertices.
            SharedPtr<const IVertexBuffer> VertexBuffer;

            /// @brief The index buffer that stores the mesh indices.
            SharedPtr<const IIndexBuffer> IndexBuffer;

            /// @brief A buffer that stores a row-major 3x4 transformation matrix applied to the vertex buffer when building the BLAS.
            ///
            /// If the transform is not set, the vertices are not further transformed, which can improve building performance.
            SharedPtr<const IBuffer> TransformBuffer;

            /// @brief The flags that control how the primitives in the geometry behaves during ray-tracing.
            GeometryFlags Flags;
        };

        /// @brief Stores a buffer that contains axis-aligned bounding boxes.
        ///
        /// You may think of this structure as a set containing voxels for procedural geometry.
        struct BoundingBoxes final {
            /// @brief A buffer containing the bounding box definitions.
            ///
            /// Each element of the buffer must contain a bounding box at the start of the buffer, where a bounding box takes up 6 single-precision floating point values, with the first triplet describing the lower
            /// corner of the bounding box and the second triplet describing the upper corner of the bounding box, as shown in the following definition:
            ///
            /// @code
            /// struct alignas(16) AABB {
            ///     Float minimum[3];
            ///     Float maximum[3];
            /// }
            /// @endcode
            ///
            /// The rest of the bounding box elements memory can be filled with arbitrary data, that can be read by shaders.
            SharedPtr<const IBuffer> Buffer;

            /// @brief The flags that control how the primitives in the geometry behaves during ray-tracing.
            GeometryFlags Flags;
        };

    protected:
        IBottomLevelAccelerationStructure() noexcept = default;
        IBottomLevelAccelerationStructure(IBottomLevelAccelerationStructure&&) noexcept = default;
        IBottomLevelAccelerationStructure(const IBottomLevelAccelerationStructure&) = delete;
        IBottomLevelAccelerationStructure& operator=(IBottomLevelAccelerationStructure&&) noexcept = default;
        IBottomLevelAccelerationStructure& operator=(const IBottomLevelAccelerationStructure&) = delete;

    public:
        ~IBottomLevelAccelerationStructure() noexcept override = default;

    public:
        /// @brief Returns an array of triangle meshes contained by the BLAS.
        ///
        /// @return The array of triangle meshes contained by the BLAS.
        virtual const Array<TriangleMesh>& triangleMeshes() const noexcept = 0;

        /// @brief Adds a triangle mesh to the BLAS.
        ///
        /// @param mesh The triangle mesh to add to the BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains bounding boxes.
        virtual void addTriangleMesh(const TriangleMesh& mesh) = 0;

        /// @brief Adds a triangle mesh to the BLAS.
        ///
        /// @param vertexBuffer The vertex buffer that stores the mesh vertices.
        /// @param indexBuffer The index buffer that stores the mesh indices.
        /// @param transformBuffer A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.
        /// @param flags The flags that control how the primitives in the geometry behaves during ray-tracing.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains bounding boxes.
        inline void addTriangleMesh(const SharedPtr<const IVertexBuffer>& vertexBuffer, const SharedPtr<const IIndexBuffer>& indexBuffer = nullptr, const SharedPtr<const IBuffer>& transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) {
            this->addTriangleMesh(TriangleMesh(vertexBuffer, indexBuffer, transformBuffer, flags));
        }

        /// @brief Returns an array of buffers, each containing axis-aligned bounding boxes stored in the BLAS.
        ///
        /// @return The array of axis-aligned bounding boxes contained by the BLAS.
        virtual const Array<BoundingBoxes>& boundingBoxes() const noexcept = 0;

        /// @brief Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        ///
        /// @param aabbs The bounding boxes to add to the BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains triangle meshes.
        virtual void addBoundingBox(const BoundingBoxes& aabbs) = 0;

        /// @brief Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        ///
        /// @param buffer A buffer containing the bounding box definitions.
        /// @param flags The flags that control how the primitives in the geometry behaves during ray-tracing.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains triangle meshes.
        inline void addBoundingBox(const SharedPtr<const IBuffer>& buffer, GeometryFlags flags = GeometryFlags::None) {
            this->addBoundingBox(BoundingBoxes { .Buffer = buffer, .Flags = flags });
        }

        /// @brief Clears all bounding boxes and triangle meshes from the acceleration structure.
        virtual void clear() noexcept = 0;

        /// @brief Removes a triangle mesh from the acceleration structure.
        ///
        /// @param mesh The triangle mesh to remove from the acceleration structure.
        /// @return `true`, if the triangle mesh was removed, otherwise `false`.
        virtual bool remove(const TriangleMesh& mesh) noexcept = 0;

        /// @brief Removes a bounding box set from the acceleration structure.
        ///
        /// @param aabb The bounding box set to remove from the acceleration structure.
        /// @return `true`, if the bounding box set was removed, otherwise `false`.
        virtual bool remove(const BoundingBoxes& aabb) noexcept = 0;

        /// @brief Copies the acceleration structure into the acceleration structure provided by @p destination.
        ///
        /// This method copies the acceleration structure into another one, which is especially useful for compression. If called without any arguments besides @p commandBuffer and @p destination, the method will
        /// create a clone of the current acceleration structure, including any build info (i.e., triangle mesh or bounding box data). If the destination acceleration structure already contains a buffer and the
        /// buffer contains enough memory to store the copy, it will be re-used and its contents will be overwritten. Otherwise, a new buffer with enough memory to store the copy will be allocated.
        ///
        /// If the @p compress option is set to `true`, the copy will be compressed. Note that this is only possible, if the acceleration structure was created with the @ref
        /// AccelerationStructureFlags::AllowCompaction flag enabled. Note that compression requires a query for the size of the compressed data, which can only be determined *after* the acceleration structure
        /// was built or updated. This implies that a copy command that is used for compression is not valid on the same command buffer that did also record the build or update commands for it. You have to use a
        /// fence to wait for the build to finish before attempting a compression.
        ///
        /// It is possible to provide a buffer for the destination acceleration structure to use after copying. This buffer can be set by providing the @p buffer parameter. This allows to re-use memory from
        /// another acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling @ref buffer) and destroy it afterwards, which
        /// enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory consumption. This is done by also
        /// providing the @p offset parameter to address where the copy should be stored. Note that the pointer passed to the @ref buffer parameter must have been initialized with the @ref
        /// BufferType::AccelerationStructure buffer type and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// To reduce memory consumption, the build info (i.e., triangle mesh and bounding box data) is not copied to the destination acceleration structure by default. However, this also implies that further
        /// updates to it are inconvenient, requiring to manually copy the data in an additional pass. To also include build data in the copy, the @p copyBuildInfo setting can be set to `true`.
        ///
        /// After a successful copy, the buffer pointer is stored by the acceleration structure @p destination and can be accessed by calling @ref buffer on it.
        ///
        /// @param commandBuffer The command buffer used to record the acceleration structure copy commands.
        /// @param destination The acceleration structure to copy the current one into.
        /// @param compress If `true`, the acceleration structure data will be compressed.
        /// @param buffer If not `nullptr`, the destination acceleration structure will be written into the provided buffer. Otherwise a new buffer is allocated, or the existing one is used depending on the
        /// available size.
        /// @param offset The offset at which to store the copy within @p buffer. Must be a multiple of 256. Ignored if @p buffer is `nullptr`.
        /// @param copyBuildInfo If `true`, the mesh data or bounding box data is copied into the acceleration structure.
        /// @throws InvalidArgumentException Thrown, if @p compress is set to `true`, but the current acceleration structure has not been created with the @ref AccelerationStructureFlags::AllowCompaction flag.
        /// @throws InvalidArgumentException Thrown, if @p offset is not aligned to 256 bytes.
        /// @throws ArgumentOutOfRangeException Thrown, if @p buffer is not `nullptr` and does not fully contain the required memory to store the copy, starting at @p offset.
        inline void copy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const {
            this->doCopy(commandBuffer, destination, compress, buffer, offset, copyBuildInfo);
        }

    public:
        /// @brief Adds a triangle mesh to the BLAS.
        ///
        /// @param mesh The triangle mesh to add to the BLAS.
        /// @return A reference to the current BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains bounding boxes.
        template <typename TSelf>
        inline auto withTriangleMesh(this TSelf&& self, const TriangleMesh& mesh) -> TSelf&& {
            self.addTriangleMesh(mesh);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a triangle mesh to the BLAS.
        ///
        /// @param vertexBuffer The vertex buffer that stores the mesh vertices.
        /// @param indexBuffer The index buffer that stores the mesh indices.
        /// @param transformBuffer A buffer that stores a row-major 3x4 transformation matrix applied to the vertices when building the BLAS.
        /// @param flags The flags that control how the primitives in the geometry behaves during ray-tracing.
        /// @return A reference to the current BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains bounding boxes.
        template <typename TSelf>
        inline auto withTriangleMesh(this TSelf&& self, const SharedPtr<const IVertexBuffer>& vertexBuffer, const SharedPtr<const IIndexBuffer>& indexBuffer = nullptr, const SharedPtr<const IBuffer>& transformBuffer = nullptr, GeometryFlags flags = GeometryFlags::None) -> TSelf&& {
            return std::forward<TSelf>(self).withTriangleMesh(TriangleMesh(vertexBuffer, indexBuffer, transformBuffer, flags));
        }

        /// @brief Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        ///
        /// @param aabb The bounding box buffer to add to the BLAS.
        /// @return A reference to the current BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains triangle meshes.
        template <typename TSelf>
        inline auto withBoundingBox(this TSelf&& self, const BoundingBoxes& aabb) -> TSelf&& {
            self.addBoundingBox(aabb);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a buffer containing axis-aligned bounding boxes to the BLAS.
        ///
        /// @param buffer A buffer containing the bounding box definitions.
        /// @param flags The flags that control how the primitives in the geometry behaves during ray-tracing.
        /// @return A reference to the current BLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure already contains triangle meshes.
        template <typename TSelf>
        inline auto withBoundingBox(this TSelf&& self, const SharedPtr<const IBuffer>& buffer, GeometryFlags flags = GeometryFlags::None) -> TSelf&& {
            return std::forward<TSelf>(self).withBoundingBox(BoundingBoxes { .Buffer = buffer, .Flags = flags });
        }

    private:
        virtual void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const = 0;
    };

    /// @brief A structure that stores the instance data for a @ref IBottomLevelAccelerationStructure.
    ///
    /// @see IBottomLevelAccelerationStructure
    class LITEFX_RENDERING_API ITopLevelAccelerationStructure : public virtual IAccelerationStructure {
    public:
#pragma warning(push)
#pragma warning(disable: 4324) // Structure was padded due to alignment specifier
        /// @brief Represents an instance of an @ref IBottomLevelAccelerationStructure.
        struct alignas(16) Instance final { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            /// @brief The bottom-level acceleration structure that contains the geometries of this instance.
            SharedPtr<const IBottomLevelAccelerationStructure> BottomLevelAccelerationStructure;

            /// @brief The transformation matrix for the instance.
            TMatrix3x4<Float> Transform = TMatrix3x4<Float>::identity();

            /// @brief The instance ID used in shaders to identify the instance.
            UInt32 Id : 24 = 0;

            /// @brief A user-defined mask value that is matched with another mask value during ray-tracing to include or discard the instance.
            UInt8 Mask : 8 = 0xFF;

            /// @brief An offset added to the address of the shader-local data of the shader record that is invoked for the instance, *after* the @ref IBottomLevelAccelerationStructure indexing rules have been
            /// applied.
            ///
            /// Shader-local data is a piece of constant data that is available to the shader during invocation. During a ray hit/miss event, the shader record is selected based on geometry (@ref
            /// IBottomLevelAccelerationStructure), instance (@ref ITopLevelAccelerationStructure and an implementation-specific offset. The selected record is then used to load the shader and pass the shader local
            /// data to it.
            ///
            /// The first part of the address is determined from the geometry index within the TLAS and a user-defined multiplier and base index specified in the shader when calling `TraceRay`. After this index is
            /// calculated the value of this property is added to it. The result is an offset into the shader-local data for the selected shader record.
            ///
            /// @see https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#addressing-calculations-within-shader-tables
            /// @see https://docs.vulkan.org/spec/latest/chapters/raytracing.html#shader-binding-table-indexing-rules
            UInt32 HitGroupOffset : 24 = 0;

            /// @brief The flags that control the behavior of this instance.
            InstanceFlags Flags : 8 = InstanceFlags::None;
        };
#pragma warning(pop)

    protected:
        ITopLevelAccelerationStructure() noexcept = default;
        ITopLevelAccelerationStructure(ITopLevelAccelerationStructure&&) noexcept = default;
        ITopLevelAccelerationStructure(const ITopLevelAccelerationStructure&) = delete;
        ITopLevelAccelerationStructure& operator=(ITopLevelAccelerationStructure&&) noexcept = default;
        ITopLevelAccelerationStructure& operator=(const ITopLevelAccelerationStructure&) = delete;

    public:
        ~ITopLevelAccelerationStructure() noexcept override = default;

    public:
        /// @brief Returns an array of instances in the TLAS.
        ///
        /// @return The array of instances in the TLAS.
        virtual const Array<Instance>& instances() const noexcept = 0;

        /// @brief Adds an instance to the TLAS.
        ///
        /// @param instance The instance to add to the TLAS.
        /// @throws RuntimeException Thrown, if the acceleration structure buffers have already been allocated.
        virtual void addInstance(const Instance& instance) = 0;

        /// @brief Adds an instance to the TLAS.
        ///
        /// @param blas The bottom-level acceleration structure that contains the geometries of the instance.
        /// @param id The instance ID used in shaders to identify the instance.
        /// @param hitGroupOffset An offset added to the shader-local data for a hit-group shader record.
        /// @param mask A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.
        /// @param flags The flags that control the behavior of the instance.
        inline void addInstance(const SharedPtr<const IBottomLevelAccelerationStructure>& blas, UInt32 id, UInt32 hitGroupOffset = 0, UInt8 mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            this->addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
        }
        
        /// @brief Adds an instance to the TLAS.
        ///
        /// @param blas The bottom-level acceleration structure that contains the geometries of the instance.
        /// @param transform The transformation matrix applied to the instance geometry.
        /// @param id The instance ID used in shaders to identify the instance.
        /// @param hitGroupOffset An offset added to the shader-local data for a hit-group shader record.
        /// @param mask A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.
        /// @param flags The flags that control the behavior of the instance.
        inline void addInstance(const SharedPtr<const IBottomLevelAccelerationStructure>& blas, const TMatrix3x4<Float>& transform, UInt32 id, UInt32 hitGroupOffset = 0, UInt8 mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            this->addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Transform = transform, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
        }

        /// @brief Clears all instances from the acceleration structure.
        virtual void clear() noexcept = 0;

        /// @brief Removes an instance from the acceleration structure.
        ///
        /// @param instance The instance to remove from the acceleration structure.
        /// @return `true`, if the instance has been removed, otherwise `false`.
        virtual bool remove(const Instance& instance) noexcept = 0;

        /// @brief Copies the acceleration structure into the acceleration structure provided by @p destination.
        ///
        /// This method copies the acceleration structure into another one, which is especially useful for compression. If called without any arguments besides @p commandBuffer and @p destination, the method will
        /// create a clone of the current acceleration structure, including any build info (i.e., triangle mesh or bounding box data). If the destination acceleration structure already contains a buffer and the
        /// buffer contains enough memory to store the copy, it will be re-used and its contents will be overwritten. Otherwise, a new buffer with enough memory to store the copy will be allocated.
        ///
        /// If the @p compress option is set to `true`, the copy will be compressed. Note that this is only possible, if the acceleration structure was created with the @ref
        /// AccelerationStructureFlags::AllowCompaction flag enabled. Note that compression requires a query for the size of the compressed data, which can only be determined *after* the acceleration structure
        /// was built or updated. This implies that a copy command that is used for compression is not valid on the same command buffer that did also record the build or update commands for it. You have to use a
        /// fence to wait for the build to finish before attempting a compression.
        ///
        /// It is possible to provide a buffer for the destination acceleration structure to use after copying. This buffer can be set by providing the @p buffer parameter. This allows to re-use memory from
        /// another acceleration structure, that no longer uses the memory. It is possible to store the buffer from an acceleration structure (acquired by calling @ref buffer) and destroy it afterwards, which
        /// enables re-use scenarios for example for caching. Alternatively, it is possible store multiple acceleration structures within the same buffer, reducing overall memory consumption. This is done by also
        /// providing the @p offset parameter to address where the copy should be stored. Note that the pointer passed to the @ref buffer parameter must have been initialized with the @ref
        /// BufferType::AccelerationStructure buffer type and must be writable (@ref ResourceUsage::AllowWrite).
        ///
        /// To reduce memory consumption, the build info (i.e., triangle mesh and bounding box data) is not copied to the destination acceleration structure by default. However, this also implies that further
        /// updates to it are inconvenient, requiring to manually copy the data in an additional pass. To also include build data in the copy, the @p copyBuildInfo setting can be set to `true`.
        ///
        /// After a successful copy, the buffer pointer is stored by the acceleration structure @p destination and can be accessed by calling @ref buffer on it.
        ///
        /// @param commandBuffer The command buffer used to record the acceleration structure copy commands.
        /// @param destination The acceleration structure to copy the current one into.
        /// @param compress If `true`, the acceleration structure data will be compressed.
        /// @param buffer If not `nullptr`, the destination acceleration structure will be written into the provided buffer. Otherwise a new buffer is allocated, or the existing one is used depending on the
        /// available size.
        /// @param offset The offset at which to store the copy within @p buffer. Must be a multiple of 256. Ignored if @p buffer is `nullptr`.
        /// @param copyBuildInfo If `true`, the mesh data or bounding box data is copied into the acceleration structure.
        /// @throws InvalidArgumentException Thrown, if @p compress is set to `true`, but the current acceleration structure has not been created with the @ref AccelerationStructureFlags::AllowCompaction flag.
        /// @throws InvalidArgumentException Thrown, if @p offset is not aligned to 256 bytes.
        /// @throws ArgumentOutOfRangeException Thrown, if @p buffer is not `nullptr` and does not fully contain the required memory to store the copy, starting at @p offset.
        inline void copy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const {
            this->doCopy(commandBuffer, destination, compress, buffer, offset, copyBuildInfo);
        }

    public:
        /// @brief Adds an instance to the current TLAS.
        ///
        /// @param instance The instance to add to the TLAS.
        /// @return A reference to the current TLAS.
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, const Instance& instance) noexcept -> TSelf&& {
            self.addInstance(instance);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an instance to the current TLAS.
        ///
        /// @param blas The bottom-level acceleration structure that contains the geometries of the instance.
        /// @param id The instance ID used in shaders to identify the instance.
        /// @param hitGroupOffset An offset added to the shader-local data for a hit-group shader record.
        /// @param mask A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.
        /// @param flags The flags that control the behavior of the instance.
        /// @return A reference to the current TLAS.
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, const SharedPtr<const IBottomLevelAccelerationStructure>& blas, UInt32 id, UInt32 hitGroupOffset = 0, UInt8 mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept -> TSelf&& { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            self.addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an instance to the current TLAS.
        ///
        /// @param blas The bottom-level acceleration structure that contains the geometries of the instance.
        /// @param transform The transformation matrix applied to the instance geometry.
        /// @param id The instance ID used in shaders to identify the instance.
        /// @param hitGroupOffset An offset added to the shader-local data for a hit-group shader record.
        /// @param mask A user defined mask value that can be used to include or exclude the instance during a ray-tracing pass.
        /// @param flags The flags that control the behavior of the instance.
        /// @return A reference to the current TLAS.
        template<typename TSelf>
        inline auto withInstance(this TSelf&& self, const SharedPtr<const IBottomLevelAccelerationStructure>& blas, const TMatrix3x4<Float>& transform, UInt32 id, UInt32 hitGroupOffset = 0, UInt8 mask = 0xFF, InstanceFlags flags = InstanceFlags::None) noexcept -> TSelf&& { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            self.addInstance(Instance { .BottomLevelAccelerationStructure = blas, .Transform = transform, .Id = id, .Mask = mask, .HitGroupOffset = hitGroupOffset, .Flags = flags });
            return std::forward<TSelf>(self);
        }

    private:
        virtual void doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const = 0;
    };

    /// @brief The interface for a barrier.
    ///
    /// Barriers are used to synchronize the GPU with itself in a command buffer. They are basically used to control the GPU command flow and ensure that resources are in he right state before using them.
    /// Generally speaking, there are two related types of barriers:
    ///
    /// - **Execution barriers** are enforcing command order by telling the GPU to wait for certain pipeline stages or which stages to block until an operation has finished. - **Memory barriers** are used to
    /// transition resources between states and are a subset of execution barriers. Each memory barrier is always also an execution barrier, but the opposite is not true.
    ///
    /// An execution barrier is simply a barrier without any *resource transitions* happening. The only properties that are set for an execution barrier are two pipeline stages defined by @ref
    /// IBarrier::syncBefore and @ref IBarrier::syncAfter. The first value defines the pipeline stages, all *previous* commands in a command buffer need to finish before execution is allowed to continue.
    /// Similarly, the second value defines the stage, all *subsequent* commands need to wait for, before they are allowed to continue execution. For example, setting `syncBefore` to `Compute` and `syncAfter`
    /// to `Vertex` logically translates to: *All subsequent commands that want to pass the vertex stage need to wait there before all previous commands passed the compute stage*. To synchronize reads in
    /// previous commands with writes in subsequent commands, this is sufficient. However, in order to do the opposite, this is not enough. Instead resource memory needs to be *transitioned* by specifying the
    /// desired @ref ResourceAccess, alongside the @ref ImageLayout for images (note that buffers always share a *common* layout that can not be changed). This is done using memory barriers. There are two
    /// types of memory barriers used for state transitions:
    ///
    /// - **Global barriers** apply to all resource memory. - **Image and buffer barriers** apply to individual images or buffers or a sub-resource of those.
    ///
    /// Any `IBarrier` can contain an arbitrary mix of one or more global and/or image/buffer barriers. A global barrier is inserted by calling @ref IBarrier::wait. This method accepts two parameters: a
    /// `before` and an `after` access mode. Those parameters specify the @ref ResourceAccess for the previous and subsequent commands. This makes it possible to describe scenarios like *wait for certain
    /// writes to finish before continuing with certain reads*. Note that a resource can be accessed in different ways at the same time (for example as copy source and shader resource) and specifying an
    /// access state will only wait for the specified subset. As a rule of thumb, you should always specify as little access as possible in order to leave most room for optimization.
    ///
    /// Image and buffer barriers additionally describe which (sub-)resources to apply the barrier to. For buffers this only applies to individual elements in a buffer array. However, due to [driver
    /// restrictions](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#buffer-barriers), buffers are always transitioned as a whole. This is different from image resources, which have
    /// addressable sub-resources (mip levels, planes and array elements). For images, it is possible to transition individual sub-resources into different @ref ImageLayouts to indicate when and how a texture
    /// is used. An image in a certain layout poses restrictions on how it can be accessed. For example, a `ReadWrite` image written by a compute shader must be transitioned into a proper layout to be read by
    /// a graphics shader. To facilitate such a transition, a barrier is required. Image barriers can be inserted by calling one of the overloads of @ref IBarrier::transition that accepts an @ref IImage
    /// parameter.
    ///
    /// @see PipelineStage
    /// @see IBuffer
    /// @see ResourceAccess
    /// @see IImage
    /// @see ImageLayout
    /// @see ICommandBuffer
    class LITEFX_RENDERING_API IBarrier {
    protected:
        IBarrier() noexcept = default;
        IBarrier(const IBarrier&) = default;
        IBarrier(IBarrier&&) noexcept = default;
        IBarrier& operator=(const IBarrier&) = default;
        IBarrier& operator=(IBarrier&&) noexcept = default;

    public:
        virtual ~IBarrier() noexcept = default;

    public:
        /// @brief Returns the stage that all previous commands need to reach before continuing execution.
        ///
        /// @return The stage that all previous commands need to reach before continuing execution.
        constexpr virtual PipelineStage syncBefore() const noexcept = 0;
        
        /// @brief Returns the stage all subsequent commands need to wait for before continuing execution.
        ///
        /// @return The stage all subsequent commands need to wait for before continuing execution.
        constexpr virtual PipelineStage syncAfter() const noexcept = 0;

        /// @brief Inserts a global barrier that waits for previous commands to finish accesses described by @p accessBefore before subsequent commands can continue with accesses described by @p accessAfter.
        ///
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        constexpr virtual void wait(ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;

        /// @brief Inserts a buffer barrier that blocks access to @p buffer of types contained in @p accessAfter for subsequent commands until previous commands have finished accesses contained in @p
        /// accessBefore.
        ///
        /// @param buffer The buffer resource to transition.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        constexpr void transition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) {
            this->doTransition(buffer, accessBefore, accessAfter);
        };

        /// @brief Inserts a buffer barrier that blocks access to a @p buffers @p element of types contained in @p accessAfter for subsequent commands until previous commands have finished accesses contained in
        /// @p accessBefore.
        ///
        /// Due to [driver restrictions](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#buffer-barriers), this is overload is currently redundant, but might be available in the future.
        /// Currently, calling this method ignores the @p element parameter and transitions the whole buffer.
        ///
        /// @param buffer The buffer resource to transition.
        /// @param element The element of the resource to transition.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        constexpr void transition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) {
            this->doTransition(buffer, element, accessBefore, accessAfter);
        }

        /// @brief Inserts an image barrier that blocks access to all sub-resources of @p image of the types contained in @p accessAfter for subsequent commands until previous commands have finished accesses
        /// contained in @p accessBefore and transitions all sub-resources into @p layout.
        ///
        /// @param image The image resource to transition.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        /// @param layout The image layout to transition into.
        constexpr void transition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
            this->doTransition(image, accessBefore, accessAfter, layout);
        }

        /// @brief Inserts an image barrier that blocks access to a sub-resource range of @p image of the types contained in @p accessAfter for subsequent commands until previous commands have finished accesses
        /// contained in @p accessBefore and transitions the sub-resource into @p layout.
        ///
        /// @param image The image resource to transition.
        /// @param level The base mip-map level of the sub-resource range.
        /// @param levels The number of mip-map levels of the sub-resource range.
        /// @param layer The base array layer of the sub-resource range.
        /// @param layers The number of array layer of the sub-resource range.
        /// @param plane The plane of the sub-resource.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        /// @param layout The image layout to transition into.
        constexpr void transition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) {
            this->doTransition(image, level, levels, layer, layers, plane, accessBefore, accessAfter, layout);
        }

        /// @brief Inserts an image barrier that blocks access to all sub-resources of @p image of the types contained in @p accessAfter for subsequent commands until previous commands have finished accesses
        /// contained in @p accessBefore and transitions all sub-resources into @p layout.
        ///
        /// This overload let's you explicitly specify the @p fromLayout. This is required, if you use any external transition mechanism that causes the engine to lose track of the image layout. If you are not
        /// running into issues with the other overloads, you probably do not want to call this method.
        ///
        /// @param image The image resource to transition.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        /// @param fromLayout The image layout to transition from.
        /// @param toLayout The image layout to transition into.
        constexpr void transition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
            this->doTransition(image, accessBefore, accessAfter, fromLayout, toLayout);
        }

        /// @brief Inserts an image barrier that blocks access to a sub-resource range of @p image of the types contained in @p accessAfter for subsequent commands until previous commands have finished accesses
        /// contained in @p accessBefore and transitions the sub-resource into @p layout.
        ///
        /// This overload let's you explicitly specify the @p fromLayout. This is required, if you use any external transition mechanism that causes the engine to lose track of the image layout. If you are not
        /// running into issues with the other overloads, you probably do not want to call this method.
        ///
        /// @param image The image resource to transition.
        /// @param level The base mip-map level of the sub-resource range.
        /// @param levels The number of mip-map levels of the sub-resource range.
        /// @param layer The base array layer of the sub-resource range.
        /// @param layers The number of array layer of the sub-resource range.
        /// @param plane The plane of the sub-resource.
        /// @param accessBefore The access types previous commands have to finish.
        /// @param accessAfter The access types that subsequent commands continue with.
        /// @param fromLayout The image layout to transition from.
        /// @param toLayout The image layout to transition into.
        constexpr void transition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) {
            this->doTransition(image, level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
        }

    private:
        constexpr virtual void doTransition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr virtual void doTransition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;
        constexpr virtual void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr virtual void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
        constexpr virtual void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;
        constexpr virtual void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;
    };

    /// @brief The interface for a descriptor set.
    class LITEFX_RENDERING_API IDescriptorSet {
    protected:
        IDescriptorSet() noexcept = default;
        IDescriptorSet(const IDescriptorSet&) = default;
        IDescriptorSet(IDescriptorSet&&) noexcept = default;
        IDescriptorSet& operator=(const IDescriptorSet&) = default;
        IDescriptorSet& operator=(IDescriptorSet&&) noexcept = default;

    public:
        virtual ~IDescriptorSet() noexcept = default;

    public:
        /// @brief Returns the allocation information for the descriptor set in the global descriptor heap indicated by @p heapType.
        ///
        /// @param heapType The type of the descriptor heap for which to obtain the heap allocation.
        /// @return The allocation for the descriptor set in the global descriptor heap.
        virtual VirtualAllocator::Allocation globalHeapAllocation(DescriptorHeapType heapType) const noexcept = 0;

        /// @brief Binds a resource directly to a descriptor heap and returns the index that can be used to access it.
        ///
        /// This method is used with the @ref GraphicsDeviceFeature::DynamicDescriptors feature and allows to bind a descriptor to the underlying descriptor heap directly by providing the corresponding resource
        /// type (indicated by @p bindingType) at bind time. The method directly returns the global heap index of the resource, that can be used by the shader to access it using the `ResourceDescriptorHeap`
        /// syntax (in HLSL).
        ///
        /// If the descriptor set does not contain a descriptor of type @ref DescriptorType::ResourceDescriptorHeap, this method will throw an exception.
        ///
        /// @param bindingType The type of the descriptor used to bind @p buffer to the heap.
        /// @param descriptor The index of the descriptor in the heap to bind @p buffer to.
        /// @param buffer The buffer to bind.
        /// @param bufferElement The index of an element inside @p buffer that should be bound.
        /// @param elements The number of elements from the buffer to bind to the descriptor set. A value of `0` binds all available elements, starting at @p bufferElement.
        /// @param texelFormat The format used to read a texel buffer. Required if @p binding binds a texel buffer and ignored otherwise.
        /// @return The global heap index that can be used to access the resource from the shader.
        /// @throws RuntimeException Thrown, if the descriptor set does not contain a descriptor that provides direct heap access to the underlying descriptor heap indicated by @p bindingType.
        inline UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, Format texelFormat = Format::None) const {
            return this->doBind(bindingType, descriptor, buffer, bufferElement, elements, texelFormat);
        }

        /// @brief Binds a resource directly to a descriptor heap and returns the index that can be used to access it.
        ///
        /// This method is used with the @ref GraphicsDeviceFeature::DynamicDescriptors feature and allows to bind a descriptor to the underlying descriptor heap directly by providing the corresponding resource
        /// type (indicated by @p bindingType) at bind time. The method directly returns the global heap index of the resource, that can be used by the shader to access it using the `ResourceDescriptorHeap`
        /// syntax (in HLSL).
        ///
        /// If the descriptor set does not contain a descriptor of type @ref DescriptorType::ResourceDescriptorHeap, this method will throw an exception.
        ///
        /// @param bindingType The type of the descriptor used to bind @p image to the heap.
        /// @param descriptor The index of the descriptor in the heap to bind @p image to.
        /// @param image The image to bind.
        /// @param firstLevel The index of the first mip-map level to bind.
        /// @param levels The number of mip-map levels to bind. A value of `0` binds all available levels, starting at @p firstLevel.
        /// @param firstLayer The index of the first layer to bind.
        /// @param layers The number of layers to bind. A value of `0` binds all available layers, starting at @p firstLayer.
        /// @return The global heap index that can be used to access the resource from the shader.
        /// @throws RuntimeException Thrown, if the descriptor set does not contain a descriptor that provides direct heap access to the underlying descriptor heap indicated by @p bindingType.
        inline UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IImage& image, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const {
            return this->doBind(bindingType, descriptor, image, firstLevel, levels, firstLayer, layers);
        }

        /// @brief Binds a sampler directly to a descriptor heap and returns the index that can be used to access it.
        ///
        /// This method is used with the @ref GraphicsDeviceFeature::DynamicDescriptors feature and allows to bind a descriptor to the underlying descriptor heap directly. The method returns the global heap index
        /// of the sampler, that can be used by the shader to access it using the `SamplerDescriptorHeap` syntax (in HLSL).
        ///
        /// If the descriptor set does not contain a descriptor of type @ref DescriptorType::SamplerDescriptorHeap, this method will throw an exception.
        ///
        /// @param descriptor The index of the descriptor in the heap to bind @p sampler to.
        /// @param sampler The sampler to bind.
        /// @return The global heap index that can be used to access the sampler from the shader.
        /// @throws RuntimeException Thrown, if the descriptor set does not contain a descriptor that provides direct heap access to the underlying descriptor heap indicated by @p bindingType.
        inline UInt32 bindToHeap(UInt32 descriptor, const ISampler& sampler) const {
            return this->doBind(descriptor, sampler);
        }

        /// @brief Updates one or more buffer descriptors within the current descriptor set.
        ///
        /// @param binding The buffer binding point.
        /// @param buffer The buffer to write to the descriptor set.
        /// @param bufferElement The index of the first element in the buffer to bind to the descriptor set.
        /// @param elements The number of elements from the buffer to bind to the descriptor set. A value of `0` binds all available elements, starting at @p bufferElement.
        /// @param firstDescriptor The index of the first descriptor in the descriptor array to update.
        /// @param texelFormat The format used to read a texel buffer. Required if @p binding binds a texel buffer and ignored otherwise.
        inline void update(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0, Format texelFormat = Format::None) const {
            this->doUpdate(binding, buffer, bufferElement, elements, firstDescriptor, texelFormat);
        }

        /// @brief Updates one or more texture descriptors within the current descriptor set.
        ///
        /// The exact representation of the level and layer parameters depends on the dimension of the provided texture, as well as the type of the descriptor identified by the @p binding parameter.
        ///
        /// If the texture itself is not an array (i.e. the number of layers equals `1`), the parameters @p firstLayer and @p layers are ignored.
        ///
        /// The descriptor type dictates, how mip-maps can be provided. If the descriptor type identifies a *writable texture*, the @p firstLevel parameter specifies the mip-map level to write to (or read from).
        /// Multiple levels are not allowed in this case, so the @p levels parameter is ignored. Instead, you have to bind them to separate descriptors. Furthermore, the @p firstLayer and @p layers parameter can
        /// be used to specify the number of depth or W-slices of a writable 3D texture or the side(s) of a cube map.
        ///
        /// @param binding The texture binding point.
        /// @param texture The texture to write to the descriptor set.
        /// @param descriptor The index of the descriptor in the descriptor array to bind the texture to.
        /// @param firstLevel The index of the first mip-map level to bind.
        /// @param levels The number of mip-map levels to bind. A value of `0` binds all available levels, starting at @p firstLevel.
        /// @param firstLayer The index of the first layer to bind.
        /// @param layers The number of layers to bind. A value of `0` binds all available layers, starting at @p firstLayer.
        inline void update(UInt32 binding, const IImage& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const {
            this->doUpdate(binding, texture, descriptor, firstLevel, levels, firstLayer, layers);
        }

        /// @brief Updates one or more sampler descriptors within the current descriptor set.
        ///
        /// @param binding The sampler binding point.
        /// @param sampler The sampler to write to the descriptor set.
        /// @param descriptor The index of the descriptor in the descriptor array to bind the sampler to.
        inline void update(UInt32 binding, const ISampler& sampler, UInt32 descriptor = 0) const {
            this->doUpdate(binding, sampler, descriptor);
        }

        /// @brief Updates an acceleration structure within the current descriptor set.
        ///
        /// @param binding The acceleration structure binding point.
        /// @param accelerationStructure The acceleration structure to write to the descriptor set.
        /// @param descriptor The index of the descriptor in the descriptor array to bind the acceleration structure to.
        inline void update(UInt32 binding, const IAccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const {
            this->doUpdate(binding, accelerationStructure, descriptor);
        }

    private:
        virtual UInt32 doBind(DescriptorType bindingType, UInt32 descriptor, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat) const = 0;
        virtual UInt32 doBind(DescriptorType bindingType, UInt32 descriptor, const IImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const = 0;
        virtual UInt32 doBind(UInt32 descriptor, const ISampler& sampler) const = 0;
        virtual void doUpdate(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor, Format texelFormat) const = 0;
        virtual void doUpdate(UInt32 binding, const IImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const = 0;
        virtual void doUpdate(UInt32 binding, const ISampler& sampler, UInt32 descriptor) const = 0;
        virtual void doUpdate(UInt32 binding, const IAccelerationStructure& accelerationStructure, UInt32 descriptor) const = 0;
    };

    /// @brief Describes a resource binding to a descriptor or descriptor set.
    ///
    /// @see IDescriptorSet
    /// @see IDescriptorSetLayout
    struct LITEFX_RENDERING_API DescriptorBinding {
    public:
        using resource_container = Variant<std::monostate, Ref<const IBuffer>, Ref<const IImage>, Ref<const ISampler>, Ref<const IAccelerationStructure>>;
        
    public:
        /// @brief The binding point to bind the resource at. If not provided (i.e., `std::nullopt`), the index within the collection of `DescriptorBindings` is used.
        Optional<UInt32> binding = std::nullopt;

        /// @brief The resource to bind or `std::monostate` if no resource should be bound.
        ///
        /// Note that not providing any resource does not perform any binding, in which case a resource needs to be manually bound to the descriptor set later (@ref IDescriptorSet::update). This is useful in
        /// situations where you frequently update the resource bound to a descriptor set or where you do no have access to the resource at the time the descriptor set is allocated.
        ///
        /// @see IBuffer
        /// @see IImage
        /// @see ISampler
        resource_container resource = {};

        /// @brief The index of the descriptor in a descriptor array at which binding the resource arrays starts.
        ///
        /// If the resource contains an array, the individual elements (*layers* for images) will be bound, starting at this descriptor. The first element/layer to be bound is identified by @ref firstElement. The
        /// number of elements/layers to be bound is stored in @ref elements.
        ///
        /// @see firstElement
        /// @see elements
        UInt32 firstDescriptor = 0;

        /// @brief The index of the first array element or image layer to bind, starting at @ref firstDescriptor.
        ///
        /// This property is ignored, if the resource is a @ref ISampler.
        ///
        /// @see firstDescriptor
        UInt32 firstElement = 0;

        /// @brief The number of array elements or image layers to bind, starting at @ref firstDescriptor.
        ///
        /// This property is ignored, if the resource is a @ref ISampler.
        ///
        /// @see firstDescriptor
        UInt32 elements = 0;

        /// @brief If the resource is an image, this describes the first level to be bound.
        ///
        /// This property is ignored, if the resource is a @ref ISampler or @ref IBuffer.
        UInt32 firstLevel = 0;

        /// @brief If the resource is an image, this describes the number of levels to be bound.
        ///
        /// This property is ignored, if the resource is a @ref ISampler or @ref IBuffer.
        UInt32 levels = 0;
    };

    /// @brief The interface for a descriptor set layout.
    class LITEFX_RENDERING_API IDescriptorSetLayout : public SharedObject {
    protected:
        IDescriptorSetLayout() noexcept = default;
        IDescriptorSetLayout(const IDescriptorSetLayout&) = default;
        IDescriptorSetLayout(IDescriptorSetLayout&&) noexcept = default;
        IDescriptorSetLayout& operator=(const IDescriptorSetLayout&) = default;
        IDescriptorSetLayout& operator=(IDescriptorSetLayout&&) noexcept = default;

    public:
        ~IDescriptorSetLayout() noexcept override = default;

    public:
        /// @brief Returns the layouts of the descriptors within the descriptor set.
        ///
        /// @return The layouts of the descriptors within the descriptor set.
        inline Enumerable<const IDescriptorLayout&> descriptors() const noexcept {
            return this->getDescriptors();
        }

        /// @brief Returns the descriptor layout for the descriptor bound to the binding point provided with @p binding.
        ///
        /// @param binding The binding point of the requested descriptor layout.
        /// @return The descriptor layout for the descriptor bound to the binding point provided with @p binding.
        virtual const IDescriptorLayout& descriptor(UInt32 binding) const = 0;

        /// @brief Returns the space index of the descriptor set.
        ///
        /// The descriptor set space maps to the space index in HLSL and the set index in GLSL.
        ///
        /// @return The space index of the descriptor set.
        virtual UInt32 space() const noexcept = 0;

        /// @brief Returns the shader stages, the descriptor set is used in.
        ///
        /// @return The shader stages, the descriptor set is used in.
        virtual ShaderStage shaderStages() const noexcept = 0;

        /// @brief Returns the number of uniform/constant buffer descriptors within the descriptor set.
        ///
        /// @return The number of uniform/constant buffer descriptors.
        virtual UInt32 uniforms() const noexcept = 0;

        /// @brief Returns the number of structured and byte address buffer descriptors within the descriptor set.
        ///
        /// @return The number of structured and byte address buffer descriptors.
        virtual UInt32 storages() const noexcept = 0;

        /// @brief Returns the number of image (i.e. texture) descriptors within the descriptor set.
        ///
        /// @return The number of image (i.e. texture) descriptors.
        virtual UInt32 images() const noexcept = 0;

        /// @brief Returns the number of texel buffer descriptors within the descriptor set.
        ///
        /// @return The number of texel buffer descriptors.
        virtual UInt32 buffers() const noexcept = 0;

        /// @brief Returns the number of dynamic sampler descriptors within the descriptor set.
        ///
        /// @return The number of dynamic sampler descriptors.
        /// @see staticSamplers
        virtual UInt32 samplers() const noexcept = 0;

        /// @brief Returns the number of static or immutable sampler descriptors within the descriptor set.
        ///
        /// @return The number of static or immutable sampler descriptors.
        /// @see samplers
        virtual UInt32 staticSamplers() const noexcept = 0;

        /// @brief Returns the number of input attachment descriptors within the descriptor set.
        ///
        /// @return The number of input attachment descriptors.
        virtual UInt32 inputAttachments() const noexcept = 0;

        /// @brief Returns `true`, if the descriptor set layout contains an unbounded runtime array and `false` otherwise.
        ///
        /// @return `true`, if the descriptor set layout contains an unbounded runtime array and `false` otherwise
        virtual bool containsUnboundedArray() const noexcept = 0;

        /// @brief Returns the offset for a descriptor within a descriptor set of this layout.
        ///
        /// @param binding The binding point for the descriptor.
        /// @param element The index of the array element of a descriptor array.
        /// @return The offset from the beginning of the descriptor set.
        virtual UInt32 getDescriptorOffset(UInt32 binding, UInt32 element = 0) const = 0;

        /// @brief Returns `true` if the descriptor set layout contains bindings for resources (i.e., bindings that aren't samplers) and `false` otherwise.
        ///
        /// @return `true` if the descriptor set layout contains bindings for resources and `false` otherwise.
        virtual bool bindsResources() const noexcept = 0;

        /// @brief Returns `true` if the descriptor set layout contains bindings for samplers and `false` otherwise.
        ///
        /// Note that this method only returns `true` if the layout binds samplers, that is, if they are not static/immutable.
        ///
        /// @return `true` if the descriptor set layout contains bindings for samplers and `false` otherwise.
        virtual bool bindsSamplers() const noexcept = 0;

    public:
        /// @brief Allocates a new descriptor set or returns an instance of an unused descriptor set.
        ///
        /// Allocating a new descriptor set may be an expensive operation. To improve performance, and prevent fragmentation, the descriptor set layout keeps track of created descriptor sets. It does this by
        /// never releasing them. Instead, when a @ref DescriptorSet instance gets destroyed, it should call @ref free in order to mark itself (i.e. its handle) as not being used any longer.
        ///
        /// Before allocating a new descriptor set from a pool (which may even result in the creation of a new pool, if the existing pools are full), the layout tries to hand out descriptor sets that marked as
        /// unused. Descriptor sets are only deleted, if the whole layout instance and therefore the descriptor pools are deleted.
        ///
        /// The above does not apply to unbounded descriptor arrays. A unbounded descriptor array is one, for which @ref IDescriptorLayout::descriptors returns `-1` (or `0xFFFFFFFF`). They must be allocated by
        /// specifying the @p descriptors parameter. This parameter defines the number of descriptors to allocate in the array.
        ///
        /// Note that descriptor sets, that contain an unbounded descriptor array must only contain one single descriptor (the one that identifies this array). Such descriptor sets are never cached. Instead, they
        /// are released when calling @ref free. It is a good practice to cache such descriptor sets as global descriptor tables once and never release them. They provide more flexibility than regular descriptor
        /// arrays, since they may be updated, even after they have been bound to a command buffer or from different threads. However, you must ensure yourself not to overwrite any descriptors that are currently
        /// in use. Because unbounded arrays are not cached, freeing and re-allocating such descriptor sets may leave the descriptor heap fragmented, which might cause the allocation to fail, if the heap is full.
        ///
        /// Note that providing bindings for descriptors of type @ref DescriptorType::ResourceDescriptorHeap or @ref DescriptorType::SamplerDescriptorHeap here is not supported and will cause an exception to be
        /// thrown.
        ///
        /// @param bindings Optional default bindings for descriptors in the descriptor set.
        /// @return The instance of the descriptor set.
        /// @see IDescriptorLayout
        inline UniquePtr<IDescriptorSet> allocate(std::initializer_list<DescriptorBinding> bindings = { }) const {
            return this->getDescriptorSet(0, bindings);
        }

        /// @copydoc allocate(std::initializer_list)
        inline UniquePtr<IDescriptorSet> allocate(Span<DescriptorBinding> bindings) const {
            return this->getDescriptorSet(0, bindings);
        }

        /// @copydoc allocate(std::initializer_list)
        inline UniquePtr<IDescriptorSet> allocate(Generator<DescriptorBinding> bindings) const {
            return this->getDescriptorSet(0, std::move(bindings));
        }

        /// @brief Allocates a new descriptor set or returns an instance of an unused descriptor set.
        ///
        /// @param descriptors The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.
        /// @param bindings Optional default bindings for descriptors in the descriptor set.
        /// @return The instance of the descriptor set.
        /// @see IDescriptorLayout
        /// @see allocate(std::initializer_list)
        inline UniquePtr<IDescriptorSet> allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const {
            return this->getDescriptorSet(descriptors, bindings);
        }

        /// @copydoc allocate(UInt32, std::initializer_list)
        inline UniquePtr<IDescriptorSet> allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const {
            return this->getDescriptorSet(descriptors, bindings);
        }

        /// @brief Allocates an array of descriptor sets.
        ///
        /// @param descriptorSets The number of descriptor sets to allocate.
        /// @param bindings A generator that generates the optional default bindings for descriptors in each descriptor set.
        /// @return The instance of the descriptor set.
        /// @see allocate(std::initializer_list)
        inline UniquePtr<IDescriptorSet> allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const {
            return this->getDescriptorSet(descriptors, std::move(bindings));
        }

        /// @brief Allocates an array of descriptor sets.
        ///
        /// @param descriptorSets The number of descriptor sets to allocate.
        /// @param bindings Optional default bindings for descriptors in each descriptor set.
        /// @return A generator that produces the descriptor set instances.
        /// @see allocate(std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const {
            return this->getDescriptorSets(descriptorSets, 0, bindings);
        }

#ifdef __cpp_lib_mdspan
        /// @copydoc allocate(UInt32, std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const {
            return this->getDescriptorSets(descriptorSets, 0, bindings);
        }
#endif

        /// @brief Allocates an array of descriptor sets.
        ///
        /// @param descriptorSets The number of descriptor sets to allocate.
        /// @param bindingFactory A factory function that is called for each descriptor in each descriptor set in order to provide the default bindings.
        /// @return A generator that produces the descriptor set instances.
        /// @see allocate(std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const {
            return this->getDescriptorSets(descriptorSets, 0, std::move(bindingFactory));
        }

        /// @brief Allocates an array of descriptor sets.
        ///
        /// @param descriptorSets The number of descriptor sets to allocate.
        /// @param descriptors The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.
        /// @param bindings Optional default bindings for descriptors in each descriptor set.
        /// @return A generator that produces the descriptor set instances.
        /// @see allocate(std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const {
            return this->getDescriptorSets(descriptorSets, descriptors, bindings);
        }

#ifdef __cpp_lib_mdspan
        /// @copydoc allocate(UInt32, UInt32, std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const {
            return this->getDescriptorSets(descriptorSets, descriptors, bindings);
        }
#endif

        /// @brief Allocates an array of descriptor sets.
        ///
        /// @param descriptorSets The number of descriptor sets to allocate.
        /// @param descriptors The number of descriptors to allocate in an unbounded descriptor array. Ignored, if the descriptor set does not contain an unbounded array.
        /// @param bindingFactory A factory function that is called for each descriptor set in order to provide the default bindings.
        /// @return A generator that produces the descriptor set instances.
        /// @see allocate(std::initializer_list)
        inline Generator<UniquePtr<IDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const {
            return this->getDescriptorSets(descriptorSets, descriptors, std::move(bindingFactory));
        }

        /// @brief Marks a descriptor set as unused, so that it can be handed out again instead of allocating a new one.
        ///
        /// @see allocate
        inline void free(const IDescriptorSet& descriptorSet) const {
            this->releaseDescriptorSet(descriptorSet);
        }

    private:
        virtual Enumerable<const IDescriptorLayout&> getDescriptors() const noexcept = 0;
        virtual UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const = 0;
        virtual UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, Span<DescriptorBinding> bindings) const = 0;
        virtual UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, Generator<DescriptorBinding> bindings) const = 0;
        virtual Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings) const = 0;
#ifdef __cpp_lib_mdspan
        virtual Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const = 0;
#endif
        virtual Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const = 0;
        virtual void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const = 0;
    };

    /// @brief Describes a range within a @ref IPushConstantsLayout.
    class LITEFX_RENDERING_API IPushConstantsRange {
    protected:
        IPushConstantsRange() noexcept = default;
        IPushConstantsRange(const IPushConstantsRange&) = default;
        IPushConstantsRange(IPushConstantsRange&&) noexcept = default;
        IPushConstantsRange& operator=(const IPushConstantsRange&) = default;
        IPushConstantsRange& operator=(IPushConstantsRange&&) noexcept = default;

    public:
        virtual ~IPushConstantsRange() noexcept = default;

    public:
        /// @brief Returns the shader space the push constants can be accessed from.
        ///
        /// @return The shader space the push constants can be accessed from.
        virtual UInt32 space() const noexcept = 0;

        /// @brief Returns the binding point or register, the push constants are made available at.
        ///
        /// @return The binding point or register, the push constants are made available at.
        virtual UInt32 binding() const noexcept = 0;

        /// @brief Returns the offset from the push constants backing memory block, the range starts at.
        ///
        /// @return The offset from the push constants backing memory block, the range starts at.
        /// @see size
        virtual UInt32 offset() const noexcept = 0;

        /// @brief Returns the size (in bytes) of the range.
        ///
        /// @return The size (in bytes) of the range.
        /// @see offset
        virtual UInt32 size() const noexcept = 0;

        /// @brief Returns the shader stage(s), the range is accessible from.
        ///
        /// @return The shader stage(s), the range is accessible from.
        virtual ShaderStage stageMask() const noexcept = 0;
    };

    /// @brief The interface for a push constants layout.
    class LITEFX_RENDERING_API IPushConstantsLayout {
    protected:
        IPushConstantsLayout() noexcept = default;
        IPushConstantsLayout(const IPushConstantsLayout&) = default;
        IPushConstantsLayout(IPushConstantsLayout&&) noexcept = default;
        IPushConstantsLayout& operator=(const IPushConstantsLayout&) = default;
        IPushConstantsLayout& operator=(IPushConstantsLayout&&) noexcept = default;

    public:
        virtual ~IPushConstantsLayout() noexcept = default;

    public:
        /// @brief Returns the size (in bytes) of the push constants backing memory.
        ///
        /// @return The size (in bytes) of the push constants backing memory.
        virtual UInt32 size() const noexcept = 0;

        /// @brief Returns all push constant ranges.
        ///
        /// @return All push constant ranges.
        /// @see range
        inline Enumerable<const IPushConstantsRange&> ranges() const {
            return this->getRanges();
        }

    private:
        virtual Enumerable<const IPushConstantsRange&> getRanges() const = 0;
    };

    /// @brief Describes a record within a shader binding table.
    ///
    /// A shader record contains a shader group, that is either a single shader of type @ref ShaderStage::RayGeneration, @ref ShaderStage::Intersection, @ref ShaderStage::Miss or @ref ShaderStage::Callable,
    /// or a pair of types @ref ShaderStage::ClosestHit and @ref ShaderStage::AnyHit, where at least one of them needs to be set.
    ///
    /// Typically you do not want to implement this interface itself. Prefer using the @ref ShaderRecord template to create shader records instead.
    ///
    /// @see ShaderRecord
    /// @see IShaderProgram::buildShaderBindingTable
    struct LITEFX_RENDERING_API IShaderRecord {
    public:
        /// @brief Describes a hit group for a triangle mesh geometry.
        ///
        /// Note that when using this structure, at least one of the contained shaders must be set. A shader record containing a mesh geometry hit group must only be ever called by triangle mesh bottom-level
        /// acceleration structures.
        ///
        /// @see IBottomLevelAccelerationStructure
        struct MeshGeometryHitGroup {
            /// @brief The closest hit shader for the triangle mesh.
            const IShaderModule* ClosestHitShader;

            /// @brief The any hit shader for the triangle mesh.
            const IShaderModule* AnyHitShader;
        };

        /// @brief Defines the type that stores the shaders of the shader group.
        using shader_group_type = Variant<const IShaderModule*, MeshGeometryHitGroup>;

    protected:
        IShaderRecord() noexcept = default;
        IShaderRecord(const IShaderRecord&) = default;
        IShaderRecord(IShaderRecord&&) noexcept = default;
        IShaderRecord& operator=(const IShaderRecord&) = default;
        IShaderRecord& operator=(IShaderRecord&&) noexcept = default;

    public:
        constexpr virtual ~IShaderRecord() noexcept = default;

    public:
        /// @brief Returns the type of the shader record.
        ///
        /// @return The type of the shader record.
        constexpr ShaderRecordType type() const {
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
        /// @brief Returns the shader group containing the modules for this record.
        constexpr virtual const shader_group_type& shaderGroup() const noexcept = 0;

        /// @brief Returns a pointer to the shader-local data of the record.
        ///
        /// Shader-local data is a piece of constant data that is available to the shader during invocation. During a ray hit/miss event, the shader record is selected based on geometry (@ref
        /// IBottomLevelAccelerationStructure), instance (@ref ITopLevelAccelerationStructure and an implementation-specific offset. The selected record is then used to load the shader and pass the shader local
        /// data to it.
        ///
        /// @return A pointer to the shader-local data of the record.
        /// @see localDataSize
        constexpr virtual const void* localData() const noexcept = 0;

        /// @brief Returns the size of the shader-local data of the record.
        ///
        /// @return The size of the shader-local data of the record.
        /// @see localData
        constexpr virtual UInt64 localDataSize() const noexcept = 0;
    };

    /// @brief Defines a generic shader record.
    ///
    /// @see ShaderRecord
    /// @see ShaderRecord
    template <typename... TLocalData>
    struct ShaderRecord;

    /// @brief The size (in bytes) to which shader record local data must be aligned.
    const size_t SHADER_RECORD_LOCAL_DATA_ALIGNMENT { 8 };

    /// @brief Denotes a shader record containing shader-local data.
    ///
    /// The @p TLocalData defines the data that is passed to a shader's local resource bindings upon invocation. Two types of elements are allowed: buffer references and constants. Buffer references can be
    /// obtained by calling @ref IBuffer::virtualAddress and are always 8 bytes long. Constants do not strictly need to follow 8 byte alignment rules, but rather can also be smaller, in which case they should
    /// be defined as an aligned array, aligned to 8 bytes within the shader-local data.
    ///
    /// @see https://github.com/crud89/LiteFX/wiki/Raytracing#local-resource-bindings
    template <typename TLocalData> requires 
        (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT) &&
        std::is_standard_layout_v<TLocalData>
    struct ShaderRecord<TLocalData> final : public IShaderRecord {
    public:
        using shader_group_type = IShaderRecord::shader_group_type;

    private:
        /// @brief Stores the shader-local data of the shader record, that gets passed to the shader local data.
        TLocalData m_payload;
        
        /// @brief Stores the shader group.
        shader_group_type m_shaderGroup;

    public:
        /// @copydoc IShaderRecord::shaderGroup()
        constexpr const shader_group_type& shaderGroup() const noexcept override {
            return m_shaderGroup;
        }

        /// @copydoc IShaderRecord::localData()
        constexpr const void* localData() const noexcept override {
            return static_cast<const void*>(&m_payload);
        }

        /// @copydoc IShaderRecord::localDataSize()
        constexpr UInt64 localDataSize() const noexcept override {
            return sizeof(TLocalData);
        }

    public:
        ShaderRecord() = delete;

        /// @copydoc IShaderRecord::~IShaderRecord()
        constexpr ~ShaderRecord() noexcept override = default;

        /// @brief Initializes a shader record.
        ///
        /// @param group The shader group containing the modules to invoke.
        /// @param payload The shader-local data to pass to the shader's local resource bindings.
        ShaderRecord(const shader_group_type& group, TLocalData payload) noexcept :
            m_payload(payload), m_shaderGroup(group) { }

        /// @brief Copies another shader record.
        ///
        /// @param _other The shader record to copy.
        constexpr ShaderRecord(const ShaderRecord& _other) = default;

        /// @brief Takes over another shader record.
        ///
        /// @param _other The shader record to take over.
        constexpr ShaderRecord(ShaderRecord&& _other) noexcept = default;

        /// @brief Copies another shader record.
        ///
        /// @param _other The shader record to copy.
        /// @return A reference to the current shader record.
        constexpr ShaderRecord& operator=(const ShaderRecord& _other) = default;

        /// @brief Takes over another shader record.
        ///
        /// @param _other The shader record to take over.
        /// @return A reference to the current shader record.
        constexpr ShaderRecord& operator=(ShaderRecord&& _other) noexcept = default;
    };

    /// @brief Denotes a shader record containing no shader-local data.
    template <>
    struct ShaderRecord<> final : public IShaderRecord {
    public:
        using shader_group_type = IShaderRecord::shader_group_type;

    private:
        /// @brief Stores the shader group.
        shader_group_type m_shaderGroup;

    public:
        /// @copydoc IShaderRecord::shaderGroup()
        constexpr const shader_group_type& shaderGroup() const noexcept override {
            return m_shaderGroup;
        }

        /// @copydoc IShaderRecord::localData()
        constexpr const void* localData() const noexcept override {
            return nullptr;
        }

        /// @copydoc IShaderRecord::localDataSize()
        constexpr UInt64 localDataSize() const noexcept override {
            return 0_ui64;
        }

    public:
        ShaderRecord() = delete;

        /// @copydoc IShaderRecord::~IShaderRecord()
        constexpr ~ShaderRecord() noexcept override = default;

        /// @brief Initializes a shader record.
        ///
        /// @param group The shader group containing the modules to invoke.
        ShaderRecord(const shader_group_type& group) noexcept :
            m_shaderGroup(group) { }

        /// @brief Copies another shader record.
        ///
        /// @param _other The shader record to copy.
        constexpr ShaderRecord(const ShaderRecord& _other) = default;

        /// @brief Takes over another shader record.
        ///
        /// @param _other The shader record to take over.
        constexpr ShaderRecord(ShaderRecord&& _other) noexcept = default;

        /// @brief Copies another shader record.
        ///
        /// @param _other The shader record to copy.
        /// @return A reference to the current shader record.
        constexpr ShaderRecord& operator=(const ShaderRecord& _other) = default;

        /// @brief Takes over another shader record.
        ///
        /// @param _other The shader record to take over.
        /// @return A reference to the current shader record.
        constexpr ShaderRecord& operator=(ShaderRecord&& _other) noexcept = default;
    };

    /// @brief Stores a set of @ref IShaderRecords in that later form a shader binding table used for ray-tracing.
    class LITEFX_RENDERING_API ShaderRecordCollection final {
        friend class IShaderProgram;

    private:
        SharedPtr<const IShaderProgram> m_program;
        Array<UniquePtr<const IShaderRecord>> m_records;

        /// @brief Initializes a new shader record collection.
        ///
        /// @param shaderProgram The shader program that contains the shader modules
        ShaderRecordCollection(SharedPtr<const IShaderProgram> shaderProgram) noexcept : 
            m_program(std::move(shaderProgram)) 
        {
            // This can only be built from a shader program, which passes the pointer to itself, which must not be nullptr. If more factory methods are added,
            // we must validate the program pointer here.
        }

    public:
        ShaderRecordCollection() = delete;

        ShaderRecordCollection(ShaderRecordCollection&&) noexcept = default;
        ShaderRecordCollection& operator=(ShaderRecordCollection&&) noexcept = default;
        ShaderRecordCollection(const ShaderRecordCollection&) = delete;
        ShaderRecordCollection& operator=(const ShaderRecordCollection&) = delete;
        ~ShaderRecordCollection() noexcept = default;

    private:
        /// @brief Finds a shader module in the parent shader program.
        ///
        /// @param name The case-sensitive name of the shader module to find.
        /// @return A pointer to the shader module, or `nullptr`, if no module with the specified name was found in the parent program.
        const IShaderModule* findShaderModule(StringView name) const noexcept;

    public:
        /// @brief Returns the parent shader program of the collection.
        ///
        /// @return The parent shader program of the collection.
        inline SharedPtr<const IShaderProgram> program() const noexcept {
            return m_program;
        }

        /// @brief Returns an array of all shader records within the shader record collection.
        ///
        /// @return The array containing all shader records within the shader record collection.
        const Array<UniquePtr<const IShaderRecord>>& shaderRecords() const noexcept;

        /// @brief Adds a new shader record to the shader record collection.
        ///
        /// @param record The shader record to add to the shader record collection.
        /// @throws ArgumentNotInitializedException Thrown, if the shader record was not initialized.
        /// @throws InvalidArgumentException Thrown, if the shader module(s) within the shader record are of invalid type, or the parent shader program does not contain the shader module(s).
        void addShaderRecord(UniquePtr<const IShaderRecord>&& record);

    public:
        /// @brief Adds a new shader record based on the name of a shader module in the parent shader program.
        ///
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an any and closest hit shader, use @ref
        /// addMeshGeometryShaderHitGroupRecord instead.
        ///
        /// @param shaderName The name of the shader module.
        /// @throws InvalidArgumentException Thrown, if no shader module with the provided name was found in the parent shader program.
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

        /// @brief Adds a new shader record based on the name of a shader module in the parent shader program.
        ///
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an any and closest hit shader, use @ref
        /// addMeshGeometryShaderHitGroupRecord instead.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param shaderName The name of the shader module.
        /// @param payload The shader-local data of the shader record.
        /// @throws InvalidArgumentException Thrown, if no shader module with the provided name was found in the parent shader program.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
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

        /// @brief Adds a new mesh geometry hit group record based on names of the shader modules.
        ///
        /// @param anyHitShaderName The name of the any hit shader module.
        /// @param closestHitShaderName The name of the closest hit shader module.
        /// @throws InvalidArgumentException Thrown, if both provided shader names are empty or not found, the shaders are not of the right type or do not belong to the parent shader program.
        inline void addMeshGeometryShaderHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName) {
            IShaderRecord::MeshGeometryHitGroup hitGroup = { 
                .ClosestHitShader = closestHitShaderName.has_value() ? this->findShaderModule(closestHitShaderName.value()) : nullptr,
                .AnyHitShader = anyHitShaderName.has_value() ? this->findShaderModule(anyHitShaderName.value()) : nullptr
            };

            this->addShaderRecord(makeUnique<ShaderRecord<>>(hitGroup));
        }

        /// @brief Adds a new mesh geometry hit group record based on names of the shader modules.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param anyHitShaderName The name of the any hit shader module.
        /// @param closestHitShaderName The name of the closest hit shader module.
        /// @param payload The shader-local data of the shader record.
        /// @throws InvalidArgumentException Thrown, if both provided shader names are empty or not found, the shaders are not of the right type or do not belong to the parent shader program.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
        inline void addMeshGeometryShaderHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName, TLocalData payload) {
            IShaderRecord::MeshGeometryHitGroup hitGroup = { 
                .ClosestHitShader = closestHitShaderName.has_value() ? this->findShaderModule(closestHitShaderName.value()) : nullptr,
                .AnyHitShader = anyHitShaderName.has_value() ? this->findShaderModule(anyHitShaderName.value()) : nullptr
            };

            this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(hitGroup, payload));
        }

        /// @brief Adds a new shader record to the shader record collection.
        ///
        /// @param shaderGroup The shader module or hit group.
        inline void addShaderRecord(const ShaderRecord<>::shader_group_type& shaderGroup) {
            this->addShaderRecord(makeUnique<ShaderRecord<>>(shaderGroup));
        }

        /// @brief Adds a new shader record to the shader record collection.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param shaderGroup The shader module or hit group.
        /// @param payload The shader-local data of the shader record.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
        inline void addShaderRecord(ShaderRecord<TLocalData>::shader_group_type shaderGroup, TLocalData payload) {
            this->addShaderRecord(makeUnique<ShaderRecord<TLocalData>>(shaderGroup, payload));
        }

        /// @brief Adds a new shader record based on the name of a shader module in the parent shader program.
        ///
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an any and closest hit shader, use @ref
        /// withMeshGeometryShaderHitGroupRecord instead.
        ///
        /// @param shaderName The name of the shader module.
        /// @return A reference to the current shader record collection.
        inline ShaderRecordCollection&& withShaderRecord(StringView shaderName) {
            this->addShaderRecord(shaderName);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// @brief Adds a new shader record based on the name of a shader module in the parent shader program.
        ///
        /// Note that this will create a new shader record for every invocation. If you want to create a shader record with a mesh geometry hit group with containing both, an any and closest hit shader, use @ref
        /// withMeshGeometryShaderHitGroupRecord instead.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param shaderName
        /// @param payload The shader-local data of the shader record.
        /// @return A reference to the current shader record collection.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
        inline ShaderRecordCollection&& withShaderRecord(StringView shaderName, TLocalData payload) {
            this->addShaderRecord(shaderName, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// @brief Adds a new mesh geometry hit group record based on names of the shader modules.
        ///
        /// @param anyHitShaderName The name of the any hit shader module.
        /// @param closestHitShaderName The name of the closest hit shader module.
        /// @return A reference to the current shader record collection.
        inline ShaderRecordCollection&& withMeshGeometryHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName) {
            this->addMeshGeometryShaderHitGroupRecord(anyHitShaderName, closestHitShaderName);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// @brief Adds a new mesh geometry hit group record based on names of the shader modules.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param anyHitShaderName The name of the any hit shader module.
        /// @param closestHitShaderName The name of the closest hit shader module.
        /// @param payload The shader-local data of the shader record.
        /// @return A reference to the current shader record collection.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
        inline ShaderRecordCollection&& withMeshGeometryHitGroupRecord(std::optional<StringView> anyHitShaderName, std::optional<StringView> closestHitShaderName, TLocalData payload) {
            this->addMeshGeometryShaderHitGroupRecord(anyHitShaderName, closestHitShaderName, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// @brief Adds a new shader record to the shader record collection.
        ///
        /// @param shaderGroup The shader module or hit group.
        /// @return A reference to the current shader record collection.
        inline ShaderRecordCollection&& withShaderRecord(ShaderRecord<>::shader_group_type shaderGroup) {
            this->addShaderRecord(shaderGroup);
            return std::forward<ShaderRecordCollection>(*this);
        }

        /// @brief Adds a new shader record to the shader record collection.
        ///
        /// @tparam TLocalData The type of the shader record local data.
        /// @param shaderGroup The shader module or hit group.
        /// @param payload The shader-local data of the shader record.
        /// @return A reference to the current shader record collection.
        template <typename TLocalData> requires (std::alignment_of_v<TLocalData> == SHADER_RECORD_LOCAL_DATA_ALIGNMENT)
        inline ShaderRecordCollection&& withShaderRecord(ShaderRecord<TLocalData>::shader_group_type shaderGroup, TLocalData payload) {
            this->addShaderRecord(shaderGroup, payload);
            return std::forward<ShaderRecordCollection>(*this);
        }
    };

    /// @brief A hint used during shader reflection to control the pipeline layout.
    ///
    /// Hints are generally used to express the desired layout to backends that cannot infer them implicitly. They do not imply an enforcement of the layout otherwise. For example, hinting a push constants
    /// range when performing shader reflection in Vulkan, where push constants are supported by the reflection library, will not affect the ultimate decision on whether the layout will contain a push
    /// constants range. In this case, shader reflection will always emit a push constants range.
    ///
    /// Backends do emit diagnostic log messages, if a hint is given that it will ignore. Hints for descriptors that are not bound will silently be ignored.
    ///
    /// @see IShaderProgram::reflectPipelineLayout
    struct LITEFX_RENDERING_API PipelineBindingHint {

        /// @brief Defines a hint that is used to mark an unbounded descriptor array.
        struct UnboundedArrayHint {
            /// @brief If the binding point binds an array, this property can be used to turn it into an unbounded array and set the maximum number of descriptors that can be bound to the array. This is especially
            /// useful to comply with Vulkan device limits.
            UInt32 MaxDescriptors{ 0 };
        };

        /// @brief Defines a hint that is used to mark a push constants range.
        struct PushConstantsHint {
            /// @brief If the binding point binds a constant or uniform buffer, setting this property to `true` will configure the binding point it as part of the root constants for the pipeline layout. If this
            /// property is set to `false`, the hint will have no effect.
            bool AsPushConstants{ false };
        };

        /// @brief Defines a hint that is used to bind a static sampler state to a sampler descriptor.
        struct StaticSamplerHint {
            /// @brief If the binding point binds a sampler, setting this property will bind a static or constant sampler, if supported by the backend.
            SharedPtr<ISampler> StaticSampler{ nullptr };
        };

        /// @brief Defines a hint that is used to initialize a dynamic descriptor heap.
        ///
        /// This hint is special, as it must not be associated with an existing binding. Instead, dynamic descriptor heaps use a proxy descriptor set to bind resources to the global descriptor heaps, that can
        /// later be directly indexed by the shader. In the DirectX 12 backend, this hint will cause a new descriptor set to be created, that is not part of the pipeline state. In Vulkan, this functionality is
        /// emulated using the `VK_EXT_mutable_descriptor_type` extension and binds to an existing descriptor set. DXC emits this descriptor set automatically, if direct heap indexing is used from a shader.
        ///
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        /// @see DescriptorType::ResourceDescriptorHeap
        /// @see DescriptorType::SamplerDescriptorHeap
        struct DescriptorHeapHint {
            /// @brief The desired type of the descriptor heap.
            DescriptorHeapType Type{ DescriptorHeapType::None };

            /// @brief The number of descriptors allocated for the heap.
            UInt32 HeapSize{ 1u };
        };

        /// @brief Defines a hint that is used to mark additional a binding as used by certain shader stages.
        ///
        /// Shader stages are set on the descriptor set, so the binding register for this hint is ignored. If specified, this hint causes shader stages that are not found in shader reflection to be included in
        /// the shader stage mask. Usually reflection leaves out shader stages that a descriptor set is not bound to, as this can improve performance. However, if a descriptor set can be re-used between
        /// compatible pipeline layouts, shader reflection may be unable to tell which shader stages the descriptor set is actually accessed from. In those scenarios, additional stages can be provided using this
        /// hint.
        ///
        /// In the DirectX 12 backend, shader stages aren't actually masked. Rather a binding is only accessible to a single shader stage, or all shader stages. However, in Vulkan this can be specified with finer
        /// granularity.
        struct ShaderStageHint {
            /// @brief A mask that contains the shader stages, that the descriptor set should be accessible from.
            ShaderStage Stages{ ShaderStage::Other };
        };

        /// @brief Defines the type of the pipeline binding hint.
        using hint_type = Variant<std::monostate, UnboundedArrayHint, PushConstantsHint, StaticSamplerHint, DescriptorHeapHint, ShaderStageHint>;

        /// @brief The binding point the hint applies to.
        DescriptorBindingPoint Binding{ };

        /// @brief Stores the underlying hint.
        hint_type Hint = std::monostate{ };

    public:
        /// @brief Initializes a hint that binds an unbounded runtime array.
        ///
        /// @param at The binding point the hint applies to.
        /// @param maxDescriptors The maximum number of descriptors that can be bound to the runtime array at the binding point.
        /// @return The initialized pipeline binding hint.
        static inline auto runtimeArray(DescriptorBindingPoint at, UInt32 maxDescriptors) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = UnboundedArrayHint { maxDescriptors } };
        }

        /// @brief Initializes a hint that binds an unbounded runtime array.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @param maxDescriptors The maximum number of descriptors that can be bound to the runtime array at the binding point.
        /// @return The initialized pipeline binding hint.
        static inline auto runtimeArray(UInt32 space, UInt32 binding, UInt32 maxDescriptors) noexcept -> PipelineBindingHint {
            return { .Binding = { .Register = binding, .Space = space }, .Hint = UnboundedArrayHint { maxDescriptors } };
        }

        /// @brief Initializes a hint that binds push constants.
        ///
        /// @param at The binding point the hint applies to.
        /// @return The initialized pipeline binding hint.
        static inline auto pushConstants(DescriptorBindingPoint at) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = PushConstantsHint { true } };
        }

        /// @brief Initializes a hint that binds push constants.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @return The initialized pipeline binding hint.
        static inline auto pushConstants(UInt32 space, UInt32 binding) noexcept -> PipelineBindingHint {
            return { .Binding = { .Register = binding, .Space = space }, .Hint = PushConstantsHint { true } };
        }

        /// @brief Initializes a hint that binds a static sampler, if supported by the backend.
        ///
        /// @param at The binding point the hint applies to.
        /// @param sampler The sampler state used to initialize the static sampler with.
        /// @return The initialized pipeline binding hint.
        static inline auto staticSampler(DescriptorBindingPoint at, SharedPtr<ISampler> sampler) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = StaticSamplerHint { std::move(sampler) } };
        }

        /// @brief Initializes a hint that binds a static sampler, if supported by the backend.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @param sampler The sampler state used to initialize the static sampler with.
        /// @return The initialized pipeline binding hint.
        static inline auto staticSampler(UInt32 space, UInt32 binding, SharedPtr<ISampler> sampler) noexcept -> PipelineBindingHint {
            return { .Binding = { .Register = binding, .Space = space }, .Hint = StaticSamplerHint { std::move(sampler) } };
        }

        /// @brief Initializes a hint that binds a proxy descriptor set to access the resource heap at the provided binding point.
        ///
        /// @param at The binding point the hint applies to.
        /// @param heapSize The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto resourceHeap(DescriptorBindingPoint at, UInt32 heapSize) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = DescriptorHeapHint { DescriptorHeapType::Resource, heapSize } };
        }

        /// @brief Initializes a hint that binds a proxy descriptor set to access the resource heap at the provided binding point.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @param heapSize The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto resourceHeap(UInt32 space, UInt32 binding, UInt32 heapSize) noexcept -> PipelineBindingHint {
            return { .Binding = { .Register = binding, .Space = space }, .Hint = DescriptorHeapHint { DescriptorHeapType::Resource, heapSize } };
        }

        /// @brief Initializes a hint that binds a proxy descriptor set to access the sampler heap at the provided binding point.
        ///
        /// @param at The binding point the hint applies to.
        /// @param heapSize The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto samplerHeap(DescriptorBindingPoint at, UInt32 heapSize) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = DescriptorHeapHint { DescriptorHeapType::Sampler, heapSize } };
        }

        /// @brief Initializes a hint that binds a proxy descriptor set to access the sampler heap at the provided binding point.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @param heapSize The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto samplerHeap(UInt32 space, UInt32 binding, UInt32 heapSize) noexcept -> PipelineBindingHint {
            return { .Binding = { .Register = binding, .Space = space }, .Hint = DescriptorHeapHint { DescriptorHeapType::Sampler, heapSize } };
        }

        /// @brief Initializes a hint provides additional shader stages, that may be not covered by shader reflection.
        ///
        /// @param at The binding point the hint applies to.
        /// @param shaderStages The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto shaderStage(DescriptorBindingPoint at, ShaderStage shaderStages) noexcept -> PipelineBindingHint {
            return { .Binding = at, .Hint = ShaderStageHint { shaderStages } };
        }

        /// @brief Initializes a hint provides additional shader stages, that may be not covered by shader reflection.
        ///
        /// @param space The descriptor space of the binding point.
        /// @param binding The register of the descriptor binding point.
        /// @param shaderStages The number of descriptors allocated for the heap when creating the descriptor set.
        /// @return The initialized pipeline binding hint.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        static inline auto shaderStage(UInt32 space, UInt32 binding, ShaderStage shaderStages) noexcept -> PipelineBindingHint {
            return { .Binding = {.Register = binding, .Space = space }, .Hint = ShaderStageHint { shaderStages } };
        }
    };

    /// @brief The interface for a shader program.
    ///
    /// A shader program differs in it's functionality as well as the contained shader modules, depending on the pipeline type it gets assigned to. A shader program can be of any of the following types:
    ///
    /// - **Rasterization:** A rasterization pipeline is a traditional pipeline, that can contain at maximum one module of the following stages: *Vertex*, *Tessellation Control*, *Tessellation Evaluation*, *Geometry*, *Fragment*. A vertex and fragment shader are required for rasterization programs.
    /// - **Mesh shading:** If mesh shader support is enabled (through the device feature @ref GraphicsDeviceFeatures::MeshShaders), a mesh shading program can contain at maximum one module of the following stages: *Task*, *Mesh*, *Fragment*. A mesh and fragment shader are required for a mesh shading program.
    /// - **Compute:** A compute shader program must only contain a single module for the *Compute* stage.
    /// - **Ray-tracing:** If ray tracing support is enabled (through the device feature @ref GraphicsDeviceFeatures::RayTracing), a ray tracing program can contain modules of the following stages: *Ray Generation*, *Any Hit*, *Closest Hit*, *Intersection*, *Miss*, *Callable*. There must be exactly one *Ray Generation* module. All other modules can occur multiple times. To build a ray tracing pipeline, all shaders should be added to a single shader program, which is then passed to the pipeline during creation.
    ///
    /// Shaders from different program types must not be mixed. For example, it is not valid to add a compute module to a rasterization program. The only exception to this is the @ref ShaderStage::Fragment
    /// module, which can be added to a mesh pipeline, as well as a rasterization pipeline.
    ///
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_RENDERING_API IShaderProgram : public SharedObject {
    protected:
        IShaderProgram() noexcept = default;

    public:
        ~IShaderProgram() noexcept override = default;

        IShaderProgram(const IShaderProgram&) = delete;
        IShaderProgram(IShaderProgram&&) noexcept = delete;
        auto operator=(const IShaderProgram&) = delete;
        auto operator=(IShaderProgram&&) noexcept = delete;

    public:
        /// @brief Returns a pointer to shader module based on its (case-sensitive) name.
        ///
        /// @param name The name or file name of the shader module.
        /// @return A pointer to the shader module, or `nullptr`, if it was not found.
        inline const IShaderModule* operator[](StringView name) const {
            auto modules = this->getModules();

            if (auto match = std::ranges::find_if(modules, [name](auto& module) { return module.fileName().compare(name) == 0; }); match != modules.end())
                return std::addressof(*match);

            return nullptr;
        }

        /// @brief Returns `true`, if the program contains a shader module with the provided name or file name and `false` otherwise.
        ///
        /// @param name The case-sensitive name or file name of the shader module to look up.
        /// @return `true`, if the program contains a shader module with the provided name or file name and `false` otherwise.
        inline bool contains(StringView name) const {
            auto modules = this->getModules();
            return std::ranges::find_if(modules, [name](const auto& module) { return module.fileName().compare(name) == 0; }) != modules.end();
        };

        /// @brief Returns `true`, if the program contains the provided shader module and `false` otherwise.
        ///
        /// @param module The module to look up in the shader program.
        /// @return `true`, if the program contains the provided shader module and `false` otherwise.
        inline bool contains(const IShaderModule& module) const {
            auto modules = this->getModules();
            return std::ranges::find_if(modules, [&module](const auto& m) { return std::addressof(m) == std::addressof(module); }) != modules.end();
        };

        /// @brief Returns the modules, the shader program is build from.
        ///
        /// @return The modules, the shader program is build from.
        inline Enumerable<const IShaderModule&> modules() const {
            return this->getModules();
        }

        /// @brief Uses shader reflection to extract the pipeline layout of a shader. May not be available in all backends.
        ///
        /// Note that shader reflection may not yield different results than you would expect, especially when using DirectX 12. For more information on how to use shader reflection and how to write portable
        /// shaders, refer to the [shader development guide](https://github.com/crud89/LiteFX/wiki/Shader-Development) in the wiki.
        ///
        /// In particular, shader reflection is not able to restore:
        ///
        /// - Input attachments in DirectX. Instead, input attachments are treated as `DescriptorType::Texture`. This is usually not a problem, since DirectX does not have a concept of render pass outputs/inputs anyway. However, keep this in mind, if you want to filter descriptors based on their type, for example.
        ///
        /// @param hints A series of individual binding hints to use to deduce explicit binding information.
        /// @return The pipeline layout extracted from shader reflection.
        /// @see PipelineBindingHint
        /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
        inline SharedPtr<IPipelineLayout> reflectPipelineLayout(Enumerable<PipelineBindingHint> hints = {}) const {
            return this->parsePipelineLayout(hints);
        };

        /// @brief Builds a shader record collection based on the current shader program.
        ///
        /// @return The shader record collection instance.
        [[nodiscard]] inline ShaderRecordCollection buildShaderRecordCollection() const {
            return { this->shared_from_this() };
        }

    private:
        virtual Enumerable<const IShaderModule&> getModules() const = 0;
        virtual SharedPtr<IPipelineLayout> parsePipelineLayout(Enumerable<PipelineBindingHint> hints) const = 0;
    };

    /// @brief The interface for a pipeline layout.
    class LITEFX_RENDERING_API IPipelineLayout : public SharedObject {
    protected:
        IPipelineLayout() noexcept = default;
        IPipelineLayout(const IPipelineLayout&) = default;
        IPipelineLayout(IPipelineLayout&&) noexcept = default;
        IPipelineLayout& operator=(const IPipelineLayout&) = default;
        IPipelineLayout& operator=(IPipelineLayout&&) noexcept = default;

    public:
        ~IPipelineLayout() noexcept override = default;

    public:
        ///// @brief Returns a reference to the parent device.
        /////
        ///// @return A reference to the parent device.
        //virtual const IGraphicsDevice& device() const noexcept;

        /// @brief Returns the descriptor set layout for the descriptor set that is bound to the space provided by @p space.
        ///
        /// @param space The space to request the descriptor set layout for.
        /// @return The descriptor set layout for the descriptor set that is bound to the space provided by @p space.
        virtual const IDescriptorSetLayout& descriptorSet(UInt32 space) const = 0;

        /// @brief Returns all descriptor set layouts, the pipeline has been initialized with.
        ///
        /// @return All descriptor set layouts, the pipeline has been initialized with.
        inline Enumerable<SharedPtr<const IDescriptorSetLayout>> descriptorSets() const {
            return this->getDescriptorSets();
        }

        /// @brief Returns the push constants layout, or `nullptr`, if the pipeline does not use any push constants.
        ///
        /// @return The push constants layout, or `nullptr`, if the pipeline does not use any push constants.
        virtual const IPushConstantsLayout* pushConstants() const noexcept = 0;

        /// @brief Returns `true`, if the pipeline supports directly indexing into a resource heap and `false` otherwise.
        ///
        /// @return `true`, if the pipeline supports directly indexing into a resource heap and `false` otherwise
        virtual bool dynamicResourceHeapAccess() const = 0;

        /// @brief Returns `true`, if the pipeline supports directly indexing into a sampler heap and `false` otherwise.
        ///
        /// @return `true`, if the pipeline supports directly indexing into a sampler heap and `false` otherwise
        virtual bool dynamicSamplerHeapAccess() const = 0;

    private:
        virtual Enumerable<SharedPtr<const IDescriptorSetLayout>> getDescriptorSets() const = 0;
    };

    /// @brief The interface for an input assembler state.
    class LITEFX_RENDERING_API IInputAssembler : public SharedObject {
    protected:
        IInputAssembler() noexcept = default;
        IInputAssembler(const IInputAssembler&) = default;
        IInputAssembler(IInputAssembler&&) noexcept = default;
        IInputAssembler& operator=(const IInputAssembler&) = default;
        IInputAssembler& operator=(IInputAssembler&&) noexcept = default;

    public:
        ~IInputAssembler() noexcept override = default;

    public:
        /// @brief Returns all vertex buffer layouts of the input assembly.
        ///
        /// @return All vertex buffer layouts of the input assembly.
        inline Enumerable<const IVertexBufferLayout&> vertexBufferLayouts() const {
            return this->getVertexBufferLayouts();
        }

        /// @brief Returns a pointer the vertex buffer layout for binding provided with @p binding.
        ///
        /// @param binding The binding point of the vertex buffer layout.
        /// @return The vertex buffer layout for binding provided with @p binding.
        /// @throws ArgumentOutOfRangeException Thrown, if no vertex buffer layout is bound to @p binding.
        virtual const IVertexBufferLayout& vertexBufferLayout(UInt32 binding) const = 0;

        /// @brief Returns a pointer to the index buffer layout, or `nullptr` if the input assembler does not handle indices.
        ///
        /// @return The index buffer layout, or `nullptr` if the input assembler does not handle indices.
        virtual const IIndexBufferLayout* indexBufferLayout() const noexcept = 0;

        /// @brief Returns the primitive topology.
        ///
        /// @return The primitive topology.
        virtual PrimitiveTopology topology() const noexcept = 0;

        /// @brief The number of control points for a patch, if the @ref topology is set to `PrimitiveTopology::PatchList`.
        ///
        /// @return The number of control points for a patch.
        virtual UInt32 controlPoints() const noexcept = 0;

    private:
        virtual Enumerable<const IVertexBufferLayout&> getVertexBufferLayouts() const = 0;
    };

    /// @brief The interface for a pipeline.
    ///
    /// @see IComputePipeline
    /// @see IRenderPipeline
    /// @see IRayTracingPipeline
    class LITEFX_RENDERING_API IPipeline : public virtual IStateResource {
    protected:
        IPipeline() noexcept = default;
        IPipeline(const IPipeline&) = delete;
        IPipeline(IPipeline&&) noexcept = default;
        IPipeline& operator=(const IPipeline&) = delete;
        IPipeline& operator=(IPipeline&&) noexcept = default;

    public:
        ~IPipeline() noexcept override = default;

    public:
        /// @brief Returns the shader program used by the pipeline.
        ///
        /// @return The shader program used by the pipeline.
        inline SharedPtr<const IShaderProgram> program() const noexcept {
            return this->getProgram();
        }

        /// @brief Returns the layout of the render pipeline.
        ///
        /// @return The layout of the render pipeline.
        inline SharedPtr<const IPipelineLayout> layout() const noexcept {
            return this->getLayout();
        }

    private:
        virtual SharedPtr<const IShaderProgram> getProgram() const noexcept = 0;
        virtual SharedPtr<const IPipelineLayout> getLayout() const noexcept = 0;
    };

    /// @brief The interface for a command buffer.
    class LITEFX_RENDERING_API ICommandBuffer : public SharedObject {
    public:
        friend class ICommandQueue;

    protected:
        ICommandBuffer() noexcept = default;
        ICommandBuffer(ICommandBuffer&&) noexcept = default;
        ICommandBuffer(const ICommandBuffer&) = default;
        ICommandBuffer& operator=(const ICommandBuffer&) = default;
        ICommandBuffer& operator=(ICommandBuffer&&) noexcept = default;

    public:
        ~ICommandBuffer() noexcept override = default;

    public:
        /// @brief Sets the command buffer into recording state, so that it can receive command that should be submitted to the parent @ref CommandQueue.
        ///
        /// Note that you have to wait for a command buffer to be executed on the parent @ref CommandQueue before you can begin recording on it again.
        ///
        /// @throws RuntimeException Thrown, if the command buffer is already recording.
        /// @see end
        virtual void begin() const = 0;

        /// @brief Ends recording commands on the command buffer.
        ///
        /// It is valid to call this method multiple times. If a command buffer is already closed, nothing will happen.
        ///
        /// @see begin
        virtual void end() const = 0;

        /// @brief Returns `true`, if the command buffer is a secondary command buffer, or `false` otherwise.
        ///
        /// @return `true`, if the command buffer is a secondary command buffer, or `false` otherwise.
        virtual bool isSecondary() const noexcept = 0;

        /// @brief Sets up tracking for a buffer, so that it will not be destroyed until the command buffer has been executed.
        ///
        /// When working with resources, often times you only need them for a single execution cycle of a command buffer. Having to manually check if the command buffer has been executed (by waiting for it's
        /// submission fence on the target queue) and releasing the resource afterwards can be difficult and intricate. Resource tracking allows the command buffer to store a reference of a resource and releasing
        /// it at some point after the command buffer has been executed automatically. Note that this does not automatically destroy the resource, which only happens if all references to it are released. However,
        /// if the only reference left is the one that is tracked by the command buffer, this process also destroys the resource.
        ///
        /// Resources can only be tracked, if the command buffer is currently recording. The command buffer will not track uninitialized resources, i.e., a submitted `nullptr` will be discarded.
        ///
        /// @param buffer The buffer to track.
        /// @throws RuntimeException Thrown, if the command buffer is not currently recording.
        /// @see track(SharedPtr&le;const IImage&ge;)
        /// @see track(SharedPtr&le;const ISampler&ge;)
        virtual void track(SharedPtr<const IBuffer> buffer) const = 0;

        /// @brief Sets up tracking for an image, so that it will not be destroyed until the command buffer has been executed.
        ///
        /// @param image The image to track.
        /// @throws RuntimeException Thrown, if the command buffer is not currently recording.
        /// @see track(SharedPtr&le;const IBuffer&ge;)
        virtual void track(SharedPtr<const IImage> image) const = 0;

        /// @brief Sets up tracking for a sampler state, so that it will not be destroyed until the command buffer has been executed.
        ///
        /// @param sampler The sampler to track.
        /// @throws RuntimeException Thrown, if the command buffer is not currently recording.
        /// @see track(SharedPtr&le;const IBuffer&ge;)
        virtual void track(SharedPtr<const ISampler> sampler) const = 0;

        /// @brief Sets up tracking for a descriptor set, so that bindings aren't released until the command buffer has been executed.
        ///
        /// @param descriptorSet The descriptor set to track.
        virtual void track(UniquePtr<const IDescriptorSet>&& descriptorSet) const = 0;

    public:
        /// @brief Gets a pointer to the command queue that this command buffer was allocated from or `nullptr`, if the queue has already been released.
        ///
        /// @return A reference to the command queue that this command buffer was allocated from.
        inline SharedPtr<const ICommandQueue> queue() const noexcept {
            return this->getQueue();
        }

        /// @brief Creates a new barrier instance.
        ///
        /// @param syncBefore The pipeline stage(s) all previous commands have to finish before the barrier is executed.
        /// @param syncAfter The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.
        /// @return The instance of the barrier.
        [[nodiscard]] inline UniquePtr<IBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const {
            return this->getBarrier(syncBefore, syncAfter);
        }

        /// @brief Executes the transitions that have been added to @p barrier.
        ///
        /// Calling this method will also update the resource states of each resource within the barrier. However, the actual state of the resource does not change until the barrier is executed on the command
        /// queue. Keep this in mind when inserting multiple barriers from different threads or in different command buffers, which may not be executed in order. You might have to manually synchronize barrier
        /// execution.
        ///
        /// @param barrier The barrier containing the transitions to perform.
        inline void barrier(const IBarrier& barrier) const noexcept {
            this->cmdBarrier(barrier);
        }

        /// @brief Performs a buffer-to-buffer transfer from @p source to @p target.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source buffer to transfer data from.
        /// @param target The target buffer to transfer data to.
        /// @param sourceElement The index of the first element in the source buffer to copy.
        /// @param targetElement The index of the first element in the target buffer to copy to.
        /// @param elements The number of elements to copy from the source buffer into the target buffer.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        /// @see IBarrier
        inline void transfer(const IBuffer& source, const IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, targetElement, elements);
        }
        
        /// @brief Performs a buffer-to-buffer transfer from @p source to @p target.
        ///
        /// This method takes shared ownership over @p source, which means that a reference is hold until the parent command queue finished using the command buffer. At this point, the command queue calls @ref
        /// releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been
        /// executed.
        ///
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging buffer and
        /// delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source buffer to transfer data from.
        /// @param target The target buffer to transfer data to.
        /// @param sourceElement The index of the first element in the source buffer to copy.
        /// @param targetElement The index of the first element in the target buffer to copy to.
        /// @param elements The number of elements to copy from the source buffer into the target buffer.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const SharedPtr<const IBuffer>& source, const IBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, targetElement, elements);
        }

        /// @brief Performs a buffer-to-buffer transfer from a temporary buffer into @p target.
        ///
        /// This method creates a temporary buffer and maps @p data into it, before transferring it into @p target. A reference of the temporary buffer is stored until the parent command queue finished using the
        /// command buffer. At this point, the command queue calls @ref releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls
        /// this method at some point after the command buffer has been executed.
        ///
        /// @param data The address that marks the beginning of the data to map.
        /// @param size The number of bytes to map.
        /// @param target The target buffer to transfer data to.
        /// @param targetElement The array element to map the data to.
        /// @param elements The number of elements to copy.
        inline void transfer(const void* const data, size_t size, const IBuffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const {
            this->cmdTransfer(data, size, target, targetElement, elements);
        }

        /// @brief Performs a buffer-to-buffer transfer from a temporary buffer into @p target.
        ///
        /// This method creates a temporary buffer and maps @p data into it, before transferring it into @p target. A reference of the temporary buffer is stored until the parent command queue finished using the
        /// command buffer. At this point, the command queue calls @ref releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls
        /// this method at some point after the command buffer has been executed.
        ///
        /// @param data The addresses that mark the beginning of the element data to map.
        /// @param elementSize The number of bytes to map for each element.
        /// @param target The target buffer to transfer data to.
        /// @param targetElement The first array element to transfer the data to.
        inline void transfer(Span<const void* const> data, size_t elementSize, const IBuffer& target, UInt32 targetElement = 0) const {
            this->cmdTransfer(data, elementSize, target, targetElement);
        }

        /// @brief Performs a buffer-to-image transfer from @p source to @p target.
        ///
        /// The @p subresource parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where resources are counted in the following order:
        ///
        /// - **Level**: Contains the mip-map levels.
        /// - **Layer**: Contains the array slices.
        /// - **Plane**: Contains planes for multi-planar formats.
        ///
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 contain the three mip-map levels
        /// of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure would be repeated for each plane, however one buffer element
        /// only maps to one sub-resource.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source buffer to transfer data from.
        /// @param target The target image to transfer data to.
        /// @param sourceElement The index of the first element in the source buffer to copy.
        /// @param firstSubresource The index of the first sub-resource of the target image to receive data.
        /// @param elements The number of elements to copy from the source buffer into the target image sub-resources.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const IBuffer& source, const IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// @brief Performs a buffer-to-image transfer from @p source to @p target.
        ///
        /// The @p subresource parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where resources are counted in the following order:
        ///
        /// - **Level**: Contains the mip-map levels.
        /// - **Layer**: Contains the array slices.
        /// - **Plane**: Contains planes for multi-planar formats.
        ///
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 contain the three mip-map levels
        /// of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure would be repeated for each plane, however one buffer element
        /// only maps to one sub-resource.
        ///
        /// This method takes shared ownership over @p source, which means that a reference is hold until the parent command queue finished using the command buffer. At this point, the command queue calls @ref
        /// releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been
        /// executed.
        ///
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging buffer and
        /// delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source buffer to transfer data from.
        /// @param target The target image to transfer data to.
        /// @param sourceElement The index of the first element in the source buffer to copy.
        /// @param firstSubresource The index of the first sub-resource of the target image to receive data.
        /// @param elements The number of elements to copy from the source buffer into the target image sub-resources.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const SharedPtr<const IBuffer>& source, const IImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(source, target, sourceElement, firstSubresource, elements);
        }

        /// @brief Performs a buffer-to-buffer transfer from a temporary buffer into @p target.
        ///
        /// This method creates a temporary buffer and maps @p data into it, before transferring it into @p target. A reference of the temporary buffer is stored until the parent command queue finished using the
        /// command buffer. At this point, the command queue calls @ref releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls
        /// this method at some point after the command buffer has been executed.
        ///
        /// @param data The address that marks the beginning of the data to map.
        /// @param size The number of bytes to map.
        /// @param target The target buffer to transfer data to.
        /// @param firstSubresource The index of the first sub-resource of the target image to receive data.
        /// @param elements The number of elements to copy from the source buffer into the target image sub-resources.
        inline void transfer(const void* const data, size_t size, const IImage& target, UInt32 subresource = 0) const {
            this->cmdTransfer(data, size, target, subresource);
        }

        /// @brief Performs a buffer-to-buffer transfer from a temporary buffer into @p target.
        ///
        /// This method creates a temporary buffer and maps @p data into it, before transferring it into @p target. A reference of the temporary buffer is stored until the parent command queue finished using the
        /// command buffer. At this point, the command queue calls @ref releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls
        /// this method at some point after the command buffer has been executed.
        ///
        /// @param data The addresses that mark the beginning of the element data to map.
        /// @param elementSize The number of bytes to map for each element.
        /// @param target The target buffer to transfer data to.
        /// @param firstSubresource The index of the first sub-resource of the target image to receive data.
        /// @param elements The number of elements to copy from the source buffer into the target image sub-resources.
        inline void transfer(Span<const void* const> data, size_t elementSize, const IImage& target, UInt32 firstSubresource = 0, UInt32 elements = 1) const {
            this->cmdTransfer(data, elementSize, target, firstSubresource, elements);
        }

        /// @brief Performs an image-to-image transfer from @p source to @p target.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source image to transfer data from.
        /// @param target The target image to transfer data to.
        /// @param sourceSubresource The index of the first sub-resource to copy from the source image.
        /// @param targetSubresource The image of the first sub-resource in the target image to receive data.
        /// @param subresources The number of sub-resources to copy between the images.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const IImage& source, const IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, sourceSubresource, targetSubresource, subresources);
        }

        /// @brief Performs an image-to-image transfer from @p source to @p target.
        ///
        /// This method takes shared ownership over @p source, which means that a reference is hold until the parent command queue finished using the command buffer. At this point, the command queue calls @ref
        /// releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been
        /// executed.
        ///
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging buffer and
        /// delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source image to transfer data from.
        /// @param target The target image to transfer data to.
        /// @param sourceSubresource The index of the first sub-resource to copy from the source image.
        /// @param targetSubresource The image of the first sub-resource in the target image to receive data.
        /// @param subresources The number of sub-resources to copy between the images.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const SharedPtr<const IImage>& source, const IImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, sourceSubresource, targetSubresource, subresources);
        }

        /// @brief Performs an image-to-buffer transfer from @p source to @p target.
        ///
        /// The @p firstSubresource parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where resources are counted in the following order:
        ///
        /// - **Level**: Contains the mip-map levels.
        /// - **Layer**: Contains the array slices.
        /// - **Plane**: Contains planes for multi-planar formats.
        ///
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 contain the three mip-map levels
        /// of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure would be repeated for each plane, however one buffer element
        /// only maps to one sub-resource.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source image to transfer data from.
        /// @param target The target buffer to transfer data to.
        /// @param firstSubresource The index of the first sub-resource to copy from the source image.
        /// @param targetElement The index of the first target element to receive data.
        /// @param subresources The number of sub-resources to copy.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const IImage& source, const IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// @brief Performs an image-to-buffer transfer from @p source to @p target.
        ///
        /// The @p firstSubresource parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where resources are counted in the following order:
        ///
        /// - **Level**: Contains the mip-map levels.
        /// - **Layer**: Contains the array slices.
        /// - **Plane**: Contains planes for multi-planar formats.
        ///
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 contain the three mip-map levels
        /// of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure would be repeated for each plane, however one buffer element
        /// only maps to one sub-resource.
        ///
        /// This method takes shared ownership over @p source, which means that a reference is hold until the parent command queue finished using the command buffer. At this point, the command queue calls @ref
        /// releaseSharedState to release all shared references. Note that this is a relaxed constraint. It is only guaranteed, that the queue calls this method at some point after the command buffer has been
        /// executed.
        ///
        /// Sharing ownership is helpful in situations where you only have a temporary buffer that you do not want to manually keep track of. For example, it makes sense to create a temporary staging buffer and
        /// delete it, if the remote resource has been initialized. In such a case, the command buffer can take ownership over the resource to release it after it has been executed.
        ///
        /// Note that you have to manually ensure that @p source and @p target are in the proper state for transfer operations. You might have to use a @ref IBarrier before starting the transfer.
        ///
        /// @param source The source image to transfer data from.
        /// @param target The target buffer to transfer data to.
        /// @param firstSubresource The index of the first sub-resource to copy from the source image.
        /// @param targetElement The index of the first target element to receive data.
        /// @param subresources The number of sub-resources to copy.
        /// @throws ArgumentOutOfRangeException Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified @p elements parameter.
        inline void transfer(const SharedPtr<const IImage>& source, const IBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const {
            this->cmdTransfer(source, target, firstSubresource, targetElement, subresources);
        }

        /// @brief Sets the active pipeline state.
        inline void use(const IPipeline& pipeline) const noexcept {
            this->cmdUse(pipeline);
        }

        /// @brief Binds the provided descriptor to the last pipeline that was used by the command buffer.
        ///
        /// @param descriptorSet The descriptor set to bind.
        /// @throws RuntimeException Thrown, if no pipeline has been used before attempting to bind the descriptor set.
        /// @see use
        inline void bind(const IDescriptorSet& descriptorSet) const {
            this->cmdBind(descriptorSet);
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @tparam T The type of the descriptor sets.
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        /// @throws RuntimeException Thrown, if no pipeline has been used before attempting to bind the descriptor set.
        template <typename TSelf, typename T>
        inline void bind(this const TSelf& self, std::initializer_list<const T*> descriptorSets) requires
            std::derived_from<T, IDescriptorSet>
        {
            // NOTE: In the future we might be able to remove this method, if P2447R4 is added to the language.
            Array<const T*> sets = descriptorSets;
            self.bind(Span<const T*>(sets));
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        /// @throws RuntimeException Thrown, if no pipeline has been used before attempting to bind the descriptor set.
        template <typename TSelf>
        inline void bind(this const TSelf& self, std::ranges::input_range auto&& descriptorSets) requires 
            std::derived_from<std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>, IDescriptorSet>
        {
            using descriptor_set_type = std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>;
            auto sets = descriptorSets | std::ranges::to<Array<const descriptor_set_type*>>();
            self.bind(Span<const descriptor_set_type*>(sets));
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        inline void bind(Span<const IDescriptorSet*> descriptorSets) const {
            this->cmdBind(descriptorSets);
        }

        /// @brief Binds the provided descriptor set to the provided pipeline.
        ///
        /// @param descriptorSet The descriptor set to bind.
        /// @param pipeline The pipeline to bind the descriptor set to.
        inline void bind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const {
            this->cmdBind(descriptorSet, pipeline);
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the last pipeline that was used by the command buffer.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @tparam T The type of the descriptor sets.
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        /// @param pipeline The pipeline to bind the descriptor set to.
        /// @throws RuntimeException Thrown, if no pipeline has been used before attempting to bind the descriptor set.
        template <typename TSelf, typename T>
        inline void bind(this const TSelf& self, std::initializer_list<const T*> descriptorSets, const typename TSelf::pipeline_type& pipeline) 
        {
            // NOTE: In the future we might be able to remove this method, if P2447R4 is added to the language.
            Array<const T*> sets = descriptorSets;
            self.bind(Span<const T*>(sets), pipeline);
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the provided pipeline.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        /// @param pipeline The pipeline to bind the descriptor set to.
        template <typename TSelf>
        inline void bind(this const TSelf& self, std::ranges::input_range auto&& descriptorSets, const typename TSelf::pipeline_type& pipeline) requires 
            std::derived_from<std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>, IDescriptorSet>
        {
            using descriptor_set_type = std::remove_cv_t<std::remove_pointer_t<std::iter_value_t<std::ranges::iterator_t<std::remove_cv_t<std::remove_reference_t<decltype(descriptorSets)>>>>>>;
            auto sets = descriptorSets | std::ranges::to<Array<const descriptor_set_type*>>();
            self.bind(Span<const descriptor_set_type*>(sets), pipeline);
        }

        /// @brief Binds an arbitrary input range of descriptor sets to the provided pipeline.
        ///
        /// Note that if an element of @p descriptorSets is `nullptr`, it will be ignored.
        ///
        /// @param descriptorSets The pointers to the descriptor sets to bind.
        /// @param pipeline The pipeline to bind the descriptor set to.
        inline void bind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const {
            this->cmdBind(descriptorSets, pipeline);
        }

        /// @brief Binds a vertex buffer to the pipeline.
        ///
        /// After binding the vertex buffer, the next call to @ref draw or @ref drawIndexed will read from it, until another vertex buffer is bound.
        ///
        /// @param buffer The vertex buffer to bind to the pipeline.
        /// @see VertexBuffer
        /// @see draw
        /// @see drawIndexed
        inline void bind(const IVertexBuffer& buffer) const {
            this->cmdBind(buffer);
        }

        /// @brief Binds a index buffer to the pipeline.
        ///
        /// After binding the index buffer, the next call to @ref drawIndexed will read from it, until another index buffer is bound.
        ///
        /// @param buffer The index buffer to bind to the pipeline.
        /// @see IndexBuffer
        /// @see drawIndexed
        inline void bind(const IIndexBuffer& buffer) const {
            this->cmdBind(buffer);
        }

        /// @brief Executes a compute shader.
        ///
        /// @param threadCount The number of thread groups per dimension.
        /// @see dispatchIndirect
        virtual void dispatch(const Vector3u& threadGroupCount) const noexcept = 0;

        /// @brief Executes a compute shader.
        ///
        /// @param x The number of thread groups along the x dimension.
        /// @param y The number of thread groups along the y dimension.
        /// @param z The number of thread groups along the z dimension.
        inline void dispatch(UInt32 x, UInt32 y, UInt32 z) const noexcept {
            this->dispatch({ x, y, z });
        }

        /// @brief Executes a set of indirect dispatches.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param batchCount The number of batches in the buffer to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @see dispatch
        inline void dispatchIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept {
            this->cmdDispatchIndirect(batchBuffer, batchCount, offset);
        }
        
        /// @brief Executes a mesh shader pipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param threadCount The number of thread groups per dimension.
        virtual void dispatchMesh(const Vector3u& threadGroupCount) const noexcept = 0;

        /// @brief Executes a mesh shader pipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param x The number of thread groups along the x dimension.
        /// @param y The number of thread groups along the y dimension.
        /// @param z The number of thread groups along the z dimension.
        inline void dispatchMesh(UInt32 x, UInt32 y, UInt32 z) const noexcept {
            this->dispatchMesh({ x, y, z });
        }

        /// @brief Executes a set of indirect mesh shader dispatches.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param batchCount The number of batches in the buffer to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @see dispatchMesh
        inline void dispatchMeshIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept {
            this->cmdDispatchMeshIndirect(batchBuffer, batchCount, offset);
        }

        /// @brief Executes a set of indirect mesh shader dispatches.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param countBuffer The buffer that contains the number of batches to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @param countOffset The offset (in bytes) to the number of batches in the @p countBuffer.
        /// @param maxBatches The maximum number of batches executed, even if there are more batches in @p countBuffer.
        /// @see dispatch
        inline void dispatchMeshIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept {
            this->cmdDispatchMeshIndirect(batchBuffer, countBuffer, offset, countOffset, maxBatches);
        }

        /// @brief Executes a query on a ray-tracing pipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param width The width of the ray-tracing query.
        /// @param height The height of the ray-tracing query.
        /// @param depth The depth of the ray-tracing query.
        /// @param offsets The offsets, sizes and strides for each shader binding table.
        /// @param rayGenerationShaderBindingTable The shader binding table that contains the ray generation shader.
        /// @param missShaderBindingTable The shader binding table that contains the miss shaders.
        /// @param hitShaderBindingTable The shader binding table that contains the hit shaders.
        /// @param callableShaderBindingTable The shader binding table that contains the callable shaders.
        inline void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable = nullptr, const IBuffer* hitShaderBindingTable = nullptr, const IBuffer* callableShaderBindingTable = nullptr) const noexcept {
            this->cmdTraceRays(width, height, depth, offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

        /// @brief Executes a query on a ray-tracing pipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param dimensions The dimensions of the ray-tracing query.
        /// @param offsets The offsets, sizes and strides for each shader binding table.
        /// @param rayGenerationShaderBindingTable The shader binding table that contains the ray generation shader.
        /// @param missShaderBindingTable The shader binding table that contains the miss shaders.
        /// @param hitShaderBindingTable The shader binding table that contains the hit shaders.
        /// @param callableShaderBindingTable The shader binding table that contains the callable shaders.
        inline void traceRays(const Vector3u& dimensions, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable = nullptr, const IBuffer* hitShaderBindingTable = nullptr, const IBuffer* callableShaderBindingTable = nullptr) const noexcept {
            this->traceRays(dimensions.x(), dimensions.y(), dimensions.z(), offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

        /// @brief Draws a number of vertices from the currently bound vertex buffer.
        ///
        /// @param vertices The number of vertices to draw.
        /// @param instances The number of instances to draw.
        /// @param firstVertex The index of the first vertex to start drawing from.
        /// @param firstInstance The index of the first instance to draw.
        /// @see drawIndirect
        virtual void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept = 0;

        /// @brief Draws all vertices from the vertex buffer provided in @p vertexBuffer.
        ///
        /// This helper method binds the vertex buffer and issues a draw command for all vertices.
        ///
        /// @param vertexBuffer The vertex buffer to draw from.
        /// @param instances The number of instances to draw.
        /// @param firstVertex The index of the first vertex to start drawing from.
        /// @param firstInstance The index of the first instance to draw.
        inline void draw(const IVertexBuffer& vertexBuffer, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const {
            this->cmdDraw(vertexBuffer, instances, firstVertex, firstInstance);
        }

        /// @brief Executes a set of indirect non-indexed draw calls.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param batchCount The number of batches in the buffer to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @see draw
        inline void drawIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept {
            this->cmdDrawIndirect(batchBuffer, batchCount, offset);
        }

        /// @brief Executes a set of indirect non-indexed draw calls.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param countBuffer The buffer that contains the number of batches to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @param countOffset The offset (in bytes) to the number of batches in the @p countBuffer.
        /// @param maxBatches The maximum number of batches executed, even if there are more batches in @p countBuffer.
        /// @see draw
        inline void drawIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept {
            this->cmdDrawIndirect(batchBuffer, countBuffer, offset, countOffset, maxBatches);
        }

        /// @brief Draws the currently bound vertex buffer with a set of indices from the currently bound index buffer.
        ///
        /// @param indices The number of indices to draw.
        /// @param instances The number of instances to draw.
        /// @param firstIndex The index of the first element of the index buffer to start drawing from.
        /// @param vertexOffset The offset added to each index to find the corresponding vertex.
        /// @param firstInstance The index of the first instance to draw.
        /// @see drawIndexedIndirect
        virtual void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept = 0;

        /// @brief Draws the currently bound vertex buffer using the index buffer provided in @p indexBuffer.
        ///
        /// This helper method binds the index buffer and issues a draw command for all indices.
        ///
        /// @param indexBuffer The index buffer to draw with.
        /// @param instances The number of instances to draw.
        /// @param firstIndex The index of the first element of the index buffer to start drawing from.
        /// @param vertexOffset The offset added to each index to find the corresponding vertex.
        /// @param firstInstance The index of the first instance to draw.
        inline void drawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->cmdDrawIndexed(indexBuffer, instances, firstIndex, vertexOffset, firstInstance);
        }

        /// @brief Draws the vertex buffer provided by @p vertexBuffer using the index buffer, provided by @p indexBuffer.
        ///
        /// This helper method binds the provided vertex and index buffers and issues a draw command for all indices.
        ///
        /// @param vertexBuffer The vertex buffer to draw from.
        /// @param indexBuffer The index buffer to draw with.
        /// @param instances The number of instances to draw.
        /// @param firstIndex The index of the first element of the index buffer to start drawing from.
        /// @param vertexOffset The offset added to each index to find the corresponding vertex.
        /// @param firstInstance The index of the first instance to draw.
        inline void drawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->cmdDrawIndexed(vertexBuffer, indexBuffer, instances, firstIndex, vertexOffset, firstInstance);
        }

        /// @brief Executes a set of indirect indexed draw calls.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param batchCount The number of batches in the buffer to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @see drawIndexed
        inline void drawIndexedIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept {
            this->cmdDrawIndexedIndirect(batchBuffer, batchCount, offset);
        }

        /// @brief Executes a set of indirect indexed draw calls.
        ///
        /// @param batchBuffer The buffer that contains the batches.
        /// @param countBuffer The buffer that contains the number of batches to execute.
        /// @param offset The offset (in bytes) to the first batch in the @p batchBuffer.
        /// @param countOffset The offset (in bytes) to the number of batches in the @p countBuffer.
        /// @param maxBatches The maximum number of batches executed, even if there are more batches in @p countBuffer.
        /// @see drawIndexed
        inline void drawIndexedIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept {
            this->cmdDrawIndexedIndirect(batchBuffer, countBuffer, offset, countOffset, maxBatches);
        }

        /// @brief Pushes a block of memory into the push constants backing memory.
        ///
        /// @param layout The layout of the push constants to update.
        /// @param memory A pointer to the source memory.
        inline void pushConstants(const IPushConstantsLayout& layout, const void* const memory) const {
            this->cmdPushConstants(layout, memory);
        }

        /// @brief Sets the viewports used for the subsequent draw calls.
        ///
        /// @param viewports The viewports used for the subsequent draw calls.
        virtual void setViewports(Span<const IViewport*> viewports) const = 0;

        /// @brief Sets the viewport used for the subsequent draw calls.
        ///
        /// @param viewport The viewport used for the subsequent draw calls.
        virtual void setViewports(const IViewport* viewport) const = 0;

        /// @brief Sets the scissor rectangles used for the subsequent draw calls.
        ///
        /// @param scissors The scissor rectangles used for the subsequent draw calls.
        virtual void setScissors(Span<const IScissor*> scissors) const = 0;

        /// @brief Sets the scissor rectangle used for the subsequent draw calls.
        ///
        /// @param scissors The scissor rectangle used for the subsequent draw calls.
        virtual void setScissors(const IScissor* scissor) const = 0;

        /// @brief Sets the blend factors for the subsequent draw calls.
        ///
        /// Blend factors are set for all render targets that use the blend modes `BlendFactor::ConstantColor`, `BlendFactor::OneMinusConstantColor`, `BlendFactor::ConstantAlpha` or
        /// `BlendFactor::OneMinusConstantAlpha`.
        ///
        /// @param blendFactors The blend factors for the subsequent draw calls.
        virtual void setBlendFactors(const Vector4f& blendFactors) const noexcept = 0;

        /// @brief Sets the stencil reference for the subsequent draw calls.
        ///
        /// @param stencilRef The stencil reference for the subsequent draw calls.
        virtual void setStencilRef(UInt32 stencilRef) const noexcept = 0;

        /// @brief Sets the depth range for the depth bounds test.
        ///
        /// In order to use the depth bounds test, the currently bound render pipeline must have been created with the depth bounds test enabled, which requires the device to be created with the @ref
        /// GraphicsDeviceFeatures::DepthBoundsTest enabled.
        ///
        /// @param minBounds
        /// @param maxBounds
        /// @see GraphicsDeviceFeatures::DepthBoundsTest
        /// @see DepthStencilState::DepthState::DepthBoundsTestEnable
        virtual void setDepthBounds(Float minBounds, Float maxBounds) const noexcept = 0;

        /// @brief Submits the command buffer to parent command
        ///
        /// @throws RuntimeException Thrown, if the command buffer is a secondary command buffer.
        virtual UInt64 submit() const = 0;

        /// @brief Writes the current GPU time stamp value for the timing event.
        ///
        /// @param timingEvent The timing event for which the time stamp is written.
        virtual void writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const = 0;

        /// @brief Executes a secondary command buffer/bundle.
        ///
        /// @param commandBuffer The secondary command buffer/bundle to execute.
        inline void execute(const SharedPtr<const ICommandBuffer>& commandBuffer) const {
            this->cmdExecute(commandBuffer);
        }

        /// @brief Executes a series of secondary command buffers/bundles.
        ///
        /// @param commandBuffers The command buffers to execute.
        inline void execute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const {
            this->cmdExecute(std::move(commandBuffers));
        }

        /// @brief Builds a bottom-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param blas The bottom-level acceleration structure to build.
        /// @param scratchBuffer The scratch buffer to use for building the acceleration structure.
        /// @param buffer The buffer that contains the acceleration structure after the build.
        /// @param offset The offset into @p buffer at which the acceleration structure gets stored after the build.
        /// @throws ArgumentNotInitializedException Thrown, if the provided @p scratchBuffer is not initialized.
        /// @see IAccelerationStructure::build
        inline void buildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdBuildAccelerationStructure(blas, scratchBuffer, buffer, offset);
        }

        /// @brief Builds a top-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param tlas The top-level acceleration structure to build.
        /// @param scratchBuffer The scratch buffer to use for building the acceleration structure.
        /// @param buffer The buffer that contains the acceleration structure after the build.
        /// @param offset The offset into @p buffer at which the acceleration structure gets stored after the build.
        /// @throws ArgumentNotInitializedException Thrown, if the provided @p scratchBuffer is not initialized.
        /// @see IAccelerationStructure::build
        inline void buildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdBuildAccelerationStructure(tlas, scratchBuffer, buffer, offset);
        }

        /// @brief Updates a bottom-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param blas The bottom-level acceleration structure to build.
        /// @param scratchBuffer The scratch buffer to use for building the acceleration structure.
        /// @param buffer The buffer that contains the acceleration structure after the build.
        /// @param offset The offset into @p buffer at which the acceleration structure gets stored after the build.
        /// @throws ArgumentNotInitializedException Thrown, if the provided @p scratchBuffer is not initialized.
        /// @see IAccelerationStructure::build
        inline void updateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdUpdateAccelerationStructure(blas, scratchBuffer, buffer, offset);
        }

        /// @brief Updates a top-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param tlas The top-level acceleration structure to build.
        /// @param scratchBuffer The scratch buffer to use for building the acceleration structure.
        /// @param buffer The buffer that contains the acceleration structure after the build.
        /// @param offset The offset into @p buffer at which the acceleration structure gets stored after the build.
        /// @throws ArgumentNotInitializedException Thrown, if the provided @p scratchBuffer is not initialized.
        /// @see IAccelerationStructure::build
        inline void updateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset = 0) const {
            this->cmdUpdateAccelerationStructure(tlas, scratchBuffer, buffer, offset);
        }

        /// @brief Copies the acceleration structure @p from into the acceleration structure @p to.
        ///
        /// Prefer calling @ref IBottomLevelAccelerationStructure::copy over directly issuing copy commands on a command buffer, as this will make sure that the destination buffer will be properly allocated and
        /// contains enough memory to store the copy. Only issue copies on the command buffer directly, if you want to retain the destination buffer and know for certain, that it contains a sufficient amount of
        /// memory.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param from The source acceleration structure to copy from.
        /// @param to The destination acceleration structure to copy to.
        /// @param compress If set to `true`, the acceleration structure will be compressed.
        inline void copyAccelerationStructure(const IBottomLevelAccelerationStructure& from, const IBottomLevelAccelerationStructure& to, bool compress = false) const noexcept {
            this->cmdCopyAccelerationStructure(from, to, compress);
        }

        /// @brief Copies the acceleration structure @p from into the acceleration structure @p to.
        ///
        /// Prefer calling @ref ITopLevelAccelerationStructure::copy over directly issuing copy commands on a command buffer, as this will make sure that the destination buffer will be properly allocated and
        /// contains enough memory to store the copy. Only issue copies on the command buffer directly, if you want to retain the destination buffer and know for certain, that it contains a sufficient amount of
        /// memory.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param from The source acceleration structure to copy from.
        /// @param to The destination acceleration structure to copy to.
        /// @param compress If set to `true`, the acceleration structure will be compressed.
        inline void copyAccelerationStructure(const ITopLevelAccelerationStructure& from, const ITopLevelAccelerationStructure& to, bool compress = false) const noexcept {
            this->cmdCopyAccelerationStructure(from, to, compress);
        }

    protected:
        /// @brief Called by the parent command queue to signal that the command buffer should release it's shared state.
        virtual void releaseSharedState() const = 0;

    private:
        virtual SharedPtr<const ICommandQueue> getQueue() const noexcept = 0;
        virtual UniquePtr<IBarrier> getBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const = 0;
        virtual void cmdBarrier(const IBarrier& barrier) const noexcept = 0;
        virtual void cmdTransfer(const IBuffer& source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(const IBuffer& source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(const IImage& source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const IImage& source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const SharedPtr<const IBuffer>& source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(const SharedPtr<const IBuffer>& source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdTransfer(const SharedPtr<const IImage>& source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const SharedPtr<const IImage>& source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const = 0;
        virtual void cmdTransfer(const void* const data, size_t size, const IBuffer& target, UInt32 targetElement, UInt32 elements) const = 0;
        virtual void cmdTransfer(Span<const void* const> data, size_t elementSize, const IBuffer& target, UInt32 targetElement) const = 0;
        virtual void cmdTransfer(const void* const data, size_t size, const IImage& target, UInt32 subresource) const = 0;
        virtual void cmdTransfer(Span<const void* const> data, size_t elementSize, const IImage& target, UInt32 firstSubresource, UInt32 elements) const = 0;
        virtual void cmdUse(const IPipeline& pipeline) const noexcept = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet) const = 0;
        virtual void cmdBind(Span<const IDescriptorSet*> descriptorSets) const = 0;
        virtual void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const = 0;
        virtual void cmdBind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const = 0;
        virtual void cmdBind(const IVertexBuffer& buffer) const = 0;
        virtual void cmdBind(const IIndexBuffer& buffer) const = 0;
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const = 0;
        virtual void cmdDispatchIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept = 0;
        virtual void cmdDispatchMeshIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept = 0;
        virtual void cmdDispatchMeshIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept = 0;
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept = 0;
        virtual void cmdDrawIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept = 0;
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const = 0;
        virtual void cmdDrawIndexedIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept = 0;
        virtual void cmdDrawIndexedIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept = 0;
        virtual void cmdExecute(const SharedPtr<const ICommandBuffer>& commandBuffer) const = 0;
        virtual void cmdExecute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffer) const = 0;
        virtual void cmdBuildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdBuildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdUpdateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdUpdateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const = 0;
        virtual void cmdCopyAccelerationStructure(const IBottomLevelAccelerationStructure& from, const IBottomLevelAccelerationStructure& to, bool compress) const noexcept = 0;
        virtual void cmdCopyAccelerationStructure(const ITopLevelAccelerationStructure& from, const ITopLevelAccelerationStructure& to, bool compress) const noexcept = 0;
        virtual void cmdTraceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable, const IBuffer* hitShaderBindingTable, const IBuffer* callableShaderBindingTable) const noexcept = 0;
    };

    /// @brief The interface for a render pipeline.
    class LITEFX_RENDERING_API IRenderPipeline : public virtual IPipeline {
    protected:
        IRenderPipeline() noexcept = default;
        IRenderPipeline(IRenderPipeline&&) noexcept = default;
        IRenderPipeline(const IRenderPipeline&) = delete;
        IRenderPipeline& operator=(IRenderPipeline&&) noexcept = default;
        IRenderPipeline& operator=(const IRenderPipeline&) = delete;

    public:
        ~IRenderPipeline() noexcept override = default;

    public:
        /// @brief Returns the input assembler state used by the render pipeline.
        ///
        /// @return The input assembler state used by the render pipeline.
        inline SharedPtr<IInputAssembler> inputAssembler() const noexcept {
            return this->getInputAssembler();
        }

        /// @brief Returns the rasterizer state used by the render pipeline.
        ///
        /// @return The rasterizer state used by the render pipeline.
        inline SharedPtr<IRasterizer> rasterizer() const noexcept {
            return this->getRasterizer();
        }

        /// @brief Returns `true`, if the pipeline uses *Alpha-to-Coverage* multi-sampling.
        ///
        /// Alpha-to-Coverage is a multi-sampling technique used for partially transparent sprites or textures (such as foliage) to prevent visible flickering along edges. If enabled, the alpha-channel of the
        /// first (non-depth/stencil) render target is used to generate a temporary coverage mask that is combined with the fragment coverage mask using a logical **AND**.
        ///
        /// @return `true`, if the pipeline uses *Alpha-to-Coverage* multi-sampling.
        /// @see https://bgolus.medium.com/anti-aliased-alpha-test-the-esoteric-alpha-to-coverage-8b177335ae4f
        /// @see https://en.wikipedia.org/wiki/Alpha_to_coverage
        /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-covg
        /// @see https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-blend-state#alpha-to-coverage
        virtual bool alphaToCoverage() const noexcept = 0;

        /// @brief Returns the multi-sampling level of the pipeline.
        ///
        /// When using the pipeline, the multi-sampling level must match the level of the render target images.
        ///
        /// @return The multi-sampling level of the pipeline.
        /// @see updateSamples
        virtual MultiSamplingLevel samples() const noexcept = 0;

        /// @brief Changes the multi-sampling level of the pipeline.
        ///
        /// Changing the multi-sampling level of a pipeline causes it to be re-created, which is considered an expensive operation. Don't use this method to change samples frequently, for example when binding
        /// frame buffers with different sample levels. Instead, use multiple pipelines for this purpose.
        virtual void updateSamples(MultiSamplingLevel samples) = 0;

    private:
        virtual SharedPtr<IInputAssembler> getInputAssembler() const noexcept = 0;
        virtual SharedPtr<IRasterizer> getRasterizer() const noexcept = 0;
    };

    /// @brief The interface for a compute pipeline.
    class LITEFX_RENDERING_API IComputePipeline : public virtual IPipeline {
    protected:
        IComputePipeline() noexcept = default;
        IComputePipeline(IComputePipeline&&) noexcept = default;
        IComputePipeline(const IComputePipeline&) = delete;
        IComputePipeline& operator=(IComputePipeline&&) noexcept = default;
        IComputePipeline& operator=(const IComputePipeline&) = delete;

    public:
        ~IComputePipeline() noexcept override = default;
    };

    /// @brief The interface for a ray tracing pipeline.
    class LITEFX_RENDERING_API IRayTracingPipeline : public virtual IPipeline {
    protected:
        IRayTracingPipeline() noexcept = default;
        IRayTracingPipeline(IRayTracingPipeline&&) noexcept = default;
        IRayTracingPipeline(const IRayTracingPipeline&) = delete;
        IRayTracingPipeline& operator=(IRayTracingPipeline&&) noexcept = default;
        IRayTracingPipeline& operator=(const IRayTracingPipeline&) = delete;

    public:
        ~IRayTracingPipeline() noexcept override = default;

    public:
        /// @brief Returns the shader record collection of the ray tracing pipeline.
        ///
        /// @return The shader record collection of the ray tracing pipeline.
        virtual const ShaderRecordCollection& shaderRecords() const noexcept = 0;

        /// @brief Returns the maximum number of ray bounces.
        ///
        /// @return The shader record collection of the ray tracing pipeline.
        virtual UInt32 maxRecursionDepth() const noexcept = 0;

        /// @brief Returns the maximum size of a single ray payload.
        ///
        /// A ray payload is the data that is passed down the `TraceRay` function call chain. It can be zero, if [Ray Payload
        /// Qualifiers](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#payload-access-qualifiers) are used. Otherwise it must be set to the largest ray payload size used in the ray-tracing
        /// pipeline.
        ///
        /// This property can currently not be queried from reflection.
        ///
        /// @return The maximum size of a single ray payload.
        virtual UInt32 maxPayloadSize() const noexcept = 0;

        /// @brief Returns the maximum size of a single ray attribute.
        ///
        /// A ray attribute is the data that is passed to a hit shader for a specific event. Different to ray payloads, it only contains the data that describe the event (such as the hit coordinates, etc.).
        ///
        /// This property can currently not be queried from reflection.
        ///
        /// @return The maximum size of the ray attribute.
        virtual UInt32 maxAttributeSize() const noexcept = 0;

        /// @brief Allocates a buffer that contains the shader binding table containing the shader groups specified by the @p groups parameter.
        ///
        /// The shader binding table consists out of individual shader records, where each record refers to a shader record plus its local data, as specified in the shader record collection that was passed to the
        /// ray-tracing pipeline during creation. The size of a record within the shader binding table is determined by the largest local data size of all records of the groups to be included. It makes sense to
        /// pack multiple records into the same buffer for efficiency, however it may generally be a good idea to separate groups that require large amount of local shader data into their own buffers to keep the
        /// other buffers smaller.
        ///
        /// The shader binding table is created on the default resource heap (@ref ResourceHeap::Dynamic). However, for best performance, consider transferring it to a buffer on the GPU resource heap (@ref
        /// ResourceHeap::Resource) afterwards.
        ///
        /// @param offsets A reference to a structure that receives the offsets and sizes to the groups within the shader binding table.
        /// @param groups The groups to include into the shader binding table.
        /// @return The buffer that stores the shader binding table.
        inline SharedPtr<IBuffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const {
            return this->getShaderBindingTable(offsets, groups);
        }

    private:
        virtual SharedPtr<IBuffer> getShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const = 0;
    };

    /// @brief The interface for a frame buffer.
    ///
    /// A frame buffer is a set of images of equal size, that are used by render targets and/or input attachments in a @ref IRenderPass. When creating a new frame buffer, it is empty by default and needs
    /// images to be added into it. When beginning a render pass during rendering, a frame buffer instance needs to be passed to it. The render pass then tries to obtain an image for each render target from
    /// the frame buffer. It does this by resolving it's render targets (@ref IRenderPass::renderTargets). A render target stores a unique identifier (@ref IRenderTarget::identifier), that is used to obtain
    /// the image. Before this resolution process can be successful, the render targets must first be mapped to the images in the frame buffer by calling @ref IFrameBuffer::mapRenderTarget. Calling this
    /// method multiple times will overwrite the mapping. It is also possible to remove a render target mapping by calling @ref IFrameBuffer::unmapRenderTarget. This will result in future attempts to resolve
    /// this render target using the frame buffer instance to fail.
    ///
    /// The images in the frame buffer can be resized by calling @ref IFrameBuffer::resize. As this involves re-creating the images, it is important to properly synchronize resizing with rendering, i.e.,
    /// resizing while a frame is still being rendered is not allowed. Calling @ref IFrameBuffer::resize invokes two events, @ref IFrameBuffer::resizing before the actual resize occurs and @ref
    /// IFrameBuffer::resized afterwards.
    ///
    /// The main purpose of those events is to provide convenient points for manually controlling image allocation for the frame buffer. Whenever the frame buffer needs to create a new image instance, it
    /// checks, if a @ref IFrameBuffer::allocation_callback_type has been provided during initialization. If provided, this callback is invoked instead of directly creating the image. Only if the callback
    /// returns `nullptr`, the frame buffer will proceed with the default behavior of allocating the image itself. This way, it is possible to selectively deviate from the default image allocation behavior,
    /// for example to allocate a fixed-size image or to provide pre-allocated images, which can be helpful if render targets should be aliased.
    class LITEFX_RENDERING_API IFrameBuffer : public virtual IStateResource, public SharedObject {
    public:
        /// @brief A function that gets invoked as a callback, if the frame buffer needs to allocate an image.
        ///
        /// A frame buffer may allocate image resources, during it's initialization stage, as well as during resize events. The default behavior, if no callback is provided, is to re-create the image resource
        /// with the same parameters as it has been initialized with, except a different resolution. The resolution in this case is always equal to the frame buffer extent.
        ///
        /// Defining render-targets that render at a different resolution is possible by providing an allocation callback that creates the resource externally. Another use case for this callback is to re-use
        /// images from a pool of potentially aliasing image resources. Only in case this callback returns `nullptr` the default behavior gets invoked.
        ///
        /// @par Example
        /// auto callback = [this](Optional<UInt64> renderTargetId, Size2d size, ResourceUsage usage, Format format, MultiSamplingLevel samples, const String& name) { return
        /// m_device->factory().createTexture(name, format, size, ImageDimensions::DIM_2, 1u, 1u, samples, usage); // Emulates the default behavior. };
        ///
        /// @ref resize
        template <typename TImage>
        using allocation_callback_type = std::function<SharedPtr<const TImage>(Optional<UInt64>, Size2d, ResourceUsage, Format, MultiSamplingLevel, const String&)>;

    public:
        /// @brief Event arguments that are published to subscribers when a frame buffer gets resized.
        ///
        /// @see IFrameBuffer::resize
        /// @see IFrameBuffer::resized
        struct ResizeEventArgs : public EventArgs {
        private:
            Size2d m_newSize;

        public:
            ResizeEventArgs(Size2d newSize) noexcept :
                EventArgs(), m_newSize(std::move(newSize)) { }
            ResizeEventArgs(const ResizeEventArgs&) = default;
            ResizeEventArgs(ResizeEventArgs&&) noexcept = default;
            ResizeEventArgs& operator=(const ResizeEventArgs&) = default;
            ResizeEventArgs& operator=(ResizeEventArgs&&) noexcept = default;
            ~ResizeEventArgs() noexcept override = default;

        public:
            /// @brief Returns the new size of the frame buffer.
            ///
            /// @return The new size of the frame buffer.
            inline const Size2d& newSize() const noexcept {
                return m_newSize;
            }
        };

        /// @brief Event arguments that are published to subscribers when a frame buffer gets released.
        ///
        /// @see IFrameBuffer::~IFrameBuffer
        /// @see IFrameBuffer::released
        struct ReleasedEventArgs : public EventArgs {
        public:
            ReleasedEventArgs() noexcept :
                EventArgs() { }
            ReleasedEventArgs(const ReleasedEventArgs&) = default;
            ReleasedEventArgs(ReleasedEventArgs&&) noexcept = default;
            ReleasedEventArgs& operator=(const ReleasedEventArgs&) = default;
            ReleasedEventArgs& operator=(ReleasedEventArgs&&) noexcept = default;
            ~ReleasedEventArgs() noexcept override = default;
        };

    protected:
        IFrameBuffer() noexcept = default;
        IFrameBuffer(IFrameBuffer&&) noexcept = default;
        IFrameBuffer(const IFrameBuffer&) = delete;
        IFrameBuffer& operator=(IFrameBuffer&&) noexcept = default;
        IFrameBuffer& operator=(const IFrameBuffer&) = delete;

    public:
        /// @brief Releases the frame buffer.
        inline ~IFrameBuffer() noexcept override {
            released.invoke(this, { });
        }

    public:
        /// @brief Invoked if the frame buffer gets resized.
        ///
        /// @see resize
        /// @see resized
        mutable Event<ResizeEventArgs> resizing;

        /// @brief Invoked after the frame buffer has been resized.
        ///
        /// @see resize
        /// @see resizing
        mutable Event<ResizeEventArgs> resized;

        /// @brief Invoked when the frame buffer gets released.
        ///
        /// Note that it is no longer valid to access the frame buffer when receiving this event. The only thing that can be assumed to still be valid is the pointer to the frame buffer. The intent of this event
        /// is to release any resources that depend on the frame buffer instance. Internally, render passes and pipelines use this event to release cached frame buffer states they hold, such as descriptor sets
        /// for input attachment bindings or command buffers associated with the frame buffer.
        ///
        /// @see ~IFrameBuffer
        mutable Event<ReleasedEventArgs> released;

    public:
        /// @brief Returns the current size of the frame buffer.
        ///
        /// @return The current size of the frame buffer.
        /// @see height
        /// @see width
        /// @see resize
        virtual const Size2d& size() const noexcept = 0;

        /// @brief Returns the current width of the frame buffer.
        ///
        /// @return The current width of the frame buffer.
        /// @see height
        /// @see size
        /// @see resize
        virtual size_t getWidth() const noexcept = 0;

        /// @brief Returns the current height of the frame buffer.
        ///
        /// @return The current height of the frame buffer.
        /// @see width
        /// @see size
        /// @see resize
        virtual size_t getHeight() const noexcept = 0;

        /// @brief Maps a render target to a frame buffer image.
        ///
        /// When calling @ref IRenderPass::begin, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be successful, a mapping first needs to be
        /// established between the render target and the image. This method establishes this mapping.
        ///
        /// Calling this method multiple times will overwrite the mapped index.
        ///
        /// @param renderTarget The render target to map the image to.
        /// @param index The index of the image to map to the render target.
        /// @throws ArgumentOutOfRangeException Thrown, if @p index does not address an image in the frame buffer.
        /// @see unmapRenderTarget
        virtual void mapRenderTarget(const RenderTarget& renderTarget, UInt32 index) = 0;

        /// @brief Maps a render target to a frame buffer image.
        ///
        /// When calling @ref IRenderPass::begin, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be successful, a mapping first needs to be
        /// established between the render target and the image. This method establishes this mapping.
        ///
        /// Calling this method multiple times will overwrite the mapped index.
        ///
        /// @param renderTarget The render target to map the image to.
        /// @param imageName The name of the image the render target maps to.
        /// @throws InvalidArgumentException Thrown, if the frame buffer does not contain an image with the name specified in @p imageName.
        /// @see unmapRenderTarget
        virtual void mapRenderTarget(const RenderTarget& renderTarget, StringView imageName) = 0;

        /// @brief Maps a render target to a frame buffer image using the render targets name to look up the image.
        ///
        /// When calling @ref IRenderPass::begin, passing a frame buffer, the render pass attempts to resolve all render target images. In order for this resolution to be successful, a mapping first needs to be
        /// established between the render target and the image. This method establishes this mapping.
        ///
        /// Calling this method multiple times will overwrite the mapped index.
        ///
        /// @param renderTarget The render target to map the image to.
        /// @throws InvalidArgumentException Thrown, if the frame buffer does not contain an image with the same name as the render target.
        /// @see unmapRenderTarget
        inline void mapRenderTarget(const RenderTarget& renderTarget) {
            this->mapRenderTarget(renderTarget, renderTarget.name());
        }

        /// @brief Maps a set of render targets to the frame buffer images, using the names of the render targets to look up the images.
        ///
        /// @param renderTargets The render targets to map to the frame buffer.
        /// @throws InvalidArgumentException Thrown, if the frame buffer cannot map the name of one or more render targets to images.
        /// @see mapRenderTarget
        /// @see unmapRenderTarget
        inline void mapRenderTargets(Span<const RenderTarget> renderTargets) {
            std::ranges::for_each(renderTargets, [this](auto& renderTarget) { this->mapRenderTarget(renderTarget); });
        }

        /// @brief Removes a mapping between a render target and an image in the frame buffer.
        ///
        /// If no image in the frame buffer is currently mapped to @p renderTarget, calling this method will have no effect.
        ///
        /// @param renderTarget The render target to remove the mapping for.
        /// @see mapRenderTarget
        virtual void unmapRenderTarget(const RenderTarget& renderTarget) noexcept = 0;

        /// @brief Returns all images contained by the frame buffer.
        ///
        /// @return A set of pointers to the images contained by the frame buffer.
        inline Enumerable<const IImage&> images() const {
            return this->getImages();
        }

        /// @brief Returns an image from the frame buffer.
        ///
        /// @param index The index of the image.
        /// @return The image from the frame buffer with the index @p index.
        /// @throws ArgumentOutOfRangeException Thrown, if the @p index does not address an image in the frame buffer.
        virtual const IImage& operator[](UInt32 index) const = 0;

        /// @brief Returns an image from the frame buffer.
        ///
        /// @param index The index of the image.
        /// @return The image from the frame buffer with the index @p index.
        /// @throws ArgumentOutOfRangeException Thrown, if the @p index does not address an image in the frame buffer.
        virtual const IImage& image(UInt32 index) const = 0;

        /// @brief Resolves a render target and returns the image mapped to it.
        ///
        /// @param renderTarget The render target to resolve.
        /// @return The image mapped to the render target.
        /// @throws InvalidArgumentException Thrown, if @p renderTarget is not mapped to an image in the frame buffer.
        virtual const IImage& operator[](const RenderTarget& renderTarget) const = 0;

        /// @brief Resolves a render target and returns the image mapped to it.
        ///
        /// @param renderTarget The render target to resolve.
        /// @return The image mapped to the render target.
        /// @throws InvalidArgumentException Thrown, if @p renderTarget is not mapped to an image in the frame buffer.
        virtual const IImage& image(const RenderTarget& renderTarget) const = 0;

        /// @brief Resolves a render target name and returns the image mapped to it.
        ///
        /// @param renderTargetName The render target name to resolve.
        /// @return The image mapped to the render target.
        /// @throws InvalidArgumentException Thrown, if @p renderTargetName is not mapped to an image in the frame buffer.
        virtual const IImage& operator[](StringView renderTargetName) const = 0;

        /// @brief Resolves a render target name and returns the image mapped to it.
        ///
        /// @param renderTargetName The render target name to resolve.
        /// @return The image mapped to the render target.
        /// @throws InvalidArgumentException Thrown, if @p renderTargetName is not mapped to an image in the frame buffer.
        virtual const IImage& image(StringView renderTargetName) const = 0;

        /// @brief Resolves a render target name hash and returns the image mapped to it.
        ///
        /// @param hash The render target name hash to resolve.
        /// @return The image mapped to the render target.
        /// @throws InvalidArgumentException Thrown, if @p hash is not mapped to an image in the frame buffer.
        /// @see image
        virtual const IImage& resolveImage(UInt64 hash) const = 0;

        /// @brief Adds an image to the frame buffer.
        ///
        /// @param format The format of the image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(format, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image to the frame buffer.
        ///
        /// @param format The format of the image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        inline void addImage(Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) {
            this->addImage("", format, samples, usage);
        }

        /// @brief Adds an image to the frame buffer.
        ///
        /// @param name The name of the image.
        /// @param format The format of the image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, StringView name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(name, format, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image to the frame buffer.
        ///
        /// @param name The name of the image.
        /// @param format The format of the image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        /// @throws InvalidArgumentException Thrown, if another image with the same name as provided in @p name has already been added to the frame buffer.
        virtual void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) = 0;

        /// @brief Adds an image for a render target to the frame buffer.
        ///
        /// @param renderTarget The render target for which to add an image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(renderTarget, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image for a render target to the frame buffer.
        ///
        /// @param renderTarget The render target for which to add an image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        inline void addImage(const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) {
            this->addImage(renderTarget.name(), renderTarget, samples, usage);
        }

        /// @brief Adds an image for a render target to the frame buffer.
        ///
        /// @param name The name of the image.
        /// @param renderTarget The render target for which to add an image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        template <typename TSelf>
        inline auto addImage(this TSelf&& self, StringView name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            self.addImage(name, renderTarget, samples, usage);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image for a render target to the frame buffer.
        ///
        /// @param name The name of the image.
        /// @param renderTarget The render target for which to add an image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        virtual void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) = 0;

        /// @brief Adds multiple images for a set of render targets to the frame buffer.
        ///
        /// Note that the names of the images are built from the render target names.
        ///
        /// @param renderTargets The render targets for which to add an image.
        /// @param samples The number of samples of the image.
        /// @param usage The desired resource usage flags for the image.
        template <typename TSelf>
        inline auto addImages(this TSelf&& self, Span<const RenderTarget> renderTargets, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) -> TSelf&& {
            std::ranges::for_each(renderTargets, [&](auto& renderTarget) { self.addImage(renderTarget.name(), renderTarget, samples, usage); });
            return std::forward<TSelf>(self);
        }

        /// @brief Causes the frame buffer to be invalidated and recreated with a new size.
        ///
        /// @param renderArea The new dimensions of the frame buffer.
        /// @see resizing
        /// @see resized
        virtual void resize(const Size2d& renderArea) = 0;

    private:
        virtual Enumerable<const IImage&> getImages() const = 0;
    };

    /// @brief The interface for a render pass.
    class LITEFX_RENDERING_API IRenderPass : public virtual IStateResource, public SharedObject {
    public:
        /// @brief Event arguments that are published to subscribers when a render pass is beginning.
        ///
        /// @see IRenderPass::beginning
        struct BeginEventArgs : public EventArgs {
        private:
            const IFrameBuffer* m_frameBuffer;

        public:
            BeginEventArgs(const IFrameBuffer& frameBuffer) noexcept :
                EventArgs(), m_frameBuffer(&frameBuffer) { }
            BeginEventArgs(const BeginEventArgs&) = default;
            BeginEventArgs(BeginEventArgs&&) noexcept = default;
            BeginEventArgs& operator=(const BeginEventArgs&) = default;
            BeginEventArgs& operator=(BeginEventArgs&&) noexcept = default;
            ~BeginEventArgs() noexcept override = default;

        public:
            /// @brief Gets the frame buffer on which the render pass is executing.
            ///
            /// @return The buffer on which the render pass is executing.
            inline const IFrameBuffer& frameBuffer() const noexcept {
                return *m_frameBuffer;
            }
        };

    protected:
        IRenderPass() noexcept = default;
        IRenderPass(IRenderPass&&) noexcept = default;
        IRenderPass(const IRenderPass&) = delete;
        IRenderPass& operator=(IRenderPass&&) noexcept = default;
        IRenderPass& operator=(const IRenderPass&) = delete;

    public:
        ~IRenderPass() noexcept override = default;

    public:
        /// @brief Invoked, when the render pass is beginning.
        ///
        /// @see begin
        mutable Event<BeginEventArgs> beginning;

        /// @brief Invoked, when the render pass is ending.
        ///
        /// @see end
        mutable Event<EventArgs> ending;

    public:
        ///// @brief Returns a reference to the parent device.
        ///// 
        ///// @return A reference to the parent device.
        //virtual const IGraphicsDevice& device() const noexcept;

        /// @brief Returns the current frame buffer from of the render pass.
        ///
        /// The frame buffer can only be obtained, if the render pass has been started by calling @ref begin. If the render pass has ended or not yet started, the method will instead raise an exception.
        ///
        /// @param buffer The index of the frame buffer.
        /// @return A pointer to the currently active frame buffer or `nullptr`, if the render pass has not been started.
        /// @see begin
        inline SharedPtr<const IFrameBuffer> activeFrameBuffer() const noexcept {
            return this->getActiveFrameBuffer();
        }

        /// @brief Returns the command queue, the render pass is executing on or `nullptr`, if the queue has already been released.
        ///
        /// @return A pointer to the command queue, the render pass is executing on.
        inline const ICommandQueue& commandQueue() const noexcept {
            return this->getCommandQueue();
        }

        /// @brief Returns all command buffers, that can be currently used for recording multi-threaded commands in the render pass.
        ///
        /// @return All command buffers, that can be currently used for recording multi-threaded commands in the render pass, or an empty set, if the render pass has not been initialized with additional command
        /// buffers, or the render pass is currently not active.
        /// @see commandBuffer
        inline Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers() const {
            return this->getCommandBuffers();
        }

        /// @brief Returns a command buffer that can be currently used for recording multi-threaded commands in the render pass.
        ///
        /// @param index The index of the command buffer.
        /// @return A command buffer that can be currently used for recording multi-threaded commands in the render pass.
        /// @throws RuntimeException Thrown, if the render pass has not been begun.
        /// @throws ArgumentOutOfRangeException Thrown, if the frame buffer does not store a command buffer at @p index.
        /// @see commandBuffers
        inline SharedPtr<const ICommandBuffer> commandBuffer(UInt32 index) const {
            return this->getCommandBuffer(index);
        }

        /// @brief Returns the number of secondary command buffers the render pass stores for multi-threaded command recording.
        ///
        /// @return The number of secondary command buffers the render pass stores for multi-threaded command recording.
        virtual UInt32 secondaryCommandBuffers() const noexcept = 0;

        /// @brief Returns the list of render targets, the render pass renders into.
        ///
        /// Note that the actual render target image resources are stored within the individual @ref FrameBuffers of the render pass.
        ///
        /// @return A list of render targets, the render pass renders into.
        /// @see IFrameBuffer
        virtual const Array<RenderTarget>& renderTargets() const noexcept = 0;

        /// @brief Returns the render target mapped to the location provided by @p location.
        ///
        /// @param location The location to return the render target for.
        /// @return The render target mapped to the location provided by @p location.
        virtual const RenderTarget& renderTarget(UInt32 location) const = 0;

        /// @brief Returns `true`, if one of the render targets is used for presentation on a swap chain.
        ///
        /// @return `true`, if one of the render targets is used for presentation on a swap chain.
        /// @see renderTargets
        virtual bool hasPresentTarget() const noexcept = 0;

        /// @brief Returns the input attachment the render pass is consuming.
        ///
        /// @return An array of input attachment mappings, that are mapped to the render pass.
        virtual const Array<RenderPassDependency>& inputAttachments() const noexcept = 0;

        /// @brief Returns the input attachment at a @p location.
        ///
        /// @return The input attachment at a @p location.
        /// @throws ArgumentOutOfRangeException Thrown, if no input attachment is defined at the specified @p location.
        virtual const RenderPassDependency& inputAttachment(UInt32 location) const = 0;

        /// @brief Returns the binding point for input attachment samplers.
        ///
        /// Note that in Vulkan this is ignored, as render pass inputs are mapped to sub-pass inputs directly, which do not need to be sampled.
        ///
        /// @return The binding point for input attachment samplers.
        virtual const Optional<DescriptorBindingPoint>& inputAttachmentSamplerBinding() const noexcept = 0;

        /// @brief Begins the render pass.
        ///
        /// @param frameBuffer The frame buffer to obtain input attachments and render targets from.
        inline void begin(const IFrameBuffer& frameBuffer) const {
            this->beginRenderPass(frameBuffer);
        };

        /// @brief Ends the render pass.
        ///
        /// If the frame buffer has a present render target, this causes the render pass to synchronize with the swap chain and issue a present command.
        ///
        /// @return The value of the fence that indicates the end of the render pass.
        virtual UInt64 end() const = 0;

        /// @brief Returns the mask that identifies the views that are enabled during rendering.
        ///
        /// @return A mask that identifies the views that are enabled during rendering.
        /// @see GraphicsDeviceFeatures::ViewInstancing
        virtual UInt32 viewMask() const noexcept = 0;

    private:
        virtual SharedPtr<const IFrameBuffer> getActiveFrameBuffer() const noexcept = 0;
        virtual void beginRenderPass(const IFrameBuffer& frameBuffer) const = 0;
        virtual const ICommandQueue& getCommandQueue() const noexcept = 0;
        virtual SharedPtr<const ICommandBuffer> getCommandBuffer(UInt32 index) const noexcept = 0;
        virtual Enumerable<SharedPtr<const ICommandBuffer>> getCommandBuffers() const = 0;
    };

    /// @brief Interface for a swap chain.
    class LITEFX_RENDERING_API ISwapChain {
    public:
        /// @brief Event arguments for a @ref ISwapChain::reseted event.
        struct ResetEventArgs : public EventArgs {
        private:
            Format m_surfaceFormat;
            Size2d m_renderArea;
            UInt32 m_buffers;
            bool m_vsync;

        public:
            ResetEventArgs(Format surfaceFormat, Size2d renderArea, UInt32 buffers, bool enableVsync) noexcept :
                EventArgs(), m_surfaceFormat(surfaceFormat), m_renderArea(std::move(renderArea)), m_buffers(buffers), m_vsync(enableVsync) { }
            ResetEventArgs(const ResetEventArgs&) = default;
            ResetEventArgs(ResetEventArgs&&) noexcept = default;
            ResetEventArgs& operator=(const ResetEventArgs&) = default;
            ResetEventArgs& operator=(ResetEventArgs&&) noexcept = default;
            ~ResetEventArgs() noexcept override = default;

        public:
            /// @brief Gets the new surface format of the swap chain back-buffers.
            ///
            /// @return The new surface format of the swap chain back-buffers.
            inline Format surfaceFormat() const noexcept {
                return m_surfaceFormat;
            }

            /// @brief Gets the new render area of the swap chain back-buffers.
            ///
            /// @return The size of the new render area of the swap chain back-buffers.
            inline const Size2d& renderArea() const noexcept {
                return m_renderArea;
            }

            /// @brief Gets the number of back-buffers in the swap chain.
            ///
            /// @return The number of back-buffers in the swap chain.
            inline UInt32 buffers() const noexcept {
                return m_buffers;
            }

            /// @brief Returns `true` if vertical synchronization is enabled or `false` otherwise.
            ///
            /// @return `true` if vertical synchronization is enabled or `false` otherwise.
            inline bool enableVsync() const noexcept {
                return m_vsync;
            }
        };

        /// @brief Event arguments for a @ref ISwapChain::swapped event.
        struct BackBufferSwapEventArgs : public EventArgs {
        private:
            UInt32 m_backBuffer;

        public:
            explicit BackBufferSwapEventArgs(UInt32 backBuffer) noexcept :
                EventArgs(), m_backBuffer(backBuffer) {
            }
            BackBufferSwapEventArgs(const BackBufferSwapEventArgs&) = default;
            BackBufferSwapEventArgs(BackBufferSwapEventArgs&&) noexcept = default;
            BackBufferSwapEventArgs& operator=(const BackBufferSwapEventArgs&) = default;
            BackBufferSwapEventArgs& operator=(BackBufferSwapEventArgs&&) noexcept = default;
            ~BackBufferSwapEventArgs() noexcept override = default;

        public:
            /// @brief Returns the index of the new back buffer on the swap chain.
            ///
            /// @return The index of the new back buffer on the swap chain.
            UInt32 backBuffer() const noexcept {
                return m_backBuffer;
            }
        };

    protected:
        ISwapChain() noexcept = default;
        ISwapChain(ISwapChain&&) noexcept = default;
        ISwapChain(const ISwapChain&) = default;
        ISwapChain& operator=(const ISwapChain&) = default;
        ISwapChain& operator=(ISwapChain&&) noexcept = default;

    public:
        virtual ~ISwapChain() noexcept = default;

    public:
        /// @brief Creates a new instance of a @ref TimingEvent.
        ///
        /// Note that registering a new timing event does invalidate previously registered events, i.e. they do not return meaningful time stamps for the next frame. Timing events should only be registered during
        /// application startup, before the first frame is rendered.
        ///
        /// @param name The name of the timing event.
        /// @return A pointer with shared ownership to the newly created timing event instance.
        [[nodiscard]] inline SharedPtr<const TimingEvent> registerTimingEvent(StringView name = "") {
            auto timingEvent = TimingEvent::create(*this, name);
            this->addTimingEvent(timingEvent);
            return timingEvent;
        }

        /// @brief Returns all registered timing events.
        ///
        /// @return An array, containing all registered timing events.
        virtual const Array<SharedPtr<const TimingEvent>>& timingEvents() const = 0;

        /// @brief Returns the timing event registered for @p queryId.
        ///
        /// @param queryId The query ID of the timing event.
        /// @return The timing event registered for @p queryId.
        virtual SharedPtr<const TimingEvent> timingEvent(UInt32 queryId) const = 0;

        /// @brief Reads the current time stamp value (in ticks) of a timing event.
        ///
        /// In order to convert the number of ticks to (milli-)seconds, this value needs to be divided by @ref IGraphicsDevice::ticksPerMillisecond. To improve precision, calculate the difference between two time
        /// stamps in ticks first and only then convert them to seconds.
        ///
        /// @param timingEvent The timing event to read the current value for.
        /// @return The current time stamp value of the timing event in ticks.
        /// @see TimingEvent::readTimestamp
        virtual UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const = 0;

        /// @brief Returns the query ID for the timing event.
        ///
        /// @param timingEvent The timing event to return the query ID for.
        /// @return The query ID for the @p timingEvent.
        /// @see TimingEvent::queryId
        virtual UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const = 0;

        /// @brief Returns the swap chain's parent device instance.
        ///
        /// @return A reference of the swap chain's parent device instance.
        /// @throws RuntimeException Thrown, if the device is already released.
        virtual const IGraphicsDevice& device() const  = 0;

    public:
        /// @brief Returns the swap chain image format.
        ///
        /// @return The swap chain image format.
        virtual Format surfaceFormat() const noexcept = 0;

        /// @brief Returns the number of images in the swap chain.
        ///
        /// @return The number of images in the swap chain.
        virtual UInt32 buffers() const noexcept = 0;

        /// @brief Returns the size of the render area.
        ///
        /// @return The size of the render area.
        virtual const Size2d& renderArea() const noexcept = 0;

        /// @brief Returns `true`, if vertical synchronization should be used, otherwise `false`.
        ///
        /// @return `true`, if vertical synchronization should be used, otherwise `false`.
        virtual bool verticalSynchronization() const noexcept = 0;

        /// @brief Returns the swap chain present image for @p backBuffer.
        ///
        /// @param backBuffer The index of the back buffer for which to return the swap chain present image.
        /// @return A pointer to the back buffers swap chain present image.
        virtual IImage* image(UInt32 backBuffer) const = 0;

        /// @brief Returns the current swap chain back buffer image.
        ///
        /// @return A reference of the current swap chain back buffer image.
        virtual const IImage& image() const noexcept = 0;

        /// @brief Returns an array of the swap chain present images.
        ///
        /// @return Returns an array of the swap chain present images.
        inline Enumerable<IImage&> images() const {
            return this->getImages();
        };

        /// @brief Queues a present that gets executed after @p fence has been signaled on the default graphics queue.
        ///
        /// You can use this overload in situations where you do not have an @ref IRenderPass or @ref IFrameBuffer to render into before presenting. Instead, you typically copy into the swap chain back buffer
        /// images directly (@ref image). This copy is done in a command buffer that must be submitted to the default graphics queue. The swap chain can then wait for the copy to finish before presenting it.
        /// Example scenarios where this is useful are, where you want to write to the back buffer from a compute shader, that does not have an equivalent to render passes.
        ///
        /// @param fence The fence to pass on the default graphics queue after which the present is executed.
        virtual void present(UInt64 fence) const = 0;

    public:
        /// @brief Invoked, when the swap chain has swapped the back buffers.
        ///
        /// @see swapBackBuffer
        mutable Event<BackBufferSwapEventArgs> swapped;

        /// @brief Invoked, after the swap chain has been reseted.
        ///
        /// @see reset
       mutable Event<ResetEventArgs> reseted;

        /// @brief Returns an array of supported formats, that can be drawn to the surface.
        ///
        /// @ref surface
        ///
        /// @return An array of supported formats, that can be drawn to the surface.
        /// @see ISurface
        virtual Enumerable<Format> getSurfaceFormats() const = 0;

        /// @brief Causes the swap chain to be re-created. All frame and command buffers will be invalidated and rebuilt.
        ///
        /// There is no guarantee, that the swap chain images will end up in the exact format, as specified by @p surfaceFormat. If the format itself is not supported, a compatible format may be looked up. If the
        /// lookup fails, the method may raise an exception.
        ///
        /// Similarly, it is not guaranteed, that the number of images returned by @ref images matches the number specified in @p buffers. A swap chain may require a minimum number of images or may constraint a
        /// maximum number of images. In both cases, @p buffers will be clamped.
        ///
        /// @param surfaceFormat The swap chain image format.
        /// @param renderArea The dimensions of the frame buffers.
        /// @param buffers The number of buffers in the swap chain.
        /// @param enableVsync `true`, if vertical synchronization should be used, otherwise `false`.
        /// @see multiSamplingLevel
        virtual void reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync = false) = 0;

        /// @brief Swaps the front buffer with the next back buffer in order.
        ///
        /// @return A reference of the front buffer after the buffer swap.
        [[nodiscard]] virtual UInt32 swapBackBuffer() const = 0;

    private:
        virtual Enumerable<IImage&> getImages() const = 0;
        virtual void addTimingEvent(SharedPtr<const TimingEvent> timingEvent) = 0;
    };

    /// @brief The interface for a command queue.
    class LITEFX_RENDERING_API ICommandQueue : public SharedObject {
    public:
        /// @brief Event arguments for a @ref ICommandQueue::submitting event.
        struct QueueSubmittingEventArgs : public EventArgs {
        private:
            Array<SharedPtr<const ICommandBuffer>> m_commandBuffers;

        public:
            QueueSubmittingEventArgs(Array<SharedPtr<const ICommandBuffer>>&& commandBuffers) :
                EventArgs(), m_commandBuffers(std::move(commandBuffers)) { }

            QueueSubmittingEventArgs(const QueueSubmittingEventArgs&) = default;
            QueueSubmittingEventArgs(QueueSubmittingEventArgs&&) noexcept = default;
            QueueSubmittingEventArgs& operator=(const QueueSubmittingEventArgs&) = default;
            QueueSubmittingEventArgs& operator=(QueueSubmittingEventArgs&&) noexcept = default;
            ~QueueSubmittingEventArgs() noexcept override = default;

        public:
            /// @brief Gets the command buffers that are about to be submitted to the queue.
            ///
            /// @return An array containing the command buffers that are about to be submitted to the queue.
            inline const Array<SharedPtr<const ICommandBuffer>>& commandBuffers() const noexcept {
                return m_commandBuffers;
            }
        };

        /// @brief Event arguments for a @ref ICommandQueue::submitted event.
        struct QueueSubmittedEventArgs : public EventArgs {
        private:
            UInt64 m_fence;

        public:
            QueueSubmittedEventArgs(UInt64 fence) noexcept :
                EventArgs(), m_fence(fence) { }
            QueueSubmittedEventArgs(const QueueSubmittedEventArgs&) = default;
            QueueSubmittedEventArgs(QueueSubmittedEventArgs&&) noexcept = default;
            QueueSubmittedEventArgs& operator=(const QueueSubmittedEventArgs&) = default;
            QueueSubmittedEventArgs& operator=(QueueSubmittedEventArgs&&) noexcept = default;
            ~QueueSubmittedEventArgs() noexcept override = default;

        public:
            /// @brief Gets the fence that is triggered, if the command buffers have been executed.
            ///
            /// @return The fence that is triggered, if the command buffers have been executed.
            inline UInt64 fence() const noexcept {
                return m_fence;
            }
        };

    protected:
        ICommandQueue() noexcept = default;
        ICommandQueue(const ICommandQueue&) = default;
        ICommandQueue(ICommandQueue&&) noexcept = default;
        ICommandQueue& operator=(const ICommandQueue&) = default;
        ICommandQueue& operator=(ICommandQueue&&) noexcept = default;

    public:
        ~ICommandQueue() noexcept override = default;

    public:
        /// @brief Returns the priority of the queue.
        ///
        /// @return The priority of the queue.
        virtual QueuePriority priority() const noexcept = 0;

        /// @brief Returns the type of the queue.
        ///
        /// @return The type of the queue.
        virtual QueueType type() const noexcept = 0;

    public:
        /// @brief The default color value for a debug region or marker, if no other has been specified.
        ///
        /// @see beginDebugRegion
        /// @see setDebugMarker
        static constexpr Vectors::ByteVector3 DEFAULT_DEBUG_COLOR = { 128_ui8, 128_ui8, 128_ui8 };

        /// @brief Starts a new debug region.
        ///
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        ///
        /// @param label The name of the debug region.
        /// @param color The color of the debug region.
        virtual void beginDebugRegion([[maybe_unused]] const String& label, [[maybe_unused]] const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept { };
        
        /// @brief Ends the current debug region.
        ///
        /// This is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        virtual void endDebugRegion() const noexcept { };

        /// @brief Inserts a debug marker.
        ///
        /// This method is a debug helper, that is not required to be implemented. In the built-in backends, it will no-op by default in non-debug builds.
        ///
        /// @param label The name of the debug marker.
        /// @param color The color of the debug marker.
        virtual void setDebugMarker([[maybe_unused]] const String& label, [[maybe_unused]] const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept { };

    public:
        /// @brief Invoked, when one or more command buffers are submitted to the queue.
        mutable Event<QueueSubmittingEventArgs> submitting;

        /// @brief Invoked, after one or more command buffers have been submitted to the queue.
        mutable Event<QueueSubmittedEventArgs> submitted;

        /// @brief Creates a command buffer that can be used to allocate commands on the queue.
        ///
        /// Specifying @p secondary allows to create secondary command buffers (aka. bundles). Those are intended to be used as efficient pre-recorded command buffers that are re-used multiple times. Using such a
        /// command buffer allows drivers to pre-apply optimizations, which causes a one-time cost during setup, but reduces cost when re- applying the command buffer multiple times. Ideally they are used as
        /// small chunks of re-occurring workloads.
        ///
        /// A secondary command buffer must not be submitted to a queue, but rather to a primary command buffer by calling @ref ICommandBuffer::execute.
        ///
        /// @param beginRecording If set to `true`, the command buffer will be initialized in recording state and can receive commands straight away.
        /// @param secondary If set to `true`, the method will create a secondary command buffer/bundle.
        /// @return The instance of the command buffer.
        inline SharedPtr<ICommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const {
            return this->getCommandBuffer(beginRecording, secondary);
        }

        /// @brief Submits a single command buffer with shared ownership and inserts a fence to wait for it.
        ///
        /// By calling this method, the queue takes shared ownership over the @p commandBuffer until the fence is passed. The reference will be released during a @ref waitFor, if the awaited fence is inserted
        /// after the associated one.
        ///
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        ///
        /// @param commandBuffer The command buffer to submit to the command queue.
        /// @return The value of the fence, inserted after the command buffer.
        /// @see waitFor
        inline UInt64 submit(const SharedPtr<const ICommandBuffer>& commandBuffer) const {
            return this->submitCommandBuffer(commandBuffer);
        }

        /// @brief Submits a single command buffer with shared ownership and inserts a fence to wait for it.
        ///
        /// By calling this method, the queue takes shared ownership over the @p commandBuffer until the fence is passed. The reference will be released during a @ref waitFor, if the awaited fence is inserted
        /// after the associated one.
        ///
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        ///
        /// @param commandBuffer The command buffer to submit to the command queue.
        /// @return The value of the fence, inserted after the command buffer.
        /// @see waitFor
        inline UInt64 submit(const SharedPtr<ICommandBuffer>& commandBuffer) const {
            return this->submitCommandBuffer(commandBuffer);
        }

        /// @brief Submits a set of command buffers with shared ownership and inserts a fence to wait for them.
        ///
        /// By calling this method, the queue takes shared ownership over the @p commandBuffers until the fence is passed. The reference will be released during a @ref waitFor, if the awaited fence is inserted
        /// after the associated one.
        ///
        /// Note that submitting a command buffer that is currently recording will implicitly close the command buffer.
        ///
        /// @param commandBuffers The command buffers to submit to the command queue.
        /// @return The value of the fence, inserted after the command buffers.
        /// @see waitFor
        inline UInt64 submit(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const {
            return this->submitCommandBuffers(std::move(commandBuffers));
        }

        /// @brief Lets the CPU wait for a certain fence value to complete on the command queue.
        ///
        /// This overload performs a CPU-side wait, i.e., the CPU blocks until the current queue has passed the fence value provided by the @p fence parameter.
        ///
        /// @param fence The value of the fence to wait for.
        /// @see submit
        virtual void waitFor(UInt64 fence) const = 0;

        /// @brief Lets the command queue wait for a certain fence value to complete on another queue.
        ///
        /// This overload performs a GPU-side wait, i.e., the current command queue waits until @p queue has passed the fence value provided by the @p fence parameter. This overload does return immediately and
        /// does not block the CPU.
        ///
        /// @param queue The queue to wait upon.
        /// @param fence The value of the fence to wait upon on the other queue.
        inline void waitFor(const ICommandQueue& queue, UInt64 fence) const {
            this->waitForQueue(queue, fence);
        }

        /// @brief Returns the value of the latest fence inserted into the queue.
        ///
        /// @return The value of the latest fence inserted into the queue.
        /// @see waitFor
        /// @see lastCompletedFence
        virtual UInt64 currentFence() const noexcept = 0;

        /// @brief Returns the last fence that was completed on the queue.
        ///
        /// @return The last fence that was completed on the queue.
        /// @see currentFence
        virtual UInt64 lastCompletedFence() const noexcept = 0;

    private:
        virtual SharedPtr<ICommandBuffer> getCommandBuffer(bool beginRecording, bool secondary) const = 0;
        virtual UInt64 submitCommandBuffer(const SharedPtr<const ICommandBuffer>& commandBuffer) const = 0;
        virtual UInt64 submitCommandBuffers(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const = 0;
        virtual void waitForQueue(const ICommandQueue& queue, UInt64 fence) const = 0;
        
    protected:
        inline void releaseSharedState(const ICommandBuffer& commandBuffer) const {
            commandBuffer.releaseSharedState();
        }
    };

    /// @brief Stores simple memory heap statistics, that can be quickly queried by calling @ref IGraphicsFactory::memoryStatistics
    struct LITEFX_RENDERING_API MemoryHeapStatistics {
        /// @brief `true`, if the heap is located in video memory and `false` otherwise.
        bool onGpu{ false };

        /// @brief `true`, of the heap is accessible for the CPU and `false` otherwise.
        bool cpuVisible{ false };

        /// @brief Returns the number of memory blocks in the heap.
        UInt32 blocks{};

        /// @brief Returns the total number of allocations in the heap.
        UInt32 allocations{};

        /// @brief Returns the total size of allocated memory across all blocks in the heap.
        UInt64 blockSize{};

        /// @brief Returns the total size of memory for all allocations in the heap. Always less or equal to @ref totalBlockSize.
        UInt64 allocationSize{};

        /// @brief Estimated memory used by the program in the heap.
        ///
        /// This value best represents the actual memory pressure of the program in the heap, as it not only factors in allocations, but also other resources.
        UInt64 usedMemory{};

        /// @brief Estimated memory available to the program in the heap.
        ///
        /// This value best represents the actual memory available to the program in the heap
        ///
        /// @see usedMemory
        UInt64 availableMemory{};
    };

    /// @brief Stores extended memory statistics, that can be queried by calling @ref IGraphicsFactory::detailedMemoryStatistics.
    ///
    /// Note that those statistics should only be used for debugging purposes, as their computation may be significantly slower compared to calling @ref IGraphicsFactory::memoryStatistics.
    struct LITEFX_RENDERING_API DetailedMemoryStatistics {
        /// @brief Defines a single statistics block.
        struct StatisticsBlock {
            /// @brief `true`, if the heap is located in video memory and `false` otherwise.
            bool onGpu{ false };

            /// @brief `true`, of the heap is accessible for the CPU and `false` otherwise.
            bool cpuVisible{ false };

            /// @brief Returns the number of memory blocks in the heap.
            UInt32 blocks{};

            /// @brief Returns the total number of allocations in the heap.
            UInt32 allocations{};

            /// @brief Returns the total size of allocated memory across all blocks in the heap.
            UInt64 blockSize{};

            /// @brief Returns the total size of memory for all allocations in the heap. Always less or equal to @ref totalBlockSize.
            UInt64 allocationSize{};

            /// @brief The number of unoccupied memory ranges between allocations.
            UInt32 unusedRangeCount{};

            /// @brief The size of the smallest allocation.
            UInt64 minAllocationSize{};

            /// @brief The size of the largest allocation.
            UInt64 maxAllocationSize{};

            /// @brief The size of the smallest unused memory range.
            UInt64 minUnusedRangeSize{};

            /// @brief The size of the largest unused memory range.
            UInt64 maxUnusedRangeSize{};
        };

        /// @brief Stores the memory statistics per location (e.g., VRAM/RAM).
        Array<StatisticsBlock> perLocation{};

        /// @brief Stores the memory statistics per @ref ResourceHeap.
        Array<StatisticsBlock> perResourceHeap{};

        /// @brief Stores the total memory statistics.
        StatisticsBlock total{};
    };

    /// @brief The interface for a graphics factory.
    class LITEFX_RENDERING_API IGraphicsFactory : public SharedObject {
    protected:
        IGraphicsFactory() noexcept = default;
        IGraphicsFactory(IGraphicsFactory&&) noexcept = default;
        IGraphicsFactory(const IGraphicsFactory&) = default;
        IGraphicsFactory& operator=(const IGraphicsFactory&) = default;
        IGraphicsFactory& operator=(IGraphicsFactory&&) noexcept = default;

    public:
        ~IGraphicsFactory() noexcept override = default;

    public:
        /// @brief Creates a virtual allocator that can be used to manage allocation from a custom block of memory.
        ///
        /// @param overallMemory The overall size (in bytes) of memory available to the allocator.
        /// @param algorithm The algorithm used to find a suitable block in the allocator memory.
        /// @return The instance of the virtual allocator.
        [[nodiscard]] virtual VirtualAllocator createAllocator(UInt64 overallMemory, AllocationAlgorithm algorithm = AllocationAlgorithm::Default) const = 0;

        /// @brief Starts a defragmentation process for the resources allocated from the factory.
        ///
        /// Defragmentation is an iterative process. Calling this method starts defragmentation, which is then advanced by alternating calls to @ref beginDefragmentationPass and @ref endDefragmentationPass. The
        /// process ends if @ref endDefragmentationPass returns `true`.
        ///
        /// During a defragmentation pass, a number of resources may get allocated and the memory and filled with memory from resources that will later be destroyed. Each iteration will only create a certain
        /// number of such move events, which is mainly influenced by the @p maxBytesToMove and @p maxAllocationsToMove parameters, as well as the provided @p strategy. This way, the number of moves per frame can
        /// be limited, which helps with keeping a steady frame rate.
        ///
        /// Moving a resource happens by recording copy commands on a command buffer created from @p queue. If a resource does not need to be copied, for example because it only contains temporary data, you can
        /// set the @ref IDeviceMemory::volatileMove property to `true`. This will only allocate a new resource, but wont copy the contents of the old allocation.
        ///
        /// Calling @ref beginDefragmentationPass returns the fence on that queue after which the move commands have been executed. You can either manually wait for the fence (e.g., by calling @ref
        /// ICommandQueue::lastCompletedFence) or call @ref endDefragmentationPass directly. Keep in mind that @ref endDefragmentationPass blocks to wait for the last fence to finish, so you might want to
        /// consider the alternative approach if you are doing per-frame defragmentation.
        ///
        /// Calling this method while another defragmentation process is active will raise an exception.
        ///
        /// @param queue The queue to execute the move commands on.
        /// @param strategy The strategy to pack the fragmented memory.
        /// @param maxBytesToMove The maximum number of bytes to move during this pass or `0`, if no limitation should be imposed.
        /// @param maxAllocationsToMove The maximum number of allocations to move during this pass or `0`, if no limitation should be imposed.
        /// @throws RuntimeException Thrown, if another defragmentation process is currently running and has not yet finished.
        virtual void beginDefragmentation(const ICommandQueue& queue, DefragmentationStrategy strategy = DefragmentationStrategy::Balanced, UInt64 maxBytesToMove = 0u, UInt32 maxAllocationsToMove = 0u) const = 0;

        /// @brief Starts a new defragmentation pass.
        ///
        /// Before calling this method, make sure you have started a defragmentation process by calling @ref beginDefragmentation. If no defragmentation process is currently started, calling this method raises an
        /// exception.
        ///
        /// You are expected to issue alternating calls to this method and @ref endDefragmentationPass. This can happen either at the beginning or end of a frame, or on a separate thread.
        ///
        /// In between a call to this method and @ref endDefragmentationPass, the moved-from resources remain valid. Only after the move has finished and @ref endDefragmentationPass has been called, they will get
        /// invalidated. This means, that you might have to update descriptor bindings for the resource. You can subscribe to the @ref IDeviceMemory::moved event for this purpose. You might also want to issue a
        /// barrier to transition an image resource back into the required layout. Calling this method will leave the new resource in a @ref ImageLayout::Common state.
        ///
        /// @throws RuntimeException Thrown, if no defragmentation process is currently active.
        /// @see beginDefragmentation
        /// @see endDefragmentationPass
        virtual UInt64 beginDefragmentationPass() const = 0;

        /// @brief Ends a defragmentation pass.
        ///
        /// Before calling this method, make sure you have started a defragmentation process by calling @ref beginDefragmentation. If no defragmentation process is currently started, calling this method raises an
        /// exception.
        ///
        /// This method waits for the fence issued by the last call to @ref beginDefragmentation before first invoking the @ref IDeviceMemory::moved event on all affected resources and finally destroying the
        /// moved-from resources.
        ///
        /// @throws RuntimeException Thrown, if no defragmentation process is currently active.
        /// @see beginDefragmentation
        /// @see beginDefragmentationPass
        virtual bool endDefragmentationPass() const = 0;

        /// @brief Allocates a single resource as described by @p allocationInfo.
        ///
        /// @param allocationInfo The description of the resource to allocate.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The resource allocation result.
        virtual ResourceAllocationResult allocate(const ResourceAllocationInfo& allocationInfo, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            if (std::holds_alternative<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo))
            {
                auto& imageInfo = std::get<ResourceAllocationInfo::ImageInfo>(allocationInfo.ResourceInfo);
                return this->createTexture(allocationInfo.Name, imageInfo.Format, imageInfo.Size, imageInfo.Dimensions, imageInfo.Levels, imageInfo.Layers, imageInfo.Samples, allocationInfo.Usage, allocationBehavior);
            }
            else
            {
                auto& bufferInfo = std::get<ResourceAllocationInfo::BufferInfo>(allocationInfo.ResourceInfo);

                if (bufferInfo.Type == BufferType::Vertex && bufferInfo.VertexBufferLayout != nullptr)
                    return std::dynamic_pointer_cast<IBuffer>(
                        this->createVertexBuffer(allocationInfo.Name, *bufferInfo.VertexBufferLayout, bufferInfo.Heap, bufferInfo.Elements, allocationInfo.Usage, allocationBehavior));
                else if (bufferInfo.Type == BufferType::Index && bufferInfo.IndexBufferLayout != nullptr)
                    return std::dynamic_pointer_cast<IBuffer>(
                        this->createIndexBuffer(allocationInfo.Name, *bufferInfo.IndexBufferLayout, bufferInfo.Heap, bufferInfo.Elements, allocationInfo.Usage, allocationBehavior));
                else
                    return this->createBuffer(allocationInfo.Name, bufferInfo.Type, bufferInfo.Heap, bufferInfo.ElementSize, bufferInfo.Elements, allocationInfo.Usage, allocationBehavior);
            }
        }

        /// @brief Allocates a set of resources as described by @p allocationInfos.
        ///
        /// If the @p alias parameter is set to `true`, the allocator attempts to overlap the resources in a single allocation, which saves memory. However, this implies that the resources must be properly
        /// synchronized and accessed according to the aliasing rules imposed by the backend. Most notably, you have to manually insert aliasing barriers to isolate access.
        ///
        /// Note that overlapping arbitrary resource types might not be possible on each GPU. You can check if aliasing is supported for the desired resources by calling @ref canAlias first. If aliasing is not
        /// possible, you can instead fallback to non-overlapping resources by setting the @p alias parameter to `false` accordingly. If you attempt to allocate aliasing resources on an unsupported GPU directly,
        /// this method will raise an error.
        ///
        /// @param allocationInfos The description of the resources to allocate.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @param alias `true` if the allocator should attempt to overlap the allocations and `false` otherwise.
        /// @return A generator that returns the individual resources that have been allocated.
        /// @throws InvalidArgumentException Thrown, if the @p alias parameter is set to `true`, but the provided @p allocationInfos cannot be overlapped on the system's GPU.
        /// @see canAlias
        /// @see https://gpuopen-librariesandsdks.github.io/D3D12MemoryAllocator/html/resource_aliasing.html
        /// @see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/resource_aliasing.html
        virtual Generator<ResourceAllocationResult> allocate(Enumerable<const ResourceAllocationInfo&> allocationInfos, AllocationBehavior allocationBehavior = AllocationBehavior::Default, bool alias = false) const = 0;

        /// @brief Checks if the resources described by @p allocationInfos can be overlapped.
        ///
        /// @param allocationInfos The resource descriptions to check.
        /// @return `true`, if the resources described by @p allocationInfos can be overlapped and `false` otherwise.
        /// @see allocate
        /// @see ResourceAllocationInfo::AliasingOffset
        virtual bool canAlias(Enumerable<const ResourceAllocationInfo&> allocationInfos) const = 0;

        /// @brief Creates a buffer of type @p type.
        ///
        /// @param type The type of the buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getBuffer(type, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer of type @p type.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param type The type of the buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetBuffer(buffer, type, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a specific descriptor.
        ///
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), heap, descriptor.elementSize(), elements, usage, allocationBehavior);
        };
        
        /// @brief Tries to create a buffer that can be bound to a specific descriptor.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer,const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->tryCreateBuffer(buffer, descriptor.type(), heap, descriptor.elementSize(), elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a specific descriptor.
        ///
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(descriptor.type(), heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a specific descriptor.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->tryCreateBuffer(buffer, descriptor.type(), heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryCreateBuffer(buffer, pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->createBuffer(pipeline.layout()->descriptorSet(space), binding, heap, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryCreateBuffer(buffer, pipeline.layout()->descriptorSet(space), binding, heap, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer of type @p type.
        ///
        /// @param name The name of the buffer.
        /// @param type The type of the buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer of type @p type.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param type The type of the buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetBuffer(buffer, name, type, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a specific descriptor.
        ///
        /// @param name The name of the buffer.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), heap, descriptor.elementSize(), elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a specific descriptor.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->tryCreateBuffer(buffer, name, descriptor.type(), heap, descriptor.elementSize(), elements, usage, allocationBehavior);
        };
        
        /// @brief Creates a buffer that can be bound to a specific descriptor.
        ///
        /// @param name The name of the buffer.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->createBuffer(name, descriptor.type(), heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a specific descriptor.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param descriptorSet The layout of the descriptors parent descriptor set.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const String& name, const IDescriptorSetLayout& descriptorSet, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            auto& descriptor = descriptorSet.descriptor(binding);
            return this->tryCreateBuffer(buffer, name, descriptor.type(), heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param name The name of the buffer.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, heap, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryCreateBuffer(buffer, name, pipeline.layout()->descriptorSet(space), binding, heap, elements, usage, allocationBehavior);
        };

        /// @brief Creates a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param name The name of the buffer.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the buffer.
        inline SharedPtr<IBuffer> createBuffer(const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->createBuffer(name, pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Tries to create a buffer that can be bound to a descriptor of a specific descriptor set.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param pipeline The pipeline that provides the descriptor set.
        /// @param space The space, the descriptor set is bound to.
        /// @param binding The binding point of the descriptor within the parent descriptor set.
        /// @param heap The heap to allocate the buffer on.
        /// @param elementSize The size of an element in the buffer (in bytes).
        /// @param elements The number of elements in the buffer (in case the buffer is an array).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateBuffer(SharedPtr<IBuffer>& buffer, const String& name, const IPipeline& pipeline, UInt32 space, UInt32 binding, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryCreateBuffer(buffer, name, pipeline.layout()->descriptorSet(space), binding, heap, elementSize, elements, usage, allocationBehavior);
        };

        /// @brief Creates a vertex buffer, based on the @p layout.
        ///
        /// A vertex buffer can be used by different @ref RenderPipelines, as long as they share a common input assembler state.
        ///
        /// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the @p elements parameter.
        ///
        /// @param layout The layout of the vertex buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of vertices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the vertex buffer.
        inline SharedPtr<IVertexBuffer> createVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getVertexBuffer(layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Tries to create a vertex buffer, based on the @p layout.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param layout The layout of the vertex buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of vertices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetVertexBuffer(buffer, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Creates a vertex buffer, based on the @p layout.
        ///
        /// A vertex buffer can be used by different @ref RenderPipelines, as long as they share a common input assembler state.
        ///
        /// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the @p elements parameter.
        ///
        /// @param name The name of the buffer.
        /// @param layout The layout of the vertex buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of vertices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the vertex buffer.
        inline SharedPtr<IVertexBuffer> createVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getVertexBuffer(name, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Tries to create a vertex buffer, based on the @p layout.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param layout The layout of the vertex buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of vertices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetVertexBuffer(buffer, name, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Creates an index buffer, based on the @p layout.
        ///
        /// An index buffer can be used by different @ref RenderPipelines, as long as they share a common input assembler state.
        ///
        /// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the @p elements parameter.
        ///
        /// @param layout The layout of the index buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of indices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the index buffer.
        inline SharedPtr<IIndexBuffer> createIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getIndexBuffer(layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Tries to create an index buffer, based on the @p layout.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param layout The layout of the index buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of indices).
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetIndexBuffer(buffer, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Creates an index buffer, based on the @p layout.
        ///
        /// An index buffer can be used by different @ref RenderPipelines, as long as they share a common input assembler state.
        ///
        /// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the @p elements parameter.
        ///
        /// @param name The name of the buffer.
        /// @param layout The layout of the index buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param usage The intended usage for the buffer.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of indices).
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the index buffer.
        inline SharedPtr<IIndexBuffer> createIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getIndexBuffer(name, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Tries to create an index buffer, based on the @p layout.
        ///
        /// @param buffer The instance of the buffer, or `nullptr`, if the buffer could not be allocated.
        /// @param name The name of the buffer.
        /// @param layout The layout of the index buffer.
        /// @param heap The heap to allocate the buffer on.
        /// @param usage The intended usage for the buffer.
        /// @param elements The number of elements within the vertex buffer (i.e. the number of indices).
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the buffer was created successfully and `false` otherwise.
        inline bool tryCreateIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetIndexBuffer(buffer, name, layout, heap, elements, usage, allocationBehavior);
        }

        /// @brief Creates a texture.
        ///
        /// A texture in LiteFX is always backed by GPU-only visible memory and thus can only be transferred to/from. Thus you typically have to create a buffer using @ref createBuffer first that holds the actual
        /// image bytes. You than can transfer/copy the contents into the texture.
        ///
        /// @param format The format of the texture image.
        /// @param size The dimensions of the texture.
        /// @param dimension The dimensionality of the texture.
        /// @param layers The number of layers (slices) in this texture.
        /// @param levels The number of mip map levels of the texture.
        /// @param samples The number of samples, the texture should be sampled with.
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the texture.
        /// @see createTextures
        inline SharedPtr<IImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getTexture(format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        /// @brief Tries to create a texture.
        ///
        /// @param image The instance of the buffer, or `nullptr`, if the texture could not be allocated.
        /// @param format The format of the texture image.
        /// @param size The dimensions of the texture.
        /// @param dimension The dimensionality of the texture.
        /// @param layers The number of layers (slices) in this texture.
        /// @param levels The number of mip map levels of the texture.
        /// @param samples The number of samples, the texture should be sampled with.
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the texture was created successfully and `false` otherwise.
        inline bool tryCreateTexture(SharedPtr<IImage>& image, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetTexture(image, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        /// @brief Creates a texture.
        ///
        /// A texture in LiteFX is always backed by GPU-only visible memory and thus can only be transferred to/from. Thus you typically have to create a buffer using @ref createBuffer first that holds the actual
        /// image bytes. You than can transfer/copy the contents into the texture.
        ///
        /// @param name The name of the texture image.
        /// @param format The format of the texture image.
        /// @param size The dimensions of the texture.
        /// @param dimension The dimensionality of the texture.
        /// @param layers The number of layers (slices) in this texture.
        /// @param levels The number of mip map levels of the texture.
        /// @param samples The number of samples, the texture should be sampled with.
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return The instance of the texture.
        /// @see createTextures
        inline SharedPtr<IImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getTexture(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        /// @brief Tries to create a texture.
        ///
        /// @param image The instance of the buffer, or `nullptr`, if the texture could not be allocated.
        /// @param name The name of the texture image.
        /// @param format The format of the texture image.
        /// @param size The dimensions of the texture.
        /// @param dimension The dimensionality of the texture.
        /// @param layers The number of layers (slices) in this texture.
        /// @param levels The number of mip map levels of the texture.
        /// @param samples The number of samples, the texture should be sampled with.
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return `true`, if the texture was created successfully and `false` otherwise.
        inline bool tryCreateTexture(SharedPtr<IImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->tryGetTexture(image, name, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        /// @brief Creates a series of textures.
        ///
        /// @param format The format of the texture images.
        /// @param size The dimensions of the textures.
        /// @param layers The number of layers (slices) in this texture.
        /// @param levels The number of mip map levels of the textures.
        /// @param samples The number of samples, the textures should be sampled with.
        /// @param usage The intended usage for the buffer.
        /// @param allocationBehavior The behavior controlling what happens if currently there is not enough memory available for the resource.
        /// @return A generator for texture instances.
        /// @see createTexture
        inline Generator<SharedPtr<IImage>> createTextures(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 layers = 1, UInt32 levels = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const {
            return this->getTextures(format, size, dimension, layers, levels, samples, usage, allocationBehavior);
        }

        /// @brief Creates a texture sampler.
        ///
        /// @param magFilter The filter operation used for magnifying.
        /// @param minFilter The filter operation used for minifying.
        /// @param borderU The border mode along the U-axis.
        /// @param borderV The border mode along the V-axis.
        /// @param borderW The border mode along the W-axis.
        /// @param mipMapMode The mip map mode.
        /// @param mipMapBias The mip map bias.
        /// @param maxLod The maximum level of detail value.
        /// @param minLod The minimum level of detail value.
        /// @param anisotropy The level of anisotropic filtering.
        /// @return The instance of the sampler.
        /// @see createSamplers
        inline SharedPtr<ISampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        /// @brief Creates a texture sampler.
        ///
        /// @param name The name of the sampler.
        /// @param magFilter The filter operation used for magnifying.
        /// @param minFilter The filter operation used for minifying.
        /// @param borderU The border mode along the U-axis.
        /// @param borderV The border mode along the V-axis.
        /// @param borderW The border mode along the W-axis.
        /// @param mipMapMode The mip map mode.
        /// @param mipMapBias The mip map bias.
        /// @param maxLod The maximum level of detail value.
        /// @param minLod The minimum level of detail value.
        /// @param anisotropy The level of anisotropic filtering.
        /// @return The instance of the sampler.
        /// @see createSamplers
        inline SharedPtr<ISampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSampler(name, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        /// @brief Creates a series of texture samplers.
        ///
        /// @param magFilter The filter operation used for magnifying.
        /// @param minFilter The filter operation used for minifying.
        /// @param borderU The border mode along the U-axis.
        /// @param borderV The border mode along the V-axis.
        /// @param borderW The border mode along the W-axis.
        /// @param mipMapMode The mip map mode.
        /// @param mipMapBias The mip map bias.
        /// @param maxLod The maximum level of detail value.
        /// @param minLod The minimum level of detail value.
        /// @param anisotropy The level of anisotropic filtering.
        /// @return A generator for sampler instances.
        /// @see createSampler
        inline Generator<SharedPtr<ISampler>> createSamplers(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const {
            return this->getSamplers(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        /// @brief Creates a bottom-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @return The bottom-level acceleration structure instance.
        /// @see IBottomLevelAccelerationStructure
        inline UniquePtr<IBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->createBottomLevelAccelerationStructure("", flags);
        }

        /// @brief Creates a bottom-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param name The name of the acceleration structure resource.
        /// @param flags The flags that define how the acceleration structure is built.
        /// @return The bottom-level acceleration structure instance.
        /// @see IBottomLevelAccelerationStructure
        inline UniquePtr<IBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->getBlas(name, flags);
        }

        /// @brief Creates a top-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @return The top-level acceleration structure instance.
        /// @see ITopLevelAccelerationStructure
        inline UniquePtr<ITopLevelAccelerationStructure> createTopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->createTopLevelAccelerationStructure("", flags);
        }

        /// @brief Creates a top-level acceleration structure.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param name The name of the acceleration structure resource.
        /// @param flags The flags that define how the acceleration structure is built.
        /// @return The top-level acceleration structure instance.
        /// @see ITopLevelAccelerationStructure
        inline UniquePtr<ITopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const {
            return this->getTlas(name, flags);
        }

        /// @brief Returns `true`, if the GPU supports resizable base address register (ReBAR) and `false` otherwise.
        ///
        /// If the GPU supports resizable base address register (ReBAR), you can use @ref ResourceHeap::GPUUpload for buffers to directly write map into GPU memory. If it is not supported, you may want to fall
        /// back to a @ref ResourceHeap::Dynamic resource instead.
        ///
        /// @return `true`, if the GPU supports resizable base address register (ReBAR) and `false` otherwise.
        virtual bool supportsResizableBaseAddressRegister() const noexcept = 0;

        /// @brief Returns an array of objects, that contain information about the current memory usage and available memory for a memory heap.
        ///
        /// @return An array of objects, containing memory statistics for a memory heap.
        virtual Array<MemoryHeapStatistics> memoryStatistics() const = 0;

        /// @brief Returns detailed memory statistics.
        ///
        /// Only call this method for debugging purposes, as it is significantly slower compared to @ref memoryStatistics, which can be called multiple times every frame without any significant performance
        /// impact.
        ///
        /// @return The detailed memory statistics of the application.
        virtual DetailedMemoryStatistics detailedMemoryStatistics() const = 0;

    private:
        virtual SharedPtr<IBuffer> getBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IBuffer> getBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IImage> getTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<IImage> getTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetBuffer(SharedPtr<IBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetBuffer(SharedPtr<IBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetTexture(SharedPtr<IImage>& image, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual bool tryGetTexture(SharedPtr<IImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual Generator<SharedPtr<IImage>> getTextures(Format format, const Size3d& size, ImageDimensions dimension, UInt32 layers, UInt32 levels, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const = 0;
        virtual SharedPtr<ISampler> getSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual SharedPtr<ISampler> getSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual Generator<SharedPtr<ISampler>> getSamplers(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const = 0;
        virtual UniquePtr<IBottomLevelAccelerationStructure> getBlas(StringView name, AccelerationStructureFlags flags) const = 0;
        virtual UniquePtr<ITopLevelAccelerationStructure> getTlas(StringView name, AccelerationStructureFlags flags) const = 0;
    };

    /// @brief Describes optional features that can be supported by a device.
    ///
    /// Device features are evaluated when creating a @ref IGraphicsDevice. If a feature is not supported by the device, an exception is raised.
    ///
    /// Note that feature support is not strictly enforced by the engine. For example, if you are calling any feature-related API, the call may succeed even if the feature is not enabled, if the GPU supports
    /// it. Graphics API validation may warn about it and the same program may fail on GPUs that do not support this feature. Enabling a feature through the settings in this structure makes the device check
    /// for support creation, resulting in a clear fail path, if a required extension is not supported by the system hardware.
    struct LITEFX_RENDERING_API GraphicsDeviceFeatures {
    public:
        /// @brief Enables or disables mesh shader support.
        bool MeshShaders { false };

        /// @brief Enables or disables ray-tracing support.
        bool RayTracing { false };

        /// @brief Enables or disables ray query and inline ray-tracing support.
        bool RayQueries { false };

        /// @brief Enables or disables support for indirect draw.
        bool DrawIndirect { false };

        /// @brief Enables or disables support for dynamic descriptor types ([SM 6.6 dynamic resources](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html) and
        /// [VK_EXT_mutable_descriptor_type](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_mutable_descriptor_type.html)).
        ///
        /// Note that support for this feature is limited in the engine. It's purpose is to bind resources of different @ref DescriptorTypes within a single descriptor array. However, you still have to allocate a
        /// @ref IDescriptorSet to be able to bind resources and retrieve resource indices by calling @ref IDescriptorSet::bindToHeap.
        ///
        /// If you are only using the DirectX 12 backend, you can generally index any bound resource this way. However, in the Vulkan backend, only resources within the descriptor set can be indexed. Descriptor
        /// sets that contain descriptors of @ref DescriptorType::ResourceDescriptorHeap or @ref DescriptorType::SamplerDescriptorHeap can be used to bind those descriptors. Such descriptor sets are called proxy
        /// sets, because they do not actually occur in the shader. Instead they are directly accessed using the `ResourceDescriptorHeap` or `SamplerDescriptorHeap` syntax. In order to acquire a resource from
        /// those heaps, you need to provide the index, that can be retrieved as described above.
        ///
        /// Be aware that dynamic descriptors (aka mutable descriptors) are considered inefficient in Vulkan. You should not use them, if you could instead use multiple descriptor sets containing unbounded
        /// descriptor arrays. They can, however, be more efficient if you can replace multiple pipeline layouts with a single one that relies on mutable type descriptors.
        bool DynamicDescriptors { false };

        /// @brief Enables support for enabling depth bounds test on @ref IRenderPipeline creation.
        ///
        /// @see https://microsoft.github.io/DirectX-Specs/d3d/DepthBoundsTest.html
        /// @see https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#features-depthBounds
        bool DepthBoundsTest { false };

        /// @brief Enables support for conservative rasterization.
        ///
        /// @see https://learn.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization
        /// @see https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_conservative_rasterization.html
        bool ConservativeRasterization { false };

        /// @brief Enables support for view instancing/multi-view.
        ///
        /// @see https://microsoft.github.io/DirectX-Specs/d3d/ViewInstancing.html#view-instance-masking
        /// @see https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_multiview.html
        bool ViewInstancing { false };
    };

    /// @brief The interface for a graphics device that.
    class LITEFX_RENDERING_API IGraphicsDevice : public SharedObject {
    protected:
        IGraphicsDevice() noexcept = default;
        IGraphicsDevice(IGraphicsDevice&&) noexcept = default;
        IGraphicsDevice(const IGraphicsDevice&) = default;
        IGraphicsDevice& operator=(const IGraphicsDevice&) = default;
        IGraphicsDevice& operator=(IGraphicsDevice&&) noexcept = default;

    public:
        ~IGraphicsDevice() noexcept override = default;

    public:
        /// @brief Returns the device state that can be used to manage resources.
        ///
        /// @return A reference of the device state.
        virtual DeviceState& state() const noexcept = 0;

        /// @brief Returns the surface, the device draws to.
        ///
        /// @return A reference of the surface, the device draws to.
        virtual const ISurface& surface() const noexcept = 0;

        /// @brief Returns the graphics adapter, the device uses for drawing.
        ///
        /// @return A reference of the graphics adapter, the device uses for drawing.
        virtual const IGraphicsAdapter& adapter() const noexcept = 0;

        /// @brief Returns the swap chain, that contains the back and front buffers used for presentation.
        ///
        /// @return The swap chain, that contains the back and front buffers used for presentation.
        virtual const ISwapChain& swapChain() const noexcept = 0;

        /// @brief Returns the swap chain, that contains the back and front buffers used for presentation.
        ///
        /// @return The swap chain, that contains the back and front buffers used for presentation.
        virtual ISwapChain& swapChain() noexcept = 0;

        /// @brief Returns the factory instance, used to create instances from the device.
        ///
        /// @return The factory instance, used to create instances from the device.
        virtual const IGraphicsFactory& factory() const noexcept = 0;

        /// @brief Returns the instance of the default @ref ICommandQueue that supports the combination of queue types specified by the @p type parameter.
        ///
        /// When the device is created, it attempts to create a queue for each singular queue type. Each GPU is expected to provide at least one queue that is capable of supporting all queue types. This queue is
        /// used as a fallback queue, if no dedicated queue for a certain type is supported. For example, if no dedicated @ref QueueType::Transfer queue can be created, calling this method for the default
        /// transfer queue will return the same queue instance as the default graphics queue, which implicitly always supports transfer operations. The same is true for compute queues. This default graphics queue
        /// is ensured to support presentation and is also created with the highest queue priority.
        ///
        /// @param type The type or a combination of types that specifies the operation the queue should support.
        /// @return The instance of the queue, used to process draw calls.
        /// @throws InvalidArgumentException Thrown, if no default queue for the combination of queue types specified with the @p type parameter has been created.
        /// @see createQueue
        inline const ICommandQueue& defaultQueue(QueueType type) const {
            return this->getDefaultQueue(type);
        }

        /// @brief Attempts to create a new queue that supports the combination of queue types specified by the @p type parameter.
        ///
        /// Note that a queue is not guaranteed to represent an *actual* hardware queue that runs in parallel to other hardware queues. Backends might create *virtual* queues, that map the same hardware queue. In
        /// this case, creating a new queue is always possible but might not yield performance benefits. As a good practice, it is advised to create only as few queues as required.
        ///
        /// If this method is not able to create a new queue (i.e., it returns `nullptr`), you can either fall back to the default queue (@ref defaultQueue) or use any queue that you created earlier instead.
        ///
        /// The @p priority parameter can be specified to request a queue with a certain priority. However, the backend is not required to return a queue with that actual priority. The default queues are always
        /// prioritized highest.
        ///
        /// @param type The type of the queue or a combination of capabilities the queue is required to support.
        /// @param priority The preferred priority of the queue.
        /// @return A pointer to the newly created queue, or `nullptr`, if no queue could be created.
        /// @see defaultQueue
        inline SharedPtr<const ICommandQueue> createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) {
            return this->getNewQueue(type, priority);
        }

        /// @brief Creates a memory barrier instance.
        ///
        /// @param syncBefore The pipeline stage(s) all previous commands have to finish before the barrier is executed.
        /// @param syncAfter The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.
        /// @return The instance of the memory barrier.
        [[nodiscard]] inline UniquePtr<IBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const {
            return this->getNewBarrier(syncBefore, syncAfter);
        }

        /// @brief Creates a new frame buffer instance.
        ///
        /// @param renderArea The initial render area of the frame buffer.
        /// @return The instance of the frame buffer.
        [[nodiscard]] inline SharedPtr<IFrameBuffer> makeFrameBuffer(const Size2d& renderArea) const {
            return this->makeFrameBuffer("", renderArea);
        }

        /// @brief Creates a new frame buffer instance.
        ///
        /// @param name The name of the frame buffer.
        /// @param renderArea The initial render area of the frame buffer.
        /// @return The instance of the frame buffer.
        [[nodiscard]] inline SharedPtr<IFrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const {
            return this->getNewFrameBuffer(name, renderArea);
        }

        /// @brief Queries the device for the maximum supported number of multi-sampling levels.
        ///
        /// This method returns the maximum supported multi-sampling level for a certain format. Typically you want to pass a back-buffer format for your swap-chain here. All lower multi-sampling levels are
        /// implicitly supported for this format.
        ///
        /// @param format The target (i.e. back-buffer) format.
        /// @return The maximum multi-sampling level.
        virtual MultiSamplingLevel maximumMultiSamplingLevel(Format format) const = 0;

        /// @brief Returns the number of GPU ticks per milliseconds.
        ///
        /// @return The number of GPU ticks per milliseconds.
        /// @see TimingEvent
        virtual double ticksPerMillisecond() const noexcept = 0;

        /// @brief Computes the required amount of device memory for an @ref IBottomLevelAccelerationStructure.
        ///
        /// Acceleration structures are built on the GPU, which requires additional memory called *scratch memory*. When creating an acceleration structure (AS), you have to provide a temporary buffer containing
        /// the scratch memory, alongside the actual buffer that stores the AS itself. This method can be used to pre-compute the buffer sizes for both buffers.
        ///
        /// This method is only supported, if the @ref GraphicsDeviceFeatures::RayTracing feature is enabled.
        ///
        /// @param blas The bottom-level acceleration structure to compute the memory requirements for.
        /// @param bufferSize The size of the acceleration structure buffer.
        /// @param scratchSize The size of the scratch memory buffer.
        /// @param forUpdate If set to `true`, @p scratchSize will contain the amount of scratch memory required for an update.
        inline void computeAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const {
            this->getAccelerationStructureSizes(blas, bufferSize, scratchSize, forUpdate);
        }

        /// @brief Computes the required amount of device memory for an @ref ITopLevelAccelerationStructure.
        ///
        /// Acceleration structures are built on the GPU, which requires additional memory called *scratch memory*. When creating an acceleration structure (AS), you have to provide a temporary buffer containing
        /// the scratch memory, alongside the actual buffer that stores the AS itself. This method can be used to pre-compute the buffer sizes for both buffers.
        ///
        /// This method is only supported, if the @ref GraphicsDeviceFeatures::RayTracing feature is enabled.
        ///
        /// @param tlas The top-level acceleration structure to compute the memory requirements for.
        /// @param bufferSize The size of the acceleration structure buffer.
        /// @param scratchSize The size of the scratch memory buffer.
        /// @param forUpdate If set to `true`, @p scratchSize will contain the amount of scratch memory required for an update.
        inline void computeAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const {
            this->getAccelerationStructureSizes(tlas, bufferSize, scratchSize, forUpdate);
        }

        /// @brief Allocates a range of descriptors in the global descriptor heaps for the provided @p descriptorSet.
        ///
        /// @param descriptorSet The descriptor set containing the descriptors to update.
        /// @param heapType The type of the descriptor heap to allocate descriptors on.
        /// @return The allocation for the descriptor set at the descriptor heap indicated by @p heapType.
        [[nodiscard]] inline VirtualAllocator::Allocation allocateGlobalDescriptors(const IDescriptorSet& descriptorSet, DescriptorHeapType heapType) const {
            return this->doAllocateGlobalDescriptors(descriptorSet, heapType);
        }

        /// @brief Releases a range of descriptors from the global descriptor heaps.
        ///
        /// This is done, if a descriptor set layout is destroyed, of a descriptor set, which contains an unbounded array is freed. It will cause the global descriptor heaps to fragment, which may result in
        /// inefficient future descriptor allocations and should be avoided. Consider caching descriptor sets with unbounded arrays instead. Also avoid relying on creating and releasing pipeline layouts during
        /// runtime. Instead, it may be more efficient to write shaders that support multiple pipeline variations, that can be kept alive for the lifetime of the whole application.
        inline void releaseGlobalDescriptors(const IDescriptorSet& descriptorSet) const {
            this->doReleaseGlobalDescriptors(descriptorSet);
        }

        /// @brief Updates a range of descriptors in the global buffer descriptor heap with the descriptors from @p descriptorSet.
        ///
        /// @param descriptorSet The descriptor set to copy the descriptors from.
        /// @param binding The binding point for which to update the descriptors.
        /// @param offset The index of the first descriptor in a descriptor array at the binding point.
        /// @param descriptors The number of descriptors in a descriptor array to copy, starting at the offset.
        inline void updateGlobalDescriptors(const IDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const {
            this->doUpdateGlobalDescriptors(descriptorSet, binding, offset, descriptors);
        }

        /// @brief Binds the descriptors of the descriptor set to the global descriptor heaps.
        ///
        /// Note that after binding the descriptor set, the descriptors must not be updated anymore, unless they are elements on unbounded descriptor arrays, in which case you have to ensure manually to not
        /// update them, as long as they may still be in use!
        ///
        /// @param commandBuffer The command buffer to bind the descriptor set on.
        /// @param descriptorSet The descriptor set to bind.
        /// @param pipeline The pipeline to bind the descriptor set to.
        inline void bindDescriptorSet(const ICommandBuffer& commandBuffer, const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const {
            this->doBindDescriptorSet(commandBuffer, descriptorSet, pipeline);
        }

        /// @brief Binds the global descriptor heap.
        ///
        /// @param commandBuffer The command buffer to issue the bind command on.
        inline void bindGlobalDescriptorHeaps(const ICommandBuffer& commandBuffer) const noexcept {
            this->doBindGlobalDescriptorHeaps(commandBuffer);
        }

    private:
        virtual void getAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const = 0;
        virtual void getAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const = 0;
        virtual VirtualAllocator::Allocation doAllocateGlobalDescriptors(const IDescriptorSet& descriptorSet, DescriptorHeapType heapType) const = 0;
        virtual void doReleaseGlobalDescriptors(const IDescriptorSet& descriptorSet) const = 0;
        virtual void doUpdateGlobalDescriptors(const IDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const = 0;
        virtual void doBindDescriptorSet(const ICommandBuffer& commandBuffer, const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const = 0;
        virtual void doBindGlobalDescriptorHeaps(const ICommandBuffer& commandBuffer) const noexcept = 0;

    public:
        /// @brief Waits until all queues allocated from the device have finished the work issued prior to this point.
        ///
        /// Note that you must synchronize calls to this method, i.e., you have to ensure no other thread is submitting work on any queue while waiting. Calling this method only guarantees that all *prior* work
        /// is finished after returning. If any other thread submits work to any queue after calling this method, this workload is not waited on.
        virtual void wait() const = 0;

    private:
        virtual UniquePtr<IBarrier> getNewBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const = 0;
        virtual SharedPtr<IFrameBuffer> getNewFrameBuffer(StringView name, const Size2d& renderArea) const = 0;
        virtual const ICommandQueue& getDefaultQueue(QueueType type) const = 0;
        virtual SharedPtr<const ICommandQueue> getNewQueue(QueueType type, QueuePriority priority) = 0;
    };

    /// @brief The interface to access a render backend.
    class LITEFX_RENDERING_API IRenderBackend : public IBackend {
    protected:
        IRenderBackend() noexcept = default;
        IRenderBackend(IRenderBackend&&) noexcept = default;
        IRenderBackend(const IRenderBackend&) = default;
        IRenderBackend& operator=(const IRenderBackend&) = default;
        IRenderBackend& operator=(IRenderBackend&&) noexcept = default;

    public:
        ~IRenderBackend() noexcept override = default;

    public:
        /// @brief Lists all available graphics adapters.
        ///
        /// @return An array of pointers to all available graphics adapters.
        inline Enumerable<SharedPtr<const IGraphicsAdapter>> listAdapters() const {
            return this->getAdapters();
        }

        /// @brief Finds an adapter using its unique ID.
        ///
        /// Note that the adapter ID is optional, which allows the back-end to return a default adapter instance. Which adapter is used as *default* adapter, depends on the actual back-end implementation. The
        /// interface does not make any constraints on the default adapter to choose. A naive implementation might simply return the first available adapter.
        ///
        /// @param adapterId The unique ID of the adapter, or `std::nullopt` to find the default adapter.
        /// @return A pointer to a graphics adapter, or `nullptr`, if no adapter could be found.
        /// @see IGraphicsAdapter
        virtual const IGraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const = 0;

        /// @brief Finds an adapter using a preference setting, based on the user preferences made in the operating system settings.
        ///
        /// Note that the backend might return any adapter if the operating system does not support GPU preference settings, or if the backend does not support querying adapters from such a setting.
        ///
        /// The Vulkan backend only supports this method if the DirectX 12 backend is also available, as it performs the query through DXGI.
        ///
        /// @param preference The profile for the preferred adapter.
        /// @return A pointer to a graphics adapter, or `nullptr`, if no adapter could be found.
        /// @see IGraphicsAdapter
        virtual const IGraphicsAdapter* findAdapter(GpuPreference preference) const = 0;

        /// @brief Looks up a device and returns a pointer to it, or `nullptr`, if no device with the provided @p name could be found.
        ///
        /// @param name The name of the device.
        /// @return A pointer to the device or `nullptr`, if no device could be found.
        virtual IGraphicsDevice* device(const String& name) = 0;

        /// @brief Looks up a device and returns a pointer to it, or `nullptr`, if no device with the provided @p name could be found.
        ///
        /// @param name The name of the device.
        /// @return A pointer to the device or `nullptr`, if no device could be found.
        virtual const IGraphicsDevice* device(const String& name) const = 0;

        /// @brief Looks up a device and returns a pointer to it, or `nullptr`, if no device with the provided @p name could be found.
        ///
        /// @param name The name of the device.
        /// @return A pointer to the device or `nullptr`, if no device could be found.
        virtual inline const IGraphicsDevice* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// @brief Looks up a device and returns a pointer to it, or `nullptr`, if no device with the provided @p name could be found.
        ///
        /// @param name The name of the device.
        /// @return A pointer to the device or `nullptr`, if no device could be found.
        virtual inline IGraphicsDevice* operator[](const String& name) noexcept {
            return this->device(name);
        };

    private:
        virtual Enumerable<SharedPtr<const IGraphicsAdapter>> getAdapters() const = 0;
    };

    /// @brief Concept that can be used to refer to render backend implementations.
    template <typename T>
    concept render_backend = meta::implements<T, IRenderBackend>;

}