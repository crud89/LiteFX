#include "sample.h"

// CLI11 parses optional values as double by default, which yields an implicit-cast warning.
#pragma warning(disable: 4244)

#include <CLI/CLI.hpp>
#include <iostream>

int main(const int argc, const char** argv)
{
	// Parse the command line parameters.
	const String appName = SampleApp::name();

	CLI::App app{ "Demonstrates basic drawing techniques.", appName };

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

	// Turn the validation layers into a list.
	Array<String> enabledLayers;

	if (validationLayers->count() > 0)
		for each (auto & result in validationLayers->results())
			enabledLayers.push_back(result);

	// Create glfw window.
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));
	const auto windowPtr = window.get();

	// Get the required extensions from glfw.
	uint32_t extensions = 0;
	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
	Array<String> requiredExtensions;

	for (uint32_t i(0); i < extensions; ++i)
		requiredExtensions.push_back(String(extensionNames[i]));

	// Create the app.
	try 
	{
		App::build<SampleApp>(std::move(window))
			.useBackend<VulkanBackend>(requiredExtensions, enabledLayers)
				.withAdapterOrDefault(adapterId)
				.withSurface([&windowPtr](const VkInstance& instance) {
					VkSurfaceKHR surface;
					
					if (::glfwCreateWindowSurface(instance, windowPtr, nullptr, &surface) != VK_SUCCESS)
						throw std::runtime_error("Unable to create GLFW window surface.");
					
					return surface;
				}).go()
			//.goFor<SampleApp>();
			.go();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}