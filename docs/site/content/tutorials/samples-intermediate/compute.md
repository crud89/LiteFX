# Compute pipelines

!!! abstract "Sample 09"

    [`Samples/Compute`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Compute) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md).

Not every GPU workload draws geometry. *Compute shaders* run freely over data, which makes them the tool for post-processing, culling,
simulation and similar work. The sample draws the geometry as before, converts the image to grayscale with a compute shader, and copies the
result into the swap chain image. Along the way, it uses three different queues.

## A writable render target

The compute shader writes into the image the render pass produced, so that image needs to allow writes. The color target is therefore no
longer a present target, and its image is allocated explicitly:

```cpp
SharedPtr<RenderPass> renderPass = device->buildRenderPass("Opaque")
    .renderTarget("Color Target", RenderTargetType::Color, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
    .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) {
    frameBuffer->addImage(renderPass->renderTarget(0), MultiSamplingLevel::x1, ResourceUsage::FrameBufferImage | ResourceUsage::AllowWrite);
    frameBuffer->addImage(renderPass->renderTarget(1));
});
```

Because no pass writes a present target anymore, the sample presents the frame itself at the end.

## The compute pipeline

A compute pipeline needs nothing but a shader program and a layout:

```cpp
SharedPtr<ShaderProgram> postProgram = device->buildShaderProgram()
    .withComputeShaderModule("shaders/compute_lum_cs." + FileExtensions<TRenderBackend>::SHADER); // RGB -> Luminosity

UniquePtr<ComputePipeline> postPipeline = device->buildComputePipeline("Post")
    .layout(postProgram->reflectPipelineLayout())
    .shaderProgram(postProgram);
```

The shader declares how many threads form a group, and reads and writes the image directly:

```hlsl
RWTexture2D<float4> FrameBuffer : register(u0, space0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float3 color = FrameBuffer.Load(id.xy).rgb;
    float Y = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    FrameBuffer[id.xy] = float4(Y, Y, Y, 1.0);
}
```

Each frame buffer's color image gets its own descriptor set, so the compute shader works on the image of the current frame:

```cpp
auto postBindings = postInputLayout.allocate(3, {
    { { .resource = m_device->state().frameBuffer("Frame Buffer 0").resolveImage("Color Target"_hash) } },
    { { .resource = m_device->state().frameBuffer("Frame Buffer 1").resolveImage("Color Target"_hash) } },
    { { .resource = m_device->state().frameBuffer("Frame Buffer 2").resolveImage("Color Target"_hash) } }
}) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();
```

## Dispatching the work

The compute pass runs on the compute queue. When no render pass is drawing into it, a frame buffer image is in the `Common` layout, the
default state of all images (see [the default state](../../guides/synchronization/barriers.md#the-default-state)). `Common` allows shaders
to *read* an image, but not to write it, so the image is transitioned into the `ReadWrite` layout before the dispatch, and back into
`Common` afterwards:

```cpp
auto& computeQueue = m_device->defaultQueue(QueueType::Compute);
auto commandBuffer = computeQueue.createCommandBuffer();
commandBuffer->use(postPipeline);

auto& image = frameBuffer["Color Target"];
auto barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Compute);
barrier->transition(image, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Common, ImageLayout::ReadWrite);
commandBuffer->barrier(*barrier);

commandBuffer->bind(postBindings);
commandBuffer->dispatch({ static_cast<UInt32>(image.extent().x()) / 8, static_cast<UInt32>(image.extent().y()) / 8, 1 });

barrier = m_device->makeBarrier(PipelineStage::Compute, PipelineStage::None);
barrier->transition(image, ResourceAccess::ShaderReadWrite, ResourceAccess::None, ImageLayout::ReadWrite, ImageLayout::Common);
commandBuffer->barrier(*barrier);
```

The first barrier waits for nothing (`PipelineStage::None`): the image was last written by the render pass in an earlier submission, and the
wait for that submission is handled by the queues, as described below. The second barrier returns the image to its default state, so the
next user can rely on it being in `Common`.

!!! note "Vulkan does not check layouts"
    The Vulkan backend keeps all images in the `GENERAL` layout, so a missing transition into `ReadWrite` would go unnoticed there. DirectX
    12 requires it for shader writes. Always write barriers as if targeting DirectX 12.

`dispatch` takes the number of thread *groups*, not threads. With groups of 8×8 threads, the image dimensions are divided by eight.

## Synchronizing the queues

Three queues work on one frame: the graphics queue draws, the compute queue post-processes, and the graphics queue copies the result into
the swap chain image. Each step must wait for the previous one, which is expressed with the fences the queues return:

```cpp
UInt64 geometryFence = renderPass.end();

// Compute queue waits for the geometry, then post-processes.
m_device->defaultQueue(QueueType::Compute).waitFor(renderPass.commandQueue(), geometryFence);
auto postProcessFence = computeQueue.submit(commandBuffer);

// Graphics queue waits for the post-processing, then copies and presents.
graphicsQueue.waitFor(m_device->defaultQueue(QueueType::Compute), postProcessFence);
auto fence = graphicsQueue.submit(commandBuffer);
m_device->swapChain().present(fence);
```

The copy itself is a transfer between two images. The post-processed image is in `Common` again, which would allow the copy, but the
sample uses the dedicated layout for copy sources, which can be more efficient. The swap chain image starts from `Undefined`, since its
previous contents are overwritten:

```cpp
barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
barrier->transition(image, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::Common, ImageLayout::CopySource);
barrier->transition(*m_device->swapChain().image(backBuffer), ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
commandBuffer->barrier(*barrier);
commandBuffer->transfer(image, *m_device->swapChain().image(backBuffer));
```

Afterwards, both images are transitioned back: the frame buffer image into `Common`, where the next render pass expects it, and the swap chain
image into the layout for presentation. Nothing in this submission uses them afterwards, so the barrier waits for the copy and blocks no
further stage (`PipelineStage::None` with `ResourceAccess::None`):

```cpp
barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::None);
barrier->transition(image, ResourceAccess::TransferRead, ResourceAccess::None, ImageLayout::CopySource, ImageLayout::Common);
barrier->transition(*m_device->swapChain().image(backBuffer), ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::Present);
commandBuffer->barrier(*barrier);
```

The synchronization guide describes this case under [copying a render
target](../../guides/synchronization/patterns.md#copying-a-render-target).

!!! tip "Debug regions"

    The sample wraps its queue work in `beginDebugRegion` and `endDebugRegion`. Graphics debuggers such as RenderDoc and PIX show these
    regions, which makes a frame much easier to read.
