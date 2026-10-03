# Defragmentation

!!! abstract "Sample 16"

    [`Samples/Defragmentation`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Defragmentation) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md).

Creating and releasing resources over time leaves gaps in GPU memory. Eventually an allocation fails even though enough memory is free, just
not in one piece. *Defragmentation* moves resources so that the free space is contiguous again. The sample allocates and releases random
textures and buffers every frame, and defragments the memory on request.

## Preparing resources for moving

A resource that is moved is copied to a new location, so the GPU must not be using it at that moment. The engine asks each resource to
prepare itself, through the `prepareMove` event. The handler records the barrier that makes the resource readable for the copy:

```cpp
static inline void setupPrepareMoveHandler(const SharedPtr<const IBuffer>& resource, ResourceAccess beforeAccess) {
    resource->prepareMove += [buffer = resource->weak_from_this(), beforeAccess](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = buffer.lock();

        if (resource)
            e.barrier().transition(*resource, beforeAccess, ResourceAccess::TransferRead);
    };
}
```

Images additionally need their layout, since the copy expects a different one than rendering:

```cpp
static inline void setupPrepareMoveHandler(const SharedPtr<const IImage>& resource, ResourceAccess beforeAccess, ImageLayout layout) {
    resource->prepareMove += [image = resource->weak_from_this(), beforeAccess, layout](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = image.lock();

        if (resource)
            e.barrier().transition(*resource, beforeAccess, ResourceAccess::TransferRead, layout);
    };
}
```

The handler is registered for every resource that may be moved, with the access it is used for:

```cpp
::setupPrepareMoveHandler(vertexBuffer, ResourceAccess::VertexBuffer);
::setupPrepareMoveHandler(indexBuffer, ResourceAccess::IndexBuffer);
::setupPrepareMoveHandler(cameraBuffer, ResourceAccess::TransferWrite | ResourceAccess::ShaderRead);
::setupPrepareMoveHandler(transformBuffer, ResourceAccess::ShaderRead);
```

Capturing the resource as a weak pointer matters: the handler must not keep the resource alive, and it checks whether the resource still
exists before using it.

## Running the defragmentation

Defragmentation runs on a queue and works in passes, so that it can be spread over several frames instead of stalling the application:

```cpp
if (!isDefragmenting)
    m_device->factory().beginDefragmentation(transferQueue, DefragmentationStrategy::Balanced, 0u, 10u);
```

The strategy decides how aggressively resources are moved, and the last arguments limit how much work a pass may do. `Balanced` is a
reasonable default; other strategies trade more moves for tighter packing.

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
