#include "SampleApp.h"

#include <vulkan/vulkan.h>
#include <iostream>

SampleApp::SampleApp() : App()
{
}

void SampleApp::start(const Array<String>& args)
{
	this->initializeRenderer();

	App::start(args);
}

void SampleApp::stop()
{
	::glfwDestroyWindow(m_window.get());
	::glfwTerminate();
}

void SampleApp::work()
{
	while (!::glfwWindowShouldClose(m_window.get())) 
	{
		::glfwPollEvents();
	}
}

void SampleApp::initializeRenderer()
{
	this->createWindow();
}

void SampleApp::createWindow()
{
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = GlfwWindowPtr(::glfwCreateWindow(800, 600, this->getName().c_str(), nullptr, nullptr));
}