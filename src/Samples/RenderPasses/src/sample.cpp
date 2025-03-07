#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

enum DescriptorSets : UInt32 // NOLINT(performance-enum-size)
{
    Constant = 0,                                       // All buffers that are immutable.
    PerFrame = 1,                                       // All buffers that are updated each frame.
};

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };

const Array<Vertex> viewPlaneVertices =
{
    { { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
    { { -1.0f, 1.0f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 1.0f, -1.0f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, 1.0f, 0.0f },   { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }
};

const Array<UInt16> viewPlaneIndices = { 0, 1, 2, 1, 3, 2 };

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

struct TransformBuffer {
    glm::mat4 World;
} transform;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
template<>
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
template<>
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RenderPass = TRenderBackend::render_pass_type;
    using RenderPipeline = TRenderBackend::render_pipeline_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;
    using FrameBuffer = TRenderBackend::frame_buffer_type;

    // Get the default device.
    auto device = backend->device("Default");

    // Create the frame buffers for all back buffers.
    auto frameBuffers = std::views::iota(0u, device->swapChain().buffers()) |
        std::views::transform([&](UInt32 index) { 
            auto frameBuffer = device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea());

            // NOTE: In this example we manually add the images to the frame buffers and map them later. This demonstrates how to share the same image 
            //       on multiple render targets. Note that the formats must match. If you intend to use multi-sampling you also have to keep the sample
            //       level in mind!
            frameBuffer->addImage("G-Buffer Color", Format::B8G8R8A8_UNORM); // Written in first render pass, read in second render pass.
            frameBuffer->addImage("Color", Format::B8G8R8A8_UNORM); // Written in second and third render pass.
            frameBuffer->addImage("Depth", Format::D32_SFLOAT); // Written first, read in third render pass for depth test.

            return std::move(frameBuffer);
        }) | std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

    // Create input assembler state.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
            .withAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0), AttributeSemantic::TextureCoordinate, 0)
            .add();

    inputAssemblerState = std::static_pointer_cast<IInputAssembler>(inputAssembler);

    // Create three render passes:
    // - The first render pass draws geometry into "G-Buffer Color" image and "Depth" image.
    // - The second is a screen-space pass, that samples "G-Buffer Color" and writes it into "Color", but does not use "Depth".
    // - The third render pass again draws geometry, but directly into "Color". It uses "Depth" as a render target, but does not write to it (see it's 
    //   rasterizer depth state for more info).
    // Note that using the same names for render targets and image resources makes mapping render targets easier, as we can call the `mapRenderTargets`.
    SharedPtr<RenderPass> firstPass = device->buildRenderPass("First Pass")
        .renderTarget("G-Buffer Color", 0, RenderTargetType::Color, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f }) // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        .renderTarget("Depth", 1, RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear | RenderTargetFlags::ClearStencil, { 1.f, 0.f, 0.f, 0.f });
    
    SharedPtr<RenderPass> secondPass = device->buildRenderPass("Second Pass")
        .inputAttachmentSamplerBinding(DescriptorBindingPoint { .Register = 0, .Space = 1 })
        .inputAttachment(DescriptorBindingPoint { .Register = 0, .Space = 0 }, *firstPass, 0)  // Map color attachment from geometry pass render target 0.
        .renderTarget("Color", RenderTargetType::Color, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f }); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    SharedPtr<RenderPass> thirdPass = device->buildRenderPass("Third Pass")
        .renderTarget("Color", 0, RenderTargetType::Present, Format::B8G8R8A8_UNORM)
        .renderTarget("Depth", 1, RenderTargetType::DepthStencil, Format::D32_SFLOAT);

    // Map all render targets to the frame buffer.
    // NOTE: As we use name matching for mapping, we do not need to map the second render pass, as the "Color" target will be mapped properly. The 
    //       "Depth" target will actually be mapped twice, so that the second mapping overwrites the first one, but the mappings are equal anyway.
    std::ranges::for_each(frameBuffers, [&firstPass, &thirdPass](auto& frameBuffer) { 
        frameBuffer->mapRenderTargets(firstPass->renderTargets()); 
        frameBuffer->mapRenderTargets(thirdPass->renderTargets());
    });

    // Create the shader programs.
    SharedPtr<ShaderProgram> geometryPassShader = device->buildShaderProgram()
        .withVertexShaderModule("shaders/geometry_pass_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/geometry_pass_fs." + FileExtensions<TRenderBackend>::SHADER);

    SharedPtr<ShaderProgram> samplingPassShader = device->buildShaderProgram()
        .withVertexShaderModule("shaders/lighting_pass_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/lighting_pass_fs." + FileExtensions<TRenderBackend>::SHADER);

    // Create a render pipeline for each render pass.
    UniquePtr<RenderPipeline> firstPipeline = device->buildRenderPipeline(*firstPass, "First Pass Pipeline")
        .inputAssembler(inputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::BackFaces)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f))
        .layout(geometryPassShader->reflectPipelineLayout())
        .shaderProgram(geometryPassShader);

    UniquePtr<RenderPipeline> secondPipeline = device->buildRenderPipeline(*secondPass, "Second Pass Pipeline")
        .inputAssembler(inputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::Disabled))
        .layout(samplingPassShader->reflectPipelineLayout())
        .shaderProgram(samplingPassShader);

    UniquePtr<RenderPipeline> thirdPipeline = device->buildRenderPipeline(*thirdPass, "Third Pass Pipeline")
        .inputAssembler(inputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::BackFaces)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f)
            .depthState(DepthStencilState::DepthState { .Write = false, .Operation = CompareOperation::Less }))
        .layout(geometryPassShader->reflectPipelineLayout())
        .shaderProgram(geometryPassShader);

    // Add the resources to the device state.
    device->state().add(std::move(firstPass));
    device->state().add(std::move(secondPass));
    device->state().add(std::move(thirdPass));
    device->state().add(std::move(firstPipeline));
    device->state().add(std::move(secondPipeline));
    device->state().add(std::move(thirdPipeline));
    std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });
}

void SampleApp::initBuffers(IRenderBackend* /*backend*/)
{
    // Get a command buffer
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);

    // Create the vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(vertices.size()));
    commandBuffer->transfer(vertices.data(), vertices.size() * sizeof(::Vertex), *vertexBuffer, 0, static_cast<UInt32>(vertices.size()));

    // Create the index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(indices.size()));
    commandBuffer->transfer(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), *indexBuffer, 0, static_cast<UInt32>(indices.size()));

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    // NOTE: We can re-use the same bindings for the first and the last render pass, as they are compatible.
    auto& geometryPipeline = m_device->state().pipeline("First Pass Pipeline");
    auto& cameraBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBindingLayout, 0, ResourceHeap::Resource);
    auto cameraBindings = cameraBindingLayout.allocate({ { 0, *cameraBuffer } });

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer, *cameraBuffer);

    // Next, we create the descriptor sets for the transform buffer. The transform changes with every frame. Since we have three frames in flight, we
    // create a buffer with three elements and bind the appropriate element to the descriptor set for every frame.
    auto& transformBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
    auto transformBuffer = m_device->factory().createBuffer("Transform", transformBindingLayout, 0, ResourceHeap::Dynamic, 3);
    auto transformBindings = transformBindingLayout.allocate(3, {
        { {.binding = 0, .resource = *transformBuffer, .firstElement = 0, .elements = 1 } },
        { {.binding = 0, .resource = *transformBuffer, .firstElement = 1, .elements = 1 } },
        { {.binding = 0, .resource = *transformBuffer, .firstElement = 2, .elements = 1 } }
    }) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();

    // Create buffers for lighting pass, i.e. the view plane vertex and index buffers.
    auto viewPlaneVertexBuffer = m_device->factory().createVertexBuffer("View Plane Vertices", m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(viewPlaneVertices.size()));
    auto viewPlaneIndexBuffer = m_device->factory().createIndexBuffer("View Plane Indices", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(viewPlaneIndices.size()));
    commandBuffer->transfer(viewPlaneVertices.data(), viewPlaneVertices.size() * sizeof(::Vertex), *viewPlaneVertexBuffer, 0, static_cast<UInt32>(viewPlaneVertices.size()));
    commandBuffer->transfer(viewPlaneIndices.data(), viewPlaneIndices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), *viewPlaneIndexBuffer, 0, static_cast<UInt32>(viewPlaneIndices.size()));

    // End and submit the command buffer.
    m_transferFence = commandBuffer->submit();

    // Add everything to the state.
    m_device->state().add(std::move(vertexBuffer));
    m_device->state().add(std::move(viewPlaneVertexBuffer));
    m_device->state().add(std::move(indexBuffer));
    m_device->state().add(std::move(viewPlaneIndexBuffer));
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(transformBuffer));
    m_device->state().add("Camera Bindings", std::move(cameraBindings));
    std::ranges::for_each(transformBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(std::format("Transform Bindings {0}", i++), std::move(binding)); });
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const
{
    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    camera.ViewProjection = projection * view;

    // Create a staging buffer and use to transfer the new uniform buffer to.
    commandBuffer.transfer(static_cast<const void*>(&camera), sizeof(camera), buffer);
}

void SampleApp::onStartup() 
{
    // Run application loop until the window is closed.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
        this->updateWindowTitle();
    }
}

void SampleApp::onShutdown()
{
    // Destroy the window.
    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::onInit()
{
    ::glfwSetWindowUserPointer(m_window.get(), this);

    ::glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int width, int height) { 
        auto app = static_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->resize(width, height); 
    });

    ::glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = static_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->keyDown(key, scancode, action, mods);
    });

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
        // Store the window handle.
        auto window = m_window.get();

        // Get the proper frame buffer size.
        int width{}, height{};
        ::glfwGetFramebufferSize(window, &width, &height);

        // Create viewport and scissors.
        m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend* backend) {
        backend->releaseDevice("Default");
    };

#ifdef LITEFX_BUILD_VULKAN_BACKEND
    // Register the Vulkan backend de-/initializer.
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
}

void SampleApp::onResize(const void* /*sender*/, const ResizeEventArgs& e)
{
    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(e.width(), e.height());
    auto vsync = m_device->swapChain().verticalSynchronization();
    m_device->swapChain().reset(surfaceFormat, renderArea, 3, vsync);

    // Resize the frame buffers. Note that we could also use an event handler on the swap chain `reseted` event to do this automatically instead.
    m_device->state().frameBuffer("Frame Buffer 0").resize(renderArea);
    m_device->state().frameBuffer("Frame Buffer 1").resize(renderArea);
    m_device->state().frameBuffer("Frame Buffer 2").resize(renderArea);

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));

    // Also update the camera.
    auto& cameraBuffer = m_device->state().buffer("Camera");
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);
    this->updateCamera(*commandBuffer, cameraBuffer);
    m_transferFence = commandBuffer->submit();
}

void SampleApp::keyDown(int key, int /*scancode*/, int action, int /*mods*/)
{
#ifdef LITEFX_BUILD_VULKAN_BACKEND
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        this->startBackend<VulkanBackend>();
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        this->startBackend<DirectX12Backend>();
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        static RectI windowRect;

        // Check if we're switching from fullscreen to windowed or the other way around.
        if (::glfwGetWindowMonitor(m_window.get()) == nullptr)
        {
            // Find the monitor, that contains most of the window.
            RectI clientRect, monitorRect;
            GLFWmonitor* currentMonitor = nullptr;
            const GLFWvidmode* currentVideoMode = nullptr;
            int monitorCount{};

            ::glfwGetWindowPos(m_window.get(), &clientRect.x(), &clientRect.y());
            ::glfwGetWindowSize(m_window.get(), &clientRect.width(), &clientRect.height());
            auto monitors = ::glfwGetMonitors(&monitorCount);
            int highestOverlap = 0;

            for (int i(0); i < monitorCount; ++i)
            {
                auto monitor = monitors[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto mode = ::glfwGetVideoMode(monitor);
                ::glfwGetMonitorPos(monitor, &monitorRect.x(), &monitorRect.y());
                monitorRect.width() = mode->width;
                monitorRect.height() = mode->height;

                auto overlap =
                    std::max(0, std::min(clientRect.x() + clientRect.width(), monitorRect.x() + monitorRect.width()) - std::max(clientRect.x(), monitorRect.x())) *
                    std::max(0, std::min(clientRect.y() + clientRect.height(), monitorRect.y() + monitorRect.height()) - std::max(clientRect.y(), monitorRect.y()));

                if (highestOverlap < overlap)
                {
                    highestOverlap = overlap;
                    currentMonitor = monitor;
                    currentVideoMode = mode;
                }
            }

            // Save the current window rect in order to restore it later.
            windowRect = clientRect;

            // Switch to fullscreen.
            if (currentVideoMode != nullptr)
                ::glfwSetWindowMonitor(m_window.get(), currentMonitor, 0, 0, currentVideoMode->width, currentVideoMode->height, currentVideoMode->refreshRate);
        }
        else
        {
            // NOTE: If we were to launch in fullscreen mode, we should use something like `max(windowRect.width(), defaultWidth)`.
            ::glfwSetWindowMonitor(m_window.get(), nullptr, windowRect.x(), windowRect.y(), windowRect.width(), windowRect.height(), 0);
        }
    }

    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
        // Wait for the device.
        m_device->wait();

        // Toggle VSync on the swap chain.
        auto& swapChain = m_device->swapChain();
        swapChain.reset(swapChain.surfaceFormat(), swapChain.renderArea(), swapChain.buffers(), !swapChain.verticalSynchronization());
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        // Close the window with the next loop.
        ::glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE);
    }
}

void SampleApp::updateWindowTitle()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - lastTime).count();

    std::stringstream title;
    title << this->name() << " | " << "Backend: " << this->activeBackend(BackendType::Rendering)->name() << " | " << static_cast<UInt32>(1000.0f / frameTime) << " FPS"; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    ::glfwSetWindowTitle(m_window.get(), title.str().c_str());
    lastTime = std::chrono::high_resolution_clock::now();
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();
    auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
    UInt64 fence = 0;

    {
        // Query state.
        auto& renderPass = m_device->state().renderPass("First Pass");
        auto& pipeline = m_device->state().pipeline("First Pass Pipeline");
        auto& transformBuffer = m_device->state().buffer("Transform");
        auto& cameraBindings = m_device->state().descriptorSet("Camera Bindings");
        auto& transformBindings = m_device->state().descriptorSet(std::format("Transform Bindings {0}", backBuffer));
        auto& vertexBuffer = m_device->state().vertexBuffer("Vertex Buffer");
        auto& indexBuffer = m_device->state().indexBuffer("Index Buffer");

        // Wait for all transfers to finish.
        renderPass.commandQueue().waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);

        // Begin rendering on the render pass and use the only pipeline we've created for it.
        renderPass.begin(frameBuffer);
        auto commandBuffer = renderPass.commandBuffer(0);
        commandBuffer->use(pipeline);
        commandBuffer->setViewports(m_viewport.get());
        commandBuffer->setScissors(m_scissor.get());

        // Get the amount of time that has passed since the first frame.
        auto now = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

        // Compute world transform and update the transform buffer.
        transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        transformBuffer.map(static_cast<const void*>(&transform), sizeof(transform), backBuffer);

        // Bind both descriptor sets to the pipeline.
        commandBuffer->bind({ &cameraBindings, &transformBindings });

        // Bind the vertex and index buffers.
        commandBuffer->bind(vertexBuffer);
        commandBuffer->bind(indexBuffer);

        // Draw the object and end the render pass.
        commandBuffer->drawIndexed(indexBuffer.elements());
        fence = renderPass.end();
    }

    {
        // Query state.
        auto& renderPass = m_device->state().renderPass("Second Pass");
        auto& pipeline = m_device->state().pipeline("Second Pass Pipeline");
        auto& viewPlaneVertexBuffer = m_device->state().vertexBuffer("View Plane Vertices");
        auto& viewPlaneIndexBuffer = m_device->state().indexBuffer("View Plane Indices");

        // Start the lighting pass.
        renderPass.commandQueue().waitFor(fence);
        renderPass.begin(frameBuffer);
        auto commandBuffer = renderPass.commandBuffer(0);
        commandBuffer->use(pipeline);
        commandBuffer->setViewports(m_viewport.get());
        commandBuffer->setScissors(m_scissor.get());

        // Draw the view plane.
        commandBuffer->bind(viewPlaneVertexBuffer);
        commandBuffer->bind(viewPlaneIndexBuffer);
        commandBuffer->drawIndexed(viewPlaneIndexBuffer.elements());

        // End the lighting pass.
        fence = renderPass.end();
    }

    {
        // Query state.
        auto& renderPass = m_device->state().renderPass("Third Pass");
        auto& pipeline = m_device->state().pipeline("Third Pass Pipeline");
        auto& transformBuffer = m_device->state().buffer("Transform");
        auto& cameraBindings = m_device->state().descriptorSet("Camera Bindings");
        auto& transformBindings = m_device->state().descriptorSet(std::format("Transform Bindings {0}", backBuffer));
        auto& vertexBuffer = m_device->state().vertexBuffer("Vertex Buffer");
        auto& indexBuffer = m_device->state().indexBuffer("Index Buffer");

        // Begin rendering on the render pass and use the only pipeline we've created for it.
        renderPass.commandQueue().waitFor(fence);
        renderPass.begin(frameBuffer);
        auto commandBuffer = renderPass.commandBuffer(0);
        commandBuffer->use(pipeline);
        commandBuffer->setViewports(m_viewport.get());
        commandBuffer->setScissors(m_scissor.get());

        // Get the amount of time that has passed since the first frame.
        auto now = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

        // Bind both descriptor sets to the pipeline.
        commandBuffer->bind({ &cameraBindings, &transformBindings });

        // Bind the vertex and index buffers.
        commandBuffer->bind(vertexBuffer);
        commandBuffer->bind(indexBuffer);

        // Draw an additional instance of the object on top of the existing contents.
        transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        transformBuffer.map(static_cast<const void*>(&transform), sizeof(transform), backBuffer);
        commandBuffer->drawIndexed(indexBuffer.elements());

        // Present the frame by ending the render pass.
        renderPass.end();
    }
}