# Quick Start

This guide walks you through the steps required to write an application that renders a simple primitive. It demonstrates the most important features and use-cases of the LiteFX engine. Before you start, make sure you've successfully setup a project by following the [project setup guide](md_docs_tutorials_project_setup.html).

## Application Infrastructure

At the core of each application that uses LiteFX are two major concepts. First, you must define an object that inherits from `LiteFX::App`. This object manages your applications lifetime as well as backend instances. Backends are the second major concept when working with LiteFX. A backend can be any external system that is accessed through a common interface. At the moment, LiteFX only supports one type of backends (graphics backends), that are used to implement interfaces to graphics APIs. There are two backends available, one for the DirectX 12 and one for Vulkan API. The builtin backends all reside within the namespace `LiteFX::Rendering::Backends`.

Before going further into detail, let's start by writing our main application header file (`main.h`). In this header file, we include every definition required to build a simple application, including the engine and *glfw* headers.

```cxx
// main.h
#pragma once

#define LITEFX_DEFINE_GLOBAL_EXPORTS
#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#include <litefx/litefx.h>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

// Include glfw.
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("No supported surface platform detected.")
#endif
```

Note that before including the core engine header, we setup two definitions:

- `LITEFX_DEFINE_GLOBAL_EXPORTS` is required for exporting symbols that are used to pick up the D3D12 Agility SDK. If you are not using the DirectX 12 backend, or if you are providing a custom version of the SDK you can remove this macro.
- `LITEFX_AUTO_IMPORT_BACKEND_HEADERS` automatically includes the headers of all supported backends for you, so that you do not have to include them on your own.

**Important:** note that when you extent your application, you should not include `main.h` elsewhere. If you include the engine headers from another header, do not re-define the `LITEFX_DEFINE_GLOBAL_EXPORTS` there. The headers must be included in exactly one location when this macro is defined.

The next step is to define an application class that implements `LiteFX::App`. We can do this in the `main.cpp` file, but you might want to move the implementation to another location later. You can take a look at the engine samples for inspiration.

```cxx
// main.cpp
#include "main.h"
#include <iostream>

class MyApp : public LiteFX::App {
public:
	static StringView Name() noexcept { return "My LiteFX App"sv; }
	StringView name() const noexcept override { return Name(); }

	static AppVersion Version() { return AppVersion(1, 0, 0, 0); }
	AppVersion version() const noexcept override { return Version(); }

private:
	GLFWwindow m_window;
	Optional<UInt32> m_adapterId;

public:
	MyApp(GLFWwindow window, Optional<UInt32> adapterId) :
		App(), m_window(window), m_adapterId(adapterId)
	{
		this->initializing += std::bind(&MyApp::onInit, this);
		this->startup += std::bind(&MyApp::onStartup, this);
		this->resized += std::bind(&MyApp::onResize, this, std::placeholders::_1, std::placeholders::_2);
		this->shutdown += std::bind(&MyApp::onShutdown, this);
	}

private:
	void onInit();
	void onStartup();
	void onShutdown();
	void onResize(const void* sender, const ResizeEventArgs& e);
};

void SampleApp::onStartup()
{
}

void SampleApp::onShutdown()
{
}

void SampleApp::onInit()
{
}

void SampleApp::onResize(const void* sender, const ResizeEventArgs& e)
{
}

int main(const int argc, const char* argv[])
{
    return EXIT_SUCCESS;
}
```

Here we define a very basic application object `MyApp`, that returns a name and a version. Furthermore, we already add event listeners to a few events that we are going to implement further down the line. The next step we need to do is to start creating an application window. For this we are going to use *glfw* in this tutorial.

### Setting up the Application Window

In this tutorial, we create the application window within the `main` function of the application. For this, we first store the application name, as we are going to need it again further later. We start by initializing *glfw*, telling it which kind of window we prefer and finally creating a window instance.

```cxx
// main.cpp
int main(const int argc, const char* argv[])
{
	// Store the app name.
	const String appName{ MyApp::Name() };

	// Create glfw window.
	if (!::glfwInit())
		throw std::runtime_error("Unable to initialize glfw.");

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));

    // ...
}
```

If we intent to use the Vulkan backend, glfw may require some extensions to be loaded. We can pass additional extensions to the backend at creation time later. But first, we need to check, if the Vulkan backend is provided. We can do this by evaluating the `LITEFX_BUILD_VULKAN_BACKEND` macro. This macro is only provided, if LiteFX is built with support for the Vulkan backend. You can omit the following code, if you are not intenting on using Vulkan, but it is recommended to keep it in anyway to keep you application portable.

```cxx
// main.cpp
int main(const int argc, const char* argv[])
{
    // ...
#ifdef LITEFX_BUILD_VULKAN_BACKEND
	uint32_t extensions = 0;
	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
	Array<String> requiredExtensions;

	for (uint32_t i(0); i < extensions; ++i)
		requiredExtensions.emplace_back(extensionNames[i]);
#endif // LITEFX_BUILD_VULKAN_BACKEND
    // ...
}
```

Here we request the names of the extensions, required by glfw and add them to an array called `requiredExtensions`. We later pass this array to the Vulkan backend.

### Running the Application

Next we setup and run our application. For this, we use the builder interface to provide the backends we want to use. We wrap everything in a `try`/`catch` block, which is the last exception handler that catches everything we did not handle earlier. We tell the application to use the backends, that are available in the current build. We do this by wrapping the `useBackend` calls with the same macro(s) we used earlier for acquiring the required Vulkan extensions. We also enable logging to both, the console, as well as a *sample.log* file.

```cxx
// main.cpp
int main(const int argc, const char* argv[])
{
    // ...
    try
	{
		UniquePtr<App> app = App::build<MyApp>(window, std::nullopt)
			.logTo<ConsoleSink>(LogLevel::Trace)
			.logTo<RollingFileSink>("sample.log", LogLevel::Debug)
#ifdef LITEFX_BUILD_VULKAN_BACKEND
			.useBackend<VulkanBackend>(requiredExtensions)
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
			.useBackend<DirectX12Backend>()
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
			;

		app->run();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << '\n' << "at: " << ex.trace() << "\n";

		return EXIT_FAILURE;
	}
    // ...
}
```

As you can see, we pass the window pointer as well as another parameter `std::nullopt` to the application constructor. This second parameter is an optional index to a phyiscal adapter. We can force execution on a certain driver here explicitly, however when provoding with the default option, the first one returned from the graphics API is used. Depending on the system, this is the one you've selected as the default GPU to use in your system settings.