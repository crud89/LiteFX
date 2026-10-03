# Multisampling

!!! abstract "Sample 06"

    [`Samples/Multisampling`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Multisampling) · builds on
    [basic rendering](basic-rendering.md).

Along the edges of a triangle, a pixel is either covered or not, which makes edges look like stairs. *Multisampling* (MSAA) evaluates
coverage at several positions within each pixel and averages the result, which smooths the edges. It is one of the cheapest improvements in
image quality, and in LiteFX it takes two changes.

## Images with multiple samples

The number of samples is chosen when the frame buffer allocates its images:

```cpp
auto samples = MultiSamplingLevel::x8;
std::ranges::for_each(frameBuffers, [&renderPass, &samples](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets(), samples); });
```

Not every level works with every format. Eight samples work on all DirectX 12 capable devices, except for RGBA32 formats; four samples are
safe for all formats, and higher levels may not be supported at all.

## The pipeline

The pipeline has to use the same number of samples as the images it draws into:

```cpp
UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
    .inputAssembler(inputAssembler)
    .rasterizer(device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::BackFaces)
        .cullOrder(CullOrder::ClockWise)
        .lineWidth(1.f))
    .layout(shaderProgram->reflectPipelineLayout())
    .shaderProgram(shaderProgram)
    .samples(samples);
```

That is all. The shaders, buffers and the draw loop are unchanged.

## Resolving

A multisampled image cannot be presented directly: the display expects one value per pixel. Combining the samples into that value is called
*resolving*, and the engine does it for you. Since the color target is a present target, the render pass resolves it into the swap chain
image when it ends.

!!! note "Resizing"

    When the window is resized, the frame buffers reallocate their images with `resize`, keeping the multisampling level they were created
    with.
