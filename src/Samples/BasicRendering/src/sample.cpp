#include "sample.h"

void SampleApp::run() 
{
    while (!::glfwWindowShouldClose(m_window.get())) 
    {
    	::glfwPollEvents();
    }

    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}