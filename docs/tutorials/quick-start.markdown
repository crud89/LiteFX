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
using namespace LiteFX::Math;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

// Include glfw.
#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
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
#include <array>

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
    this->shutdown += std::bind(&MyApp::onShutdown, this);
    }

private:
    void onInit();
    void onStartup();
    void onShutdown();
};

void MyApp::onStartup()
{
}

void MyApp::onShutdown()
{
}

void MyApp::onInit()
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

If we intent to use the Vulkan backend, glfw may require some extensions to be loaded. We can pass additional extensions to the backend at creation time later. But first, we need to check, if the Vulkan backend is provided. We can do this by evaluating the `LITEFX_BUILD_VULKAN_BACKEND` macro. This macro is only provided, if LiteFX is built with support for the Vulkan backend. You can omit the following code, if you are not intending on using Vulkan, but it is recommended to keep it in anyway to keep you application portable.

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

As you can see, we pass the window pointer as well as another parameter `std::nullopt` to the application constructor. This second parameter is an optional index to a physical adapter. We can force execution on a certain driver here explicitly, however when providing with the default option, the first one returned from the graphics API is used. Depending on the system, this is the one you've selected as the default GPU to use in your system settings.

## Initializing the Engine

Currently, if we start the application, a window will open for a short time and immediately close again. This is expected, as we do not have defined any logic just yet. In LiteFX, an `App` goes through a series of stages, namely *initialization*, *startup* and *shutdown*. Each of those stages invokes event handlers where the actual logic is implemented. We already defined the events and bound event handlers to them. All we have to do is implementing those event handlers in their corresponding methods. We start by implementing the `onInit` method, that we bound to the `initializing` event in the `MyApp` constructor. In this method, we setup everything that's required for the application to run. Besides custom logic, this mainly involves providing start and stop handlers for the backends. Just as the app instance has separate stages it goes through in its lifetime, backends invoke event handlers whenever they are started or stopped. We need to the proper logic for the those handlers. For this, we use a template lambda, that receives a pointer to the backend for which the handler is invoked.

```cxx
// main.cpp
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

Note that there can only be one backend of a certain type be active at a time, the app instance makes sure to call the handlers in the appropriate order. Also note that the first backend we register with an application of each type is automatically started after initialization. Starting a backend involves calling the first handler we are going to implement. In this start handler, we first acquire the current framebuffer size from the glfw window instance, which we use to initialize the drawing region with. We also define a viewport and scissor rectangle over the whole area of the drawing region and store them in a member variable, as we need to access them again later. Finally, we lookup the adapter from the adapter id we provided earlier, create a surface instance and finally create a device, of which we store a pointer. This device is the key object required to allocate any further rendering resources from.

```cxx
// main.cpp
class MyApp : public LiteFX::App {
    // ...
private:
    GLFWwindow m_window;
    Optional<UInt32> m_adapterId;
    SharedPtr<Viewport> m_viewport;
    SharedPtr<Scissor> m_scissor;
    IGraphicsDevice* m_device;

    // ...
};

// ...

void MyApp::onInit()
{
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Get the frame buffer size.
    int width{}, height{};
    ::glfwGetFramebufferSize(m_window, &width, &height);

    // Create viewport and scissors.
    m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Find the adapter, create a surface and initialize the device.
    auto adapter = backend->findAdapter(m_adapterId);

    if (adapter == nullptr)
        adapter = backend->findAdapter(std::nullopt);

    auto surface = backend->createSurface(::glfwGetWin32Window(window));

    // Create the device.
    auto device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

    // ...

    // Store the device and return,
    m_device = device;
    return true;
    };

    // ...
}
```

Note how we only store a pointer to the device instance. We do this, as the lifetime of the device is managed by the backend. In order to release the device when we shutdown the backend, we need to provide a stop handler. This looks similar to the start handler, but has much less logic to it, as all we need to do is to instruct the backend to release our device.

```cxx
// main.cpp
void MyApp::onInit()
{
    // ...

    auto stopCallback = []<typename TBackend>(TBackend* backend) {
    backend->releaseDevice("Default");
    };

    // ...
}
```

## Application Start and Stop

The next step is to implement the main application loop. After the application has been successfully initialized, the startup handler is called. Here, we have to provide the main application loop. As we are using glfw in this example, we can use it to query for any exit events. As long as we did not hit any, all we have to do is to poll the next events from the message pump. Later, we are also going to insert the actual frame drawing logic here, but for now, let's keep it this way to ensure the application remains running until we say otherwise.

```cxx
// main.cpp
void SampleApp::onStartup()
{
    while (!::glfwWindowShouldClose(m_window))
    {
    ::glfwPollEvents();

    // ...
    }
}
```

After leaving the startup handler, the application calls the shutdown handler. In this handler, we tell glfw to destroy our window and terminate the application. Note that the stop handlers for all active backends are called before this handler executes.

```cxx
// main.cpp
void SampleApp::onShutdown()
{
    ::glfwDestroyWindow(m_window);
    ::glfwTerminate();
}
```

Now if we run our application, the window should remain open, but still we did not draw anything into it. Let's change this by drawing a your first triangle!

## Preparation

Every rendering engine requires different resources to be setup before drawing anything to the window. First, we need tell the engine *what* to draw by providing a vertex and an index buffer. Next, we need to tell the GPU *how* to draw our triangle by providing a pipeline state. On top of this, we need to define the *where* (frame buffer) and *when* (render pass) context for the render process. We will take care of the other resources in a bit, but first let's start by defining a data structure for our vertices and providing enough memory for our triangle vertex and index buffers.

```cxx
// main.cpp
struct Vertex {
    Vector4f position;
    Vector4f color;
};

class MyApp : public LiteFX::App {
    // ...
private:
    GLFWwindow m_window;
    Optional<UInt32> m_adapterId;
    SharedPtr<Viewport> m_viewport;
    SharedPtr<Scissor> m_scissor;
    IGraphicsDevice* m_device;

    std::array<Vertex, 3> m_vertices {
    Vertex { { 0.1, 0.1, 1.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
    Vertex { { 0.9, 0.1, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
    Vertex { { 0.5, 0.9, 1.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } }
    };
    std::array<UInt16, 3> m_indices { 0, 1, 2 };

    // ...
};
```

Currently our data only lives on the CPU and in order to draw the triangle, we need to transfer it to the GPU first. However, before we can do this, we need to setup the rest of the renderer first. We start by defining the frame buffer and render pass.

### Frame Buffer and Render Pass

In graphics, the frame buffer is a concept that refers to a set of images that the GPU can draw into. Such images are called *render targets* and a frame buffer can have multiple of them. Different to other engines, the frame buffer in LiteFX is a shared container for all available render targets, including those for multiple "frames in flight" (i.e., frames that are potentially already rendered, before an earlier frame is even displayed). Render targets can have different formats, depending on the information they store. On top of this, a render target has a type that describes for what it is used for. Depending on this type, only a set of formats is valid. LiteFX provides the following types of render targets:

- **Color**: a common image that stores "color" information. However this color may encode different things besides actual color, for example normals or material properties.
- **Depth/Stencil**: a buffer that can have a depth component, a stencil component or both. This buffer is special, as it can be used to selectively discard pixels during rendering, depending on its values.
- **Present**: similar to a color target, but special in a sense that there can only be one present target in a render pass chain. Ending a render pass that contains a present target will submit a present call, so that the contents of this render target will be displayed to the user.

In our simple example we only need to define a present target, as all we want to do is draw the pixels of the triangle onto the screen. We start by defining a render pass that only writes into this render target.

> For more involved examples on how to use frame buffers, check out the [engine samples](https://github.com/crud89/LiteFX/tree/tutorial-rewrite/src/Samples), especially the [render pass sample](https://github.com/crud89/LiteFX/tree/tutorial-rewrite/src/Samples/RenderPasses).

```cxx
// main.cpp
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;

    // ...

    // Create the device.
    auto device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, m_viewport->getRectangle().extent(), 3, false));

    // Create a render pass.
    SharedPtr<RenderPass> renderPass = device->buildRenderPass("Geometry")
    .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });

    // ...

    // Store the device and return,
    m_device = device;
    return true;
};
```

Here we define a render pass called "Geometry" with one present render target called "Color Target". Note that the color format is chosen deliberately, as present targets only support a few selected formats, depending on the display. The B8G8R8A8_UNORM is the default format for non HDR images and is always available. The last two settings define how that render target should be cleared. Clearing refers to the overdrawing of all pixels in the render target image with a constant color when starting the render pass. The very last parameter for the render target is the clear color, which in our case is a very dark gray (with an opaque alpha value).

The render pass itself does not allocate the images that are drawn to. A render target merely describes how the render pass handles the image passed to it for this matter. The images themselves are stored by a frame buffer, which is the next resource we are going to create.

```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;

    // ...

    // Create a render pass.
    SharedPtr<RenderPass> renderPass = device->buildRenderPass("Geometry")
    .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });
    
    // Create the frame buffer.
    auto frameBuffers = std::views::iota(0u, device->swapChain().buffers()) |
    std::views::transform([&](UInt32 index) { return device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea()); }) |
    std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

    std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets()); });
    
    // ...

    // Store the device and return,
    m_device = device;
    return true;
};
```

Here we first create a frame buffer for each frame in the swap chain. The frame buffer itself stores a render area, which we directly pass from the swap chain. It uses this as a default size for images it allocated. After creating the frame buffer, we can call `addImages` on it to create an image for a series of render targets. In our case, we pass the render target of the render pass we created earlier.

### The Shader Program

Now let's begin describing the *how* part of the render pipeline. We start of by creating our first shader program. A shader program is basically a program that runs on the GPU in order to draw our geometry. This for the most part works very different to any program we write on the CPU. For our simple triangle we only take a look at one type of shader program - the graphics program. The graphics pipeline is the oldest pipeline available to GPUs. Historically, it wasn't even programmable but entirely *fixed-function*, meaning that the individual stages could only be influenced by different parameters. Nowadays, we have programmable graphics pipelines, where individual *shader stages* can be programmed using *shader modules*, where each stage can have a module. There are, however, still some fixed-function stages. All the settings (beside the shader program) we are providing here to setup the rendering process are used to configure those stages. A shader program is built from the shader modules provided to it. A graphics program requires at least a vertex and a fragment shader (or pixel shader in DirectX terms) and not more than one shader module must be provided per shader stage.

In modern graphics APIs, shader modules are provided in pre-compiled shader byte-code. Each graphics backend uses their own format for this, but D3D12 will too support SPIR-V in the future, meaning we can re-use the same shaders for Vulkan and DirectX! However, as for now, we need to compile the shaders for each of those backends. LiteFX provides utilities for this in form of CMake macros. To start off, let's first create a new folder within the root directory of our project called `shaders`. Within this folder, create two text files `tutorial_vs.hlsl` (for the vertex shader) and `tutorial_fs.hlsl` (for the fragment shader). Note that we write our shaders in HLSL, as we can compile it into both byte-code variants, which is not trivially possible using GLSL.

To include the shaders into our project, open the CMakeLists.txt file and add the following lines:

```cmake
# CMakeLists.txt

# ...

ADD_EXECUTABLE(MyApp
  "main.h"
  "main.cpp"
)

TARGET_LINK_LIBRARIES(MyApp PRIVATE LiteFX.Backends.Vulkan LiteFX.Backends.DirectX12 glfw)

ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.VS SOURCE "shaders/tutorial_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS SPIRV SHADER_MODEL 6_5 COMPILER DXC)
ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.FS SOURCE "shaders/tutorial_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL 6_5 COMPILER DXC)
ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.VS SOURCE "shaders/tutorial_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL 6_5 COMPILER DXC)
ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.PS SOURCE "shaders/tutorial_fs.hlsl" LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL 6_5 COMPILER DXC)

TARGET_LINK_SHADERS(${PROJECT_NAME} 
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS ${PROJECT_NAME}.Vk.Shaders.VS ${PROJECT_NAME}.Vk.Shaders.FS
)

TARGET_LINK_SHADERS(${PROJECT_NAME} 
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS ${PROJECT_NAME}.Dx.Shaders.VS ${PROJECT_NAME}.Dx.Shaders.PS
)
```

Here, we first declare the shader modules individually, once targeting each byte-code backend. We then tell CMake to link them to our application. This results in the shaders being compiled and then copied over to the binary directory, from which we can load them during application runtime. Let's take a look at the individual shaders.

#### The Vertex Shader

The first shader module we are looking at is the vertex shader. This shader is executed for each individual vertex in the vertex buffer. In this shader module, we first define the data structure of the input vertex data. Note how this is similar to the vertex structure we defined in our program earlier. However, you will also notice the `POSITION` and `COLOR` declarators behind the field names. Those are called *attribute semantics* and inform the GPU of what the data stored within the field refers to.

The second structure defines the output vertex data. In our simple example, each vertex simply outputs its color and position without any transformation. When the fragment shader module is executed later, the GPU interpolates between the values returned for each vertex of the triangle to compute an actual color value that will be passed to the fragment shader.

```hlsl
// tutorial_vs.hlsl
struct VertexInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};

struct VertexData
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    vertex.Position = input.Position;
    vertex.Color = input.Color;
 
    return vertex;
}
```

#### The Fragment/Pixel Shader

The second shader module in our pipeline is the fragment or pixel shader. This shader is executed for each fragment (which is slightly different from an actual pixel, but equivalent without MSAA). Despite defined identical to the output data above, the input data here contains interpolated per-fragment information, as a fragment may be located inside a triangle. The output data is also very simple, as we only return the interpolated fragment color here. The `SV_TARGET` semantic informs the GPU that the value should be stored in the (first) render target.

```hlsl
// tutorial_fs.hlsl
struct VertexData
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

struct FragmentData
{
    float4 Color : SV_TARGET;
};

FragmentData main(VertexData input)
{
    FragmentData fragment;
    fragment.Color = input.Color;
    return fragment;
}
```

#### Setting up the Shader Program

When re-compiling the program, you should now notice how the shaders will be included in the build process. Also note how they are stored within the `shaders` subdirectory of the binary directory, where two versions of each shader exist, one with the `.dxi` extension that stores the DXIL byte-code and one with the `.spv` extension, that stores the SPIR-V byte-code. Back in our application we now need to load the proper shader for the backend. For this, we first check which backend we are currently starting and then store the extension accordingly. After this, we define a shader program using the names of the shader module files we just created.

```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;

    // ...

    // Create a render pass.
    // ...
    
    // Create the frame buffer.
    // ...
    
    // Find the proper file extension.
    String extension;

    if constexpr (std::is_same_v<TBackend, VulkanBackend>)
        extension = "spv";
    else if constexpr (std::is_same_v<TBackend, DirectX12Backend>)
        extension = "dxi";

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule(std::format("shaders/tutorial_vs.{}", extension))
        .withFragmentShaderModule(std::format("shaders/tutorial_fs.{}", extension));

    // ...

    // Store the device and return,
    m_device = device;
    return true;
};
```

### Render Pipeline State

Last on our list is the render pipeline state. This state tells the GPU how to render the geometry. For this, it needs a good amount of information. First, we need to define the input assembler state. The input assembler is part of the fixed-function pipeline and loads the data we provide to the GPU into the render pipeline. Using the input assembler state, we describe how this data is laid out and what the individual values represent.

```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;
    using InputAssembler = TBackend::input_assembler_type;

    // ...

    // Create a render pass.
    // ...
    
    // Create the frame buffer.
    // ...

    // Find the proper file extension.
    // ...

    // Create the shader program.
    // ...
    
    // Create input assembler state.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZW32F, offsetof(Vertex, position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, color), AttributeSemantic::Color)
            .add();

    // ...

    // Store the device and return,
    m_device = device;
    return true;
};
```

Here we tell the GPU that we are providing it a list of triangles, which means that each triplet of vertices in the vertex buffer forms a new triangle. The indices of the vertices define the order of the corners of the triangle and thus the winding order in which the triangle is drawn. This order is important to decide if the triangle is facing towards or away from the camera. The indices themselves are stored in unsigned 16 bit integers. Finally, our vertex buffer contains vertices with two components, each containing 4 dimensional vectors. The first component refers to the position vector, the second to the vertex color.

Next we define the rasterizer state. Just like the input assembler, the rasterizer is part of the fixed-function pipeline and tells the GPU, how to generate fragments from the primitives that were output by the vertex shader invocations.

```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;
    using InputAssembler = TBackend::input_assembler_type;
    using Rasterizer = TBackend::rasterizer_type;

    // ...

    // Create a render pass.
    // ...
    
    // Create the frame buffer.
    // ...

    // Find the proper file extension.
    // ...

    // Create the shader program.
    // ...
    
    // Create input assembler state.
    // ...

    // Create rasterizer state.
    SharedPtr<Rasterizer> rasterizer = device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::BackFaces)
        .cullOrder(CullOrder::CounterClockWise);

    // ...

    // Store the device and return,
    m_device = device;
    return true;
};
```

Here we tell the GPU to create a solid face from the vertices, so that the fragments inside the triangle are filled with color. We also set the cull mode to `BackFaces`, which causes back faces not to be drawn. As mentioned above, the front and back faces are defined by the winding order in the index buffer. We can declare which winding order defines a front-face using the `CullOrder`. Note that in our simple example, we will never actually get to see the back face, but for the sake of further expansion, we keep the setting enabled.

With all pipeline stages configured, we can now finally setup the actual render pipeline state. For the most part, we simply pass in the states created earlier here. However, one important aspect to note here, is that we actually need to describe where additional resources should be bound in the shader. We do this by providing a *pipeline layout*. A layout is required, even if (as in our case), we do not bind any additional resources! Fortunately, LiteFX supports shader reflection, which can create a pipeline layout from a shader program by calling `reflectPipelineLayout` on it.


```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;
    using InputAssembler = TBackend::input_assembler_type;
    using Rasterizer = TBackend::rasterizer_type;
    using RenderPipeline = TBackend::render_pipeline_type;

    // ...

    // Create a render pass.
    // ...
    
    // Create the frame buffer.
    // ...

    // Find the proper file extension.
    // ...

    // Create the shader program.
    // ...
    
    // Create input assembler state.
    // ...

    // Create rasterizer state.
    // ...

    // Create a render pipeline.
    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry Pipeline")
        .inputAssembler(inputAssembler)
        .rasterizer(rasterizer)
        .shaderProgram(shaderProgram)
        .layout(shaderProgram->reflectPipelineLayout());

    // Store the device and return,
    m_device = device;
    return true;
};
```

> Note that in a real-world application you might end up with lots of pipeline state objects. During rendering, switching between different pipeline states can be a costly operation. A good amount of performance potential when using modern graphics APIs is based on making those aspects of rendering explicit for the application. A good practice is to group draw calls by their pipeline state and to minimize the amount of pipelines required in order to minimize context switches.

### Managing state resources

Now that we've setup all state resources (i.e., the pipeline state, render pass, and frame buffer), the startup callback will be left, which would cause them to be released immediately again. As we need them during rendering, we also need to think about where to store the state resources. One option is to store the resources in member variables inside our application class. Another option is to use the *device state* provided by the device we've created earlier. The device state can be accessed by calling the `state` method on the device. By handing over the state resources to the device state, we do not need to take care of managing their lifetimes. We can access the state resources later using the names we provided earlier. Note that we do not need to store the input assembler, rasterizer or shader program states, as those are stored by the pipeline state.

```cxx
auto startCallback = [this]<typename TBackend>(TBackend* backend) {

    // ...

    // Create a render pipeline.
    // ...
    
    // Store state resources in the device state.
    device->state().add(std::move(renderPass));
    device->state().add(std::move(renderPipeline));
    std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });

    // Store the device and return,
    m_device = device;
    return true;
};
```

## Buffer Transfers

Currently the vertex and index buffers reside as member variables of our application instance in CPU memory. Before the GPU can access them, we need to transfer them. We do this by allocating a buffer for each of them before entering the applications main loop and transferring the data into the buffers. Those transfers are issued in a command buffer, which we record once and then let the driver execute it. As this is an asynchronous operation, we need to wait for its execution, before we can use the buffers. We do all of this in the application startup handler, just before entering the main loop.

In order to create the vertex and index buffers, we need to provide their layout, which we already defined earlier as part of the input assembler state. To acquire it, we can retrieve a reference of the pipeline from the device state. As there are different types of pipelines and only graphics pipelines require input assembler and rasterizer states, we need to upcast the reference to the appropriate interface to request the input assembler. We then use this input assembler state to create the vertex and index buffer instances and issue transfer commands to initialize them. Finally, we submit the command buffer and wait for it to be executed on the underlying command queue. After this point we can be sure that the resources are available for rendering.

```cxx
// main.cpp
void SampleApp::onStartup()
{
    // Request the input assembler from the pipeline state.
	auto& geometryPipeline = dynamic_cast<IRenderPipeline&>(m_device->state().pipeline("Geometry Pipeline"));
	auto inputAssembler = geometryPipeline.inputAssembler();

    // Create a new command buffer from the transfer queue.
    auto& transferQueue = m_device->defaultQueue(QueueType::Transfer);
    auto commandBuffer = transferQueue.createCommandBuffer(true);

    // Create the vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(m_vertices.size()));
    commandBuffer->transfer(m_vertices.data(), m_vertices.size() * sizeof(::Vertex), *vertexBuffer, 0, static_cast<UInt32>(m_vertices.size()));

    // Create the index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(m_indices.size()));
    commandBuffer->transfer(m_indices.data(), m_indices.size() * inputAssembler->indexBufferLayout()->elementSize(), *indexBuffer, 0, static_cast<UInt32>(m_indices.size()));

    // Submit the command buffer and wait for its execution.
    auto fence = commandBuffer->submit();
    transferQueue.waitFor(fence);

    // This is the main application loop. Add any per-frame logic below.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        // Poll UI events.
        ::glfwPollEvents();

        // ...
    }
}
```

## Frame Loop

Finally, let's get to the actual rendering loop. The first thing we need to do is swap the back buffer on the swap chain. This operation returns us with an index into our frame buffer, indicating which frame we are going to write. Accordingly, we are retrieving the proper frame buffer instance from the device state. Also, we acquire the render pass, as we need it to execute the actual rendering logic in a moment.

```cxx
// main.cpp
void SampleApp::onStartup()
{
    // ...

    // This is the main application loop. Add any per-frame logic below.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        // Poll UI events.
        ::glfwPollEvents();
        
        // Swap the back buffers for the next frame.
        auto backBuffer = m_device->swapChain().swapBackBuffer();

        // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
        auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
        auto& renderPass = m_device->state().renderPass("Geometry");

        // ...
    }
}
```

A render pass needs to be started by passing the frame buffer into it. Remember how we setup a render target and mapped it to the frame buffer during setup? This way, the render pass can now tell which image to draw into while it executes. The render pass also provides us with a command buffer, we can record all drawing commands into. The advantage of using this command buffer is, that it already sets up synchronization for us. In multi-threaded applications, we can also have multiple command buffers per render pass - one for each thread that should record commands.

Within the command buffer, we setup the pipeline state, telling the GPU how to draw the object (as explained during the setup). We also set the viewport and scissor region we declared earlier. Next, we bind the vertex and index buffer for our geometry and call `drawIndexed` to invoke the actual rendering. Finally, ending the render pass submits the command buffer and issues the present command on the swap chain.

```cxx
// main.cpp
void SampleApp::onStartup()
{
    // ...

    // This is the main application loop. Add any per-frame logic below.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        // ...

        // Begin rendering on the render pass and use the only pipeline we've created for it.
        renderPass.begin(frameBuffer);
        auto commandBuffer = renderPass.commandBuffer(0);
        commandBuffer->use(geometryPipeline);
        commandBuffer->setViewports(m_viewport.get());
        commandBuffer->setScissors(m_scissor.get());

        // Bind the vertex and index buffers.
        commandBuffer->bind(*vertexBuffer);
        commandBuffer->bind(*indexBuffer);

        // Draw the object and present the frame by ending the render pass.
        commandBuffer->drawIndexed(indexBuffer->elements());
        renderPass.end();
    }
}
```

With everything set up and running, we can now execute the program and should see a single triangle in the top-right corner of the window. Feel free to experiment with the vertices and indices to change the shape of the triangle, but take care of the index order or the triangle might be culled!

## Final Words

This bare-bones tutorial should serve as a quick guide on how to get started working with the engine. For a more elaborate example, consult the samples from the [engine sources](https://github.com/crud89/LiteFX/tree/main/src/Samples) or [this standalone sample](https://github.com/crud89/LiteFX-Sample). If you want to quick-start your application, you can use the [project template](https://github.com/crud89/LiteFX-Template). This sets up all the bare-bones infrastructure required to start working with the engine, but it does not provide any code for drawing yet. The full source code for this tutorial can be found [in this repository](https://github.com/crud89/LiteFX-Tutorial). If you have further questions feel free to join our [discussions](https://github.com/crud89/LiteFX/discussions) or open an [issue](https://github.com/crud89/LiteFX/issues) if you encounter any problems. If you want to contribute to the project, feel free to open a [pull request](https://github.com/crud89/LiteFX/pulls).