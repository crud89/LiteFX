#include "sample.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
};

const Array<::Vertex> vertices =
{
    { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    { { 0.5f, 0.5f, 0.0f },  { 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }
};

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
                .make<VulkanBufferLayout>(sizeof(::Vertex))
                    .addAttribute(0, BufferFormat::XYZ32F, offsetof(::Vertex, position))
                    .addAttribute(1, BufferFormat::XYZW32F, offsetof(::Vertex, color))
                    .go()
                .go()
            .go()
        .make<VulkanShaderProgram>()
            .addVertexShaderModule("shaders/default.vert.spv")
            .addFragmentShaderModule("shaders/default.frag.spv")
            //TODO: .make<VulkanDescriptorPool> ? (Collecting UBO layouts here)
            .go()
        .make<VulkanRenderPass>()
            .withColorTarget()
            .go()
        .go();
}

void SampleApp::initBuffers()
{
    m_vertexBuffer = m_pipeline->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    m_vertexBuffer->map(vertices.data(), vertices.size() * sizeof(::Vertex));
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
    renderPass->draw(vertices.size());

    // NOTE: This is actually an asynchronous operation, meaning that it does not wait for the frame to be actually rendered and presented.
    //       We need to implement a way around this, so that there are no race conditions.
    m_pipeline->endFrame();
}