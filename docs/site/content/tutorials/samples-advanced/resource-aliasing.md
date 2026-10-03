# Resource aliasing

!!! abstract "Sample 17"

    [`Samples/ResourceAliasing`](https://github.com/crud89/LiteFX/tree/main/src/Samples/ResourceAliasing) · builds on
    [render passes](render-passes.md).

Render targets are often *transient*: a pass writes them, a later pass reads them, and after that they are no longer needed. If the lifetimes
of two such images do not overlap, they can share the same memory. This is called *aliasing*, and it saves exactly the memory the second
image would have needed. The sample is functionally the same as the render passes sample, but places two of its images in one memory region.

## Which images can be aliased

The sample uses two render passes:

- **First pass:** writes `Color` and `Depth`.
- **Second pass:** writes `Post Color` and reads `Color`.

`Depth` is finished when the first pass ends, and `Post Color` is only written by the second one. Their lifetimes do not overlap, so the two
can share memory. `Color` cannot, since it outlives the first pass.

## Allocating aliased resources

Aliased resources are allocated together, so the engine can lay them out in one memory region. The allocation is described first, then
checked and performed:

```cpp
auto resourceInfos = std::array {
    ResourceAllocationInfo(ResourceAllocationInfo::ImageInfo { .Format = Format::D32_SFLOAT, .Size = renderArea }, ResourceUsage::FrameBufferImage, "Depth"),
    ResourceAllocationInfo(ResourceAllocationInfo::ImageInfo { .Format = Format::B8G8R8A8_UNORM, .Size = renderArea }, ResourceUsage::FrameBufferImage, "Post Color")
};

auto canAlias = m_device->factory().canAlias(resourceInfos);

if (!canAlias)
    LITEFX_WARNING("SampleApp"sv, "Render targets can't be aliased and will be created as non-overlapping images.");

auto resources = m_device->factory().allocate(resourceInfos, AllocationBehavior::Default, canAlias)
    | std::views::take(2)
    | std::ranges::to<std::vector>();

m_depthBuffer = resources[0].image<const IImage>();
m_postColorBuffer = resources[1].image<const IImage>();
```

`canAlias` asks whether the combination is possible at all. Not every driver allows arbitrary resources to overlap, so the sample falls back
to separate images and only logs a warning.

## Handing the images to the frame buffer

Frame buffers normally allocate their images themselves. To use the aliased ones instead, the frame buffer is created with an allocation
callback, which it asks for each image:

```cpp
auto frameBuffer = device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea(),
    std::bind_front(&SampleApp::frameBufferAllocationCallback<TRenderBackend>, &app));
```

The callback returns the aliased images by name, and `nullptr` for everything else, which lets the frame buffer allocate as usual:

```cpp
SharedPtr<const typename TRenderBackend::image_type> frameBufferAllocationCallback(Optional<UInt64> renderTargetId, const Size2d&, ResourceUsage, Format, MultiSamplingLevel, const String& name) const {
    switch (renderTargetId.value_or(hash(name)))
    {
    case hash("Depth"):
        return std::dynamic_pointer_cast<const typename TRenderBackend::image_type>(m_depthBuffer);
    case hash("Post Color"):
        return std::dynamic_pointer_cast<const typename TRenderBackend::image_type>(m_postColorBuffer);
    default:
        // Let the frame buffer perform the allocation using the default behavior.
        return nullptr;
    }
}
```

Names matter here: mapping render targets to images works by name, so the names in the allocation, in the callback and in the render targets
have to match.

## Resizing

When the window changes size, the images have to be reallocated, and aliased ones only together. The frame buffer announces a resize before
it happens, which the sample uses to allocate the new pair:

```cpp
frameBuffer->resizing += std::bind_front(&SampleApp::onFrameBufferResizing, &app);
```

The handler calls `initAliasingBuffers` with the new size, so the callback can hand out the new images during the resize.

!!! warning "Overlapping memory"

    Aliased resources share their memory, so writing one invalidates the other. The engine inserts the necessary barriers between the
    passes, but the application has to be sure the lifetimes really do not overlap. If they do, a pass will read data that another one has
    overwritten.
