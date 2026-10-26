# Command buffers

A *command buffer* stores commands until a queue executes them. Commands are recorded on the CPU, which is cheap and can happen on any thread,
and only reach the GPU when the command buffer is submitted.

## Recording and submitting

Command buffers are created by the queue they are submitted to:

```cpp
auto commandBuffer = computeQueue.createCommandBuffer();

commandBuffer->use(pipeline);
commandBuffer->bind(*bindings);
commandBuffer->dispatch({ 64, 64, 1 });

UInt64 fence = commandBuffer->submit();
```

A command buffer is a one-time object, which goes through these states:

1. **Recording.** A new command buffer is ready to receive commands right away. `ICommandBuffer::recording` returns `true` in this state.
2. **Pending.** `submit` ends the recording and hands the command buffer to the queue, which owns it until the GPU has executed it, that is,
   until the queue has passed the returned fence.
3. **Released.** Once the command buffer has been executed and no one holds a reference to it anymore, it is released.

There is no way back into the recording state. After a command buffer has been submitted, recording further commands into it throws an
exception, and so does submitting it a second time.

Inside a submission, i.e. within one command buffer and across the command buffers passed to the same `submit` call, commands *start* in
the order they were recorded. They do not necessarily *finish* in that order, and later commands may run in parallel with earlier ones.
Whenever a command depends on the result of a previous one in the same submission, a [barrier](barriers.md) has to order them, even if both
are in the same command buffer.

Between submissions, no barrier is needed: a queue only starts a submission once the previous one has [finished](queues-and-fences.md#submissions-run-one-after-another).
Splitting work into several submissions is therefore a valid, if slower, alternative to a barrier.

## One command buffer per submission

Since command buffers are used once, work that repeats, for example in every frame, creates a new command buffer for each submission. This is
cheap: the engine recycles the memory of executed command buffers internally, so creating one rarely allocates anything.

The queue keeps a reference to a submitted command buffer until it has been executed, so you can let go of your own reference right after
submitting. There is also no need to keep one command buffer per frame in flight, or to check fences before recording.

## Secondary command buffers

`createCommandBuffer(true)` creates a *secondary* command buffer, also called a *bundle*. It cannot be submitted to a queue. Instead, a primary
command buffer executes it:

```cpp
primaryCommandBuffer->execute(secondaryCommandBuffer);
```

Executing a secondary command buffer ends its recording. Unlike a primary command buffer, it can then be executed again, by the same or by
other primary command buffers, which is what makes bundles useful for small workloads that recur often.

Secondary command buffers do not introduce any synchronization of their own. Their commands behave exactly as if they were recorded into the
primary command buffer at the point of the `execute` call.

!!! warning "Create secondary command buffers on graphics queues"
    Vulkan allows secondary command buffers on any queue, but DirectX 12 only on graphics queues. For code that works with both backends,
    only create them from a `QueueType::Graphics` queue.

Render passes use secondary command buffers to let several threads record draw calls in parallel. `IRenderPass::commandBuffer(index)` returns
them, and `IRenderPass::end` executes all of them in the order of their index. See the [multithreading
sample](../../tutorials/samples-beginner/multithreading.md) for an example, and [render passes and
frames](render-passes-and-frames.md#sharing-a-command-buffer-between-render-passes) for how a render pass can record into a command buffer
of yours.

## Threads

A command buffer must only be recorded by one thread at a time. Different command buffers can be recorded on different threads in parallel,
which is the main reason they exist. Submitting is thread-safe, as described on the [previous page](queues-and-fences.md#thread-safety).

## Resource lifetime

The GPU reads resources long after you recorded the command that uses them. If you destroy a resource, or overwrite memory that the GPU still
reads, the result is undefined. LiteFX helps you here: a command buffer can keep resources alive until it has been executed.

```cpp
auto stagingBuffer = factory.createBuffer(BufferType::Other, ResourceHeap::Staging, size);
stagingBuffer->map(data, size);

commandBuffer->transfer(stagingBuffer, *texture);   // Takes a `SharedPtr`, so the command buffer tracks the staging buffer.
commandBuffer->submit();
// `stagingBuffer` can go out of scope here.
```

- **`transfer` overloads that take a `SharedPtr`** to the source track it automatically.
- **`transfer` overloads that take raw memory** create a staging buffer internally and track it the same way.
- **`track`** lets you hand over any buffer, image, sampler, descriptor set or descriptor range yourself, for example a descriptor set you
  allocated only for one dispatch:

```cpp
commandBuffer->bind(*bindings);
commandBuffer->dispatch({ 16, 16, 1 });
commandBuffer->track(std::move(bindings));
```

The queue releases tracked resources once the command buffer has been executed. This happens lazily: the next time something is submitted to
the same queue, or the CPU waits for a fence on it, the queue releases the resources of all submissions that have been executed by then.
Tracked resources therefore live longer than strictly necessary, but never shorter. `ICommandBuffer::trackedResources` shows what a command
buffer currently keeps alive, which can be useful when debugging resource lifetimes.

!!! tip
    Tracking only delays destruction. It does not order any work: if a later submission on another queue uses the same resource, it still has
    to [wait for the fence](queues-and-fences.md#waiting-on-another-queue).

### Usage flags for copies

`transfer` checks the usage of the resources it copies between: the source must have been created with `ResourceUsage::TransferSource`, and
the destination with `ResourceUsage::TransferDestination`. Otherwise it throws an `InvalidArgumentException`. DirectX 12 itself does not
require these flags, but Vulkan does, so LiteFX enforces them on both backends. `ResourceUsage::Default` contains both, and frame buffer
images are always created with `TransferSource`.

### Memory the CPU writes

Buffers in the `ResourceHeap::Dynamic` heap are written by the CPU with `map`, without any command. The GPU sees the new data in all
submissions that are submitted *after* the `map` call, so no barrier is needed. The danger is the opposite direction: overwriting data that a
previous, still running submission reads. Avoid it by giving each frame in flight its own element of the buffer, as described in [frames in
flight](render-passes-and-frames.md#frames-in-flight).

### Memory the CPU reads

Buffers in the `ResourceHeap::Readback` heap receive data from the GPU through a `transfer`. Before you read them with `map`, wait for the fence
of the submission that contains the transfer on the CPU, as shown in the [readback pattern](patterns.md#reading-results-on-the-cpu).
