#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>

enum DescriptorSets : UInt32 // NOLINT(performance-enum-size, cppcoreguidelines-use-enum-class)
{
    Constant = 0,                                       // All buffers that are immutable.
    PerFrame = 1,                                       // All buffers that are updated each frame.
};

// NOLINTBEGIN(bugprone-throwing-static-initialization)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };

struct Allocation {
    Variant<SharedPtr<IImage>, SharedPtr<IBuffer>> resource;
    UInt32 lifetime{};
};

Array<::Allocation> allocations{};

std::random_device rnd;
std::mt19937 rng{ rnd() };
bool isDefragmenting{ false }, isInDefragmentationPass{ false };
UInt64 renderFence{ 0ull }, defragmentationFence{ 0ull };

static struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

static struct TransformBuffer {
    glm::mat4 World;
} transform;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(bugprone-throwing-static-initialization)

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
template<>
const String FileExtensions<VulkanBackend>::SHADER = "spv"; // NOLINT(bugprone-throwing-static-initialization)
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
template<>
const String FileExtensions<DirectX12Backend>::SHADER = "dxi"; // NOLINT(bugprone-throwing-static-initialization)
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

static inline void setupMoveHandlers(const SharedPtr<const IBuffer>& resource) {
    resource->prepareMove += [buffer = resource->weak_from_this()](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = buffer.lock();

        // NOTE: We can use `ResourceAccess::None` here, because we are running defragmentation on a dedicated queue. There is no previous access to the resource on
        //       that queue anyway. However, this still means that we must synchronize queues, i.e., we need to ensure that other queues are finished with their
        //       accesses to the resources. This is done through fences.
        if (resource)
            e.barrier().transition(*resource, ResourceAccess::None, ResourceAccess::TransferRead);
    };
}

static inline void setupMoveHandlers(const SharedPtr<const IImage>& resource) {
    resource->prepareMove += [image = resource->weak_from_this()](const void* /*sender*/, const IDeviceMemory::PrepareMoveEventArgs& e) {
        auto resource = image.lock();

        // NOTE: Similar to above, access synchronization means queue synchronization. What's different here is that we also need provide a target layout for the image.
        //       Since the image is moved anyway, which requires a copy to be made, we can initialize it from undefined to common layout, clearing it in the process.
        if (resource)
            e.barrier().transition(*resource, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::Common);
    };
}

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
        std::views::transform([&](UInt32 index) { return device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea()); }) |
        std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

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
    SharedPtr<RenderPass> renderPass = device->buildRenderPass("Opaque")
        .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f }) // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

    // Map all render targets to the frame buffer.
    std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets()); });

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule("shaders/defragmentation_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/defragmentation_fs." + FileExtensions<TRenderBackend>::SHADER);

    // Create a render pipeline.
    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
        .inputAssembler(inputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::BackFaces)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f))
        .layout(shaderProgram->reflectPipelineLayout())
        .shaderProgram(shaderProgram);

    // Add the resources to the device state.
    device->state().add(std::move(renderPass));
    device->state().add(std::move(renderPipeline));
    std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });
}

void SampleApp::initBuffers(IRenderBackend* /*backend*/)
{
    // NOTE: We setup the `prepareMove` event for each of the resources allocated below, as we need to make sure they are properly synchronized, in case defragmentation attempts to move them.

    // Get a command buffer
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);

    // Create the vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(vertices.size()));
    commandBuffer->transfer(vertices.data(), vertices.size() * sizeof(::Vertex), *vertexBuffer, 0, static_cast<UInt32>(vertices.size()));
    ::setupMoveHandlers(vertexBuffer);

    // Create the index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(indices.size()));
    commandBuffer->transfer(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), *indexBuffer, 0, static_cast<UInt32>(indices.size()));
    ::setupMoveHandlers(indexBuffer);

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& cameraBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBindingLayout, 0, ResourceHeap::Resource);
    m_cameraBindings = cameraBindingLayout.allocate({ { .resource = *cameraBuffer } });
    ::setupMoveHandlers(cameraBuffer);
    
    // If the camera buffer is moved, we also need to invalidate the descriptor sets that bind it. For this, we set a flag that indicates that the buffer should be rebound. This then
    // happens during `drawFrame`, as we need to synchronize binding updates with rendering.
    cameraBuffer->moving += [this](const void* /*sender*/, const IDeviceMemory::ResourceMovingEventArgs& /*e*/) noexcept { m_rebindCamera = true; };

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer, *cameraBuffer);

    // Next, we create the descriptor sets for the transform buffer. The transform changes with every frame. Since we have three frames in flight, we
    // create a buffer with three elements and bind the appropriate element to the descriptor set for every frame.
    auto& transformBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
    auto transformBuffer = m_device->factory().createBuffer("Transform", transformBindingLayout, 0, ResourceHeap::Dynamic, 3);
    auto transformBindings = transformBindingLayout.allocate(3, {
        { { .resource = *transformBuffer, .firstElement = 0, .elements = 1 } },
        { { .resource = *transformBuffer, .firstElement = 1, .elements = 1 } },
        { { .resource = *transformBuffer, .firstElement = 2, .elements = 1 } }
    }) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();

    // The transform buffer is put onto the dynamic heap. Only resources on the `Resource` heap are moved, so it doesn't need rebinding logic.
    ::setupMoveHandlers(transformBuffer);
    
    // End and submit the command buffer.
    m_transferFence = commandBuffer->submit();
    
    // Add everything to the state.
    m_device->state().add(std::move(vertexBuffer));
    m_device->state().add(std::move(indexBuffer));
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(transformBuffer));
    std::ranges::for_each(transformBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(std::format("Transform Bindings {0}", i++), std::move(binding)); });
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const
{
    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
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

        auto adapter = m_adapterId.has_value() ? backend->findAdapter(m_adapterId) : backend->findAdapter(GpuPreference::Performance);
        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = [this]<typename TBackend>(TBackend* backend) {
        // Release the camera bindings, so that they don't leak.
        m_cameraBindings.reset();

        // Reset allocations.
        allocations.clear();
        isDefragmenting = false;

        // Release the backend device.
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
    // Generate new resources.
    {
        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        static std::uniform_int_distribution<std::mt19937::result_type> imageDice(0, 2); // Generate between 0 and 2 images per frame.
        static std::uniform_int_distribution<std::mt19937::result_type> bufferDice(0, 5); // Generate between 0 and 5 buffers per frame.
        static std::uniform_int_distribution<std::mt19937::result_type> resolutionDice(1, 1024);
        static std::uniform_int_distribution<std::mt19937::result_type> frameDice(1, 10);
        static const size_t maxResources = 1'000u;
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

        // Remove all resources that are too old.
        std::ranges::for_each(allocations, [](auto& allocation) { allocation.lifetime--; });
        std::erase_if(allocations, [](const auto& allocation) { return allocation.lifetime == 0; });

        // Generate new images.
        auto images = imageDice(rng);

        for (UInt32 i{}; i < images && allocations.size() < maxResources; ++i)
        {
            auto& allocation = allocations.emplace_back(m_device->factory().createTexture(Format::R8G8B8A8_SRGB, { resolutionDice(rng), resolutionDice(rng) , 1u }), frameDice(rng));
            ::setupMoveHandlers(std::get<SharedPtr<IImage>>(allocation.resource));
        }

        // Generate new buffers.
        auto buffers = bufferDice(rng);

        for (UInt32 i{}; i < buffers && allocations.size() < maxResources; ++i)
        {
            auto& allocation = allocations.emplace_back(m_device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, resolutionDice(rng)), frameDice(rng));
            ::setupMoveHandlers(std::get<SharedPtr<IBuffer>>(allocation.resource));
        }
    }

    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
    auto& renderPass = m_device->state().renderPass("Opaque");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& transformBuffer = m_device->state().buffer("Transform");
    auto& transformBindings = m_device->state().descriptorSet(std::format("Transform Bindings {0}", backBuffer));
    auto& vertexBuffer = m_device->state().vertexBuffer("Vertex Buffer");
    auto& indexBuffer = m_device->state().indexBuffer("Index Buffer");
    auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
    auto& renderQueue = renderPass.commandQueue();

    // NOTE: Defragmentation is a sequential process. The process is started by calling `beginDefragmentation` and then executes multiple passes. During each of those passes,
    //       resources are copied. In order for the rendering to pick up the up-to-date resources, it must be synchronized with the queue that handles defragmentation. For 
    //       this reason, we need to let the rendering queue wait for the transfer queue.
    //       However, there might still be frames in flight that access the old resources. Those are only deleted in `endDefragmentationPass`, but in order for it not to release
    //       any resources too early, we must wait for the frames in flight up to the point where we started the defragmentation pass to be executed. We do this by remembering 
    //       the fence of the rendering queue that rendered the frame before we submitted the defragmentation pass. When ending the defragmentation pass we first check if the
    //       rendering queue has passed this fence and only then perform the actual ending. We only start a new defragmentation pass, if the previous one has been ended.

    // Begin defragmentation.
    if (!isDefragmenting) {
        m_device->factory().beginDefragmentation(transferQueue, DefragmentationStrategy::Balanced, 0u, 20u); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        isDefragmenting = true;
    }

    // The transfer fence is the latest fence on the transfer queue, the rendering queue needs to wait on: either the resize-fence or the defragmentation fence.
    auto transferFence = m_transferFence;

    if (!isInDefragmentationPass) {
        transferFence = std::max(transferFence, m_device->factory().beginDefragmentationPass());

        // Prevent the next frames to start a new defragmentation pass, if we're still waiting for resources to be released.
        isInDefragmentationPass = true;

        // Remember the fence that marks the last usage of the resources that should be released.
        defragmentationFence = renderFence;
    }

    // Wait for all transfers to finish.
    renderQueue.waitFor(transferQueue, transferFence);

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    renderPass.begin(frameBuffer);
    auto commandBuffer = renderPass.commandBuffer(0);
    commandBuffer->use(geometryPipeline);
    commandBuffer->setViewports(m_viewport.get());
    commandBuffer->setScissors(m_scissor.get());

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

    // Compute world transform and update the transform buffer.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    transformBuffer.map(static_cast<const void*>(&transform), sizeof(transform), backBuffer);

    // Check if we need to rebind the camera buffer after a move.
    if (m_rebindCamera) {
        auto& cameraBuffer = m_device->state().buffer("Camera");
        auto& layout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);

        // Track the current camera bindings, such that they are released when the previous frames are finished.
        commandBuffer->track(std::move(m_cameraBindings));
        
        // Allocate a new binding that can be used from the current frame onward and disable the rebinding marker.
        m_cameraBindings = layout.allocate({ { .resource = cameraBuffer } });
        m_rebindCamera = false;
    }

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind({ m_cameraBindings.get(), &transformBindings});

    // Bind the vertex and index buffers.
    commandBuffer->bind(vertexBuffer);
    commandBuffer->bind(indexBuffer);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->drawIndexed(indexBuffer.elements());
    renderFence = renderPass.end();

    // End defragmentation if the resources that should be released aren't used by any frames in flight anymore.
    if (renderQueue.lastCompletedFence() >= defragmentationFence) {
        isDefragmenting = !m_device->factory().endDefragmentationPass();

        // Allow the next frame to start a new defragmentation pass.
        isInDefragmentationPass = false;
    }
}