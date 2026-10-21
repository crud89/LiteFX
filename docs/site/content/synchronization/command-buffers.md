# Command buffers

A *command buffer* stores commands until a queue executes them. Commands are recorded on the CPU, which is cheap and can happen on any thread,
and only reach the GPU when the command buffer is submitted.

## Recording and submitting

Command buffers are created by the queue they are submitted to:

```cpp
auto commandBuffer = computeQueue.createCommandBuffer(true);   // `true` begins recording right away.

commandBuffer->use(pipeline);
commandBuffer->bind(*bindings);
commandBuffer->dispatch({ 64, 64, 1 });

UInt64 fence = commandBuffer->submit();
```

A command buffer goes through these states:

1. **Initial.** Created with `createCommandBuffer(false)`. Call `begin` to start recording.
2. **Recording.** Commands can be added. `createCommandBuffer(true)` returns a command buffer in this state.
3. **Executable.** `end` closes the command buffer. You rarely call it yourself, because submitting a recording command buffer closes it
   implicitly.
4. **Pending.** After `submit`, the queue owns the command buffer until the GPU has executed it, that is, until the queue has passed the
   returned fence.

Inside a submission, i.e. within one command buffer and across the command buffers passed to the same `submit` call, commands *start* in
the order they were recorded. They do not necessarily *finish* in that order, and later commands may run in parallel with earlier ones.
Whenever a command depends on the result of a previous one in the same submission, a [barrier](barriers.md) has to order them, even if both
are in the same command buffer.

Between submissions, no barrier is needed: a queue only starts a submission once the previous one has [finished](queues-and-fences.md#submissions-run-one-after-another).
Splitting work into several submissions is therefore a valid, if slower, alternative to a barrier.

## Reusing command buffers

There are two ways to deal with command buffers that you need repeatedly, for example in every frame:

- **Create a new one for each submission.** This is the simplest approach. The queue keeps a reference to the submitted command buffer until
  it has executed, so you can let go of your own reference right after submitting.
- **Record the same one again.** Call `begin` on it once it is no longer pending, i.e. after the queue has passed the fence of its last
  submission. Recording a pending command buffer is undefined behavior. With frames in flight, keep one command buffer per frame and check
  the fence before you reuse it.

## Secondary command buffers

`createCommandBuffer(false, true)` creates a *secondary* command buffer, also called a *bundle*. It cannot be submitted to a queue. Instead, a
primary command buffer executes it:

```cpp
primaryCommandBuffer->execute(secondaryCommandBuffer);
```

Secondary command buffers do not introduce any synchronization of their own. Their commands behave exactly as if they were recorded into the
primary command buffer at the point of the `execute` call.

Render passes use secondary command buffers to let several threads record draw calls in parallel. `IRenderPass::commandBuffer(index)` returns
them, and `IRenderPass::end` executes all of them in the order of their index. See the [multithreading
sample](../samples-beginner/multithreading.md) for an example.

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
- **`track`** lets you hand over any buffer, image, sampler or descriptor set yourself, for example a descriptor set you allocated only for one
  dispatch:

```cpp
commandBuffer->bind(*bindings);
commandBuffer->dispatch({ 16, 16, 1 });
commandBuffer->track(std::move(bindings));
```

The queue releases tracked resources once the command buffer has been executed. This happens lazily, the next time the CPU waits for a fence
on that queue at or past the submission, or when the command buffer is recorded again. In applications that never wait on a queue, tracked
resources therefore live longer than strictly necessary, but never shorter.

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
