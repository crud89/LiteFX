#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

constexpr UInt32 NUM_INSTANCES = 163840u;  // 10 * 128 * 128

enum DescriptorSets : UInt32
{
    PerFrame = 0,
    Constant = 1,
    Indirect = 2
};

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };

struct alignas(16) CameraBuffer {
    glm::mat4 ViewProjection;
    glm::mat4 Projection;
    glm::vec4 Position;
    glm::vec4 Forward;
    glm::vec4 Up;
    glm::vec4 Right;
    float NearPlane;
    float FarPlane;
    float Frustum[4];
} camera;

struct alignas(16) ObjectBuffer {
    glm::mat4 Transform;
    glm::vec4 Color;
    float BoundingRadius;
    UInt32 IndexCount;
    UInt32 FirstIndex;
    int VertexOffset;
} objects[NUM_INSTANCES];

template<typename TRenderBackend> requires
    rtti::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

static constexpr inline glm::vec4 normalizePlane(const glm::vec4& plane) {
    return plane / glm::length(glm::vec3(plane));
}

static inline void initializeObjects() {
    std::srand(std::time(nullptr));

    for (UInt32 i{ 0 }; i < NUM_INSTANCES; ++i)
    {
        int x = i % 128;
        int y = (i / 128) % 128;
        int z = i / 16384;

        auto& instance = objects[i];
        instance.Transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(x - 50, y - 50, z - 5) * 2.0f) * glm::eulerAngleXYZ(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX);
        instance.Color = glm::vec4(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, 1.0f);
        instance.BoundingRadius = glm::length(glm::vec3(0.5f, 0.5f, 0.5f));
        instance.FirstIndex = 0;
        instance.VertexOffset = 0;
        instance.IndexCount = 12;
    }
}

template<typename TRenderBackend> requires
    rtti::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RenderPass = TRenderBackend::render_pass_type;
    using RenderPipeline = TRenderBackend::render_pipeline_type;
    using ComputePipeline = TRenderBackend::compute_pipeline_type;
    using PipelineLayout = TRenderBackend::pipeline_layout_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;
    using Rasterizer = TRenderBackend::rasterizer_type;

    // Get the default device.
    auto device = backend->device("Default");

    // Create input assembler state.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
            .add();

    inputAssemblerState = std::static_pointer_cast<IInputAssembler>(inputAssembler);

    // Create a geometry render pass.
    UniquePtr<RenderPass> renderPass = device->buildRenderPass("Opaque")
        .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
        .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule("shaders/indirect_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/indirect_fs." + FileExtensions<TRenderBackend>::SHADER);

    // Create a render pipeline.
    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
        .inputAssembler(inputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::BackFaces)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f)
            .depthState(DepthStencilState::DepthState{ .Operation = CompareOperation::LessEqual }))
        .layout(shaderProgram->reflectPipelineLayout())
        .shaderProgram(shaderProgram);

    // Create culling pre-pass pipeline.
    SharedPtr<ShaderProgram> cullProgram = device->buildShaderProgram()
        .withComputeShaderModule("shaders/indirect_cull_cs." + FileExtensions<TRenderBackend>::SHADER);

    UniquePtr<ComputePipeline> cullPipeline = device->buildComputePipeline("Cull")
        .layout(cullProgram->reflectPipelineLayout())
        .shaderProgram(cullProgram);

    // Add the resources to the device state.
    device->state().add(std::move(renderPass));
    device->state().add(std::move(renderPipeline));
    device->state().add(std::move(cullPipeline));
}

void SampleApp::initBuffers(IRenderBackend* backend)
{
    // Get a command buffer
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);

    // Create the staging buffer.
    // NOTE: The mapping works, because vertex and index buffers have an alignment of 0, so we can treat the whole buffer as a single element the size of the 
    //       whole buffer.
    auto stagedVertices = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);

    // Create the actual vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", *m_inputAssembler->vertexBufferLayout(0), BufferUsage::Resource, vertices.size());
    commandBuffer->transfer(asShared(std::move(stagedVertices)), *vertexBuffer, 0, 0, vertices.size());

    // Create the staging buffer for the indices. For infos about the mapping see the note about the vertex buffer mapping above.
    auto stagedIndices = m_device->factory().createIndexBuffer(*m_inputAssembler->indexBufferLayout(), BufferUsage::Staging, indices.size());
    stagedIndices->map(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), 0);

    // Create the actual index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), BufferUsage::Resource, indices.size());
    commandBuffer->transfer(asShared(std::move(stagedIndices)), *indexBuffer, 0, 0, indices.size());

    // Initialize the camera buffer.
    // NOTE: Since we bind the same resource to pipelines of different type (compute and graphics), we need two descriptor sets targeting the same buffers.
    auto& cullPipeline = m_device->state().pipeline("Cull");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& cameraCullBindingLayout = cullPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
    auto& cameraGeometryBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
    auto cameraBuffer = m_device->factory().createBuffer("Camera Buffer", cameraGeometryBindingLayout, 0, BufferUsage::Dynamic, 3);
    auto cameraCullBindings = cameraCullBindingLayout.allocateMultiple(3, {
        { { .resource = *cameraBuffer, .firstElement = 0, .elements = 1 } },
        { { .resource = *cameraBuffer, .firstElement = 1, .elements = 1 } },
        { { .resource = *cameraBuffer, .firstElement = 2, .elements = 1 } }
    });
    auto cameraGeometryBindings = cameraGeometryBindingLayout.allocateMultiple(3, {
        { { .resource = *cameraBuffer, .firstElement = 0, .elements = 1 } },
        { { .resource = *cameraBuffer, .firstElement = 1, .elements = 1 } },
        { { .resource = *cameraBuffer, .firstElement = 2, .elements = 1 } }
    });

    // Next, we create the objects buffer.
    auto& objectsCullBindingLayout = cullPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto& objectsGeometryBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto objectsStagingBuffer = m_device->factory().createBuffer(objectsGeometryBindingLayout, 0, BufferUsage::Staging, sizeof(ObjectBuffer) * NUM_INSTANCES, 1, false);
    auto objectsBuffer = m_device->factory().createBuffer("Objects Buffer", objectsGeometryBindingLayout, 0, BufferUsage::Resource, sizeof(ObjectBuffer) * NUM_INSTANCES, 1);
    auto objectsCullBinding = objectsCullBindingLayout.allocate({ { .resource = *objectsBuffer } });
    auto objectsGeometryBinding = objectsGeometryBindingLayout.allocate({ { .resource = *objectsBuffer } });

    objectsStagingBuffer->map(objects, sizeof(ObjectBuffer) * NUM_INSTANCES);
    commandBuffer->transfer(asShared(std::move(objectsStagingBuffer)), *objectsBuffer);

    // Create a buffer for recording the indirect draw calls.
    // NOTE: Reflection cannot determine, that the buffer records indirect commands, so we need to explicitly state the usage.
    auto& indirectBindingLayout = cullPipeline.layout()->descriptorSet(DescriptorSets::Indirect);
    auto indirectCounterBuffer = m_device->factory().createBuffer("Indirect Counter", BufferType::Indirect, BufferUsage::Dynamic, sizeof(UInt32), 3, true);
    auto indirectCommandsBuffer = m_device->factory().createBuffer("Indirect Commands", BufferType::Indirect, BufferUsage::Resource, sizeof(IndirectIndexedBatch) * NUM_INSTANCES, 3, true);
    auto indirectBindings = indirectBindingLayout.allocateMultiple(3, {
        { { .resource = *indirectCounterBuffer, .firstElement = 0, .elements = 1 }, { .resource = *indirectCommandsBuffer, .firstElement = 0, .elements = 1 } },
        { { .resource = *indirectCounterBuffer, .firstElement = 1, .elements = 1 }, { .resource = *indirectCommandsBuffer, .firstElement = 1, .elements = 1 } },
        { { .resource = *indirectCounterBuffer, .firstElement = 2, .elements = 1 }, { .resource = *indirectCommandsBuffer, .firstElement = 2, .elements = 1 } }
    });

    // End and submit the command buffer.
    m_transferFence = commandBuffer->submit();
    
    // Add everything to the state.
    m_device->state().add(std::move(vertexBuffer));
    m_device->state().add(std::move(indexBuffer));
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(objectsBuffer));
    m_device->state().add(std::move(indirectCounterBuffer));
    m_device->state().add(std::move(indirectCommandsBuffer));
    m_device->state().add("Objects Cull Bindings", std::move(objectsCullBinding));
    m_device->state().add("Objects Geometry Bindings", std::move(objectsGeometryBinding));
    std::ranges::for_each(cameraCullBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Camera Cull Bindings {0}", i++), std::move(binding)); });
    std::ranges::for_each(cameraGeometryBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Camera Geometry Bindings {0}", i++), std::move(binding)); });
    std::ranges::for_each(indirectBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Indirect Bindings {0}", i++), std::move(binding)); });
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer, UInt32 backBuffer) const
{
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
    const float speed = 0.3f;

    glm::vec3 position = { 0.0f, 0.0f, 0.35f };
    glm::vec3 target   = { std::sinf(time * speed), std::cosf(time * speed), 0.0f };
    glm::vec3 forward  = glm::normalize(target - position);
    glm::vec3 right    = glm::normalize(glm::cross({ 0.0f, 0.0f, 1.0f }, forward));
    glm::vec3 up       = glm::normalize(glm::cross(forward, right));
    const float nearPlane = 0.0001f, farPlane = 1000.0f;

    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(position, target, up);
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
    camera.ViewProjection = projection * view;
    camera.Projection = projection;
    camera.Position = glm::vec4(position, 1.0f);
    camera.Forward = glm::vec4(forward, 1.0f);
    camera.Up = glm::vec4(up, 1.0f);
    camera.Right = glm::vec4(right, 1.0f);
    camera.NearPlane = nearPlane;
    camera.FarPlane = farPlane;
    
    // Compute frustum side planes.
    auto projectionTransposed = glm::transpose(projection);
    glm::vec4 frustumX = ::normalizePlane(projectionTransposed[3] + projectionTransposed[0]);
    glm::vec4 frustumY = ::normalizePlane(projectionTransposed[3] + projectionTransposed[1]);

    camera.Frustum[0] = frustumX.x;
    camera.Frustum[1] = frustumX.z;
    camera.Frustum[2] = frustumY.y;
    camera.Frustum[3] = frustumY.z;

    // Create a staging buffer and use to transfer the new uniform buffer to.
    buffer.map(reinterpret_cast<const void*>(&camera), sizeof(camera), backBuffer);
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
        auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->resize(width, height); 
    });

    ::glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->keyDown(key, scancode, action, mods);
    });

    // Initialize objects.
    ::initializeObjects();

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this]<typename TBackend>(TBackend * backend) {
        // Store the window handle.
        auto window = m_window.get();

        // Get the proper frame buffer size.
        int width, height;
        ::glfwGetFramebufferSize(window, &width, &height);

        // Create viewport and scissors.
        m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3);

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend * backend) {
        backend->releaseDevice("Default");
    };

#ifdef LITEFX_BUILD_VULKAN_BACKEND
    // Register the Vulkan backend de-/initializer.
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    // We do not need to provide a root signature for shader reflection (refer to the project wiki for more information: https://github.com/crud89/LiteFX/wiki/Shader-Development).
    DirectX12ShaderProgram::suppressMissingRootSignatureWarning();

    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
}

void SampleApp::onResize(const void* sender, ResizeEventArgs e)
{
    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(e.width(), e.height());
    m_device->swapChain().reset(surfaceFormat, renderArea, 3);

    // NOTE: Important to do this in order, since dependencies (i.e. input attachments) are re-created and might be mapped to images that do no longer exist when a dependency
    //       gets re-created. This is hard to detect, since some frame buffers can have a constant size, that does not change with the render area and do not need to be 
    //       re-created. We should either think of a clever implicit dependency management for this, or at least document this behavior!
    m_device->state().renderPass("Opaque").resizeFrameBuffers(renderArea);

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));
}

void SampleApp::keyDown(int key, int scancode, int action, int mods)
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
            int monitorCount;

            ::glfwGetWindowPos(m_window.get(), &clientRect.x(), &clientRect.y());
            ::glfwGetWindowSize(m_window.get(), &clientRect.width(), &clientRect.height());
            auto monitors = ::glfwGetMonitors(&monitorCount);
            int highestOverlap = 0;

            for (int i(0); i < monitorCount; ++i)
            {
                auto monitor = monitors[i];
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
    title << this->name() << " | " << "Backend: " << this->activeBackend(BackendType::Rendering)->name() << " | " << static_cast<UInt32>(1000.0f / frameTime) << " FPS";

    ::glfwSetWindowTitle(m_window.get(), title.str().c_str());
    lastTime = std::chrono::high_resolution_clock::now();
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& renderPass = m_device->state().renderPass("Opaque");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& cullPipeline = m_device->state().pipeline("Cull");
    auto& cameraBuffer = m_device->state().buffer("Camera Buffer");
    auto& cameraGeometryBindings = m_device->state().descriptorSet(fmt::format("Camera Geometry Bindings {0}", backBuffer));
    auto& cameraCullBindings = m_device->state().descriptorSet(fmt::format("Camera Cull Bindings {0}", backBuffer));
    auto& indirectCounterBuffer = m_device->state().buffer("Indirect Counter");
    auto& indirectCommandsBuffer = m_device->state().buffer("Indirect Commands");
    auto& indirectBindings = m_device->state().descriptorSet(fmt::format("Indirect Bindings {0}", backBuffer));
    auto& vertexBuffer = m_device->state().vertexBuffer("Vertex Buffer");
    auto& indexBuffer = m_device->state().indexBuffer("Index Buffer");
    auto& objectsGeometryBindings = m_device->state().descriptorSet("Objects Geometry Bindings");
    auto& objectsCullBindings = m_device->state().descriptorSet("Objects Cull Bindings");

    // Wait for all transfers to finish.
    auto& queue = renderPass.commandQueue();
    queue.waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);

    // Create a command buffer to execute the cull pass on.
    auto cullCommands = queue.createCommandBuffer(true);

    // Start by updating the camera.
    this->updateCamera(*cullCommands, cameraBuffer, backBuffer);

    // Clear the counter.
    static const UInt32 zero = 0u;
    indirectCounterBuffer.map(&zero, sizeof(UInt32), backBuffer);

    // Bind cull pipeline and all descriptor sets.
    cullCommands->use(cullPipeline);
    cullCommands->bind(cameraCullBindings);
    cullCommands->bind(objectsCullBindings);
    cullCommands->bind(indirectBindings);

    // Dispatch cull pass.
    cullCommands->dispatch({ NUM_INSTANCES / 128, 1, 1 });
    //cullCommands->dispatch({ 1, 1, 1 });

    // Submit the cull pass commands.
    queue.submit(cullCommands);

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    renderPass.begin(backBuffer);
    auto commandBuffer = renderPass.activeFrameBuffer().commandBuffer(0);
    commandBuffer->use(geometryPipeline);
    commandBuffer->setViewports(m_viewport.get());
    commandBuffer->setScissors(m_scissor.get());

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind(cameraGeometryBindings);
    commandBuffer->bind(objectsGeometryBindings);

    // Bind the vertex and index buffers.
    commandBuffer->bind(vertexBuffer);
    commandBuffer->bind(indexBuffer);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->drawIndexedIndirect(indirectCommandsBuffer, indirectCounterBuffer, backBuffer * indirectCommandsBuffer.alignedElementSize(), backBuffer * indirectCounterBuffer.alignedElementSize());
    renderPass.end();
}