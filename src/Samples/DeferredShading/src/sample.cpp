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
    { { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f },   { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };

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
        .attachDepthTarget(true, true)
        .attachColorTarget(true)
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
        .addDependency(m_geometryPass.get())
        .attachPresentTarget(true)
        .setPipeline()
            .defineLayout()
                .setShaderProgram()
                    .addVertexShaderModule("shaders/deferred_shading_lighting_pass.vert.spv")
                    .addFragmentShaderModule("shaders/deferred_shading_lighting_pass.frag.spv")
                    .addDescriptorSet(DescriptorSets::PerFrame, ShaderStage::Vertex | ShaderStage::Fragment)
                        .addUniform(0, sizeof(CameraBuffer))
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
}

void SampleApp::initBuffers()
{
    // Create the staging buffer.
    auto stagedVertices = m_geometryPass->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_geometryPass->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    m_vertexBuffer->transferFrom(m_device->getTransferQueue(), stagedVertices.get(), stagedVertices->getSize());

    // Create the staging buffer for the indices.
    auto stagedIndices = m_geometryPass->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagedIndices->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_geometryPass->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    m_indexBuffer->transferFrom(m_device->getTransferQueue(), stagedIndices.get(), stagedIndices->getSize());

    // Create a uniform buffers for the camera and transform information.
    m_perFrameBindings = m_geometryPass->makeBufferPool(DescriptorSets::PerFrame);
    m_cameraBuffer = m_perFrameBindings->makeBuffer(0, BufferUsage::Dynamic);
    m_perObjectBindings = m_geometryPass->makeBufferPool(DescriptorSets::PerInstance);
    m_transformBuffer = m_perObjectBindings->makeBuffer(0, BufferUsage::Dynamic);
}

void SampleApp::run() 
{
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_SRGB);
    this->createRenderPasses();
    this->initBuffers();

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
    m_cameraBuffer = nullptr;
    m_transformBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;

    // Destroy the pipeline and the device.
    m_geometryPass = nullptr;
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
        auto layout = m_geometryPass->getPipeline()->getLayout();
        auto viewport = layout->remove(layout->getViewports().front());
        viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        viewport->getScissors().clear();
        viewport->getScissors().push_back(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        layout->use(std::move(viewport));
        
        // Recreate the pipeline.
        m_geometryPass->reset();
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Begin rendering.
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

    // End the frame.
    m_geometryPass->end(true);
}