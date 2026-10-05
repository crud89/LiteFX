# Defragmentation

!!! abstract "Sample 16"

    [`Samples/Defragmentation`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Defragmentation) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md).

Creating and releasing resources over time leaves gaps in GPU memory. Eventually an allocation fails even though enough memory is free, just
not in one piece. *Defragmentation* moves resources so that the free space is contiguous again. The sample allocates and releases random
textures and buffers every frame, and defragments the memory on request.

## Preparing resources for moving

A resource that is moved is copied to a new location, which means it is read by a transfer. The engine asks each resource to prepare itself
for this through the `prepareMove` event, whose handler records the barrier that makes the resource readable for the copy:

```cpp
static inline void setupMoveHandlers(const SharedPtr<const IBuffer>& resource) {
    resource->prepareMove += [buffer = resource->weak_from_this()](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = buffer.lock();

        if (resource)
            e.barrier().transition(*resource, ResourceAccess::None, ResourceAccess::TransferRead);
    };
}
```

The barrier starts from `ResourceAccess::None`, because defragmentation runs on its own queue, where nothing has accessed the resource
before. That does not mean the resource is unused: other queues may still be reading it. Those accesses are synchronized between the queues
with fences, as described below, not with this barrier.

Images additionally need a target layout for the copy. The sample uses `Common`, which allows any kind of access, including the transfer
read:

```cpp
static inline void setupMoveHandlers(const SharedPtr<const IImage>& resource) {
    resource->prepareMove += [image = resource->weak_from_this()](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = image.lock();

        if (resource)
            e.barrier().transition(*resource, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::Common);
    };
}
```

The handlers are registered for every resource that may be moved:

```cpp
::setupMoveHandlers(vertexBuffer);
::setupMoveHandlers(indexBuffer);
::setupMoveHandlers(cameraBuffer);
::setupMoveHandlers(transformBuffer);
```

Capturing the resource as a weak pointer matters: the handler must not keep the resource alive, and it checks whether the resource still
exists before using it.

## Updating descriptor sets after a move

A moved resource lives at a new address, so descriptors that point to the old one become invalid. Only resources on the `Resource` heap are
moved; the transform buffer is on the dynamic heap, so its descriptor sets stay valid. The camera buffer, however, can be moved, and its
descriptor set has to be replaced when that happens.

The `moving` event tells the application that a resource is about to move. Since descriptor sets must not change while frames that use them
are still in flight, the handler only sets a flag:

```cpp
m_cameraBindings = cameraBindingLayout.allocate({ { .resource = *cameraBuffer } });
cameraBuffer->moving += [this](const void* /*sender*/, const IDeviceMemory::ResourceMovingEventArgs& /*e*/) noexcept { m_rebindCamera = true; };
```

The next frame replaces the descriptor set before binding it. The old set may still be used by earlier frames, so it cannot simply be
released. Instead, it is handed to the command buffer with `track`, which keeps it alive until the command buffer has been executed:

```cpp
if (m_rebindCamera) {
    auto& cameraBuffer = m_device->state().buffer("Camera");
    auto& layout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);

    commandBuffer->track(std::move(m_cameraBindings));
    m_cameraBindings = layout.allocate({ { .resource = cameraBuffer } });
    m_rebindCamera = false;
}

commandBuffer->bind({ m_cameraBindings.get(), &transformBindings });
```

This is also why the sample keeps the camera's descriptor set in a member instead of the device state: transferring ownership to a command
buffer is not possible for resources owned by the device state.

## Running the defragmentation

Defragmentation is a sequence of *passes*, so that the work can be spread over several frames instead of stalling the application. It is
started once, on the queue that performs the copies:

```cpp
if (!isDefragmenting) {
    m_device->factory().beginDefragmentation(transferQueue, DefragmentationStrategy::Balanced, 0u, 20u);
    isDefragmenting = true;
}
```

The strategy decides how aggressively resources are moved, and the last arguments limit how much work a pass may do. `Balanced` is a
reasonable default; other strategies trade more moves for tighter packing.

Each pass copies resources and returns the fence of that work on the transfer queue. Rendering has to wait for it, so it uses the resources
at their new locations:

```cpp
auto transferFence = m_transferFence;

if (!isInDefragmentationPass) {
    transferFence = std::max(transferFence, m_device->factory().beginDefragmentationPass());
    isInDefragmentationPass = true;

    // Remember the fence that marks the last usage of the resources that should be released.
    defragmentationFence = renderFence;
}

renderQueue.waitFor(transferQueue, transferFence);
```

Ending a pass releases the old copies of the moved resources. Frames that were already in flight when the pass started may still use them,
so the pass is only ended once the render queue has finished those frames. The fence of the last frame before the pass marks that point:

```cpp
renderFence = renderPass.end();

// End defragmentation if the resources that should be released aren't used by any frames in flight anymore.
if (renderQueue.lastCompletedFence() >= defragmentationFence) {
    isDefragmenting = !m_device->factory().endDefragmentationPass();
    isInDefragmentationPass = false;
}
```

`endDefragmentationPass` returns whether the defragmentation is complete. Until then, the next frame starts another pass.

## Watching it work

The sample creates a few random textures and buffers per frame and releases them after a random lifetime, which fragments the memory quickly:

```cpp
auto& allocation = allocations.emplace_back(m_device->factory().createTexture(Format::R8G8B8A8_SRGB, { resolutionDice(rng), resolutionDice(rng), 1u }), frameDice(rng));

std::ranges::for_each(allocations, [](auto& allocation) { allocation.lifetime--; });
std::erase_if(allocations, [](const auto& allocation) { return allocation.lifetime == 0; });
```

The window title shows the current memory statistics, so the effect of a defragmentation run is visible directly.

!!! note "When this is worth it"

    Defragmentation costs copies and synchronization, so it is not something to run every frame. It pays off in applications that stream
    resources continuously, for example when loading and unloading parts of a large scene.
