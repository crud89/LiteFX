# Queues and fences

A *command queue* executes command buffers on the GPU, in the order they were submitted. Queues run independently of each other and of the
CPU, so the only way to know where a queue is at, is a *fence*.

## Submissions run one after another

A queue starts a submission only after the previous submission on the same queue has finished, and all memory it wrote is visible. You
never need a barrier to synchronize with an *earlier submission* on the same queue: the first barrier in a submission can use
`PipelineStage::None` and `ResourceAccess::None` as before values, even for resources that the previous submission wrote. Barriers are only
needed between commands *within* a submission, as described on the [barriers page](barriers.md).

This is the same in both backends. DirectX 12 guarantees it by specification, for each `ExecuteCommandLists` call. Vulkan would allow
submissions to overlap, so the Vulkan backend enforces the order: each submission waits for the previous submission on the same queue. That
way, the same barriers are correct for both backends (see [backend differences](index.md#backend-differences)).

The flip side is that work on a single queue never overlaps across submissions. Use several queues if you want work to run in parallel.

## Queue types

Every queue has a `QueueType`, which defines the work it can execute:

| Type | Can execute | Typical use |
|---|---|---|
| `QueueType::Graphics` | Draws, dispatches, ray tracing, copies | Render passes, everything that needs to happen in a frame |
| `QueueType::Compute` | Dispatches, ray tracing, copies | Asynchronous compute work, post-processing, simulation |
| `QueueType::Transfer` | Copies | Uploading resources, readback, defragmentation |

Each type includes the capabilities of the types below it, so a graphics queue can do everything. The device creates one *default queue* of
each type, which you get by calling `defaultQueue`:

```cpp
auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
auto& computeQueue  = m_device->defaultQueue(QueueType::Compute);
auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
```

If you need more queues, for example to stream resources at a lower priority, create them with `createQueue`:

```cpp
SharedPtr<const ICommandQueue> streamingQueue = m_device->createQueue(QueueType::Transfer, QueuePriority::Normal);
```

`QueuePriority::Realtime` is reserved for the default queues; use `Normal` or `High` for your own.

!!! note "Queues are not necessarily parallel"
    A queue in LiteFX does not guarantee a separate hardware queue. If the GPU offers fewer queues of a type than you request, backends may
    map several queues to the same hardware queue. Your synchronization has to be correct either way, but you should not expect two queues to
    always run in parallel.

### Which queue to use

Using a dedicated queue lets work overlap: a transfer queue can upload the next level's textures while the graphics queue renders. It comes
at a price, though, because every hand-over between queues needs a fence. As a rule of thumb:

- **Upload large, long-lived resources on the transfer queue.** The hand-over happens once, and the copy does not block rendering.
- **Update small, per-frame data on the graphics queue,** or map it directly from the CPU. Synchronizing two queues every frame often costs
  more than the copy itself.
- **Move compute work to the compute queue,** if it can run while the graphics queue does something else, for example a simulation for the
  next frame. Separate submissions to the *same* queue never run in parallel. Post-processing that the very same frame waits for gains little from a separate queue.
- **Use the graphics queue for work that needs graphics-only operations,** such as generating mip maps with the [blitter](patterns.md#textures-with-mip-maps)
  in the Vulkan backend.

## Fences

Each queue owns a timeline: a 64-bit counter that only grows. Every submission increments it and returns the new value, the *fence* of that
submission:

```cpp
UInt64 fence = transferQueue.submit(commandBuffer);   // or: commandBuffer->submit();
```

Once the GPU has finished the submission, and every submission before it on the same queue, the queue *passes* the fence. Fence values belong
to their queue: fence `12` on the transfer queue has nothing to do with fence `12` on the graphics queue, so always keep a fence together with
the queue it came from.

You get fences from these calls:

- **`ICommandQueue::submit` and `ICommandBuffer::submit`** return the fence of the submission. Submitting several command buffers at once
  returns one fence for all of them.
- **`IRenderPass::end`** submits the render pass's command buffers and returns their fence on the render pass's queue,
  `renderPass.commandQueue()`.
- **`IGraphicsFactory::beginDefragmentationPass`** returns the fence of the copies of a defragmentation pass.
- **`ICommandQueue::currentFence`** returns the fence of the latest submission, if you did not store it.

### Waiting on the CPU

`waitFor` with only a fence blocks the calling thread until the queue has passed it:

```cpp
auto fence = commandBuffer->submit();
transferQueue.waitFor(fence);
// All work up to `fence` has finished. Its results can be read, and its resources reused.
```

Use it when the CPU needs a result, for example after a readback, or when you are about to release or overwrite memory the GPU might still
use. Do not use it to order work between queues; that would stall the CPU, and with it the submission of all other work.

To check without blocking, compare against `lastCompletedFence`:

```cpp
if (transferQueue.lastCompletedFence() >= m_streamingFence)
{
    // The streamed resources are ready.
}
```

To wait for *all* queues, for example before shutting down or before resizing the swap chain, call `IGraphicsDevice::wait`. It blocks until
the device is idle.

### Waiting on another queue

`waitFor` with a queue and a fence makes this queue wait until the other queue has passed the fence. It returns immediately and does not block
the CPU; the wait happens on the GPU:

```cpp
// The upload has been submitted to the transfer queue...
auto uploadFence = transferQueue.submit(uploadCommands);

// ...and the graphics queue must not start rendering until it has finished.
graphicsQueue.waitFor(transferQueue, uploadFence);
graphicsQueue.submit(renderCommands);
```

The wait is attached to the next submission to this queue. As submissions on a queue run one after another, every later submission waits as
well. So call it before you submit the work that depends on the other queue, and before `IRenderPass::end` at the latest if the dependent
work is a render pass, which submits when it ends (unless it records into a [shared command
buffer](render-passes-and-frames.md#sharing-a-command-buffer-between-render-passes), in which case the wait applies to the submission of that
command buffer). Calling it before `begin` makes the intent clearer:

```cpp
renderPass.commandQueue().waitFor(transferQueue, m_transferFence);
renderPass.begin(frameBuffer);
```

A cross-queue wait also makes all memory written before the fence visible to the waiting queue. You do not need an additional barrier for
that, but you may still need one to bring an image into the layout the waiting queue needs; see [cross-queue
access](barriers.md#accessing-resources-from-multiple-queues).

!!! warning "Avoid waits that can never complete"
    A queue that waits for a fence nobody submits blocks forever, and so does every queue that waits for it. Only wait for fences that have
    already been returned by a `submit` call, and never let two queues wait for each other. `waitFor` logs a warning, if the fence is beyond
    the other queue's `currentFence`, i.e. if it belongs to a submission that has not happened yet.

A fence of `0` is always satisfied, so waiting for it does nothing. This is convenient for fences that are only set under some conditions,
for example an upload that may not have happened yet: initialize them with `0`, and wait for them unconditionally.

## Queue events and debug regions

Every queue raises a `submitting` event before and a `submitted` event after a submission. The latter contains the fence, which makes it a
convenient place to collect fences in systems that do not submit work themselves.

To make captures in tools such as RenderDoc or PIX easier to read, group submissions with debug regions and add markers:

```cpp
computeQueue.beginDebugRegion("Post-Processing");
// ... submit work ...
computeQueue.setDebugMarker("Bloom");
computeQueue.endDebugRegion();
```

## Thread safety

Submitting to a queue and waiting on it is thread-safe: several threads may submit to the same queue. The order in which their submissions
reach the GPU is the order in which the `submit` calls happen, so if two submissions from different threads depend on each other, you need to
order the calls yourself. Recording into command buffers is covered on the [next page](command-buffers.md#threads).
