#include "sample.h"
#include "config.h"

// CLI11 parses optional values as double by default, which yields an implicit-cast warning.
#pragma warning(disable: 4244)

#include <CLI/CLI.hpp>
#include <iostream>
#include <filesystem>
#include <shlobj.h>

#ifdef BUILD_EXAMPLES_DX12_PIX_LOADER
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
	std::wcout << "Found PIX: " << pixPath.c_str() << std::endl;
	::LoadLibraryW(pixPath.c_str());

	return true;
}
#endif

int main(const int argc, const char** argv)
{
	// Parse the command line parameters.
	const String appName = SampleApp::name();

	CLI::App app{ "Demonstrates push constants usage.", appName };

	Optional<uint32_t> adapterId;
	app.add_option("-a,--adapter", adapterId)->take_first();

#ifdef BUILD_EXAMPLES_DX12_PIX_LOADER
	bool loadPix{ false };
	app.add_option("--load-pix", loadPix)->take_first();
#endif

	try
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& ex)
	{
		return app.exit(ex);
	}

#ifdef BUILD_EXAMPLES_DX12_PIX_LOADER
	if (loadPix && !loadPixCapturer())
		std::cout << "No PIX distribution found. Make sure you have installed PIX for Windows." << std::endl;
#endif

	// Create glfw window.
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));
	
	// Create the app.
	try 
	{
		App::build<SampleApp>(std::move(window), adapterId)
			.logTo<ConsoleSink>(LogLevel::Trace)
			.logTo<RollingFileSink>("sample.log", LogLevel::Debug)
			.useBackend<DirectX12Backend>()
			.go();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "\033[3;41;37mUnhandled exception: " << ex.what() << "\033[0m" << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}