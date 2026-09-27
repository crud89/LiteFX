# Render pass and frame buffers

Before the GPU can draw anything, it needs several resources:

- **What** to draw: the triangle's vertices and indices, stored in a vertex and an index buffer.
- **Where** to draw: the images the triangle is drawn into, stored in *frame buffers*.
- **When** to draw: a *render pass*, which groups the drawing commands that write into the same images.
- **How** to draw: the *render pipeline*, which describes the shaders and settings the GPU uses.

This part covers the triangle's data and the *where* and *when*.

## The triangle

A triangle consists of three vertices, each of which has a position and a color. Define a structure for the vertices and add the triangle's
vertices and indices to the application class:

```cpp title="main.cpp"
struct Vertex {
    Vector4f position;
    Vector4f color;
};

class MyApp : public LiteFX::App {
    // ...

private:
    // ...

    std::array<Vertex, 3> m_vertices {
        Vertex { { 0.1f, 0.1f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        Vertex { { 0.9f, 0.1f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        Vertex { { 0.5f, 0.9f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    std::array<UInt16, 3> m_indices { 0, 1, 2 };

    // ...
};
```

The positions are given in the coordinate system the GPU draws in, where both axes run
from `-1` to `1`. The triangle therefore covers part of the upper right quarter of the window. Each corner has a different color: red, green
and blue.

For now, the data only lives in the application's memory. It is transferred to the GPU in the [last part](drawing.md) of the tutorial.

## Render targets and render passes

The images the GPU draws into are called *render targets*. A render target has a *format*, which describes what each pixel stores, and a
*type*, which describes what the render target is used for:

- **Color:** an image that stores colors, or other information, such as normals or material properties.
- **Depth/Stencil:** an image that stores depth and/or stencil values. It is used to discard pixels during rendering, e.g. those of objects
  that are hidden behind others.
- **Present:** a color image that is shown in the window. Only one render target of this type can exist in a chain of render passes. When a
  render pass that writes into it ends, the image is presented.

The triangle is drawn straight into the window, so a single present target is sufficient. Create a render pass with this render target in
the start handler, after the device has been created:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;

    // ...

    // Create the device.
    // ...

    // Create a render pass.
    SharedPtr<RenderPass> renderPass = device->buildRenderPass("Geometry")
        .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });

    // ...

    m_device = device;
    return true;
};
```

The render pass is called `"Geometry"`, and its render target `"Color Target"`. The render target uses the same format as the swap chain,
since present targets only support a few formats. The `Clear` flag tells the render pass to fill the image with a constant color whenever the
render pass begins. The last argument is this color: a dark gray, which is fully opaque.

## Frame buffers

A render pass only describes how it uses its render targets. The actual images are stored in *frame buffers*. Create one frame buffer for
each back buffer of the swap chain, so that each frame that is processed at the same time has its own images:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;

    // ...

    // Create a render pass.
    // ...

    // Create a frame buffer for each back buffer of the swap chain.
    auto frameBuffers = std::views::iota(0u, device->swapChain().buffers()) |
        std::views::transform([&](UInt32 index) { return device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea()); }) |
        std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

    // Allocate the images for the render targets of the render pass.
    std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets()); });

    // ...

    m_device = device;
    return true;
};
```

Each frame buffer has a *render area*, which is the default size of the images it allocates. Here, it is taken from the swap chain, so that
the images match the window. `addImages` then allocates an image for each render target of the render pass.

!!! tip

    The [render passes sample](https://github.com/crud89/LiteFX/tree/main/src/Samples/RenderPasses) shows how multiple render passes share
    their render targets through frame buffers.

With the *where* and *when* in place, the next step is the *how*, which starts with the [shaders](shaders.md).
