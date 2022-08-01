#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

enum DescriptorSets : UInt32
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

struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

struct TransformBuffer {
    glm::mat4 World;
} transform;

template<typename TRenderBackend> requires
    rtti::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef BUILD_VULKAN_BACKEND
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // BUILD_VULKAN_BACKEND
#ifdef BUILD_DIRECTX_12_BACKEND
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // BUILD_DIRECTX_12_BACKEND

template<typename TRenderBackend> requires
    rtti::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IViewport> viewport, SharedPtr<IScissor> scissor, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RenderPass = TRenderBackend::render_pass_type;
    using RenderPipeline = TRenderBackend::render_pipeline_type;
    using PipelineLayout = TRenderBackend::pipeline_layout_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;
    using Rasterizer = TRenderBackend::rasterizer_type;
    using ShaderProgram = TRenderBackend::shader_program_type;

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
        .renderTarget(RenderTargetType::Present, Format::B8G8R8A8_UNORM, { 0.1f, 0.1f, 0.1f, 1.f }, true, false, false)
        .renderTarget(RenderTargetType::DepthStencil, Format::D32_SFLOAT, { 1.f, 0.f, 0.f, 0.f }, true, false, false);

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule("shaders/basic_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/basic_fs." + FileExtensions<TRenderBackend>::SHADER);

    // Create a render pipeline.
    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
        .viewport(viewport)
        .scissor(scissor)
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
}

void SampleApp::initBuffers(IRenderBackend* backend)
{
    // Get a command buffer
    auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);

    // Create the staging buffer.
    // NOTE: The mapping works, because vertex and index buffers have an alignment of 0, so we can treat the whole buffer as a single element the size of the 
    //       whole buffer.
    auto stagedVertices = m_device->factory().createVertexBuffer(m_inputAssembler->vertexBufferLayout(0), BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);
    
    // Create the actual vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", m_inputAssembler->vertexBufferLayout(0), BufferUsage::Resource, vertices.size());
    commandBuffer->transfer(*stagedVertices, *vertexBuffer, 0, 0, vertices.size());

    // Create the staging buffer for the indices. For infos about the mapping see the note about the vertex buffer mapping above.
    auto stagedIndices = m_device->factory().createIndexBuffer(m_inputAssembler->indexBufferLayout(), BufferUsage::Staging, indices.size());
    stagedIndices->map(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout().elementSize(), 0);

    // Create the actual index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", m_inputAssembler->indexBufferLayout(), BufferUsage::Resource, indices.size());
    commandBuffer->transfer(*stagedIndices, *indexBuffer, 0, 0, indices.size());

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& cameraBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
    auto& cameraBufferLayout = cameraBindingLayout.descriptor(0);
    auto cameraStagingBuffer = m_device->factory().createBuffer("Camera Staging", cameraBufferLayout.type(), BufferUsage::Staging, cameraBufferLayout.elementSize(), 1);
    auto cameraBuffer = m_device->factory().createBuffer("Camera", cameraBufferLayout.type(), BufferUsage::Resource, cameraBufferLayout.elementSize(), 1);

    // Allocate the descriptor set and bind the camera buffer to it.
    auto cameraBindings = cameraBindingLayout.allocate();
    cameraBindings->update(cameraBufferLayout.binding(), *cameraBuffer, 0);

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer, *cameraStagingBuffer, *cameraBuffer);

    // Next, we create the descriptor sets for the transform buffer. The transform changes with every frame. Since we have three frames in flight, we
    // create a buffer with three elements and bind the appropriate element to the descriptor set for every frame.
    auto& transformBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::PerFrame);
    auto& transformBufferLayout = transformBindingLayout.descriptor(0);
    auto transformBindings = transformBindingLayout.allocate(3);
    auto transformBuffer = m_device->factory().createBuffer("Transform", transformBufferLayout.type(), BufferUsage::Dynamic, transformBufferLayout.elementSize(), 3);
    std::ranges::for_each(transformBindings, [&transformBufferLayout, &transformBuffer, i = 0](const auto& descriptorSet) mutable { descriptorSet->update(transformBufferLayout.binding(), *transformBuffer, i++); });
    
    // End and submit the command buffer.
    auto fence = m_device->bufferQueue().submit(*commandBuffer);
    m_device->bufferQueue().waitFor(fence);

    // Add everything to the state.
    m_device->state().add(std::move(vertexBuffer));
    m_device->state().add(std::move(indexBuffer));
    m_device->state().add(std::move(cameraStagingBuffer));
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(transformBuffer));
    m_device->state().add("Camera Bindings", std::move(cameraBindings));
    std::ranges::for_each(transformBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Transform Bindings {0}", i++), std::move(binding)); });
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& stagingBuffer, const IBuffer& buffer) const
{
    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    camera.ViewProjection = projection * view;
    stagingBuffer.map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    commandBuffer.transfer(stagingBuffer, buffer);
}

void SampleApp::run() 
{
    // Store the window handle.
    auto window = m_window.get();

    // Get the proper frame buffer size.
    int width, height;
    ::glfwGetFramebufferSize(window, &width, &height);

    // Create viewport and scissors.
    m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this, &window]<typename TBackend>(TBackend* backend) {
        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3);

        // Initialize resources.
        ::initRenderGraph(backend, m_viewport, m_scissor, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend * backend) {
        backend->releaseDevice("Default");
    };

#ifdef BUILD_VULKAN_BACKEND
    // Register the Vulkan backend de-/initializer.
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
#endif // BUILD_VULKAN_BACKEND

#ifdef BUILD_DIRECTX_12_BACKEND
    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
#endif // BUILD_DIRECTX_12_BACKEND

    // Start the first registered rendering backend.
    auto backends = this->getBackends(BackendType::Rendering);
    
    if (!backends.empty())
    {
        this->startBackend(typeid(*backends[0]));

        // Run application loop until the window is closed.
        while (!::glfwWindowShouldClose(window))
        {
            this->handleEvents();
            this->drawFrame();
            this->updateWindowTitle();
        }
    }

    // Destroy the window.
    ::glfwDestroyWindow(window);
    ::glfwTerminate();
}

void SampleApp::initialize()
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
}

void SampleApp::resize(int width, int height)
{
    App::resize(width, height);

    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(width, height);
    m_device->swapChain().reset(surfaceFormat, renderArea, 3);

    // NOTE: Important to do this in order, since dependencies (i.e. input attachments) are re-created and might be mapped to images that do no longer exist when a dependency
    //       gets re-created. This is hard to detect, since some frame buffers can have a constant size, that does not change with the render area and do not need to be 
    //       re-created. We should either think of a clever implicit dependency management for this, or at least document this behavior!
    m_device->state().renderPass("Opaque").resizeFrameBuffers(renderArea);

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Also update the camera.
    auto& cameraStagingBuffer = m_device->state().buffer("Camera Staging");
    auto& cameraBuffer = m_device->state().buffer("Camera");
    auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);
    this->updateCamera(*commandBuffer, cameraStagingBuffer, cameraBuffer);
    auto fence = m_device->bufferQueue().submit(*commandBuffer);
    m_device->bufferQueue().waitFor(fence);
}

void SampleApp::keyDown(int key, int scancode, int action, int mods)
{
#ifdef BUILD_VULKAN_BACKEND
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        this->startBackend<VulkanBackend>();
#endif // BUILD_VULKAN_BACKEND

#ifdef BUILD_DIRECTX_12_BACKEND
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        this->startBackend<DirectX12Backend>();
#endif // BUILD_DIRECTX_12_BACKEND
}

void SampleApp::updateWindowTitle()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - lastTime).count();

    std::stringstream title;
    title << this->name() << " | " << "Backend: " << this->activeBackend(BackendType::Rendering)->name() << " | " << (1000.0f / frameTime) << " FPS";

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
    auto& renderPass = m_device->state().renderPass("Opaque");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& transformBuffer = m_device->state().buffer("Transform");
    auto& cameraBindings = m_device->state().descriptorSet("Camera Bindings");
    auto& transformBindings = m_device->state().descriptorSet(fmt::format("Transform Bindings {0}", backBuffer));
    auto& vertexBuffer = m_device->state().vertexBuffer("Vertex Buffer");
    auto& indexBuffer = m_device->state().indexBuffer("Index Buffer");

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    renderPass.begin(backBuffer);
    auto& commandBuffer = renderPass.activeFrameBuffer().commandBuffer(0);
    commandBuffer.use(geometryPipeline);

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

    // Compute world transform and update the transform buffer.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    transformBuffer.map(reinterpret_cast<const void*>(&transform), sizeof(transform), backBuffer);

    // Bind both descriptor sets to the pipeline.
    commandBuffer.bind(cameraBindings, geometryPipeline);
    commandBuffer.bind(transformBindings, geometryPipeline);

    // Bind the vertex and index buffers.
    commandBuffer.bind(vertexBuffer);
    commandBuffer.bind(indexBuffer);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer.drawIndexed(indexBuffer.elements());
    renderPass.end();
}