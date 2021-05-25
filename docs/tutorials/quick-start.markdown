# Quick Start

This guide walks you through the steps required to quickly setup a simple application that renders a simple primitive. It demonstrates the most important features and use-cases of the LiteFX engine.

## Setup Project using CMake

...

## Running an Application

At the core of each LiteFX application lies the `Backend`. In theory, an application can provide different back-ends, however currently only one type of back-ends is implemented: the `RenderingBackend`. This back-end comes in two flavors: `VulkanBackend` and `DirectX12Backend`. For now, let's create a simple app, that uses the Vulkan backend and uses [GLFW](https://www.glfw.org/) as a cross-platform window manager:

```cxx
#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>   // Alternatively you can include dx12.hpp here.
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

class SimpleApp : public LiteFX::App {
public:
	String getName() const noexcept override { return "Simple App"; }
	AppVersion getVersion() const noexcept override { return AppVersion(1, 0, 0, 0); }

private:
    const GLFWwindow* m_window;

public:
	SimpleApp(GLFWwindow* window) : 
		App(), m_window(window)
	{
	}

public:
	virtual void initialize() override;
	virtual void run() override;
	virtual void resize(int width, int height) override;
};

static void onResize(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<SimpleApp*>(::glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void SimpleApp::initialize()
{
    ::glfwSetWindowUserPointer(m_window, this);
    ::glfwSetFramebufferSizeCallback(m_window, ::onResize); 
}

void SimpleApp::run() 
{
}

void SimpleApp::resize(int width, int height) 
{
}

int main(const int argc, const char** argv)
{
    // Create glfw window.
	if (!::glfwInit())
    {
        std::cout << "Unable to initialize glfw." << std::endl;
		return EXIT_FAILURE;
    }

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = ::glfwCreateWindow(800, 600, "Simple App", nullptr, nullptr);

    try
    {
        App::build<SimpleApp>(window)
            .logTo<ConsoleSink>(LogLevel::Trace)
            .make<VulkanBackend>()                   // Alternatively, you can use the DirectX12Backend here.
            .go();
    }
    catch (const LiteFX::Exception& ex)
	{
		std::cerr << "An unhandled exception occurred: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
    
    ::glfwDestroyWindow(window);
	return EXIT_SUCCESS;
}
```

Let's take a look at the code together. First, we create a window to paint on using *GLFW*. This is straightforward, but any other window manager can also be used. LiteFX does not make any restrictions on how the rendering surface is created. The only restriction is, that it needs to be compatible with the rendering back-end. However, since it is possible to also request a `HWND` handle from *GLFW* using `glfwGetWin32Window`, we can use *GLFW* it for both backends. The window pointer is passed to the `App::build<SimpleApp>()` call, which forwards the pointer to the `SimpleApp` constructor, where we store it.

Furthermore, we specify the log target (which is completely optional) to be a console window. and tell the app to initialize a `VulkanBackend` for rendering. The `.go()` calls cause the builder to perform the actual object initialization and you will see it frequently when using the fluent API. However, using the fluent builder syntax is also optional - you could in fact create all the instances on your own. All classes follow an [RAII](https://en.cppreference.com/w/cpp/language/raii) idiom, so it is clear from the constructors parameter, which objects are required to be initialized in which order.

Let's go on and take a look at the `SimpleApp` class. It implements the `LiteFX::App` base class, which is an abstract class, that requires us to provide some overrides, that implement the basic application control flow. Most notably, those are:

- `initialize`, which is called by the `LiteFX::App` constructor.
- `run`, which is called by the `AppBuilder`, after the app has been initialized. Note that, if you prefer not using the fluent builder syntax, you would have to call it on your own.
- `resize`, which is called if an resize event occurs.

Since the app itself is agnostic towards the actual window manager, we have to manually invoke the resize event. We do this by storing the application instance pointer using `glfwSetWindowUserPointer` and calling the `resize` method within the *GLFW* framebuffer resize event callback.

### Running the Application

...

### Resizing the Framebuffer

...