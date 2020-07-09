#include "sample.h"

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f },  { 0.0f, 1.0f, 1.0f, 1.0f } },
    { { 0.5f, 0.5f, 0.0f },   { 1.0f, 0.0f, 1.0f, 1.0f } },
    { { -0.5f, 0.5f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f } }
};

const Array<UInt16> indices = { 2, 1, 0, 3, 2, 0 };

const struct TransformBuffer {
    glm::mat4 WorldViewProjection;
} transform;

static void onResize(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void SampleApp::createPipeline()
{
    m_pipeline = m_device->build<VulkanRenderPipeline>()
        .make<VulkanRenderPipelineLayout>()
            .make<VulkanRasterizer>()
                .withPolygonMode(PolygonMode::Solid)
                .withCullMode(CullMode::BackFaces)
                .withCullOrder(CullOrder::CounterClockWise)
                .withLineWidth(1.f)
                .go()
            .make<VulkanViewport>()
                .withRectangle(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                //.addScissor(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                .go()
            .make<VulkanInputAssembler>()
                .withTopology(PrimitiveTopology::TriangleList)
                .make<VulkanBufferLayout>(BufferType::Vertex, sizeof(::Vertex), 0)
                    .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                    .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                    .go()
                .make<VulkanBufferLayout>(BufferType::Uniform, sizeof(TransformBuffer), 0)
                    .go()
                .go()
            .go()
        .make<VulkanShaderProgram>()
            .addVertexShaderModule("shaders/default.vert.spv")
            .addFragmentShaderModule("shaders/default.frag.spv")
            .go()
        .make<VulkanRenderPass>()
            .withColorTarget()
            .go()
        .go();
}

void SampleApp::initBuffers()
{
    // Create the staging buffer.
    auto stagingBuffer = m_pipeline->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagingBuffer->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_pipeline->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    stagingBuffer->transfer(m_device->getTransferQueue(), m_vertexBuffer.get(), vertices.size() * sizeof(::Vertex));
    
    // Create the staging buffer for the indices.
    stagingBuffer = m_pipeline->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagingBuffer->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_pipeline->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    stagingBuffer->transfer(m_device->getTransferQueue(), m_indexBuffer.get(), indices.size() * sizeof(UInt16));
}

void SampleApp::run() 
{
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_UNORM_SRGB);
    this->createPipeline();
    this->initBuffers();

    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
    }

    // Shut down the device.
    m_device->wait();

    // Destroy all buffers.
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;

    // Destroy the pipeline and the device.
    m_pipeline = nullptr;
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
        // Resize the device.
        m_device->resize(width, height);

        // Recreate the pipeline.
        this->createPipeline();
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    m_pipeline->beginFrame();

    // Bind the buffers.
    auto renderPass = m_pipeline->getRenderPass();
    m_vertexBuffer->bind(renderPass);
    m_indexBuffer->bind(renderPass);
    renderPass->drawIndexed(indices.size());

    // NOTE: This is actually an asynchronous operation, meaning that it does not wait for the frame to be actually rendered and presented.
    //       We need to implement a way around this, so that there are no race conditions.
    m_pipeline->endFrame();
}