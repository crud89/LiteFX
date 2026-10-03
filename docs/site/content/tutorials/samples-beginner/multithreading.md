# Multithreading

!!! abstract "Sample 07"

    [`Samples/Multithreading`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Multithreading) · builds on
    [basic rendering](basic-rendering.md).

Recording commands costs CPU time, and with many objects it can become the bottleneck. Command buffers can be recorded on several threads in
parallel and submitted together. The sample draws nine objects, each recorded by its own worker thread.

## Several command buffers per render pass

A render pass can provide more than one command buffer, which is decided when it is created:

```cpp
constexpr UInt32 NUM_WORKERS = 9;

SharedPtr<RenderPass> renderPass = device->buildRenderPass("Opaque", NUM_WORKERS)
    .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
    .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });
```

Each worker then records into its own buffer, which it gets by index:

```cpp
auto commandBuffer = renderPass->commandBuffer(index);
```

The render pass submits all of its command buffers in order when it ends, so the result is the same as if one thread had recorded them.

## Resources per thread

The important rule is that **no two threads may write to the same resource**. In the basic rendering sample there is one transform buffer
with one element per frame in flight. Here, each worker needs its own:

```cpp
Array<SharedPtr<IBuffer>> transformBuffers(NUM_WORKERS);
std::ranges::generate(transformBuffers, [&, i = 0]() mutable {
    return m_device->factory().createBuffer(std::format("Transform {0}", i++), transformBindingLayout, 0, ResourceHeap::Dynamic, 3);
});
```

The same applies to descriptor sets: a set that a thread writes to must not be shared. The sample therefore allocates one set per worker and
frame in flight, which it fills with a generator:

```cpp
auto transformBindings = transformBindingLayout.allocate(3 * NUM_WORKERS, [transformBuffers](UInt32 set) -> Generator<DescriptorBinding> {
    co_yield { .binding = 0, .resource = *transformBuffers[set % NUM_WORKERS], .firstElement = set / NUM_WORKERS, .elements = 1 };
}) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();
```

Each set points to the buffer of its worker and the element of its frame, so a worker never touches another's memory.

## Recording in parallel

Drawing one object is an ordinary sequence of commands, with the worker index selecting the resources:

```cpp
void SampleApp::drawObject(const IRenderPass* renderPass, int index, int backBuffer, float time)
{
    auto& transformBuffer = m_device->state().buffer(std::format("Transform {0}", index));
    auto& transformBindings = m_device->state().descriptorSet(std::format("Transform Bindings {0}", backBuffer * NUM_WORKERS + index));
    auto commandBuffer = renderPass->commandBuffer(index);

    commandBuffer->use(...);
    transform[index].World = glm::translate(glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)), translations[index]);
    transformBuffer.map(static_cast<const void*>(&transform[index]), sizeof(TransformBuffer), backBuffer);

    commandBuffer->bind({ &cameraBindings, &transformBindings });
    commandBuffer->bind(vertexBuffer);
    commandBuffer->bind(indexBuffer);
    commandBuffer->drawIndexed(indexBuffer.elements());
}
```

The frame then starts the workers, waits for them, and ends the render pass:

```cpp
renderPass.begin(frameBuffer);

std::ranges::generate(m_workers, [this, &backBuffer, &time, &renderPass, i = 0]() mutable {
    return std::async(std::launch::async, &SampleApp::drawObject, this, &renderPass, i++, backBuffer, time);
});

std::ranges::for_each(m_workers, [](std::future<void>& future) { future.wait(); });
renderPass.end();
```

Everything outside the command buffers stays on the main thread: beginning and ending the render pass, swapping the back buffer and
presenting.

!!! warning "What is safe to share"

    Reading from a resource on several threads is fine, so vertex buffer, index buffer and the camera's descriptor set are shared. Only
    writes need their own resource per thread.
