# Render passes and frames

Most synchronization in a typical frame is done by the engine: render passes transition their render targets, and the swap chain makes sure
that you never draw into an image that is still on screen. This page explains what they do, what they expect from you, and where their
responsibility ends.

## What a render pass does

`IRenderPass::begin` and `IRenderPass::end` record barriers into the render pass's own command buffers. Between the two calls, you only record
draw calls.

When the render pass **begins**:

- **Render targets that are cleared** are transitioned from `Undefined`, which discards their previous contents. They can be in any state
  before.
- **Render targets that are loaded**, i.e. created without the clear flag, are transitioned from `Common`. Their contents are preserved.
- **Depth/stencil targets** are handled per aspect: the depth aspect follows `clearBuffer`, the stencil aspect follows `clearStencil`. You
  can, for example, clear the stencil buffer while keeping the depth values of a previous pass.
- **Input attachments** are transitioned from `Common` into `ShaderResource`, so that the shaders can sample them.

When the render pass **ends**:

- **All render targets and input attachments are transitioned back to `Common`.** Their writes are complete, but not yet visible to any
  later access; the barrier that starts the next use takes care of that.
- **The present target is copied to the swap chain** or resolved into it, and presented, if the render pass has one.
- **The command buffers are submitted** to `renderPass.commandQueue()`, and `end` returns the fence of the submission.

A render pass is a submission of its own, so everything submitted to its queue before has finished when it begins. As a result, render
passes that use the same frame buffer on the same queue need no further synchronization. This is what the [render passes
sample](../../tutorials/samples-beginner/render-passes.md) relies on.

## Using render targets outside of render passes

Between render passes, the images of a frame buffer are in the default state. To use one in your own commands, for example in a compute
shader, transition it out of `Common` and back:

```cpp
auto& colorTarget = frameBuffer.image("Color");

auto preBarrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::Compute);
preBarrier->transition(colorTarget, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Common, ImageLayout::ReadWrite);
commandBuffer->barrier(*preBarrier);

// ... dispatch ...

auto postBarrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::None);
postBarrier->transition(colorTarget, ResourceAccess::ShaderReadWrite, ResourceAccess::None, ImageLayout::ReadWrite, ImageLayout::Common);
commandBuffer->barrier(*postBarrier);
```

Neither barrier needs to wait for anything outside of this command buffer: the render pass was submitted before, so it has finished when
this submission starts, and the next render pass will only start after this submission has finished. If you record more commands that use
the image after the post barrier *in the same submission*, use their stages and access as after values instead of `None`.

If this happens on a different queue than the render pass, the queue has to [wait for the fence](queues-and-fences.md#waiting-on-another-queue)
returned by `end` first, and the render pass that uses the image next has to wait for the fence of your work. The [compute
sample](../../tutorials/samples-intermediate/compute.md) shows the complete chain.

## The swap chain

The swap chain owns the images that are shown on screen, the *back buffers*. Each frame starts by asking for the next one:

```cpp
UInt32 backBuffer = m_device->swapChain().swapBackBuffer();
```

`swapBackBuffer` blocks until the back buffer can be drawn into again, i.e. until the frame that last used it has been presented. It returns the
index of the back buffer, which you use to pick the resources of the frame.

A render pass with a present target presents the frame on its own when it ends. If you produce the final image in a different way, for example
by copying the result of a compute shader into the back buffer, transition the back buffer to `ImageLayout::Present` after your last write, and
present it with the fence of the submission that contains that write:

```cpp
auto fence = graphicsQueue.submit(commandBuffer);
m_device->swapChain().present(fence);
```

The swap chain waits for the fence on the GPU before it presents, so you do not need to wait for it yourself.

## Frames in flight

The swap chain has several back buffers, typically three. While the GPU still renders one frame, the CPU can already record the next one. Those
frames are *in flight* at the same time, and every resource that a frame writes to has to exist once per frame in flight, or two frames would
overwrite each other's data:

- **Frame buffers**, one per back buffer.
- **Per-frame buffers**, such as the transform buffer of the [basic rendering sample](../../tutorials/samples-beginner/basic-rendering.md), either as
  separate buffers or as one buffer with one element per back buffer.
- **Descriptor sets** that point to per-frame resources.
- **Command buffers** that you [reuse](command-buffers.md#reusing-command-buffers).

Index all of them with the back buffer index returned by `swapBackBuffer`. Once `swapBackBuffer` returns, everything that was submitted for the
previous frame with the same index on the presenting queue has finished, so the resources of that index can be written again without any
further wait.

!!! warning "Work that is not part of the frame"
    `swapBackBuffer` only covers work that the presentation waited for: everything submitted to the presenting queue before the frame was
    presented, and everything on other queues that this queue waited for. Work on another queue that the frame did not wait for, for example a
    simulation that runs ahead on the compute queue, is not covered. Track its fences yourself.

Resources that are written only once, for example meshes uploaded at startup, do not need copies per frame. Data that changes every frame but is
only written by the CPU, such as uniform buffers in the `Dynamic` heap, does: write it with `map` into the element of the current back buffer.

## Resizing

When the window is resized, frame buffers and the swap chain need to be recreated, while the GPU may still use them. Wait until the device is
idle first:

```cpp
m_device->wait();
m_device->swapChain().reset(surfaceFormat, renderArea, 3, vsync);
frameBuffer.resize(renderArea);
```

The same applies to any other operation that releases resources that are possibly still in use, for example switching scenes or shutting down.
