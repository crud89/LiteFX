# Buffers, textures and samplers

Buffers, textures and samplers are the resources your shaders work with. All of them are created by the device's *factory*, which you get
with `device.factory()`. This guide explains how to create them, where their memory lives, and how to get data into and out of them. How to
order accesses to them on the GPU is covered by the [synchronization guide](../synchronization/index.md), and how to make them available to
shaders by the [descriptor bindings guide](descriptor-bindings.md).

Every creation function optionally takes a name as its first argument. Names are shown by graphics debuggers such as RenderDoc and PIX, so
they are worth setting for anything but throwaway resources.

## Heaps

The *heap* decides where the memory of a resource lives, and who can access it. You pass it as a `ResourceHeap` when creating a buffer:

| Heap | Memory | CPU | GPU | Typical use |
|---|---|---|---|---|
| `Resource` | GPU | — | Read and write | Data the GPU accesses often, written once or by the GPU itself |
| `Dynamic` | CPU | Write | Read | Data that changes every frame, such as transforms |
| `GPUUpload` | GPU | Write | Read | Like `Dynamic`, but in GPU memory, if the device supports it |
| `Staging` | CPU | Write | Transfer source | Intermediate copies, before transferring into a `Resource` buffer |
| `Readback` | CPU | Read | Transfer destination | Results the CPU needs, copied there by the GPU |

`GPUUpload` relies on the GPU exposing all of its memory to the CPU, called *resizable BAR*. Check
`IGraphicsFactory::supportsResizableBaseAddressRegister`, and fall back to `Dynamic` otherwise:

```cpp
auto heap = m_device->factory().supportsResizableBaseAddressRegister() ? ResourceHeap::GPUUpload : ResourceHeap::Dynamic;
```

Textures always live on the GPU, so their creation functions take no heap.

## Usage flags

The `ResourceUsage` flags state what a resource may be used for beyond being read by shaders. Some of them are required by certain
descriptors or operations, and the engine checks them:

| Flag | Allows |
|---|---|
| `AllowWrite` | Writes from shaders, e.g. through `RWStructuredBuffer` or `RWTexture2D`. Not allowed for vertex, index and uniform buffers. |
| `TransferSource` | Copying from the resource. Set implicitly for `Staging` buffers and render targets. |
| `TransferDestination` | Copying into the resource. Set implicitly for `Readback` buffers and render targets. |
| `RenderTarget` | Using an image as a render target. |
| `AccelerationStructureBuildInput` | Using a buffer as input to build a ray tracing acceleration structure. |
| `Volatile` | Skipping the copy of the resource's contents when it is moved during [defragmentation](../../tutorials/samples-advanced/defragmentation.md). |

A few combinations have names of their own: `Default` (`TransferSource | TransferDestination`, the default of all creation functions),
`FrameBufferImage` (`TransferSource | RenderTarget`) and `MipMapTarget` (`TransferSource | TransferDestination | AllowWrite`, which mip map
generation requires).

## Buffers

### Buffer types

The type of a buffer states what it is used for, and decides which descriptors it can be bound to:

| Type | Used as |
|---|---|
| `Vertex`, `Index` | Vertex and index buffers for draw calls |
| `Uniform` | Constant buffers (`ConstantBuffer<T>`) |
| `Storage` | Structured and byte address buffers (`StructuredBuffer<T>`, `ByteAddressBuffer` and their writable versions) |
| `Texel` | Typed buffers (`Buffer<T>`, `RWBuffer<T>`) |
| `Indirect` | Arguments for indirect draw and dispatch calls, which can also be bound like storage buffers |
| `AccelerationStructure`, `ShaderBindingTable` | Ray tracing |
| `Other` | Copy sources and destinations that are never bound to a descriptor |

### Creating buffers

A buffer consists of a number of *elements* of the same size. You can state the type and element size explicitly:

```cpp
auto instanceBuffer = m_device->factory().createBuffer("Instance Buffer", BufferType::Storage, ResourceHeap::Resource, sizeof(InstanceData), NUM_INSTANCES);
```

If the buffer is bound to a known descriptor, it is usually easier to let the engine take type and element size from the pipeline layout,
i.e. from the shader. You only pass the descriptor set layout and the binding point:

```cpp
auto& cameraBindingLayout = pipeline.layout()->descriptorSet(DescriptorSets::Constant);
auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBindingLayout, 0, ResourceHeap::Resource);
```

Vertex and index buffers are created from the layouts of the input assembler, which describe their elements:

```cpp
auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, vertexCount);
auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *inputAssembler->indexBufferLayout(), ResourceHeap::Resource, indexCount);
```

Elements are also the natural unit for per-frame data: a buffer with one element per frame in flight lets the CPU write the data of the next
frame, while the GPU still reads the current one.

### Buffer layout

The engine aligns each element to the requirements of the descriptors the buffer can be bound to. The aligned size,
`IBuffer::alignedElementSize`, can therefore be larger than the size you requested, `IBuffer::elementSize`. How large depends on the buffer
type, the backend and the device: constant buffers in DirectX 12, for example, are aligned to 256 bytes.

This matters when you fill a buffer with more than one element. A C++ array is packed tightly, while the buffer's elements are not
necessarily, so copying the array in one block only works if the element size happens to match the alignment. Upload such data element by
element instead, by passing one pointer per element and the size of an element. The engine then places each element at its aligned offset:

```cpp
auto elements = data
    | std::views::transform([](const InstanceData& element) { return static_cast<const void*>(&element); })
    | std::ranges::to<Array<const void*>>();

commandBuffer->transfer(elements, sizeof(InstanceData), *buffer, 0);   // For buffers in GPU memory.
buffer->map(elements, sizeof(InstanceData));                           // For buffers the CPU can write.
```

### Writing and reading buffers

How data gets into a buffer depends on its heap:

- **CPU-writable heaps** (`Dynamic`, `GPUUpload` and `Staging`) are written directly with `map`, for example
  `buffer->map(&transform, sizeof(transform), element)` writes one element. The write happens immediately, so make sure the GPU is not
  reading that element at the same time, for example by using one element per frame in flight.
- **`Resource` buffers** are written by a transfer command, which copies data from CPU memory or from another buffer:
  `commandBuffer->transfer(data, size, *buffer, firstElement, elements)`. The data is copied when the GPU executes the command buffer.
- **`Readback` buffers** are read with `map` as well, by passing `false` as its last argument, which copies from the buffer instead of into
  it: `buffer->map(&result, sizeof(result), 0, false)`. Wait for the fence of the submission that wrote the buffer first (see
  [fences](../synchronization/queues-and-fences.md#fences)).

## Textures

Textures are created with their format, size and dimensions, and optionally the number of mip levels, layers and samples:

```cpp
auto texture = m_device->factory().createTexture("Texture", Format::R8G8B8A8_UNORM, Size3d(width, height, 1), ImageDimensions::DIM_2,
    levels, 1, MultiSamplingLevel::x1, ResourceUsage::MipMapTarget);
```

A few things to keep in mind:

- **A new texture has no defined content and layout.** Treat it as if it were in the `Undefined` layout, and transition it from there before
  its first use (see [the default state](../synchronization/barriers.md#the-default-state)).
- **Textures are filled by transfer commands**, like `Resource` buffers. The [textures sample](../../tutorials/samples-beginner/textures.md)
  and the guide's pattern for [uploading images](../synchronization/patterns.md#uploading-images) show the barriers around it.
- **Mip maps** can be generated on the GPU with the engine's `Blitter`, which requires the `MipMapTarget` usage.
- **Textures that shaders write** need `AllowWrite`, and a format that supports storage access on the device (see
  [images](descriptor-bindings.md#images)).

Render targets are images as well, but you rarely create them yourself: frame buffers allocate them for the render targets of a render pass.

## Samplers

A sampler describes how a shader reads a texture: how it filters between pixels and mip levels, and what happens outside the texture's
bounds. Samplers are independent of textures, so one sampler can be used with any number of them:

```cpp
auto sampler = m_device->factory().createSampler("Sampler",
    FilterMode::Linear, FilterMode::Linear,                     // Magnification and minification filter.
    BorderMode::Repeat, BorderMode::Repeat, BorderMode::Repeat, // Behavior outside of the bounds in U, V and W direction.
    MipMapMode::Linear,                                         // Filtering between mip levels.
    0.f, std::numeric_limits<Float>::max(), 0.f,                // Mip map bias, maximum and minimum level of detail.
    16.f);                                                      // Maximum anisotropy, or 0 to disable anisotropic filtering.
```

## Lifetime

Resources are returned as shared pointers and live as long as someone holds a reference. The GPU uses them later than the CPU records the
commands, though, so releasing your reference right after recording can be too early. Command buffers only keep some resources alive on
their own: the sources of `transfer` commands passed as shared pointers, and the staging buffers they create internally. Anything else that a
command buffer uses, for example a buffer or descriptor set you only bound, must stay alive until the GPU has executed it, either by
keeping a reference yourself, or by handing it to the command buffer with `track` (see [resource
lifetime](../synchronization/command-buffers.md#resource-lifetime)).

Resources that live as long as a device, such as the buffers of a scene, are best stored in the device state, which releases them together
with the device.
