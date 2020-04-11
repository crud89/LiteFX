// CLI11 parses optional values as double by default, which yields an implicit-cast warning.
#pragma warning(disable: 4244)

#include "sample.h"

#include <vulkan/vulkan.h>
#include <CLI/CLI.hpp>
#include <iostream>

SampleApp::SampleApp() noexcept :
	App(Platform::Win32), m_renderBackend(nullptr), m_window(nullptr), m_surface(nullptr), m_device(nullptr)
{
}

int SampleApp::start(const int argc, const char** argv)
{
	Math::Vector4f _test1 = glm::vec4(10.f, 0.f, 0.f, 1.f);
	Math::Vector4f _test2 = DirectX::XMFLOAT4(0.f, 10.f, 0.f, 1.f);
	_test1 = _test2;
	_test2 = glm::vec4(0.f, 0.f, 10.f, 1.f);
	glm::vec4 a = _test1;
	DirectX::XMFLOAT4 b = _test1;
	auto _x = _test1.x();
	auto _y = _test1.y();
	auto _z = _test1.z();
	auto _w = _test1.w();

	Math::Vector<String, 2> strings;
	auto x = strings.x();
	auto y = strings.y();
	//auto z = strings.z();
	//auto w = strings.w();

	Math::Size2d s1;
	Math::Size2d s2(s1);
	Math::Size2d s3;
	//s3 = s1;

	// Parse the command line parameters.
	String appName = this->getName();
	CLI::App app { "Demonstrates basic drawing techniques.", appName };

	auto listValidationLayers{ false }, listAdapters{ false };
	Optional<uint32_t> adapterId;

	auto listCommand = app.add_subcommand("list", "Validation Layers");
	listCommand->add_flag("-l, --layers", listValidationLayers);
	listCommand->add_flag("-a, --adapters", listAdapters);

	auto validationLayers = app.add_option("-l,--layers")->take_all();
	app.add_option("-a,--adapter", adapterId)->take_first();
	
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
	if (listAdapters)
		for each (auto & adapter in m_renderBackend->getAdapters())
			std::cout << "[" << adapter->getDeviceId() << "]: " << adapter->getName() << std::endl;

	// Use either the selected device, or the first one.
	const IGraphicsAdapter* adapter = nullptr;

	try
	{
		adapter = m_renderBackend->getAdapter(adapterId);
	}
	catch (...)
	{
		adapter = m_renderBackend->getAdapter(std::nullopt);
	}

	if (adapter == nullptr)
		throw std::runtime_error("No compatible graphics adapters found.");

	// Create a graphics device for the window surface.
	m_device = adapter->createDevice(m_surface.get());
	auto swapChain = m_device->getSwapChain();

	// Start event loop, if command line parameters do not suggest otherwise.
	if (!listCommand->parsed())
		this->work();

	return EXIT_SUCCESS;
}

int SampleApp::start(const Array<String>& args)
{
	int argc = static_cast<int>(args.size());
	Array<const char*> argv(argc);
	std::generate(argv.begin(), argv.end(), [&args, i = 0] () mutable {
		return args[i++].c_str();
	});

	return this->start(argc, argv.data());
}

void SampleApp::stop()
{
	m_device = nullptr;
	m_surface = nullptr;
	m_renderBackend = nullptr;

	::glfwDestroyWindow(m_window.get());
	::glfwTerminate();
}

void SampleApp::work()
{
	while (!::glfwWindowShouldClose(m_window.get())) 
	{
		::glfwPollEvents();
	}

	this->stop();
}

void SampleApp::initializeRenderer(const Array<String>& validationLayers)
{
	// Create a window.
	this->createWindow();

	// Get the required extensions from glfw.
	uint32_t extensions = 0;
	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
	Array<String> requiredExtensions;

	for (uint32_t i(0); i < extensions; ++i)
		requiredExtensions.push_back(String(extensionNames[i]));

	// Create a rendering backend.
	m_renderBackend = makeUnique<VulkanBackend>(*this, requiredExtensions, validationLayers);

	// Create a surface.
	m_surface = VulkanSurface::createSurface(*m_renderBackend, [&](const VkInstance& instance) {
		VkSurfaceKHR surface;

		if (::glfwCreateWindowSurface(instance, m_window.get(), nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("Unable to create GLFW window surface.");

		return surface;
	});
}

void SampleApp::createWindow()
{
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = GlfwWindowPtr(::glfwCreateWindow(800, 600, this->getName().c_str(), nullptr, nullptr));
}