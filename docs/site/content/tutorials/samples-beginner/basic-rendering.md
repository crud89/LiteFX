# Basic rendering

!!! abstract "Sample 01"

    [`Samples/BasicRendering`](https://github.com/crud89/LiteFX/tree/main/src/Samples/BasicRendering) · builds on the
    [first triangle](../first-triangle/index.md) tutorial.

This sample extends the triangle into a small 3D renderer: a rotating object, viewed through a camera, with a depth buffer and one set of
per-frame resources for each frame in flight. It is the baseline for all following tutorials, which only describe what they change about it.

## Vertices and depth

The engine provides a `Vertex` structure (`litefx/gfx/vertex.hpp`) with a position, a color, a normal and texture coordinates. The sample
draws a tetrahedron from four vertices and twelve indices.

Since the object is three-dimensional, the render pass needs a second render target for the depth values:

```cpp
SharedPtr<RenderPass> renderPass = device->buildRenderPass("Opaque")
    .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
    .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });
```

The depth target is cleared to `1.f`, the largest possible depth value, so that every object drawn afterwards passes the depth test. Frame
buffers allocate an image for it like for any other render target, as `addImages` adds all render targets of the render pass.

## Passing data to shaders

The shaders need two matrices: the camera's view-projection matrix, which is the same for all objects, and the model matrix of the object,
which changes every frame. Both are stored in buffers that the shaders access through *descriptors*. A *descriptor set* groups descriptors
that are bound together, and the sample uses one set per update frequency:

```cpp
enum DescriptorSets : UInt32
{
    Constant = 0, // All buffers that are immutable.
    PerFrame = 1, // All buffers that are updated each frame.
};
```

In the shader, the sets are the *spaces* of the bindings:

```hlsl
ConstantBuffer<CameraData> camera       : register(b0, space0);
ConstantBuffer<TransformData> transform : register(b0, space1);
```

Grouping by update frequency keeps the number of descriptor updates low: constant data is written once, per-frame data once per frame.

The pipeline layout is derived from the shader program, as in the triangle tutorial, so the descriptor set layouts come from the shaders:

```cpp
UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
    .inputAssembler(inputAssembler)
    .rasterizer(device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::BackFaces)
        .cullOrder(CullOrder::ClockWise)
        .lineWidth(1.f))
    .layout(shaderProgram->reflectPipelineLayout())
    .shaderProgram(shaderProgram);
```

## Buffers and descriptor sets

The camera buffer is written once, so it lives in GPU memory (`ResourceHeap::Resource`) and is filled with a transfer command. The transform
buffer is written every frame, so it is allocated on a heap the CPU can write to, with one element per frame in flight:

```cpp
auto preferredDynamicHeap = m_device->factory().supportsResizableBaseAddressRegister() ? ResourceHeap::GPUUpload : ResourceHeap::Dynamic;

auto& cameraBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBindingLayout, 0, ResourceHeap::Resource);
auto cameraBindings = cameraBindingLayout.allocate({ { .resource = *cameraBuffer } });
this->updateCamera(*commandBuffer, *cameraBuffer);

auto& transformBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
auto transformBuffer = m_device->factory().createBuffer("Transform", transformBindingLayout, 0, preferredDynamicHeap, 3);
auto transformBindings = transformBindingLayout.allocate(3, {
    { { .resource = *transformBuffer, .firstElement = 0, .elements = 1 } },
    { { .resource = *transformBuffer, .firstElement = 1, .elements = 1 } },
    { { .resource = *transformBuffer, .firstElement = 2, .elements = 1 } }
}) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();
```

`supportsResizableBaseAddressRegister` checks whether the GPU exposes all of its memory to the CPU (*ReBAR*). If it does, the buffer can be
written directly into GPU memory, which saves a copy.

Each of the three descriptor sets points to one element of the transform buffer. This is what makes *frames in flight* work: while the GPU
still draws frame 0, the CPU can already write the transform of frame 1, because they use different memory.

## Drawing a frame

The draw loop looks up the resources of the current back buffer, updates the model matrix, and binds both descriptor sets:

```cpp
auto backBuffer = m_device->swapChain().swapBackBuffer();
auto& transformBindings = m_device->state().descriptorSet(std::format("Transform Bindings {0}", backBuffer));

renderPass.commandQueue().waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);
renderPass.begin(frameBuffer);
auto commandBuffer = renderPass.commandBuffer(0);
commandBuffer->use(geometryPipeline);
commandBuffer->setViewports(m_viewport.get());
commandBuffer->setScissors(m_scissor.get());

transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
transformBuffer.map(static_cast<const void*>(&transform), sizeof(transform), backBuffer);

commandBuffer->bind({ &cameraBindings, &transformBindings });
commandBuffer->bind(vertexBuffer);
commandBuffer->bind(indexBuffer);
commandBuffer->drawIndexed(indexBuffer.elements());
renderPass.end();
```

Two details are worth noting:

- **`waitFor`** makes the render pass's queue wait for the transfer that uploaded the vertices, indices and camera. Queues run
  independently, so this synchronization is explicit.
- **`map` with the back buffer index** writes into the element of the transform buffer that belongs to the current frame.

## Switching backends at runtime

The sample registers both backends and lets you switch between them while it runs (++f9++ for Vulkan, ++f10++ for DirectX 12). The engine
stops the active backend, which releases its device and with it all resources in its device state, and runs the start handler of the other
one. Because all resources are created in that handler, the application only has to request the switch:

```cpp
void SampleApp::keyDown(int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        this->startBackend<VulkanBackend>();
    else if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        this->startBackend<DirectX12Backend>();
}
```

This is the main reason the samples put their whole setup into a backend start handler, and the following tutorials keep that structure.
