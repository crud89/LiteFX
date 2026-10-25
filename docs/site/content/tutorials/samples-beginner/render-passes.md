# Render passes

!!! abstract "Sample 02"

    [`Samples/RenderPasses`](https://github.com/crud89/LiteFX/tree/main/src/Samples/RenderPasses) · builds on
    [basic rendering](basic-rendering.md).

Most renderers draw a frame in several steps: geometry into a G-buffer, lighting on top of it, post-processing, user interface. Each step is
a *render pass*, and later passes read what earlier ones wrote. This sample uses three passes:

1. **First pass:** draws the geometry into a color target and a depth target.
2. **Second pass:** draws a screen-filling quad that samples the result of the first pass.
3. **Third pass:** draws the geometry again on top of it, into the swap chain image.

## Images in the frame buffer

Passes share images through the frame buffer. Instead of letting each render pass allocate its own images, the sample adds them to the frame
buffer explicitly and maps the render targets of the passes onto them afterwards:

```cpp
auto frameBuffer = device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea());
frameBuffer->addImage("G-Buffer Color", Format::B8G8R8A8_UNORM); // Written in first render pass, read in second render pass.
frameBuffer->addImage("Color", Format::B8G8R8A8_UNORM);          // Written in second and third render pass.
frameBuffer->addImage("Depth", Format::D32_SFLOAT);              // Written first, read in third render pass for depth test.
```

Render targets are mapped to images by name, so two passes that name a target identically write into the same image:

```cpp
std::ranges::for_each(frameBuffers, [&firstPass, &thirdPass](auto& frameBuffer) {
    frameBuffer->mapRenderTargets(firstPass->renderTargets());
    frameBuffer->mapRenderTargets(thirdPass->renderTargets());
});
```

## Input attachments

The second pass reads the color target of the first one. Such a *input attachment* is declared on the render pass, together with the sampler
the shader uses to read it:

```cpp
SharedPtr<RenderPass> secondPass = device->buildRenderPass("Second Pass")
    .inputAttachmentSamplerBinding(DescriptorBindingPoint { .Register = 0, .Space = 1 })
    .inputAttachment(DescriptorBindingPoint { .Register = 0, .Space = 0 }, *firstPass, 0)  // Map color attachment from geometry pass render target 0.
    .renderTarget("Color", RenderTargetType::Color, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });
```

The binding points tell the engine where the shader expects the image and the sampler. The engine binds them when the pass begins, and
inserts the barriers that make the image readable, so the second pass's shader only has to declare them:

```hlsl
Texture2D gBufferColor : register(t0, space0);
SamplerState gBufferSampler : register(s0, space1);
```

## Targets with explicit locations

When a pass writes into a subset of the frame buffer's images, the render targets need explicit locations, i.e. the index of the output the
shader writes to:

```cpp
SharedPtr<RenderPass> thirdPass = device->buildRenderPass("Third Pass")
    .renderTarget("Color", 0, RenderTargetType::Present, Format::B8G8R8A8_UNORM)
    .renderTarget("Depth", 1, RenderTargetType::DepthStencil, Format::D32_SFLOAT);
```

Neither target is cleared here: the color image already contains the result of the second pass, and the depth image the values of the first
one. The third pass reuses that depth buffer to test its geometry against, but must not write into it, which the rasterizer state expresses:

```cpp
.rasterizer(device->buildRasterizer()
    .polygonMode(PolygonMode::Solid)
    .cullMode(CullMode::BackFaces)
    .cullOrder(CullOrder::ClockWise)
    .lineWidth(1.f)
    .depthState(DepthStencilState::DepthState { .Write = false, .Operation = CompareOperation::Less }))
```

The screen-filling quad of the second pass needs no depth test and no culling at all, so its pipeline uses `CullMode::Disabled`.

## Drawing the passes

Each pass could submit on its own when it ends, as in the basic rendering sample. Instead, the sample records all three into one command
buffer and submits that once. This saves two submissions per frame, and lets the GPU overlap work of consecutive passes where no barrier
orders it:

```cpp
auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
graphicsQueue.waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);
auto commandBuffer = graphicsQueue.createCommandBuffer();
```

Each pass is then begun on that command buffer. The draw calls go into the pass's own secondary command buffer, which the pass executes in
the shared one when it ends:

```cpp
firstPass.begin(frameBuffer, *commandBuffer);
auto secondary = firstPass.commandBuffer(0);
// ... draw the geometry as in the basic rendering sample ...
firstPass.end();   // Does not submit, as the command buffer is ours.

secondPass.begin(frameBuffer, *commandBuffer);
secondary = secondPass.commandBuffer(0);
secondary->use(secondPipeline);
secondary->setViewports(m_viewport.get());
secondary->setScissors(m_scissor.get());
secondary->bind(viewPlaneVertexBuffer);
secondary->bind(viewPlaneIndexBuffer);
secondary->drawIndexed(viewPlaneIndexBuffer.elements());
secondPass.end();

thirdPass.begin(frameBuffer, *commandBuffer);
// ... draw the geometry again ...
thirdPass.end();
```

Because the passes write into the same frame buffer, the engine knows their dependencies and inserts the necessary barriers, also when they
share a command buffer. Pipeline and binding state does not carry over from one pass to the next, so each pass binds its pipeline and
buffers again.

Since no pass submits, none presents either, although the third pass writes into the present target. The sample submits the command buffer
and presents the frame itself:

```cpp
auto fence = graphicsQueue.submit(commandBuffer);
m_device->swapChain().present(fence);
```

The guide covers this in detail under [sharing a command buffer between render
passes](../../guides/synchronization/render-passes-and-frames.md#sharing-a-command-buffer-between-render-passes).

### One transform per draw call

The first and the third pass both draw the geometry, each with its own transform. The transform is written by the CPU with `map`, which
happens immediately, while the draw calls read it only when the GPU executes them. If both passes used the same element, both draw calls
would see whichever transform the CPU wrote last.

The sample therefore allocates two elements per frame in flight, and one descriptor set for each:

```cpp
auto transformBuffer = m_device->factory().createBuffer("Transform", transformBindingLayout, 0, ResourceHeap::Dynamic, 6);
auto transformBindings = std::views::iota(0u, 6u)
    | std::views::transform([&](UInt32 i) { return transformBindingLayout.allocate({ { .binding = 0, .resource = *transformBuffer, .firstElement = i, .elements = 1 } }); })
    | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();
```

The first pass uses the element `backBuffer * 2`, the third pass the element `backBuffer * 2 + 1`. The same applies to any per-frame data
that several draw calls of one submission write: each needs its own copy.

The quad is drawn from its own vertex and index buffers, built from four vertices with texture coordinates that cover the whole screen.

!!! tip "Transient images"

    All three passes allocate their own images here. The [resource aliasing](../samples-advanced/resource-aliasing.md) tutorial shows how images whose
    lifetimes do not overlap can share the same memory.
