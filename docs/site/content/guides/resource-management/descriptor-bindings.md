# Descriptor bindings

Shaders access buffers, images and samplers through *descriptors*. This guide explains how to describe which resource a descriptor points to,
and the rules both backends impose on it. LiteFX checks those rules whenever a resource is bound, and throws an exception that names the
violated rule, rather than leaving you with undefined behavior on the GPU.

The rules apply to both backends. Code that passes them on one backend is portable to the other, except for the format and size limits of
the device, which can differ between GPUs.

## Describing bindings

A `DescriptorBinding` describes how a resource is bound: which resource, at which binding point, and which part of it. You pass bindings
when allocating a descriptor set, and when updating one later. The easiest way to create them are the factory functions, one for each kind
of descriptor:

| Factory function | Binds |
|---|---|
| `DescriptorBinding::constantBuffer(buffer, element, elements)` | Constant buffer elements, each to its own descriptor |
| `DescriptorBinding::structuredBuffer(buffer, firstElement, elements, elementsPerDescriptor, stride)` | A (writable) structured buffer |
| `DescriptorBinding::byteAddressBuffer(buffer, firstElement, elements, elementsPerDescriptor)` | A (writable) byte address buffer |
| `DescriptorBinding::texelBuffer(buffer, texelFormat, firstElement, elements, elementsPerDescriptor)` | A (writable) texel buffer |
| `DescriptorBinding::texture(image, firstLevel, levels, firstLayer, layers)` | A texture, optionally a range of its levels and layers |
| `DescriptorBinding::writableTexture(image, level, firstLayer, layers)` | A single level of a writable texture |
| `DescriptorBinding::inputAttachment(image)` | An input attachment |
| `DescriptorBinding::sampler(sampler)` | A sampler |
| `DescriptorBinding::accelerationStructure(accelerationStructure)` | A ray tracing acceleration structure |

All parameters after the resource are optional. Two functions adjust a binding afterwards:

- **`at(binding)`** sets the binding point. Without it, a binding in a list targets the binding point of its position in that list.
- **`toDescriptor(index)`** starts binding at a descriptor other than the first one of a descriptor array in a descriptor set.

```cpp
// Allocate a descriptor set, binding a constant buffer at binding point 0 and a texture at binding point 1.
auto bindings = layout.allocate({ DescriptorBinding::constantBuffer(*cameraBuffer), DescriptorBinding::texture(*texture) });

// Later, replace the texture.
bindings->update(DescriptorBinding::texture(*otherTexture).at(1));
```

Designated initializers work as well, for example `{ .resource = *buffer, .firstElement = 1, .elements = 1 }`, which is convenient for bindings
that only need a few of the fields. A binding without a resource binds nothing, which lets you allocate a descriptor set first and fill it
later.

*Descriptor ranges*, which hold descriptors directly on the global descriptor heap (see the [dynamic descriptors
sample](../../tutorials/samples-advanced/dynamic-descriptors.md)), are updated with the same bindings. A range has no layout that defines
descriptor types, so the type is passed along with the bindings. The binding point selects the descriptor within the range: `at(index)`
starts binding at descriptor `index`, and without it, a binding in a list starts at its position in that list:

```cpp
// Bind the instances, starting at the first descriptor of the range.
descriptorRange->update(DescriptorType::StructuredBuffer, DescriptorBinding::structuredBuffer(*instanceBuffer, 0u, NUM_INSTANCES));

// Bind another buffer to the descriptor at index 16 of the range.
descriptorRange->update(DescriptorType::StructuredBuffer, DescriptorBinding::structuredBuffer(*otherBuffer).at(16));
```

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

The element range you bind (`firstElement` and `elements`) has to lie within the buffer. Elements are aligned to the requirements of the
descriptors a buffer can be bound to (see [buffer layout](buffers-textures-samplers.md#buffer-layout)), and their size is limited: constant
buffer elements must not exceed the device's limit for constant buffers (64 KiB in DirectX 12), storage buffer elements the limit for
storage buffers. Byte address buffers additionally require an element size that is a multiple of 4 bytes.

### Elements and descriptors

A buffer binding distributes the bound elements across descriptors, which `elementsPerDescriptor` controls:

- **`1` (the default)** binds each element to its own descriptor. Binding several elements thus fills consecutive descriptors of a
  descriptor array, as the [bindless sample](../../tutorials/samples-intermediate/bindless.md) does with its instances.
- **`0`** binds all bound elements to a single descriptor. The shader then sees them as one array, for example a `StructuredBuffer<T>` that it
  indexes itself.
- **Any other value** distributes the elements across `elements / elementsPerDescriptor` descriptors, each viewing that many elements. The
  number of bound elements must be a multiple of it.

A descriptor that views more than one element requires a buffer without padding between its elements, i.e., an aligned element size that
equals the element size. If the alignment rules pad the elements, binding fails, and the elements have to be bound to their own descriptors.

Constant buffers always bind each element to its own descriptor, which `DescriptorBinding::constantBuffer` reflects: arrays of constant
buffers are expressed as descriptor arrays.

### Structured buffers

A structured buffer descriptor sees its memory as an array of structures, and the *stride* is the size of one structure. By default
(`stride` is `0`), it is derived from the element size of the buffer, so each element holds one structure. This is usually what you want:
store one structure per element, and bind the elements with `elementsPerDescriptor` set to `0` to view them as an array.

Only set the stride explicitly if an element holds more than one structure, for example if a single element contains all draw commands of a
frame:

```cpp
auto bindings = layout.allocate({ DescriptorBinding::structuredBuffer(*commandsBuffer, 0, 1, 1, sizeof(IndirectIndexedBatch)) });
```

The stride has to divide both the element size and the aligned element size, so that the structures line up with every element. If that is
not the case, pad the structure to a size that does.

!!! warning "Portability of the default stride"
    On Vulkan, the stride is always defined by the shader, so a missing stride goes unnoticed there. On DirectX 12, the descriptor uses the
    stride you provide, or the buffer's element size. If an element holds several structures and no stride is set, the descriptor only
    covers the first structure of each element: reads beyond it return zeros, and writes are discarded.

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
    sample](../../tutorials/samples-intermediate/compute.md) does this.

## Samplers

Samplers can only be bound to `Sampler` descriptors.
