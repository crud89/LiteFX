#include "sample.h"

void SampleApp::run() 
{
    auto renderBackend = this->getRenderBackend();
    auto device = renderBackend->build<VulkanDevice>()
        .withFormat(Format::B8G8R8A8_UNORM_SRGB)
        .withQueue(QueueType::Graphics)
        .go();

    auto pipeline = device->build<VulkanRenderPipeline>()
        .make<VulkanRenderPipelineLayout>()
            .make<VulkanRasterizer>()
                .withPolygonMode(PolygonMode::Solid)
                .withCullMode(CullMode::BackFaces)
                .withCullOrder(CullOrder::CounterClockWise)
                .withLineWidth(1.f)
                .go()
            .make<VulkanViewport>()
                .withRectangle(RectF(0.f, 0.f, static_cast<Float>(device->getBufferWidth()), static_cast<Float>(device->getBufferHeight())))
                .go()
            .make<VulkanInputAssembler>()
                .go()
            //.make<VulkanShaderProgram>()
            //.use(shaderProgram)
            .go()
        .go();

    while (!::glfwWindowShouldClose(m_window.get()))
        this->handleEvents();

    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();

    // TODO: Write event handlers.
}