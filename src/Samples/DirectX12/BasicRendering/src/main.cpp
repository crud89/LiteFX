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

	Optional<uint32_t> adapterId;
	app.add_option("-a,--adapter", adapterId)->take_first();

	try
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& ex)
	{
		return app.exit(ex);
	}

	// Create glfw window.
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));
	const auto windowPtr = window.get();

	// Create the app.
	try 
	{
		App::build<SampleApp>(std::move(window))
			.logTo<ConsoleSink>(LogLevel::Trace)
			.logTo<RollingFileSink>("sample.log", LogLevel::Debug)
			.make<DirectX12Backend>()
				.withAdapterOrDefault(adapterId)
		//		.withSurface([&windowPtr](const VkInstance& instance) {
		//			VkSurfaceKHR surface;
		//			
		//			if (::glfwCreateWindowSurface(instance, windowPtr, nullptr, &surface) != VK_SUCCESS)
		//				throw std::runtime_error("Unable to create GLFW window surface.");
		//			
		//			return surface;
		//		}).go()
			.go();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "\033[3;41;37mUnhandled exception: " << ex.what() << "\033[0m" << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}