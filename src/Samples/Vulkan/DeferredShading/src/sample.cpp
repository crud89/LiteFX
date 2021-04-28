#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

enum DescriptorSets : UInt32
{
    PerFrame = 0,                                       // All buffers that are updated for each frame.
    PerInstance = 1,                                    // All buffers that are updated for each rendered instance.
    VertexData = std::numeric_limits<UInt32>::max()     // Unused, but required to correctly address buffer sets.
};

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f },   { 1.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };

const Array<Vertex> viewPlaneVertices =
{
    { { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { -1.0f, 1.0f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 1.0f, -1.0f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 1.0f, 1.0f, 0.0f },   { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
};

const Array<UInt16> viewPlaneIndices = { 0, 2, 1, 1, 2, 3 };

struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

struct TransformBuffer {
    glm::mat4 World;
} transform;

static void onResize(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void SampleApp::createRenderPasses()
{
    m_geometryPass = m_device->buildRenderPass()
        .attachColorTarget(true)
        .attachDepthTarget(true, true)
        .setPipeline()
            .defineLayout()
                .setShaderProgram()
                    .addVertexShaderModule("shaders/deferred_shading_geometry_pass.vert.spv")
                    .addFragmentShaderModule("shaders/deferred_shading_geometry_pass.frag.spv")
                    .addDescriptorSet(DescriptorSets::PerFrame, ShaderStage::Vertex | ShaderStage::Fragment)
                        .addUniform(0, sizeof(CameraBuffer))
                        .go()
                    .addDescriptorSet(DescriptorSets::PerInstance, ShaderStage::Vertex)
                        .addUniform(0, sizeof(TransformBuffer))
                        .go()
                    .go()
                .setRasterizer()
                    .withPolygonMode(PolygonMode::Solid)
                    .withCullMode(CullMode::BackFaces)
                    .withCullOrder(CullOrder::ClockWise)
                    .withLineWidth(1.f)
                    .go()
                .setInputAssembler()
                    .withTopology(PrimitiveTopology::TriangleList)
                    .withIndexType(IndexType::UInt16)
                    .addVertexBuffer(sizeof(Vertex), 0)
                        .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                        .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                        .go()
                    .go()
                .addViewport()
                    .withRectangle(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .addScissor(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .go()
                .go()
            .go()
        .go();

    m_lightingPass = m_device->buildRenderPass()
        .dependsOn(m_geometryPass.get())
        .attachPresentTarget(true)
        .setPipeline()
            .defineLayout()
                .setShaderProgram()
                    .addVertexShaderModule("shaders/deferred_shading_lighting_pass.vert.spv")
                    .addFragmentShaderModule("shaders/deferred_shading_lighting_pass.frag.spv")
                    .addDescriptorSet(DescriptorSets::PerFrame, ShaderStage::Fragment)
                        .addInputAttachment(0)  // Color attachment
                        .addInputAttachment(1)  // Depth attachment
                        .go()
                    .go()
                .setRasterizer()
                    .withPolygonMode(PolygonMode::Solid)
                    .withCullMode(CullMode::BackFaces)
                    .withCullOrder(CullOrder::ClockWise)
                    .withLineWidth(1.f)
                    .go()
                .setInputAssembler()
                    .withTopology(PrimitiveTopology::TriangleList)
                    .withIndexType(IndexType::UInt16)
                    .addVertexBuffer(sizeof(Vertex), 0)
                        .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                        .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                        .addAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0))
                        .go()
                    .go()
                .addViewport()
                    .withRectangle(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .addScissor(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .go()
                .go()
            .go()
        .go();
}

void SampleApp::initBuffers()
{
    // Create buffers for geometry pass.
    // Create the staging buffer.
    auto stagedVertices = m_geometryPass->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_geometryPass->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    m_vertexBuffer->transferFrom(m_device->bufferQueue(), stagedVertices.get(), stagedVertices->getSize());

    // Create the staging buffer for the indices.
    auto stagedIndices = m_geometryPass->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagedIndices->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_geometryPass->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    m_indexBuffer->transferFrom(m_device->bufferQueue(), stagedIndices.get(), stagedIndices->getSize());

    // Create a uniform buffers for the camera and transform information.
    m_perFrameBindings = m_geometryPass->makeBufferPool(DescriptorSets::PerFrame);
    m_cameraBuffer = m_perFrameBindings->makeBuffer(0, BufferUsage::Dynamic);
    m_perObjectBindings = m_geometryPass->makeBufferPool(DescriptorSets::PerInstance);
    m_transformBuffer = m_perObjectBindings->makeBuffer(0, BufferUsage::Dynamic);

    // Create buffers for lighting pass.
    stagedVertices = m_lightingPass->makeVertexBuffer(BufferUsage::Staging, viewPlaneVertices.size());
    stagedVertices->map(viewPlaneVertices.data(), viewPlaneVertices.size() * sizeof(::Vertex));
    m_viewPlaneVertexBuffer = m_lightingPass->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    m_viewPlaneVertexBuffer->transferFrom(m_device->bufferQueue(), stagedVertices.get(), stagedVertices->getSize());

    // Create the staging buffer for the indices.
    stagedIndices = m_lightingPass->makeIndexBuffer(BufferUsage::Staging, viewPlaneIndices.size(), IndexType::UInt16);
    stagedIndices->map(viewPlaneIndices.data(), viewPlaneIndices.size() * sizeof(UInt16));
    m_viewPlaneIndexBuffer = m_lightingPass->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    m_viewPlaneIndexBuffer->transferFrom(m_device->bufferQueue(), stagedIndices.get(), stagedIndices->getSize());

    // Create the G-Buffer bindings.
    m_gBufferBindings = m_lightingPass->makeBufferPool(DescriptorSets::PerFrame);
}

void SampleApp::run() 
{
    // Get the proper frame buffer size.
    int width, height;
    ::glfwGetFramebufferSize(m_window.get(), &width, &height);

    // Create the device with the initial frame buffer size and triple buffering.
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_SRGB, Size2d(width, height), 3);

    // Initialize resources.
    this->createRenderPasses();
    this->initBuffers();

    // Run application loop until the window is closed.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
    }

    // Shut down the device.
    m_device->wait();

    // Destroy all resources.
    m_perObjectBindings = nullptr;
    m_perFrameBindings = nullptr;
    m_gBufferBindings = nullptr;
    m_cameraBuffer = nullptr;
    m_transformBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_viewPlaneVertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_viewPlaneIndexBuffer = nullptr;

    // Destroy the pipeline and the device.
    m_geometryPass = nullptr;
    m_lightingPass = nullptr;
    m_device = nullptr;

    // Destroy the window.
    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::initialize()
{
    ::glfwSetFramebufferSizeCallback(m_window.get(), ::onResize); 
}

void SampleApp::resize(int width, int height)
{
    App::resize(width, height);

    if (m_device == nullptr)
        return;
    else
    {
        // Resize the frame buffer and recreate the swap chain.
        m_device->resize(width, height);

        // Resize the viewport.
        auto renderPasses = { m_geometryPass.get(), m_lightingPass.get() };

        std::for_each(std::begin(renderPasses), std::end(renderPasses), [width, height](auto renderPass) {
            auto layout = renderPass->getPipeline()->getLayout();
            auto viewport = layout->remove(layout->getViewports().front());
            viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
            viewport->getScissors().clear();
            viewport->getScissors().push_back(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
            layout->use(std::move(viewport));
        
            // Recreate the pipeline.
            renderPass->reset();
        });
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Begin geometry pass.
    m_geometryPass->begin();

    // Update transform buffer.
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
    auto aspectRatio = static_cast<float>(m_device->getBufferWidth()) / static_cast<float>(m_device->getBufferHeight());

    // Compute camera view and projection.
    glm::mat4 view       = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    projection[1][1] *= -1.f;   // Fix GLM clip coordinate scaling.
    camera.ViewProjection = projection * view;
    m_cameraBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    m_perFrameBindings->update(m_cameraBuffer.get());
    m_geometryPass->bind(m_perFrameBindings.get());

    // Draw the model.
    m_geometryPass->bind(m_vertexBuffer.get());
    m_geometryPass->bind(m_indexBuffer.get());
    
    // Compute world transform.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformBuffer->map(reinterpret_cast<const void*>(&transform), sizeof(transform));
    m_perObjectBindings->update(m_transformBuffer.get());
    m_geometryPass->bind(m_perObjectBindings.get());

    // Draw the object.
    m_geometryPass->drawIndexed(indices.size());

    // End geometry pass.
    m_geometryPass->end(false);

    // Begin lighting pass.
    m_lightingPass->begin();

    // Bind the G-Buffer.
    m_gBufferBindings->attach(0, m_geometryPass.get(), 0);
    m_gBufferBindings->attach(1, m_geometryPass.get(), 1);

    // Draw the view plane.
    m_lightingPass->bind(m_viewPlaneVertexBuffer.get());
    m_lightingPass->bind(m_viewPlaneIndexBuffer.get());
    m_lightingPass->bind(m_gBufferBindings.get());
    m_lightingPass->drawIndexed(viewPlaneIndices.size());

    // End lighting pass and present.
    m_lightingPass->end(true);
}