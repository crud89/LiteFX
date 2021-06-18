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
using namespace LiteFX::Graphics;
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
		this->initialize();
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

The app model automatically calls `SampleApp::run` as soon as the app is ready. This method is the main method, that should implement the game loop. If it returns, the application will close. In its most simple form, the method can be implemented like this:

```cxx
// TODO: initialize.

// Run application loop until the window is closed.
while (!::glfwWindowShouldClose(window))
{
    ::glfwPollEvents();
	// TODO: draw frame.
}

// TODO: cleanup.

// Destroy the window.
::glfwDestroyWindow(window);
::glfwTerminate();
```

There are three to-dos here:

1. Initialization, includes creating a device, setting up the render passes and pipelines and creating buffers and descriptors.
2. Rendering, which repeatedly draws a frame to a back buffer.
3. Cleanup, which means releasing all resources before destroying the device and quitting the app.

In the following, we will take a closer look into each of the steps.

#### Creating a Device

Before we can do any rendering, we need to create a device. A device is the central instance for each GPU job. Conceptually it represents the hardware interface for the application. It does, however, not represent the actual hardware itself - that's the purpose of *Adapters*. There can be multiple devices for an adapter and there can be devices that make use of different adapters. For our example, we use the default adapter, which is typically the main GPU. To request the default adapter, we pass `std::nullopt` to the `findAdapter` method of the rendering backend of our app.

The next thing we need is a surface, which is a way of telling the device where to render to. The way surfaces are created slightly differs between DirectX 12 and Vulkan. For DirectX 12 a surface is nothing more than a `HWND`. For Vulkan, however, we need to create a surface and we can use *GLFW* for it.

```cxx
auto backend = this->findBackend<VulkanBackend>(BackendType::Rendering);
auto adapter = backend->findAdapter(std::nullopt);

// For Vulkan:
auto surface = backend->createSurface([this](const VkInstance& instance) {
	VkSurfaceKHR surface;
	raiseIfFailed<RuntimeException>(::glfwCreateWindowSurface(instance, m_window, nullptr, &surface), "Unable to create GLFW window surface.");

	return surface;
});

// For DX12:
auto surface = makeUnique<DirectX12Surface>(::glfwGetWin32Window(m_window));
```

With the surface and adapter, we can now proceed to creating our device. Creating a device automatically initializes the *Swap Chain*, which we will talk about in detail later. We can simply create it with a default extent, but it is more efficient to directly tell the swap chain how large the surface is from the beginning. This way, we prevent it from beeing re-created after the window first gets drawn to. In order to do this, we can request the frame buffer size from *GLFW*. Note that the frame buffer size is not always equal to the window size, depending on the monitor. High DPI monitors use a more coarse window coordinate system. You can read about it in more detail [here](https://www.glfw.org/docs/3.3/group__window.html#ga0e2637a4161afb283f5300c7f94785c9).

With the adapter, surface and frame buffer extent, we can go ahead to create our device. We also specify the output format (`Format::B8G8R8A8_SRGB`) and the number of frames, which we concurrently want to draw. This is commonly referred to as *frames in flight*, or *back buffers* throughout the engine, though there is a slight difference. Back buffers refer to the number of frame buffers in the swap chain, whilst frames in flight is a broader concept, that for example influences how many buffers or descriptor sets you want to allocate later.

```cxx
int width, height;
::glfwGetFramebufferSize(m_window, &width, &height);
m_device = backend->createDevice(*adapter, *m_surface, Format::B8G8R8A8_SRGB, Size2d(width, height), 3);
```

We store the device in a variable `m_device`, which we define as a member variable of `SampleApp`, since we are going to make heavy use of it throughout the whole application.

```cxx
UniquePtr<VulkanDevice> m_device;   // or UniquePtr<DirectX12Device>
```

Note how the `createDevice` method returns a `UniquePtr`. Receiving a unique pointer from any call transfers ownership to your application. This means, that from now on, you are responsible for managing the device lifetime and make sure that it gets released properly. We will do this later, when we talk about cleaning up. From now on, all member variables that should be stored are marked with the `m_` prefix and their declaration will not be explicitly mentioned, as long as it does only involve declaring a simple pointer or reference.

#### Creating a Render Pass

Each frame is drawn by at least one render pass. A render pass is a timeline entity of each frame. There can only be one render pass active at one time. Render passes, like everything in modern graphics APIs are executed asynchronously, though. Each call that you do on a render pass records a command to be executed, if the GPU allocates a time slot for it. By itself, the render pass does not store much information and is more an entity that is used to describe the flow of images through the render process of a frame. This is called *Render Graph*. A render graph is described by a set of inter-dependend render passes. 

Note that LiteFX does not automatically build the render graph for you. Instead you are responsible to begin and end each render pass manually, as well as map the input attachments to a render pass. It does, however, provide you with two tools to model the image flow: *Render Targets* and *Input Attachment Mappings*. Render targets describe the output of a render pass. Input attachment mappings do the exact opposite and tell a render pass to use the render target of another render pass as an input. This way, there's an implicit relationship between render passes. You have to end a render pass before you can use its render targets as input attachments for another render pass.

In our example, however, we do not use multiple render passes and instead only create one render pass, that directly draws to our surface. We do this by defining a render pass with one render target, that has the `Present` type. This type is special in a way, that there can only be one present target throughout all render passes. The present target is whats written to the front buffer if a swap chain is swapped. Present targets can also not be used as input attachments. Other render target types are `Color`, which can store arbitrary data and `DepthStencil`, which can only exist once per render pass and stores depth, stencil or depth and stencil data.

The other values that are provided to a render target are:

- The render target format, which in our example is dictated by the swap chain format we've chosen earlier.
- The multisampling level, which we set to `x1`, since we do not yet want to use multi sampling.
- A clear value vector, which contains the values that the render target will be cleared with when starting the render pass. For our *BGRA* image, we want to clear it with black and an alpha value of `0.0`.
- A boolean switch to enable or disable clearing the values, which we set to true, since we want to clear our image with the clear values specified earlier.
- A boolean switch to enable clearing for stencil buffers. This switch is only used, if the render target is a `DepthStencil` target and the format supports stencil values. It can be used to disable clearing stencil values and only clear depth values for depth/stencil targets.
- A boolean switch that states, if we want to preserve the contents of the image after the render pass has finished. Since we do not want to use our render target as input attachment for another render pass, we also set this value to `false`.

```cxx
m_renderPass = m_device->buildRenderPass()
	.renderTarget(RenderTargetType::Present, Format::B8G8R8A8_SRGB, MultiSamplingLevel::x1, { 0.f, 0.f, 0.f, 0.f }, true, false, false)
	.go();
```

#### Creating a Render Pipeline

Next on our list is creating the render pipeline. The singular here is important, because in most applications you will have many pipelines created from one render pass. A pipeline contains the whole state that is used to render a buffer. It tells the GPU how to interpret inputs, how to store the outputs and which shader programs to use. More importantly, it tells the GPU about the shader program parameters and how they are layed out. A pipeline itself is a state object, that can be bound to a render pass. There can only be one pipeline active at a time and since there can also be only one render pass active at a time, there's only one active pipeline at each point in the drawing timeline. Changing the pipeline state may be an expensive operation and you should not do this frequently. Instead, group together objects that can be rendered with the same pipeline and draw all of them before switching to another pipeline.

The first important states, a pipeline stores are the *Viewport* and *Scissor* states. Both tell the GPU which pixels to draw to the output render area, however a scissor does not imply a viewport transform (that is a scaling of the image to fit the viewport). For our example, we set both (the viewport and the scissor) to the whole size of our frame buffer. Note that it is possible to have multiple viewports and scissors. In order to do this, we use the `width` and `height` we've requested earlier.

Furthermore, each pipeline can be assigned an ID, which must be unique for the render pass and a name, which is soley used for debugging purposes. Let's start by creating our viewport and scissor and then continue with building our pipeline state.

```cxx
m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

m_pipeline = m_renderPass->makePipeline(0, "Basic Pipeline")
	.withViewport(m_viewport)
	.withScissor(m_scissor)
```

##### Input Assembler State

Next we tell our pipeline how to handle geometry inputs. Geometry is described by vertices and indices. A geometry has a certain topology, which is described by the `PrimitiveTopology` enumeration. Our geometry is represented as a list of triangles, so we set it to `TriangleList`. Since our geometry vertices must not be ordered, we use an index buffer to tell which vertices to use for triangle rendering. We set the index type to `UInt16`, which has enough space for `65535` vertices and is enough for our example.

Finally, we define out vertex buffer layout. This means, that we tell the input assembler state about the memory layout of a single vertex. We use the `Vertex` object, defined in the `LiteFX::Graphics` namespace, but it is possible to use a custom structure, too. Each property is of the vertex is mapped to an attribute, which describes the memory layout of the property and where to find it within the buffer. Furthermore, it tells the renderer, where the attribute should be bound to. For example, the `Position` property of the vertex object is a 3-component 32-bit float vector, so its format is `XYZ32F`. In the shader, the position is the first element of the vertex, so we set its location to `0`.

```cxx
	.inputAssembler()
		.withTopology(PrimitiveTopology::TriangleList)
		.withIndexType(IndexType::UInt16)
		.addVertexBuffer(sizeof(Vertex), 0)
			.addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
			.addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
			.go()
		.go()
```

#### Rasterizer State

Next, we tell the pipeline about how those primitives (i.e. triangles in our example) should be drawn. We want to draw solid faces, so we set the `PolygonMode` to `Solid`. Another property of the rasterizer state is the face culling state. First, we set the order of vertices, which dictates which side of the primitive is interpreted as *front* and which one is the *back*. We set the `CullOrder` to `ClockWise` to tell the pipeline to treat this ordering as *front face*. Finally, we tell the pipeline not to draw back faces by setting the `CullMode` to `BackFaces`.

```cxx
	.rasterizer()
		.withPolygonMode(PolygonMode::Solid)
		.withCullOrder(CullOrder::ClockWise)
		.withCullMode(CullMode::BackFaces)
		.go()
```

##### Render Pipeline Layout

Each pipeline is defined using a *Pipeline Layout*. The layout stores meta-data about the pipeline state. This includes the shader program to use and the how the buffers are addressed by this shader. We start by defining the shader program, which in our simple example should contain two stages: *Vertex* and *Fragment* shaders (those are also called *Pixel* shaders in DirectX). A program is built from multiple modules, where each module type may only exist once within a program. The modules are loaded from files and must be in a compatible binary format. For Vulkan this format is *SPIR-V*, for DirectX it's *DXIL*. The shaders used in this example are taken from the *BasicRendering* sample of the engine sources.

```cxx
	.layout()
		.shaderProgram()
			.addVertexShaderModule("shaders/basic.vert.spv")		// .dxi for DXIL
			.addFragmentShaderModule("shaders/basic.frag.spv")
			.go()
```

Finally we need to tell the pipeline layout about the buffers that are used by the shader. Buffers are grouped into descriptor sets. Each descriptor set can contain multiple buffers and is visible to a pre-defined range of shader stages. Each buffer is bound to a certain location within the descriptor set. It is a good pracitce to group buffers into descriptor sets, based on update frequency. We have two buffers in our example, that are updated in different frequencies:

- The `CameraBuffer`, which is only updated when the viewport changes (our example camera is static). The camera buffer will be bound to location `0` of set `0` and will be visible to the vertex and fragment shader stages.
- The `TransformBuffer`, which is updated every frame (we want to draw a rotating rectangle). The transform buffer will be bound to location `0` of set `1` and will only be visible to the vertex shader.

```cxx
		.addDescriptorSet(0, ShaderStage::Vertex | ShaderStage::Fragment)
			.addUniform(0, sizeof(CameraBuffer))
			.go()
		.addDescriptorSet(1, ShaderStage::Vertex)
			.addUniform(0, sizeof(TransformBuffer))
			.go()
		.go()	// Build pipeline layout.
	.go();	// Build render pipeline.
```

For more details about buffers and descriptor sets, kindly refer to the [project wiki](https://github.com/crud89/LiteFX/wiki/Resource-Bindings) or read the API documentation about descriptor sets.

#### Creating and Managing Buffers

```
auto inputAssembler = m_pipeline->inputAssembler();
```

### Handling Resize-Events

...