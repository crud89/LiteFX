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

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

static struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

static struct TransformBuffer {
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

void SampleApp::allocImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* gpu_handle)
{
    // Get the app and device reference.
    auto app = static_cast<SampleApp*>(context->UserData);
    auto& device = dynamic_cast<const DirectX12Device&>(*app->m_device);

    // Allocate an externally managed descriptor.
    auto [offset, size] = device.allocateGlobalDescriptors(1u, DescriptorHeapType::Resource);

    // Initialize the CPU and GPU handles.
    auto descriptorHandleIncrement = device.handle().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(device.globalBufferHeap()->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), descriptorHandleIncrement);
    CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(device.globalBufferHeap()->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), descriptorHandleIncrement);
    cpu_handle->ptr = targetHandle.ptr;
    gpu_handle->ptr = targetGpuHandle.ptr;

    // Store the descriptor range, so we can properly release it later.
    app->m_d3dDescriptorRanges.emplace(targetHandle.ptr, offset);
}

void SampleApp::releaseImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE /*gpu_handle*/)
{
    // Get the app and device reference.
    auto app = static_cast<SampleApp*>(context->UserData);
    auto& device = dynamic_cast<const DirectX12Device&>(*app->m_device);

    // Lookup the descriptor range.
    auto match = app->m_d3dDescriptorRanges.find(cpu_handle.ptr);

    if (match != app->m_d3dDescriptorRanges.end())
    {
        // Release the descriptor range.
        device.releaseGlobalDescriptors(DescriptorHeapType::Resource, match->second, 1u);
        app->m_d3dDescriptorRanges.erase(cpu_handle.ptr);
    }
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
        .withVertexShaderModule("shaders/imgui_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/imgui_fs." + FileExtensions<TRenderBackend>::SHADER);

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
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& cameraBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBindingLayout, 0, ResourceHeap::Resource);
    auto cameraBindings = cameraBindingLayout.allocate({ { .resource = *cameraBuffer } });

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
    
    // End and submit the command buffer.
    m_transferFence = commandBuffer->submit();
    
    // Add everything to the state.
    m_device->state().add(std::move(vertexBuffer));
    m_device->state().add(std::move(indexBuffer));
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(transformBuffer));
    m_device->state().add("Camera Bindings", std::move(cameraBindings));
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
    // Destroy ImGui context.
    ImGui::DestroyContext();

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

    // Initialize ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableSetMousePos;

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
        static constexpr UInt32 backBuffers = 3u;

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
        auto device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), backBuffers, false));
        m_device = device;

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        // Initialize UI state.
        if constexpr (std::is_same<TBackend, DirectX12Backend>())
        {
            // Setup D3D12 init info.
            ImGui_ImplDX12_InitInfo initInfo = {};
            initInfo.Device = std::as_const(*device).handle().Get();
            initInfo.CommandQueue = device->defaultQueue(QueueType::Graphics).handle().Get();
            initInfo.NumFramesInFlight = backBuffers;
            initInfo.RTVFormat = LiteFX::Rendering::Backends::DX12::getFormat(Format::B8G8R8A8_UNORM);
            initInfo.DSVFormat = LiteFX::Rendering::Backends::DX12::getFormat(Format::D32_SFLOAT);
            initInfo.SrvDescriptorHeap = device->globalBufferHeap();
            initInfo.UserData = this;
            initInfo.SrvDescriptorAllocFn = SampleApp::allocImGuiD3D12DescriptorsCallback;
            initInfo.SrvDescriptorFreeFn = SampleApp::releaseImGuiD3D12DescriptorsCallback;

            // Initialize the ImGui implementation.
            if (!ImGui_ImplDX12_Init(&initInfo) || !ImGui_ImplGlfw_InitForOther(window, true))
                return false;

            // Setup callbacks for UI rendering.
            m_beginUiCallback = []() {
                ImGui_ImplDX12_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            };

            m_endUiCallback = [](const ICommandBuffer& commandBuffer) {
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer).handle().Get());
            };
        }
        else if constexpr (std::is_same<TBackend, VulkanBackend>())
        {
            // The Vulkan backend uses dynamic rendering and ImGui requires us to provide a pipeline rendering info in this case, so we build one from the render pass, that 
            // renders the UI. In our simple example, we only have one render pass, so we use this to set it up.
            auto& renderPass = device->state().renderPass("Opaque");
            auto renderTargets = renderPass.renderTargets();

            auto formats = renderTargets |
                std::views::filter([](auto& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
                std::views::transform([](auto& renderTarget) { return Vk::getFormat(renderTarget.format()); }) |
                std::ranges::to<Array<VkFormat>>();
            auto depthStencilFormats = renderTargets |
                std::views::filter([](auto& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }) |
                std::views::transform([](auto& renderTarget) { return renderTarget.format(); }) |
                std::ranges::to<Array<Format>>();
            auto depthFormat = depthStencilFormats.size() > 0 && ::hasDepth(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;
            auto stencilFormat = depthStencilFormats.size() > 0 && ::hasStencil(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;

            // Setup Vulkan init info.
            ImGui_ImplVulkan_InitInfo initInfo = {};
            initInfo.ApiVersion = VK_API_VERSION_1_3;
            initInfo.Instance = std::as_const(*backend).handle();
            initInfo.PhysicalDevice = std::as_const(*adapter).handle();
            initInfo.Device = std::as_const(*device).handle();
            initInfo.QueueFamily = device->defaultQueue(QueueType::Graphics).familyId();
            initInfo.Queue = device->defaultQueue(QueueType::Graphics).handle();
            // Randomly guessed... not sure how many are required by ImGui.
            initInfo.DescriptorPoolSize = 10u;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            initInfo.MinImageCount = backBuffers;
            initInfo.ImageCount = backBuffers;
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .colorAttachmentCount = static_cast<UInt32>(formats.size()),
                .pColorAttachmentFormats = formats.data(),
                .depthAttachmentFormat = depthFormat,
                .stencilAttachmentFormat = stencilFormat
            };

            // Initialize the ImGui implementation.
            if (!ImGui_ImplVulkan_Init(&initInfo) || !ImGui_ImplGlfw_InitForOther(window, true))
                return false;

            // Setup callbacks for UI rendering.
            m_beginUiCallback = []() {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            };

            m_endUiCallback = [](const ICommandBuffer& commandBuffer) {
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), dynamic_cast<const VulkanCommandBuffer&>(commandBuffer).handle());
            };
        }

        return true;
    };

    auto stopCallback = [this]<typename TBackend>(TBackend* backend) {
        // Wait for the device to finish rendering.
        m_device->wait();

        // Release ImGui.
        if constexpr (std::is_same<TBackend, DirectX12Backend>())
        {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplGlfw_Shutdown();
        }
        else if constexpr (std::is_same<TBackend, VulkanBackend>())
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
        }

        // Release the device.
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

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
    auto& renderPass = m_device->state().renderPass("Opaque");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
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
    commandBuffer->use(geometryPipeline);
    commandBuffer->setViewports(m_viewport.get());
    commandBuffer->setScissors(m_scissor.get());

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

    // Compute world transform and update the transform buffer.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    transformBuffer.map(static_cast<const void*>(&transform), sizeof(transform), backBuffer);

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind({ &cameraBindings, &transformBindings });

    // Bind the vertex and index buffers.
    commandBuffer->bind(vertexBuffer);
    commandBuffer->bind(indexBuffer);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->drawIndexed(indexBuffer.elements());

    // Draw the UI.
    ImGui::GetIO().DisplaySize = { static_cast<float>(m_device->swapChain().renderArea().width()), static_cast<float>(m_device->swapChain().renderArea().height()) };

    // Begin UI rendering.
    m_beginUiCallback();
    ImGui::NewFrame();

    // Show demo window.
    ImGui::ShowDemoWindow();

    // End UI rendering.
    ImGui::Render();
    m_endUiCallback(*commandBuffer);

    // End the render pass.
    renderPass.end();
}