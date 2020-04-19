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
    //    .withTopology(PrimitiveTopology::TriangleStrip)
    //    .add<VulkanViewport>()
    //        //.addScissor(RectF(0, 0, device->getFrameBuffer()->getWidth(), device->getFrameBuffer()->getHeight()))
    //        .go()
    //    .make<VulkanRasterizer>()
    //        .withPolygonMode(PolygonMode::Solid)
    //        .withCullMode(CullMode::BackFaces)
    //        .withCullOrder(CullOrder::CounterClockWise)
    //        .withLineWidth(1.f)
    //        .go()
    //    .use(shaderProgram)
    //    .make<VulkanShaderProgram>()
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