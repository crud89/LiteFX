# Your first triangle

In this tutorial, you write an application that draws a single, colored triangle. It is the "hello world" of graphics programming and
introduces the most important concepts of LiteFX along the way:

1. **[The application](application.md):** create an application with a window and run it with the available backends.
2. **[The device](device.md):** start a backend, create a graphics device and implement the application loop.
3. **[Render pass and frame buffers](render-pass.md):** describe *where* the triangle is drawn.
4. **[Shaders](shaders.md):** write the shaders that run on the GPU and compile them as part of your build.
5. **[The render pipeline](pipeline.md):** describe *how* the triangle is drawn.
6. **[Drawing](drawing.md):** upload the triangle to the GPU and draw it every frame.

The application supports both backends, Vulkan and DirectX 12, from the same code.

!!! info "Before you start"

    Set up a project as described in the [project setup](../../getting-started/project-setup.md) guide. The tutorial writes the two source
    files of that project, `main.h` and `main.cpp`.
