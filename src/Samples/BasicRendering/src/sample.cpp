#include "sample.h"

#include <vulkan/vulkan.h>
#include <CLI/CLI.hpp>
#include <iostream>

SampleApp::SampleApp() : App()
{
}

int SampleApp::start(int argc, char** argv)
{
	// Parse the command line parameters.
	String appName = this->getName();
	CLI::App app { "Demonstrates basic drawing techniques.", appName };
	auto validationLayerOption = app.add_option("-v,--validationLayer", "Adds a Vulkan validation layer.")->take_all();
	
	try 
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& ex)
	{
		return app.exit(ex);
	}

	// Turn the validation layers into a list.
	Array<String> validationLayers;

	if (validationLayerOption->count() > 0)
		for each (auto & result in validationLayerOption->results())
			validationLayers.push_back(result);

	// Initialize the rendering backend.
	this->initializeRenderer(validationLayers);

	// Call the event loop.
	return App::start(argc, argv);
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

void SampleApp::initializeRenderer(const Array<String>& validationLayers)
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
	m_renderBackend = makeUnique<VulkanBackend>(*this, requiredExtensions, validationLayers);
}

void SampleApp::createWindow()
{
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = GlfwWindowPtr(::glfwCreateWindow(800, 600, this->getName().c_str(), nullptr, nullptr));
}