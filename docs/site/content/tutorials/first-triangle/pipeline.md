# The render pipeline

The *render pipeline* combines everything the GPU needs to know about *how* to draw: the shader program from the previous part, and the
settings of the fixed-function stages. In this part, you configure those stages, create the pipeline, and keep all resources alive for
rendering.

## Input assembler

The *input assembler* is the first stage of the pipeline. It reads the vertices and indices, and assembles them into primitives, such as
triangles. Its state describes how the data is laid out:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;
    using InputAssembler = TBackend::input_assembler_type;

    // ...

    // Create the shader program.
    // ...

    // Create the input assembler state.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZW32F, offsetof(Vertex, position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, color), AttributeSemantic::Color)
            .add();

    // ...

    m_device = device;
    return true;
};
```

This describes the triangle's data as follows:

- **The topology** is a *triangle list*: each group of three vertices forms a triangle.
- **The indices** are 16-bit unsigned integers, matching `m_indices`.
- **The vertex buffer** contains elements of the size of `Vertex`, each with two *attributes*: a position and a color, both stored as four
  32-bit floating point values. Each attribute has a location, its format, its offset within the vertex, and a semantic that matches the
  vertex shader's input.

## Rasterizer

The *rasterizer* is another fixed-function stage. It determines which pixels each triangle covers, so that the fragment shader can run for
them:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    // ...
    using Rasterizer = TBackend::rasterizer_type;

    // ...

    // Create the input assembler state.
    // ...

    // Create the rasterizer state.
    SharedPtr<Rasterizer> rasterizer = device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::BackFaces)
        .cullOrder(CullOrder::CounterClockWise);

    // ...

    m_device = device;
    return true;
};
```

- **`PolygonMode::Solid`** fills the triangle, instead of only drawing its edges or corners.
- **`CullMode::BackFaces`** skips triangles that face away from the viewer. This saves work for closed objects, where those triangles are
  hidden anyway.
- **`CullOrder::CounterClockWise`** defines which side of a triangle is the front: the side from which its vertices, in the order of the
  indices, appear counter-clockwise.

The order of the indices therefore matters: if you reverse it, the triangle faces away from the viewer and is not drawn.

## Creating the render pipeline

With all stages configured, create the render pipeline for the render pass. Besides the states from above, it needs a *pipeline layout*,
which describes which resources (e.g. buffers or textures) the shaders access. Even though the shaders in this tutorial do not access any
resources, a layout is required. The engine can create it for you, by *reflecting* the shader program:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    // ...
    using RenderPipeline = TBackend::render_pipeline_type;

    // ...

    // Create the rasterizer state.
    // ...

    // Create the render pipeline.
    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry Pipeline")
        .inputAssembler(inputAssembler)
        .rasterizer(rasterizer)
        .shaderProgram(shaderProgram)
        .layout(shaderProgram->reflectPipelineLayout());

    // ...

    m_device = device;
    return true;
};
```

!!! tip "Pipeline states in larger applications"

    Real applications often use many pipelines. Switching between them while drawing can be costly, so it is good practice to group drawing
    commands by pipeline and to keep the number of pipelines low.

## Keeping the resources

All resources created so far are local variables of the start handler, so they would be released when it returns. To keep them for
rendering, hand them over to the *device state*. It manages their lifetime, and makes them available by their names later on:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // ...

    // Create the render pipeline.
    // ...

    // Store the resources in the device state.
    device->state().add(std::move(renderPass));
    device->state().add(std::move(renderPipeline));
    std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });

    m_device = device;
    return true;
};
```

The input assembler, rasterizer and shader program do not need to be stored, as the render pipeline keeps them.

Everything is now prepared to draw. In the [last part](drawing.md), you upload the triangle to the GPU and draw it.
