#include "sample.h"

void SampleApp::run() 
{
    auto renderBackend = this->getRenderBackend();
    auto device = renderBackend->createDevice<VulkanDevice>(Format::B8G8R8A8_UNORM_SRGB);
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
                //.addScissor(RectF(0.f, 0.f, static_cast<Float>(device->getBufferWidth()), static_cast<Float>(device->getBufferHeight())))
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
        this->drawFrame(pipeline);
    }

    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();

    // TODO: Write event handlers.
}

void SampleApp::drawFrame(UniquePtr<VulkanRenderPipeline>& pipeline)
{
    pipeline->beginFrame();

    // TODO: Write draw logic.

    // NOTE: This is actually an asynchronous operation, meaning that it does not wait for the frame to be actually rendered and presented.
    //       We need to implement a way around this, so that there are no race conditions.
    pipeline->endFrame();
}