# Barriers

A *barrier* is a command that separates the commands before it from the commands after it, within the same submission. It does up to
three things:

1. **It orders execution.** Commands after the barrier wait until commands before it have reached a certain point in the pipeline.
2. **It makes memory visible.** Writes of the commands before the barrier become visible to reads and writes of the commands after it. Without
   this, a later command could read stale data from a cache, even if it runs after the write.
3. **It transitions images.** It changes the layout of an image, so that the commands after the barrier can access it in a different way.

Barriers only reach back to the start of their submission. Everything that was submitted to the same queue earlier has already completed
when a submission starts, and its writes are visible. For the barriers you write, this means:

- **A resource that is accessed for the first time in a submission** needs a barrier only if its layout has to change. That barrier can use
  `PipelineStage::None` and `ResourceAccess::None` as before values.
- **A resource that a previous command in the same submission accessed** needs a barrier with the stages and access of that command, if one
  of the two accesses is a write, or if the layout changes.

## Recording a barrier

Barriers are created with `makeBarrier`, either on the device or on a command buffer, and recorded with `ICommandBuffer::barrier`:

```cpp
auto barrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::Fragment);
barrier->transition(*particleBuffer, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead);
barrier->transition(*heightMap, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
commandBuffer->barrier(*barrier);
```

The two stages passed to `makeBarrier` apply to the whole barrier: the commands after it wait at `PipelineStage::Fragment` until the commands
before it have finished `PipelineStage::Compute`. Each `transition` then describes one resource: how it was accessed before the barrier, how it
is accessed after it, and for images, the layout before and after.

Keep these rules in mind:

- **A barrier is recorded when you call `barrier`.** Transitions you add to the barrier object afterwards are not part of that recording.
- **A barrier object can be recorded several times,** for example in every iteration of a loop that repeats the same dependency.
- **A barrier without transitions does nothing.** To make *all* memory visible without naming resources, use a [global
  barrier](#global-barriers).
- **Batch transitions.** One barrier with ten transitions is cheaper than ten barriers with one transition each. Collect everything that changes
  at the same point into one barrier.

## Pipeline stages

The stages describe *where* in the pipeline the commands before the barrier have to be finished (`syncBefore`), and where the commands after
it have to wait (`syncAfter`):

| Stage | Covers |
|---|---|
| `None` | Nothing. Before: do not wait for anything. After: nothing waits. |
| `All` | Every command. The safe, but slowest choice. |
| `Draw` | All stages of draw calls. |
| `InputAssembly` | Reading vertex and index buffers. |
| `Vertex`, `TessellationControl`, `TessellationEvaluation`, `Geometry` | The respective shader stages. |
| `Task`, `Mesh` | Task (amplification) and mesh shaders. |
| `Fragment` | Fragment (pixel) shaders. |
| `DepthStencil` | Depth and stencil tests. |
| `RenderTarget` | Writing to render targets. |
| `Indirect` | Reading arguments of indirect draws and dispatches. |
| `Compute` | Compute shaders. |
| `Transfer` | Copies. |
| `Resolve` | Resolving multi-sampled images. |
| `AllShading` | Every shader stage, but none of the fixed-function stages. |
| `AccelerationStructureBuild`, `AccelerationStructureCopy`, `Raytracing` | Building and copying acceleration structures, and ray tracing shaders. |

Stages can be combined, for example `PipelineStage::Compute | PipelineStage::Transfer`, except for `None`, which stands alone.

Pick the stages as narrow as possible: `syncBefore` should name the stages that *wrote* (or read) the resource, `syncAfter` the stages that
*will* access it. The wider they are, the more work stalls.

## Resource access

The access values describe *how* a resource is used before and after the barrier:

| Access | Meaning |
|---|---|
| `None` | Not accessed. |
| `VertexBuffer`, `IndexBuffer` | Read as vertex or index buffer. |
| `UniformBuffer` | Read as uniform (constant) buffer. |
| `ShaderRead` | Read by a shader through a *read-only* binding: a sampled texture or a read-only buffer. |
| `ShaderReadWrite` | Accessed by a shader through a *writable* binding (`RWTexture2D`, `RWStructuredBuffer`, ...), whether it is actually written or only read. |
| `RenderTarget` | Read and written as a color render target. |
| `DepthStencilRead`, `DepthStencilWrite` | Read or written by the depth and stencil tests. `DepthStencil` combines both. |
| `Indirect` | Read as arguments of an indirect draw or dispatch. |
| `TransferRead`, `TransferWrite` | Source or destination of a copy. |
| `ResolveRead`, `ResolveWrite` | Source or destination of a multi-sampling resolve. |
| `AccelerationStructureRead`, `AccelerationStructureWrite` | Read or written as an acceleration structure. |
| `Common` | Any access. Useful as the access *before* a barrier, when you do not know or do not want to spell out how a resource was used. |

Two rules apply to all barriers:

- **A stage of `None` requires an access of `None`** on the same side of the barrier: if nothing is waited for, no access can be made
  visible, and if nothing waits, nothing can be accessed. The opposite is allowed: a transition can use `ResourceAccess::None` in a barrier
  with other stages, for example for an image that is used for the first time.
- **The binding decides between `ShaderRead` and `ShaderReadWrite`, not the shader code.** A texture bound as `RWTexture2D` is accessed with
  `ShaderReadWrite`, even if the shader only reads it.

## Image layouts

Buffers are plain memory. Images, however, have a *layout*, and each layout only allows certain kinds of access:

| Layout | Allows | Typical use |
|---|---|---|
| `Undefined` | Nothing. Only valid as the layout *before* a barrier. | The first use of an image, or when its contents can be discarded. |
| `Common` | `ShaderRead`, `TransferRead`, `TransferWrite`, `Common` | The [default state](#the-default-state). Uploads, sampling, copies. |
| `ShaderResource` | `ShaderRead` | Sampled textures and input attachments. |
| `ReadOnly` | `ShaderRead`, `TransferRead` | An image that is sampled and copied from, without transitions in between. |
| `ReadWrite` | `ShaderReadWrite` | Storage images written or read by shaders. |
| `CopySource`, `CopyDestination` | `TransferRead`, `TransferWrite` | Copies, when a dedicated layout is preferred over `Common`. |
| `RenderTarget` | `RenderTarget` | Color render targets. Managed by render passes. |
| `DepthRead` | `DepthStencilRead`, `ShaderRead` | Depth buffers that are tested against and sampled at the same time. |
| `DepthWrite` | `DepthStencilRead`, `DepthStencilWrite` | Depth buffers that are written. Managed by render passes. |
| `ResolveSource`, `ResolveDestination` | `ResolveRead`, `ResolveWrite` | Multi-sampling resolves. Managed by render passes. |
| `Present` | `Common` | Swap chain images that are about to be presented. Managed by render passes. |

Transitioning *from* `Undefined` tells the GPU that the previous contents of the image are not needed. It is the cheapest transition, and the
only valid one for an image that has never been used. Never transition from `Undefined` if you need the contents of the image afterwards.

!!! note "Layouts in Vulkan"
    The Vulkan backend stores all images in the `GENERAL` layout, except for `Undefined` and `Present`. A transition in Vulkan therefore only
    orders execution and memory access, and a wrong layout never causes an error. DirectX 12 enforces the table above. Always specify the
    layouts as if you were targeting DirectX 12.

## The default state

To keep code that hands resources from one system to another simple, LiteFX defines one *default state*: **an image that is not currently
used for something specific is in `ImageLayout::Common`.** Everything the engine does on its own follows this rule:

| Operation | Expects | Leaves |
|---|---|---|
| Creating an image | | `Undefined`. The first transition must start from `Undefined`. |
| `IRenderPass::begin` and `end` | Render targets that are loaded, and input attachments, in `Common`. Cleared render targets in any layout. Depth and stencil are treated separately. | All render targets and input attachments in `Common`. |
| Copies (`ICommandBuffer::transfer`) | Images in `Common`, `CopySource` or `CopyDestination` | Unchanged |
| Submissions on a transfer queue | | Images in `Common` (on DirectX 12, images *decay* to it) |
| `Blitter::generateMipMaps` | The image in `Common` | The image in `Common` |
| Defragmentation | Your `prepareMove` handler makes the resource readable for copies, keeping images in `Common` | The resource in `Common` |

Follow the same rule in your own code: when you need an image in a different layout, transition it there right before you use it, and back to
`Common` when you are done. The image is then always in a known state, and other systems can use it without knowing who used it before.

The `transition` overloads that take only one layout use this rule as well: they transition from `Common` to the given layout. These two lines
are equivalent:

```cpp
barrier->transition(*image, ResourceAccess::Common, ResourceAccess::ShaderReadWrite, ImageLayout::ReadWrite);
barrier->transition(*image, ResourceAccess::Common, ResourceAccess::ShaderReadWrite, ImageLayout::Common, ImageLayout::ReadWrite);
```

!!! tip
    If a barrier transitions an image *out of* the default state at the start of a submission, use `PipelineStage::None` and
    `ResourceAccess::None` as before values: whoever left the image there did so in an earlier submission. Within a submission, name the
    stages and access of the previous use. If you do not know them, `PipelineStage::All` and `ResourceAccess::Common` cover any previous use.

## Sub-resources

Every `transition` overload for images is also available with a sub-resource range: the first mip level and the number of levels, the first
array layer and the number of layers, and the plane:

```cpp
// Transition mip level `i` of layer `0` only.
barrier->transition(*image, i, 1, 0, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
```

Different sub-resources of the same image can be in different layouts. Make sure that all of them are back in `Common` before you hand the
image over to another system.

The `transition` overloads *without* a sub-resource range cover all mip levels, array layers and planes. The overloads *with* a range cover
exactly one plane. For depth/stencil formats, plane `0` is the depth aspect and plane `1` the stencil aspect, so you can, for example,
discard the stencil aspect while keeping the depth values:

```cpp
barrier->transition(*depthStencil, 0, 1, 0, 1, 0, ResourceAccess::None, ResourceAccess::DepthStencil, ImageLayout::Common, ImageLayout::DepthWrite);     // Depth: keep.
barrier->transition(*depthStencil, 0, 1, 0, 1, 1, ResourceAccess::None, ResourceAccess::DepthStencil, ImageLayout::Undefined, ImageLayout::DepthWrite);  // Stencil: discard.
```

## Buffers

Buffers have no layout, so a buffer transition only describes the access before and after the barrier. The overload with an element index is
accepted for future use, but currently always applies to the whole buffer.

## Global barriers

`wait` adds a barrier that applies to all resources at once:

```cpp
auto barrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::Compute);
barrier->wait(ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderReadWrite);
commandBuffer->barrier(*barrier);
```

This is the typical way to separate two dispatches that write and read the same storage resources, when you do not want to list every one of
them. A global barrier never changes a layout.

## The barrier builder

As an alternative to `makeBarrier`, the device provides a builder that reads like a sentence. It is used in some of the samples:

```cpp
auto barrier = device.buildBarrier()
    .waitFor(PipelineStage::Transfer).toContinueWith(PipelineStage::Fragment)
    .blockAccessTo(*texture, ResourceAccess::ShaderRead).transitionLayout(ImageLayout::Common, ImageLayout::ShaderResource).whenFinishedWith(ResourceAccess::TransferWrite)
    .blockAccessTo(*buffer, ResourceAccess::ShaderRead).untilFinishedWith(ResourceAccess::TransferWrite);
```

`blockAccessTo(access)` without a resource creates a global barrier. `transitionLayout` with a single layout transitions from `Common`.

## Accessing resources from multiple queues

When a resource is written on one queue and used on another, synchronization happens in two steps:

1. **The fence orders the work and makes the memory visible.** The queue that uses the resource [waits for the
   fence](queues-and-fences.md#waiting-on-another-queue) of the queue that wrote it. No barrier is needed for the memory.
2. **A barrier on the using queue transitions the image,** if it needs a layout other than the one it was left in. As the work before the
   fence has already completed, the barrier can use `PipelineStage::None` and `ResourceAccess::None` as before values.

If you follow the default state, the second step only happens where you need a specific layout anyway. You never have to transfer the
*ownership* of a resource between queues: LiteFX creates all resources so that they can be accessed by every queue.

Some layouts are not available on every queue type:

- **Transfer queues** only support `Common`. Images used on them stay in the default state.
- **Compute queues** do not support the layouts that only render passes use: `RenderTarget`, `DepthRead`, `DepthWrite`, `ResolveSource`,
  `ResolveDestination` and `Present`.
