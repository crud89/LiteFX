#include "sample.h"

void SampleApp::run() 
{
    auto renderBackend = this->getRenderBackend();
    auto device = renderBackend->build<VulkanDevice>()
        .withFormat(Format::B8G8R8A8_UNORM_SRGB)
        .withQueue(QueueType::Graphics)
        .go();

    while (!::glfwWindowShouldClose(m_window.get())) 
    {
    	::glfwPollEvents();
    }

    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}