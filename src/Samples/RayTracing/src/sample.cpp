#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

// Currently there's only one geometry.
#define NUM_GEOMETRIES 1

enum class DescriptorSets : UInt32
{
    StaticData   = 0, // Camera and acceleration structures.
    FrameBuffer  = 1, // The frame buffer descriptor to write into.
    Materials    = 2, // The bind-less material properties array.
    GeometryData = 3  // The shader-local per-geometry data.
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
    glm::mat4 InverseView;
    glm::mat4 InverseProjection;
} camera;

struct MaterialData {
    glm::vec4 Color = { 0.4f, 0.3f, 0.6f, 1.0f };
} materials[NUM_GEOMETRIES];

struct alignas(8) GeometryData {
    UInt32 Index;
    UInt32 Padding[3];
};

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RenderPass = TRenderBackend::render_pass_type;
    using RayTracingPipeline = TRenderBackend::ray_tracing_pipeline_type;
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

    // Create the shader program.
    // NOTE: The hit shader here receives per-invocation data at the descriptor bound to register 0, space/set 1.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withRayGenerationShaderModule("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
        .withClosestHitShaderModule("shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, DescriptorBindingPoint { .Register = 0, .Space = 1 })
        .withMissShaderModule("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER);

    // Build a shader record collection and create a ray-tracing pipeline.
    // NOTE: The local data (payload) for the shader invocation must be defined before building the shader binding table. A shader module may occur multiple times with different 
    // payloads, which can become hard to read and debug, hence it is preferred to use local shader data as sparingly as possible. In this particular case we pass the geometry 
    // index to the shader and since out BLAS (defined later) only contains a single geometry, we only need one entry here. If you only target hardware that supports DXR 1.1 or,
    // you can eliminate the payload entirely by calling the `GeometryIndex()` intrinsic from the shader.
    UniquePtr<RayTracingPipeline> rayTracingPipeline = device->buildRayTracingPipeline("RT Geometry",
        shaderProgram->buildShaderRecordCollection()
            .withShaderRecord("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
            .withShaderRecord("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER)
            .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, GeometryData { .Index = 0 }))
        .maxBounces(4)
        .layout(shaderProgram->reflectPipelineLayout());

    // Add the resources to the device state.
    device->state().add(std::move(rayTracingPipeline));
}

void SampleApp::initBuffers(IRenderBackend* backend)
{
    // Get a command buffer. Note that we use the graphics queue here, as it also supports transfers, but additionally allows us to build acceleration structures.
    auto commandBuffer = m_device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

    // Create the staging buffer.
    // NOTE: The mapping works, because vertex and index buffers have an alignment of 0, so we can treat the whole buffer as a single element the size of the 
    //       whole buffer.
    auto stagedVertices = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);

    // Create the actual vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", *m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, vertices.size(), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(asShared(std::move(stagedVertices)), *vertexBuffer, 0, 0, vertices.size());

    // Create the staging buffer for the indices. For infos about the mapping see the note about the vertex buffer mapping above.
    auto stagedIndices = m_device->factory().createIndexBuffer(*m_inputAssembler->indexBufferLayout(), ResourceHeap::Staging, indices.size());
    stagedIndices->map(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), 0);

    // Create the actual index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, indices.size(), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(asShared(std::move(stagedIndices)), *indexBuffer, 0, 0, indices.size());

    // Pre-build acceleration structures. We start with 1 bottom-level acceleration structure (BLAS) for our simple geometry and a few top-level acceleration structures (TLAS) for the instances.
    // NOTE: If there are more meshes/geometries, we would need to increase `NUM_GEOMETRIES`.
    auto blas = asShared(std::move(m_device->factory().createBottomLevelAccelerationStructure()));
    blas->withTriangleMesh({ asShared(std::move(vertexBuffer)), asShared(std::move(indexBuffer)) });
    blas->allocateBuffer(*m_device);

    auto tlas = m_device->factory().createTopLevelAccelerationStructure("TLAS");
    tlas->withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, -2.0f, 0.0f))), 0, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 0.0f))), 1, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 2.0f, 0.0f))), 2, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, -2.0f, 0.0f))), 3, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f))), 4, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 2.0f, 0.0f))), 5, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, -2.0f, 0.0f))), 6, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f))), 7, 0)
        .withInstance(blas, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 2.0f, 0.0f))), 8, 0);

    tlas->allocateBuffer(*m_device);

    // Create a scratch buffer.
    auto scratchBufferSize = std::max(blas->requiredScratchMemory(), tlas->requiredScratchMemory());
    auto scratchBuffer = asShared(std::move(m_device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, scratchBufferSize, 1, ResourceUsage::AllowWrite)));

    // Build the BLAS and the TLAS. We need to barrier in between both to prevent simultaneous scratch buffer writes.
    commandBuffer->buildAccelerationStructure(*blas, scratchBuffer);
    auto barrier = m_device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
    commandBuffer->barrier(*barrier);
    commandBuffer->buildAccelerationStructure(*tlas, scratchBuffer);

    // TODO: The TLAS allocates an instance buffer that we could release after building the acceleration structure, if we do not need it for future updates.

    // Create a shader binding table from the pipeline and transfer it into a GPU buffer (not necessarily required for such a small SBT, but for demonstration purposes).
    auto& geometryPipeline = dynamic_cast<IRayTracingPipeline&>(m_device->state().pipeline("RT Geometry"));
    auto stagingSBT = geometryPipeline.allocateShaderBindingTable(m_offsets);
    auto shaderBindingTable = m_device->factory().createBuffer("Shader Binding Table", BufferType::ShaderBindingTable, ResourceHeap::Resource, stagingSBT->elementSize(), stagingSBT->elements(), ResourceUsage::TransferDestination);
    commandBuffer->transfer(asShared(std::move(stagingSBT)), *shaderBindingTable, 0, 0, shaderBindingTable->elements());

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    auto& staticDataBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::StaticData));
    auto cameraBuffer = m_device->factory().createBuffer("Camera", staticDataBindingsLayout, 0, ResourceHeap::Resource);
    auto staticDataBindings = staticDataBindingsLayout.allocate({ { .resource = *cameraBuffer }, { .resource = *tlas }});

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer, *cameraBuffer);
        
    // Bind the swap chain back buffers to the ray-tracing pipeline output.
    auto& swapChain = m_device->swapChain();
    auto& outputBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::FrameBuffer));
    auto outputBindings = outputBindingsLayout.allocateMultiple(3, {
        { { .resource = *swapChain.image(0) } },
        { { .resource = *swapChain.image(1) } },
        { { .resource = *swapChain.image(2) } }
    });

    // Bind the material data.
    auto& materialBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Materials));
    auto materialBuffer = m_device->factory().createBuffer("Material Buffer", materialBindingsLayout, 0, ResourceHeap::Dynamic, sizeof(MaterialData), NUM_GEOMETRIES);
    auto materialBindings = materialBindingsLayout.allocate(NUM_GEOMETRIES, { { .resource = *materialBuffer } });
    materialBuffer->map(reinterpret_cast<const void*>(&materials[0]), sizeof(MaterialData));

    // End and submit the command buffer and wait for it to finish.
    auto fence = commandBuffer->submit();
    m_device->defaultQueue(QueueType::Graphics).waitFor(fence);
    
    // Add everything to the state.
    m_device->state().add(std::move(tlas)); // No need to store the BLAS, as it is contained in the TLAS.
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(materialBuffer));
    m_device->state().add(std::move(shaderBindingTable));
    m_device->state().add("Static Data Bindings", std::move(staticDataBindings));
    std::ranges::for_each(outputBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Output Bindings {0}", i++), std::move(binding)); });
    m_device->state().add("Material Bindings", std::move(materialBindings));
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const
{
    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    camera.ViewProjection = projection * view;
    camera.InverseView = glm::inverse(view);
    camera.InverseProjection = glm::inverse(projection);

    // Create a staging buffer and use to transfer the new uniform buffer to.
    auto cameraStagingBuffer = m_device->factory().createBuffer(m_device->state().pipeline("RT Geometry"), std::to_underlying(DescriptorSets::StaticData), 0, ResourceHeap::Staging);
    cameraStagingBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    commandBuffer.transfer(asShared(std::move(cameraStagingBuffer)), buffer);
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

    // Re-bind swap chain back buffers to ray-tracing pipeline output.

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));

    // Also update the camera.
    auto& cameraBuffer = m_device->state().buffer("Camera");
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);
    this->updateCamera(*commandBuffer, cameraBuffer);
    m_transferFence = commandBuffer->submit();
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
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();

    // TODO: Clear back buffer (vkCmdClearColorImage, ClearUnorderedAccessViewFloat).

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& geometryPipeline = m_device->state().pipeline("RT Geometry");
    auto& staticDataBindings = m_device->state().descriptorSet("Static Data Bindings");
    auto& materialBindings = m_device->state().descriptorSet("Material Bindings");
    auto& outputBindings = m_device->state().descriptorSet(fmt::format("Output Bindings {0}", backBuffer));
    auto& shaderBindingTable = m_device->state().buffer("Shader Binding Table");

    // Wait for all transfers to finish.
    auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
    graphicsQueue.beginDebugRegion("Ray-Tracing");
    graphicsQueue.waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);
    auto commandBuffer = graphicsQueue.createCommandBuffer(true);

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    commandBuffer->use(geometryPipeline);
    //commandBuffer->setViewports(m_viewport.get());
    //commandBuffer->setScissors(m_scissor.get());

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

    // TODO: Rotate camera.

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind(staticDataBindings);
    commandBuffer->bind(outputBindings);
    commandBuffer->bind(materialBindings);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->traceRays(m_viewport->getRectangle().width(), m_viewport->getRectangle().height(), 1, m_offsets, shaderBindingTable, &shaderBindingTable, &shaderBindingTable);

    // Present.
    auto fence = graphicsQueue.submit(commandBuffer);
    graphicsQueue.endDebugRegion();
    m_device->swapChain().present(fence);
}