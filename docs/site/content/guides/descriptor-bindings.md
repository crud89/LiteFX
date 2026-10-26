# Descriptor bindings

Shaders access buffers, images and samplers through *descriptors*, which you write when allocating a descriptor set (`allocate`) or later
(`update`). Both backends have rules for which resources can be bound to which descriptor, and how. LiteFX checks them whenever a resource
is bound, and throws an exception that names the violated rule, rather than leaving you with undefined behavior on the GPU.

The rules on this page apply to both backends. Code that passes them on one backend is portable to the other, except for the format and
size limits of the device, which can differ between GPUs.

## Buffers

The type of a buffer decides which descriptors it can be bound to. Descriptors that allow shaders to write additionally require the buffer to
be created with `ResourceUsage::AllowWrite`:

| Descriptor type | HLSL | Buffer type | Requires `AllowWrite` |
|---|---|---|:-:|
| `ConstantBuffer` | `ConstantBuffer<T>` | `Uniform` | |
| `StructuredBuffer` | `StructuredBuffer<T>` | `Storage` or `Indirect` | |
| `ByteAddressBuffer` | `ByteAddressBuffer` | `Storage` or `Indirect` | |
| `RWStructuredBuffer` | `RWStructuredBuffer<T>` | `Storage` or `Indirect` | :material-check: |
| `RWByteAddressBuffer` | `RWByteAddressBuffer` | `Storage` or `Indirect` | :material-check: |
| `Buffer` | `Buffer<T>` | `Texel` | |
| `RWBuffer` | `RWBuffer<T>` | `Texel` | :material-check: |

The element range you bind (`firstElement` and `elements`) has to lie within the buffer.

### Buffer layout

A buffer consists of *elements*, and the engine aligns each element to the requirements of the descriptors it can be bound to. The aligned
size, `IBuffer::alignedElementSize`, can therefore be larger than the size you requested, `IBuffer::elementSize`. How large depends on the
descriptor type, the backend and the device: constant buffers in DirectX 12, for example, are aligned to 256 bytes.

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

The size of an element is limited as well. Constant buffer elements must not exceed the device's limit for constant buffers (64 KiB in
DirectX 12), storage buffer elements the limit for storage buffers. Byte address buffers additionally require an element size that is a
multiple of 4 bytes.

### Structured buffers

A structured buffer descriptor sees its memory as an array of structures. By default, a descriptor treats each buffer element as one
structure. If an element holds an array of structures instead, for example all draw commands of a frame, pass the size of one structure as
`stride`:

```cpp
auto bindings = layout.allocate({ { .resource = *commandsBuffer, .firstElement = 0, .elements = 1, .stride = sizeof(IndirectIndexedBatch) } });
```

The stride has to divide both the element size and the aligned element size, so that the structures line up with every element. If that is
not the case, pad the structure to a size that does.

### Texel buffers

Texel buffers (`Buffer<T>` and `RWBuffer<T>`) interpret their memory in a texture format, which you pass as `texelFormat` when binding. The
format is required, and the device must support it for the descriptor, which you can check upfront with
`IGraphicsDevice::supportsTexelBufferFormat`. The element size has to be a multiple of the texel size, and an element must not contain more
texels than the device supports.

!!! warning "Reading writable texel buffers"
    DirectX 12 only guarantees *typed loads* from a `RWBuffer<T>` for the formats `R32_FLOAT`, `R32_UINT` and `R32_SINT`. Other formats can
    still be written, but reading them returns undefined values on devices that do not support it. The engine logs a warning when you bind
    such a buffer.

## Images

Images can be bound to `Texture`, `RWTexture` and `InputAttachment` descriptors, with these restrictions:

- **Writable images** (`RWTexture`) must be created with `ResourceUsage::AllowWrite`, and their format must support storage reads and writes
  on the device. You can check that with `IGraphicsDevice::supportsStorageImageFormat`.
- **The mip levels and layers you bind** have to lie within the image.
- **3D images** are always bound with their full depth: Vulkan cannot select individual depth slices.
- **Multisampled images** have no mip levels, so only their first level can be bound.
- **Cube map arrays** are currently not supported.

!!! tip "Prefer RGBA for images written by shaders"
    HLSL accesses `RWTexture` without declaring a format in the shader. Vulkan does not guarantee that BGRA formats such as `B8G8R8A8_UNORM`
    support this, so binding such an image fails on some devices. Use `R8G8B8A8_UNORM` for images that shaders write. If the image is copied into the swap chain afterwards, the
    swap chain needs the same format, since copies between images require matching formats. The [compute
    sample](../tutorials/samples-intermediate/compute.md) does this.

## Samplers

Samplers can only be bound to `Sampler` descriptors.
