# Textures

!!! abstract "Sample 05"

    [`Samples/Textures`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Textures) · builds on
    [basic rendering](basic-rendering.md).

This sample draws a textured quad. It covers loading an image, uploading it to the GPU, generating mip maps and sampling it in the shader.

## Texture coordinates

The shader needs to know where on the image each vertex sits, which is what texture coordinates describe. The `Vertex` structure already has
them, so only the input assembler gains another attribute:

```cpp
.withAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0), AttributeSemantic::TextureCoordinate, 0)
```

In the fragment shader, the texture and its sampler are two separate bindings:

```hlsl
ConstantBuffer<CameraData> camera : register(b0, space0);
Texture2D diffuseMap              : register(t1, space0);
SamplerState diffuse              : register(s2, space0);

FragmentData PSMain(VertexData input)
{
    FragmentData fragment;
    fragment.Depth = input.Position.z;
    fragment.Color = diffuseMap.Sample(diffuse, input.TextureCoordinate) * input.Color;
    return fragment;
}
```

The *sampler* describes how the image is read: how it is filtered between pixels and mip levels, and what happens outside its bounds.

## Creating the texture

The image file is loaded with [stb_image](https://github.com/nothings/stb), and the texture is created with six mip levels:

```cpp
auto imageData = ImageDataPtr(::stbi_load("assets/logo_quad.tga", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

if (imageData == nullptr)
    throw std::runtime_error("Texture could not be loaded: \"assets/logo_quad.tga\".");

texture = device.factory().createTexture("Texture", Format::R8G8B8A8_UNORM, Size2d(width, height), ImageDimensions::DIM_2, 6, 1,
    MultiSamplingLevel::x1, ResourceUsage::AllowWrite | ResourceUsage::TransferDestination | ResourceUsage::TransferSource);
```

The usage flags say what the texture is used for: it is written by the mip map generation (`AllowWrite`), and it is both the destination of
the upload and the source when the mip levels are generated from each other.

## Barriers and layouts

Unlike buffers, images have a *layout* that describes how the GPU currently stores them. A layout is optimal for exactly one purpose, so the
image has to be transitioned before it is used differently. A *barrier* performs that transition, and at the same time orders the commands
around it:

```cpp
UniquePtr<TBarrier> barrier = device.buildBarrier()
    .waitFor(PipelineStage::None).toContinueWith(PipelineStage::Transfer)
    .blockAccessTo(*texture, ResourceAccess::TransferWrite).transitionLayout(ImageLayout::CopyDestination).whenFinishedWith(ResourceAccess::None);

commandBuffer->barrier(*barrier);
commandBuffer->transfer(imageData.get(), texture->size(0), *texture);
```

Read as a sentence: wait for nothing, continue with transfer commands, and block their write access to the texture until it has been
transitioned to the layout for copy destinations.

After the upload, the smaller mip levels are generated from the largest one. The engine provides a `Blitter` for this, which records the
necessary dispatches and barriers:

```cpp
auto blitter = Blitter<TBackend>::create(device);
blitter->generateMipMaps(*texture, *commandBuffer);
```

Finally, the texture is transitioned into the layout for shader resources, so the fragment shader can sample it:

```cpp
barrier = device.buildBarrier()
    .waitFor(PipelineStage::All).toContinueWith(PipelineStage::Fragment)
    .blockAccessTo(*texture, ResourceAccess::ShaderRead).transitionLayout(ImageLayout::ShaderResource).whenFinishedWith(ResourceAccess::None);

commandBuffer->barrier(*barrier);
auto transferFence = commandBuffer->submit();
```

Mip map generation is a compute workload, so the command buffer is created on the graphics queue here, not on the transfer queue.

## The sampler

The sampler is created separately from the texture, and the same sampler can be used with many textures:

```cpp
sampler = device.factory().createSampler("Sampler", FilterMode::Linear, FilterMode::Linear,
    BorderMode::Repeat, BorderMode::Repeat, BorderMode::Repeat, MipMapMode::Linear,
    0.f, std::numeric_limits<Float>::max(), 0.f, 16.f);
```

Linear filtering interpolates between neighboring pixels and mip levels, `Repeat` tiles the image outside its bounds, and the last argument
enables anisotropic filtering, which keeps steeply viewed surfaces sharp.

Texture and sampler share the descriptor set with the camera, since all three are written once and then stay unchanged:

```cpp
auto staticBindings = staticBindingLayout.allocate({ { 0, *cameraBuffer }, { 1, *texture }, { 2, *sampler } });
```
