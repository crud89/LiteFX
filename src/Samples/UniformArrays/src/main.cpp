#define LITEFX_DEFINE_GLOBAL_EXPORTS
#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#include <litefx/litefx.h>

#include "sample.h"

// CLI11 parses optional values as double by default, which yields an implicit-cast warning.
#pragma warning(disable: 4244)

#include <CLI/CLI.hpp>
#include <iostream>
#include <filesystem>
#include <print>

#ifdef LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER
#include <shlobj.h>

bool loadPixCapturer()
{
	// Check if Pix has already been loaded.
	if (::GetModuleHandleW(L"WinPixGpuCapturer.dll") != 0)
		return true;

	// Search for latest version of Pix.
	LPWSTR programFilesPath = nullptr;
	::SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::filesystem::path pixInstallationPath = programFilesPath;
	pixInstallationPath /= "Microsoft PIX";

	std::wstring newestVersionFound;

	for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
		if (directory_entry.is_directory())
			if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
				newestVersionFound = directory_entry.path().filename().c_str();

	if (newestVersionFound.empty())
		return false;

	auto pixPath = pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
#ifdef __cpp_lib_format_path 
	std::println("Found PIX: {0}", pixPath);
#else
	std::println("Found PIX: {0}", pixPath.string());
#endif
	::LoadLibraryW(pixPath.c_str());

	return true;
}
#endif // LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER

#ifdef LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER
RENDERDOC_API_1_5_0* renderDoc = nullptr;

bool loadRenderDocApi()
{
	HMODULE renderDocModule = ::GetModuleHandleW(L"renderdoc.dll");

	if (renderDocModule != 0)
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)::GetProcAddress(renderDocModule, "RENDERDOC_GetAPI");
		int result = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, static_cast<void**>(&::renderDoc));

		return result == 1;
	}

	return false;
}
#endif // LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER

int main(const int argc, const char** argv) // NOLINT(bugprone-exception-escape)
{
#ifdef WIN32
	// Enable console colors.
	HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode = 0;

	if (console == INVALID_HANDLE_VALUE || !::GetConsoleMode(console, &consoleMode))
		return static_cast<int>(::GetLastError());

	::SetConsoleMode(console, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

	// Parse the command line parameters.
	const String appName{ SampleApp::Name() };

	CLI::App commandLine{ "Demonstrates how to setup and use uniform/constant buffer arrays.", appName };
	
	Optional<UInt32> adapterId;
	commandLine.add_option("-a,--adapter", adapterId)->take_first();
	auto validationLayers = commandLine.add_option("-l,--vk-validation-layers")->take_all();

#ifdef LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER
	bool loadPix{ false };
	commandLine.add_option("--dx-load-pix", loadPix)->take_first();
#endif // LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER

#ifdef LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER
	bool loadRenderDoc{ false };
	commandLine.add_option("--load-render-doc", loadRenderDoc)->take_first();
#endif // LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER

	try
	{
		commandLine.parse(argc, argv);
	}
	catch (const CLI::ParseError& ex)
	{
		return commandLine.exit(ex);
	}

#ifdef LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER
	if (loadPix && !loadPixCapturer())
		std::println("No PIX distribution found. Make sure you have installed PIX for Windows.");
#endif // LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER

#ifdef LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER
	if (loadRenderDoc && !loadRenderDocApi())
		std::println("RenderDoc API could not be loaded. Make sure you have version 1.5 or higher installed on your system.");
#endif // LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER

	// Turn the validation layers into a list.
	Array<String> enabledLayers;

	if (validationLayers->count() > 0)
		for (const auto& result : validationLayers->results())
			enabledLayers.push_back(result);

	// Create glfw window.
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

	// Get the required Vulkan extensions from glfw.
	uint32_t extensions = 0;
	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
	Array<String> requiredExtensions;

	for (uint32_t i(0); i < extensions; ++i)
		requiredExtensions.emplace_back(extensionNames[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

	// Create the app.
	try 
	{
		UniquePtr<App> app = App::build<SampleApp>(std::move(window), adapterId)
			.logTo<ConsoleSink>(LogLevel::Trace)
			.logTo<RollingFileSink>("sample.log", LogLevel::Debug)
#ifdef LITEFX_BUILD_VULKAN_BACKEND
			.useBackend<VulkanBackend>(requiredExtensions, enabledLayers)
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
			.useBackend<DirectX12Backend>()
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
			;

		app->run();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "\033[3;41;37mUnhandled exception: " << ex.what() << '\n' << "at: " << ex.trace() << "\033[0m\n";

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}