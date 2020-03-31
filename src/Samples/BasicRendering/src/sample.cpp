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

	auto listValidationLayers{ false }, listDevices{ false };

	auto listCommand = app.add_subcommand("list", "Validation Layers");
	listCommand->add_flag("-l, --layers", listValidationLayers);
	listCommand->add_flag("-d, --devices", listDevices);

	auto validationLayers = app.add_option("-l,--layers")->take_all();
	auto deviceId = app.add_option("-d,--device")->take_first();
	
	try 
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& ex)
	{
		return app.exit(ex);
	}

	// Print validation layers.
	if (listValidationLayers)
		for each (auto & layer in VulkanBackend::getValidationLayers())
			std::cout << layer << std::endl;

	// Turn the validation layers into a list.
	Array<String> enabledLayers;

	if (validationLayers->count() > 0)
		for each (auto & result in validationLayers->results())
			enabledLayers.push_back(result);

	// Initialize the rendering backend.
	this->initializeRenderer(enabledLayers);

	// Print device list.
	if (listDevices)
		for each (auto & device in m_renderBackend->getDevices())
			std::cout << device->getDeviceId() << ": " << device->getName() << std::endl;

	// Start event loop, if command line parameters do not suggest otherwise.
	if (!listCommand->parsed())
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