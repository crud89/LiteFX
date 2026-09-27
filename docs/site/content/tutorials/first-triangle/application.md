# The application

Every application that uses LiteFX is built around two concepts:

- **The application object** is a class that derives from `LiteFX::App`. It manages the lifetime of your application and of its backends.
- **Backends** are external systems that the application accesses through a common interface. Currently, all backends are *graphics
  backends*, which implement the interface for a graphics API. LiteFX provides two of them: one for Vulkan and one for DirectX 12. They are
  located in the `LiteFX::Rendering::Backends` namespace.

## The main header

Start with the header file `main.h`, which includes the engine and GLFW:

```cpp title="main.h"
#pragma once

#define LITEFX_DEFINE_GLOBAL_EXPORTS
#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#include <litefx/litefx.h>

#if defined(_WIN32)
#  define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace LiteFX;
using namespace LiteFX::Math;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

// Destroys a GLFW window, when the pointer that owns it is released.
struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) noexcept {
        ::glfwDestroyWindow(window);
    }
};

using GlfwWindowPtr = UniquePtr<GLFWwindow, GlfwWindowDeleter>;
```

Two definitions come before the engine header:

- **`LITEFX_DEFINE_GLOBAL_EXPORTS`** defines symbols that the DirectX 12 runtime looks for in your executable, to load the
  [Agility SDK](https://devblogs.microsoft.com/directx/directx12agility/) that is copied next to your application. If you do not use the
  DirectX 12 backend, you can remove it. 
- **`LITEFX_AUTO_IMPORT_BACKEND_HEADERS`** includes the headers of all backends the engine was built with, so you do not have to include them
  yourself.

!!! warning "Define the exports only once"

    The exports must be defined in exactly one source file of your executable. Only include `main.h` in `main.cpp`. If other files of your
    application need the engine, include `<litefx/litefx.h>` there, without defining `LITEFX_DEFINE_GLOBAL_EXPORTS`.

The header also defines `GlfwWindowPtr`, a smart pointer that destroys the window automatically, when it is no longer needed.

## The application class

Next, define the application class in `main.cpp`. It returns the application name and version, and it takes the window and an optional
adapter ID, which selects the GPU to use:

```cpp title="main.cpp"
#include "main.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <format>
#include <iostream>
#include <ranges>

class MyApp : public LiteFX::App {
public:
    static StringView Name() noexcept { return "My LiteFX App"sv; }
    StringView name() const noexcept override { return Name(); }

    static AppVersion Version() { return AppVersion(1, 0, 0, 0); }
    AppVersion version() const noexcept override { return Version(); }

private:
    GlfwWindowPtr m_window;
    Optional<UInt32> m_adapterId;

public:
    MyApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) :
        App(), m_window(std::move(window)), m_adapterId(adapterId)
    {
        this->initializing += std::bind(&MyApp::onInit, this);
        this->startup += std::bind(&MyApp::onStartup, this);
        this->shutdown += std::bind(&MyApp::onShutdown, this);
    }

private:
    void onInit();
    void onStartup();
    void onShutdown();
};

void MyApp::onInit()
{
}

void MyApp::onStartup()
{
}

void MyApp::onShutdown()
{
}
```

An application goes through three stages: *initializing*, *startup* and *shutdown*. The constructor binds a handler to each of them, which
you implement in the next part of the tutorial.

## Creating the window

The window is created in the `main` function, before the application is built. It first initializes GLFW, then creates a resizable window.
The `GLFW_NO_API` hint tells GLFW not to create an OpenGL context, as the engine manages the graphics API itself.

```cpp title="main.cpp"
int main()
{
    const String appName{ MyApp::Name() };

    if (!::glfwInit())
    {
        std::cerr << "Unable to initialize GLFW.\n";
        return EXIT_FAILURE;
    }

    ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = GlfwWindowPtr(::glfwCreateWindow(800, 600, appName.c_str(), nullptr, nullptr));

    // ...
}
```

The Vulkan backend needs to know which instance extensions GLFW requires to draw into the window. GLFW provides their names, which you pass
to the backend in the next step:

```cpp title="main.cpp"
int main()
{
    // ...

#ifdef LITEFX_BUILD_VULKAN_BACKEND
    uint32_t extensionCount = 0;
    const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensionCount);
    Array<String> requiredExtensions(extensionNames, extensionNames + extensionCount);
#endif // LITEFX_BUILD_VULKAN_BACKEND

    // ...
}
```

`LITEFX_BUILD_VULKAN_BACKEND` is defined, if the engine was built with the Vulkan backend. Similarly, `LITEFX_BUILD_DIRECTX_12_BACKEND` is
defined for the DirectX 12 backend. Checking these macros keeps your application portable between builds with different backends.

## Running the application

Finally, build and run the application. The builder interface configures logging, both to the console and to a log file, and registers the
backends that are available in the current build:

```cpp title="main.cpp"
int main()
{
    // ...

    try
    {
        UniquePtr<App> app = App::build<MyApp>(std::move(window), std::nullopt)
            .logTo<ConsoleSink>(LogLevel::Trace)
            .logTo<RollingFileSink>("myapp.log", LogLevel::Debug)
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
        std::cerr << "Unhandled exception: " << ex.what() << "\nat: " << ex.trace() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

The arguments of `App::build` are passed to the constructor of `MyApp`. The second one, `std::nullopt`, is the adapter ID: without it, the
application picks the most powerful GPU in the system (i.e., the user preference for "high performance" under Windows). The `try`/`catch` 
block handles any exception that the application does not handle itself.

The application now builds and runs. However, the window closes immediately again, as the application does not do anything yet. This is what
the [next part](device.md) changes.
