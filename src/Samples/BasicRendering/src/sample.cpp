#include "sample.h"

void SampleApp::run() 
{
    auto renderBackend = this->getRenderBackend();
    auto device = renderBackend->build<VulkanDevice>()
        .withFormat(Format::B8G8R8A8_UNORM_SRGB)
        .withQueue(QueueType::Graphics)
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