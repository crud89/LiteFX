# The device

In this part, you start the backends, create a graphics device, and implement the application loop that keeps the window open.

## Backend start and stop handlers

When the application initializes, it calls the `onInit` handler you bound in the constructor. Its main task is to tell each backend what to
do when it is started or stopped. The handlers are the same for all backends, so they are written as template lambdas that receive the
backend being started or stopped:

```cpp title="main.cpp"
void MyApp::onInit()
{
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
        // ...

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend* backend) {
        // ...
    };

#ifdef LITEFX_BUILD_VULKAN_BACKEND
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
}
```

Only one backend of each type can be active at a time, and the application calls the handlers in the correct order when you switch between
backends. After initialization, the application automatically starts the first backend you registered with it.

## Creating the device

The start handler creates the resources that the backend needs to draw. First, it reads the size of the window's drawing area from GLFW and
creates a *viewport* and a *scissor* rectangle that cover all of it. The viewport maps the rendered image to the window, while the scissor
rectangle limits which pixels can be written. Both are needed again when drawing, so store them in member variables.

Next, it selects a GPU (called *adapter*), creates a *surface* for the window, and creates a *device* from both. The device is the central
object of each backend: all other rendering resources are created from it.

```cpp title="main.cpp"
class MyApp : public LiteFX::App {
    // ...

private:
    GlfwWindowPtr m_window;
    Optional<UInt32> m_adapterId;
    SharedPtr<IViewport> m_viewport;
    SharedPtr<IScissor> m_scissor;
    IGraphicsDevice* m_device{ nullptr };

    // ...
};

void MyApp::onInit()
{
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
        // Get the size of the window's drawing area.
        int width{}, height{};
        ::glfwGetFramebufferSize(m_window.get(), &width, &height);

        // Create a viewport and scissor rectangle that cover the whole drawing area.
        m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

        // Select an adapter and create a surface for the window.
        auto adapter = m_adapterId.has_value() ? backend->findAdapter(m_adapterId) : backend->findAdapter(GpuPreference::Performance);
        auto surface = backend->createSurface(::glfwGetWin32Window(m_window.get()));

        // Create the device.
        auto device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface),
            Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

        // ...

        m_device = device;
        return true;
    };

    // ...
}
```

The device is created with a name (`"Default"`), which identifies it within the backend. The remaining arguments configure its *swap chain*,
which provides the images that are shown in the window:

- **The format** of the images. `B8G8R8A8_UNORM` is the default format for images without HDR, and is available on all displays.
- **The size** of the images, which matches the window's drawing area.
- **The number of images** (*back buffers*). With three of them, the GPU can already work on the next frames while the current one is
  displayed.
- **Whether VSync is enabled**, which limits the frame rate to the refresh rate of the display.

The application only stores a pointer to the device, because its lifetime is managed by the backend. The stop handler therefore only asks the
backend to release it:

```cpp title="main.cpp"
void MyApp::onInit()
{
    // ...

    auto stopCallback = []<typename TBackend>(TBackend* backend) {
        backend->releaseDevice("Default");
    };

    // ...
}
```

## The application loop

After initialization, the application calls the `onStartup` handler, which contains the application loop. It keeps running until the window
is closed, and processes the window's events in each iteration. In the last part of the tutorial, you add the drawing code to it.

```cpp title="main.cpp"
void MyApp::onStartup()
{
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        ::glfwPollEvents();

        // ...
    }
}
```

When the loop ends, the application stops all active backends, then calls the `onShutdown` handler. It destroys the window and releases
GLFW:

```cpp title="main.cpp"
void MyApp::onShutdown()
{
    m_window.reset();
    ::glfwTerminate();
}
```

When you run the application now, the window stays open until you close it. It is still empty, though. In the next parts, you prepare
everything that is needed to draw a triangle, starting with the [render pass and frame buffers](render-pass.md).
