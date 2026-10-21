# Patterns and pitfalls

This page collects recipes for situations that come up in most applications, followed by common mistakes and ways to find them. Each recipe
follows the [default state](barriers.md#the-default-state): images start and end in `ImageLayout::Common`.

## Uploading buffers

Upload long-lived buffers on the transfer queue, and let the first queue that uses them wait for the fence:

```cpp
auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
auto commandBuffer = transferQueue.createCommandBuffer(true);
commandBuffer->transfer(vertices.data(), vertices.size() * sizeof(Vertex), *vertexBuffer, 0, static_cast<UInt32>(vertices.size()));
m_transferFence = commandBuffer->submit();

// Later, before the first render pass that draws the mesh:
renderPass.commandQueue().waitFor(transferQueue, m_transferFence);
```

Buffers need no barrier: they have no layout, and the fence makes the copied data visible. The vertex buffer must have been created with
`ResourceUsage::TransferDestination`, which is part of `ResourceUsage::Default`. Calling `transferQueue.waitFor(m_transferFence)`
on the CPU instead works as well, but blocks your application until the copy has finished.

## Uploading images

Freshly created images are `Undefined`, so the upload starts with a transition into the default state:

```cpp
auto barrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
barrier->transition(*texture, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::Common);
commandBuffer->barrier(*barrier);

commandBuffer->transfer(imageData.data(), imageData.size(), *texture);
m_transferFence = commandBuffer->submit();
```

The image remains in `Common`, which can be sampled. The queue that samples it only has to wait for the fence. If you want the image in
`ShaderResource` instead, transition it on the queue that uses it, not on the transfer queue, which only supports `Common`.

## Textures with mip maps

The Vulkan backend generates mip maps with blits, which require a graphics queue, so the whole upload happens there:

```cpp
auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
auto commandBuffer = graphicsQueue.createCommandBuffer(true);

auto barrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
barrier->transition(*texture, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::Common);
commandBuffer->barrier(*barrier);
commandBuffer->transfer(imageData.data(), imageData.size(), *texture);

// Expects the texture in `Common` and leaves it there.
blitter->generateMipMaps(*texture, *commandBuffer);

m_textureFence = commandBuffer->submit();
```

Create the texture with `ResourceUsage::MipMapTarget`, which contains the usage flags both backends need to generate mip maps.

## Successive dispatches on the same resources

Two dispatches that write and read the same storage resources need a barrier between them, even inside one command buffer. A global barrier
saves you from listing every resource:

```cpp
commandBuffer->dispatch(firstPassGroups);

auto barrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::Compute);
barrier->wait(ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderReadWrite);
commandBuffer->barrier(*barrier);

commandBuffer->dispatch(secondPassGroups);
```

The same applies when a dispatch reads a resource through a read-only binding that the previous dispatch wrote. In that case, the second
dispatch needs the image in a different layout, so use a transition instead:

```cpp
barrier->transition(*image, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
```

## Compute work for the graphics queue

When a compute queue produces data that a render pass consumes, the queues hand the data over with a fence, and each side uses a barrier for
its own layout changes:

```cpp
// On the compute queue: bring the image into the layout for writing, write it, and return it to the default state.
auto pre = computeCommands->makeBarrier(PipelineStage::None, PipelineStage::Compute);
pre->transition(*heightMap, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Common, ImageLayout::ReadWrite);
computeCommands->barrier(*pre);

computeCommands->dispatch(groups);

auto post = computeCommands->makeBarrier(PipelineStage::Compute, PipelineStage::None);
post->transition(*heightMap, ResourceAccess::ShaderReadWrite, ResourceAccess::None, ImageLayout::ReadWrite, ImageLayout::Common);
computeCommands->barrier(*post);

auto computeFence = computeQueue.submit(computeCommands);

// On the graphics queue: wait for the compute work, then render.
renderPass.commandQueue().waitFor(computeQueue, computeFence);
renderPass.begin(frameBuffer);
```

Both barriers use `PipelineStage::None` on their outer side: everything before this submission has finished when it starts, either because
it was submitted to the same queue earlier, or because the queue waited for its fence. Likewise, nothing after it starts before it has
finished.

## Indirect arguments written by a shader

Indirect draws and dispatches read their arguments in a separate stage, which the barrier has to name:

```cpp
auto barrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::Indirect);
barrier->transition(*indirectCommands, ResourceAccess::ShaderReadWrite, ResourceAccess::Indirect);
barrier->transition(*indirectCounter, ResourceAccess::ShaderReadWrite, ResourceAccess::Indirect);
commandBuffer->barrier(*barrier);
```

If the arguments are written on a compute queue and consumed by a render pass, the fence makes them visible instead, as in the previous
recipe. Before the compute queue writes them again, it has to wait until the render pass has read them, i.e. for the fence returned by `end`.

## Copying a render target

To keep a copy of a render target, for example to sample the depth buffer while it is being written, copy it between two render passes. As
the copy is a submission of its own, it does not need to wait for the render passes around it:

```cpp
auto pre = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
pre->transition(depthTarget, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::Common, ImageLayout::CopySource);
pre->transition(*depthCopy, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
commandBuffer->barrier(*pre);

commandBuffer->transfer(depthTarget, *depthCopy);

auto post = commandBuffer->makeBarrier(PipelineStage::Transfer, PipelineStage::None);
post->transition(depthTarget, ResourceAccess::TransferRead, ResourceAccess::None, ImageLayout::CopySource, ImageLayout::Common);
post->transition(*depthCopy, ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::Common);
commandBuffer->barrier(*post);
```

The copy target is transitioned from `Undefined`, as it is overwritten completely. It must have been created with
`ResourceUsage::TransferDestination`; frame buffer images already carry `ResourceUsage::TransferSource`.

## Reading results on the CPU

To read data that the GPU produced, copy it into a buffer in the `Readback` heap, wait for the fence on the CPU, and map the buffer:

```cpp
auto readback = factory.createBuffer(BufferType::Storage, ResourceHeap::Readback, sizeof(Statistics));

auto barrier = commandBuffer->makeBarrier(PipelineStage::Compute, PipelineStage::Transfer);
barrier->transition(*statisticsBuffer, ResourceAccess::ShaderReadWrite, ResourceAccess::TransferRead);
commandBuffer->barrier(*barrier);
commandBuffer->transfer(*statisticsBuffer, *readback);

auto fence = commandBuffer->submit();
computeQueue.waitFor(fence);

Statistics statistics{};
readback->map(&statistics, sizeof(Statistics), 0, false);
```

If you do not want to block, keep one readback buffer per frame in flight and read the one whose fence has been passed, as reported by
`lastCompletedFence`. The result is then a few frames old, which is usually acceptable for statistics and similar data.

## Streaming in the background

To load resources while the application keeps running, submit uploads to a separate transfer queue and check their fence once per frame:

```cpp
if (m_streamingQueue->lastCompletedFence() >= m_pendingUpload.Fence)
    m_scene.activate(std::move(m_pendingUpload.Resources));
```

The check itself does not synchronize the GPU. The queue that renders the streamed resources still has to wait for the fence, but as it has
already been passed, the wait costs nothing.

## Common mistakes

**Missing barrier between two uses inside the same submission.** Commands start in order, but do not finish in order. A dispatch that reads
what the previous one wrote needs a barrier, even in the same command buffer. Across submissions on the same queue, no barrier is needed.

**Expecting submissions on one queue to overlap.** Each submission waits for the previous one on its queue. Work that should run in parallel
needs to go to different queues.

**Copying without the usage flags.** `transfer` throws, if the source was not created with `ResourceUsage::TransferSource` or the
destination not with `ResourceUsage::TransferDestination`.

**Transitioning from `Undefined` an image whose contents you need.** `Undefined` discards the contents. Use it only for images that are written
completely afterwards.

**Using `ShaderRead` for a resource bound as writable.** The access follows the binding. A `RWTexture2D` is accessed with `ShaderReadWrite`,
even when the shader only reads it. DirectX 12 reports this as an error, Vulkan does not.

**Waiting for a fence of the wrong queue.** Fence values only mean something on their own queue. Store fences together with their queue.

**Modifying a barrier after recording it.** `commandBuffer->barrier(*barrier)` records the barrier's current transitions. Transitions added
afterwards are only recorded the next time the barrier is executed.

**Combining a stage of `None` with an access other than `None`.** If nothing is waited for, nothing can be made visible. Use a real stage, or
`ResourceAccess::None`.

**Using `PipelineStage::None` in the middle of a submission.** `None` as before value is only correct if no earlier command *in the same
submission* accessed the resource. Otherwise, name the stages of that command.

**Overwriting per-frame data that is still in use.** Data written by the CPU every frame needs one copy per frame in flight. See [frames in
flight](render-passes-and-frames.md#frames-in-flight).

**Leaving an image in a special layout.** A system that receives an image expects it in `Common`. Return images to the default state when you
are done with them, including all mip levels and layers you transitioned individually.

**Waiting on the CPU to order GPU work.** `waitFor(fence)` stalls the application. To order work between queues, use `waitFor(queue, fence)`.

## Debugging synchronization

Synchronization errors are often invisible: the application works on one GPU and flickers on another, or only fails when the frame rate drops.
These tools help to find them early:

- **Enable the validation layers.** The Vulkan validation layers with *synchronization validation* enabled detect missing barriers and wrong
  access masks. The DirectX 12 debug layer, ideally with *GPU-based validation*, detects wrong layouts and access types. Both find different
  problems, so use both.
- **Name your resources.** Names passed to the factory appear in validation messages and in captures.
- **Use debug regions and markers.** `beginDebugRegion`, `endDebugRegion` and `setDebugMarker` on queues structure the
  captures of RenderDoc and PIX, which show every barrier with its stages, access and layouts.
- **Make it serial to confirm a suspicion.** If you suspect a missing barrier, temporarily replace the stages of the nearest barrier with
  `PipelineStage::All` and its access with `ResourceAccess::Common`, or split the submission in two at that point.
  Since submissions on a queue run one after another, a split acts like a full barrier. If the problem disappears, narrow
  the barrier down again until you find the missing dependency.

## Checklist

Before you submit work, ask yourself:

1. Does any command in this submission use a result of another queue? Then this queue waits for that queue's fence.
2. Does any command use a result of a previous command in the same submission? Then a barrier separates them, naming the right stages and
   access. Results of earlier submissions on the same queue need no barrier.
3. Does any image need a layout other than `Common`? Then a barrier transitions it before, and back to `Common` after its use.
4. Is any resource used for the first time? Then its first transition starts from `Undefined`.
5. Does the CPU need the result, or will it release or overwrite memory the GPU uses? Then it waits for the fence first.
6. Is this resource written every frame? Then it exists once per frame in flight.
