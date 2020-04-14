#include "sample.h"

void SampleApp::run() 
{
    auto renderBackend = this->getRenderBackend();
    //auto device = renderBackend->createDevice()

    while (!::glfwWindowShouldClose(m_window.get())) 
    {
    	::glfwPollEvents();
    }

    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}