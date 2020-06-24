#include "sample.h"

static void onResize(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void SampleApp::run() 
{
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_UNORM_SRGB);
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

    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame(m_pipeline);
    }

    // Shut down the device.
    m_device->wait();
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
        m_device->resize(width, height);
        //m_pipeline->reset();
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame(UniquePtr<VulkanRenderPipeline>& pipeline)
{
    pipeline->beginFrame();

    // TODO: Write draw logic.

    // NOTE: This is actually an asynchronous operation, meaning that it does not wait for the frame to be actually rendered and presented.
    //       We need to implement a way around this, so that there are no race conditions.
    pipeline->endFrame();
}