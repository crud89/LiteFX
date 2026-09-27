# Drawing

In this last part, you upload the triangle to the GPU and draw it in every iteration of the application loop.

## Uploading the triangle

The vertices and indices are still stored in the application's memory, which the GPU cannot access for drawing. They need to be copied into
*buffers* on the GPU first. Copying is done by *commands*, which are recorded into a *command buffer* and then executed by a *command queue* of
the device.

Do this at the beginning of the `onStartup` handler, before the application loop starts:

```cpp title="main.cpp"
void MyApp::onStartup()
{
    // Get the render pipeline and its input assembler from the device state.
    auto& geometryPipeline = dynamic_cast<IRenderPipeline&>(m_device->state().pipeline("Geometry Pipeline"));
    auto inputAssembler = geometryPipeline.inputAssembler();

    // Create a command buffer on the transfer queue.
    auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
    auto transferCommands = transferQueue.createCommandBuffer(true);

    // Create the vertex buffer and copy the vertices into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(m_vertices.size()));
    transferCommands->transfer(m_vertices.data(), m_vertices.size() * sizeof(Vertex), *vertexBuffer, 0, static_cast<UInt32>(m_vertices.size()));

    // Create the index buffer and copy the indices into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(m_indices.size()));
    transferCommands->transfer(m_indices.data(), m_indices.size() * inputAssembler->indexBufferLayout()->elementSize(), *indexBuffer, 0, static_cast<UInt32>(m_indices.size()));

    // Submit the command buffer and wait until the GPU has executed it.
    auto fence = transferCommands->submit();
    transferQueue.waitFor(fence);

    // The application loop.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        ::glfwPollEvents();

        // ...
    }
}
```

Step by step:

1. **The layouts** of both buffers are defined by the input assembler, which you configured in the [previous part](pipeline.md). It is
   stored in the render pipeline, which you get from the device state by its name. The device state returns a general pipeline, so it is cast
   to a render pipeline first, which provides the input assembler.
2. **The command buffer** is created on the *transfer queue*, which is meant for copying data. The argument `true` starts recording right away.
3. **The buffers** are created by the device's *factory*. `ResourceHeap::Resource` places them in GPU memory, where they can be accessed
   fastest while drawing.
4. **The `transfer` commands** copy the data from the application's memory into the buffers.
5. **Submitting** the command buffer executes it on the GPU. This happens asynchronously: `submit` returns a *fence*, a value that the queue
   reaches once the command buffer has been executed. `waitFor` blocks until then, which guarantees that the buffers are ready before drawing.

## The frame loop

Now, draw the triangle in each iteration of the application loop. First, ask the swap chain for the next back buffer. It returns its index,
which selects the frame buffer to draw into:

```cpp title="main.cpp"
void MyApp::onStartup()
{
    // ...

    while (!::glfwWindowShouldClose(m_window.get()))
    {
        ::glfwPollEvents();

        // Swap the back buffers for the next frame.
        auto backBuffer = m_device->swapChain().swapBackBuffer();

        // Get the frame buffer for the back buffer and the render pass.
        auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
        auto& renderPass = m_device->state().renderPass("Geometry");

        // ...
    }
}
```

!!! tip

    Looking up resources by name is convenient, but not free. In larger applications, look them up once and keep references to them,
    or manage them solely outside of the *device state*, instead of looking them up in every frame.

Then, record and execute the drawing commands:

```cpp title="main.cpp"
void MyApp::onStartup()
{
    // ...

    while (!::glfwWindowShouldClose(m_window.get()))
    {
        // ...

        // Begin the render pass on the frame buffer and use the render pipeline.
        renderPass.begin(frameBuffer);
        auto commandBuffer = renderPass.commandBuffer(0);
        commandBuffer->use(geometryPipeline);
        commandBuffer->setViewports(m_viewport.get());
        commandBuffer->setScissors(m_scissor.get());

        // Bind the vertex and index buffers.
        commandBuffer->bind(*vertexBuffer);
        commandBuffer->bind(*indexBuffer);

        // Draw the triangle, then end the render pass to present the frame.
        commandBuffer->drawIndexed(indexBuffer->elements());
        renderPass.end();
    }
}
```

- **Beginning the render pass** connects it to the frame buffer, whose images it draws into. It also provides a command buffer for the
  drawing commands, which is already set up to synchronize with the render pass. Multithreaded applications can use several of them, one per
  thread.
- **`use`** selects the render pipeline, and **`setViewports`** and **`setScissors`** set the rectangles you created in the
  [second part](device.md).
- **`bind`** selects the buffers with the triangle's vertices and indices.
- **`drawIndexed`** draws the triangle, using all indices of the index buffer.
- **Ending the render pass** submits the command buffer. As the render pass writes into the present target, the frame is then shown in the
  window.

## Running the application

When you run the application now, it shows a triangle in the upper right part of the window, with its colors blending from red over green to
blue. It works with both backends: the application starts the first backend you registered, and draws with the same code in either of them.

Feel free to experiment with the vertices and indices, to change the shape of the triangle. Keep the order of the indices in mind, though:
if you reverse it, the triangle faces away from the viewer and is culled.

## The complete source

??? example "Complete `main.cpp`"

    ```cpp title="main.cpp"
    #include "main.h"

    #include <algorithm>
    #include <array>
    #include <cstdlib>
    #include <format>
    #include <iostream>
    #include <ranges>

    struct Vertex {
        Vector4f position;
        Vector4f color;
    };

    class MyApp : public LiteFX::App {
    public:
        static StringView Name() noexcept { return "My LiteFX App"sv; }
        StringView name() const noexcept override { return Name(); }

        static AppVersion Version() { return AppVersion(1, 0, 0, 0); }
        AppVersion version() const noexcept override { return Version(); }

    private:
        GlfwWindowPtr m_window;
        Optional<UInt32> m_adapterId;
        SharedPtr<IViewport> m_viewport;
        SharedPtr<IScissor> m_scissor;
        IGraphicsDevice* m_device{ nullptr };

        std::array<Vertex, 3> m_vertices {
            Vertex { { 0.1f, 0.1f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            Vertex { { 0.9f, 0.1f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            Vertex { { 0.5f, 0.9f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        std::array<UInt16, 3> m_indices { 0, 1, 2 };

    public:
        MyApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) :
            App(), m_window(std::move(window)), m_adapterId(adapterId)
        {
            this->initializing += std::bind(&MyApp::onInit, this);
            this->startup += std::bind(&MyApp::onStartup, this);
            this->shutdown += std::bind(&MyApp::onShutdown, this);
        }

    private:
        void onInit();
        void onStartup();
        void onShutdown();
    };

    void MyApp::onInit()
    {
        auto startCallback = [this]<typename TBackend>(TBackend* backend) {
            // Alias type names for improved readability.
            using RenderPass = TBackend::render_pass_type;
            using FrameBuffer = TBackend::frame_buffer_type;
            using ShaderProgram = TBackend::shader_program_type;
            using InputAssembler = TBackend::input_assembler_type;
            using Rasterizer = TBackend::rasterizer_type;
            using RenderPipeline = TBackend::render_pipeline_type;

            // Get the size of the window's drawing area.
            int width{}, height{};
            ::glfwGetFramebufferSize(m_window.get(), &width, &height);

            // Create a viewport and scissor rectangle that cover the whole drawing area.
            m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
            m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

            // Select an adapter and create a surface for the window.
            auto adapter = m_adapterId.has_value() ? backend->findAdapter(m_adapterId) : backend->findAdapter(GpuPreference::Performance);
            auto surface = backend->createSurface(::glfwGetWin32Window(m_window.get()));

            // Create the device.
            auto device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface),
                Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

            // Create a render pass.
            SharedPtr<RenderPass> renderPass = device->buildRenderPass("Geometry")
                .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });

            // Create a frame buffer for each back buffer of the swap chain.
            auto frameBuffers = std::views::iota(0u, device->swapChain().buffers()) |
                std::views::transform([&](UInt32 index) { return device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea()); }) |
                std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

            // Allocate the images for the render targets of the render pass.
            std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets()); });

            // Select the shader byte code for the backend.
            const String extension = std::is_same_v<TBackend, VulkanBackend> ? "spv" : "dxi";

            // Create the shader program.
            SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
                .withVertexShaderModule(std::format("shaders/tutorial_vs.{}", extension), "VSMain")
                .withFragmentShaderModule(std::format("shaders/tutorial_fs.{}", extension), "PSMain");

            // Create the input assembler state.
            SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
                .topology(PrimitiveTopology::TriangleList)
                .indexType(IndexType::UInt16)
                .vertexBuffer(sizeof(Vertex), 0)
                    .withAttribute(0, BufferFormat::XYZW32F, offsetof(Vertex, position), AttributeSemantic::Position)
                    .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, color), AttributeSemantic::Color)
                    .add();

            // Create the rasterizer state.
            SharedPtr<Rasterizer> rasterizer = device->buildRasterizer()
                .polygonMode(PolygonMode::Solid)
                .cullMode(CullMode::BackFaces)
                .cullOrder(CullOrder::CounterClockWise);

            // Create the render pipeline.
            UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry Pipeline")
                .inputAssembler(inputAssembler)
                .rasterizer(rasterizer)
                .shaderProgram(shaderProgram)
                .layout(shaderProgram->reflectPipelineLayout());

            // Store the resources in the device state.
            device->state().add(std::move(renderPass));
            device->state().add(std::move(renderPipeline));
            std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });

            m_device = device;
            return true;
        };

        auto stopCallback = []<typename TBackend>(TBackend* backend) {
            backend->releaseDevice("Default");
        };

    #ifdef LITEFX_BUILD_VULKAN_BACKEND
        this->onBackendStart<VulkanBackend>(startCallback);
        this->onBackendStop<VulkanBackend>(stopCallback);
    #endif // LITEFX_BUILD_VULKAN_BACKEND

    #ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
        this->onBackendStart<DirectX12Backend>(startCallback);
        this->onBackendStop<DirectX12Backend>(stopCallback);
    #endif // LITEFX_BUILD_DIRECTX_12_BACKEND
    }

    void MyApp::onStartup()
    {
        // Get the render pipeline and its input assembler from the device state.
        auto& geometryPipeline = dynamic_cast<IRenderPipeline&>(m_device->state().pipeline("Geometry Pipeline"));
        auto inputAssembler = geometryPipeline.inputAssembler();

        // Create a command buffer on the transfer queue.
        auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
        auto transferCommands = transferQueue.createCommandBuffer(true);

        // Create the vertex buffer and copy the vertices into it.
        auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(m_vertices.size()));
        transferCommands->transfer(m_vertices.data(), m_vertices.size() * sizeof(Vertex), *vertexBuffer, 0, static_cast<UInt32>(m_vertices.size()));

        // Create the index buffer and copy the indices into it.
        auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(m_indices.size()));
        transferCommands->transfer(m_indices.data(), m_indices.size() * inputAssembler->indexBufferLayout()->elementSize(), *indexBuffer, 0, static_cast<UInt32>(m_indices.size()));

        // Submit the command buffer and wait until the GPU has executed it.
        auto fence = transferCommands->submit();
        transferQueue.waitFor(fence);

        // The application loop.
        while (!::glfwWindowShouldClose(m_window.get()))
        {
            ::glfwPollEvents();

            // Swap the back buffers for the next frame.
            auto backBuffer = m_device->swapChain().swapBackBuffer();

            // Get the frame buffer for the back buffer and the render pass.
            auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
            auto& renderPass = m_device->state().renderPass("Geometry");

            // Begin the render pass on the frame buffer and use the render pipeline.
            renderPass.begin(frameBuffer);
            auto commandBuffer = renderPass.commandBuffer(0);
            commandBuffer->use(geometryPipeline);
            commandBuffer->setViewports(m_viewport.get());
            commandBuffer->setScissors(m_scissor.get());

            // Bind the vertex and index buffers.
            commandBuffer->bind(*vertexBuffer);
            commandBuffer->bind(*indexBuffer);

            // Draw the triangle, then end the render pass to present the frame.
            commandBuffer->drawIndexed(indexBuffer->elements());
            renderPass.end();
        }
    }

    void MyApp::onShutdown()
    {
        m_window.reset();
        ::glfwTerminate();
    }

    int main()
    {
        const String appName{ MyApp::Name() };

        if (!::glfwInit())
        {
            std::cerr << "Unable to initialize GLFW.\n";
            return EXIT_FAILURE;
        }

        ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        ::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));

    #ifdef LITEFX_BUILD_VULKAN_BACKEND
        uint32_t extensionCount = 0;
        const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensionCount);
        Array<String> requiredExtensions(extensionNames, extensionNames + extensionCount);
    #endif // LITEFX_BUILD_VULKAN_BACKEND

        try
        {
            UniquePtr<App> app = App::build<MyApp>(std::move(window), std::nullopt)
                .logTo<ConsoleSink>(LogLevel::Trace)
                .logTo<RollingFileSink>("myapp.log", LogLevel::Debug)
    #ifdef LITEFX_BUILD_VULKAN_BACKEND
                .useBackend<VulkanBackend>(requiredExtensions)
    #endif // LITEFX_BUILD_VULKAN_BACKEND
    #ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
                .useBackend<DirectX12Backend>()
    #endif // LITEFX_BUILD_DIRECTX_12_BACKEND
                ;

            app->run();
        }
        catch (const LiteFX::Exception& ex)
        {
            std::cerr << "Unhandled exception: " << ex.what() << "\nat: " << ex.trace() << '\n';
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    ```

## Next steps

You have now seen the core concepts of LiteFX: the application and its backends, devices, render passes, frame buffers, shaders and render
pipelines. The [samples](https://github.com/crud89/LiteFX/tree/main/src/Samples/) build on them and show more techniques, for example:

- **[Basic rendering](https://github.com/crud89/LiteFX/tree/main/src/Samples/BasicRendering):** a 3D object with a camera, using descriptor
  sets to pass data, such as transformations, to the shaders.
- **[Render passes](https://github.com/crud89/LiteFX/tree/main/src/Samples/RenderPasses):** multiple render passes that share render
  targets.
- **[Textures](https://github.com/crud89/LiteFX/tree/main/src/Samples/Textures):** loading and sampling textures.

The [API reference](../../api/index.md) documents all types and functions of the engine.
