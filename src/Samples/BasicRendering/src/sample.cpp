#include "sample.h"

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
	// Create a window.
	this->createWindow();

	// Get the required extensions from glfw.
	uint32_t extensions = 0;
	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
	Array<String> requiredExtensions;

	for (int i(0); i < extensions; ++i)
		requiredExtensions.push_back(String(extensionNames[i]));

	// Create a rendering backend.
	m_renderBackend = makeUnique<VulkanBackend>(*this, requiredExtensions);
}

void SampleApp::createWindow()
{
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = GlfwWindowPtr(::glfwCreateWindow(800, 600, this->getName().c_str(), nullptr, nullptr));
}