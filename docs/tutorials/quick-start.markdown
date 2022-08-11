# Quick Start

This guide walks you through the steps required to write an application that renders a simple primitive. It demonstrates the most important features and use-cases of the LiteFX engine. Before you start, make sure you've successfully setup a project by following the [project setup guide](md_docs_tutorials_project_setup.html).

## Defining an Application

At the core of each LiteFX application lies the `Backend`. In theory, an application can provide different back-ends, however currently only one type of back-ends is implemented: the `RenderingBackend`. This back-end comes in two flavors: `VulkanBackend` and `DirectX12Backend`. For now, let's create a simple app, that uses the Vulkan backend and uses [GLFW](https://www.glfw.org/) as a cross-platform window manager. In order to do this, we first need to extent the *CMakeLists.txt* file, created in the [project setup guide](md_docs_tutorials_project_setup.html). Add a `FIND_PACKAGE` command below the line where you are searching for LiteFX:

```cmake
FIND_PACKAGE(LiteFX 1.0 CONFIG REQUIRED)
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
```

In order to make GLFW available to your project, you have to also define the dependency using `TARGET_LINK_LIBRARIES`:

```cmake
TARGET_LINK_LIBRARIES(MyLiteFXApp PRIVATE LiteFX.Backends.Vulkan glfw)
```

Also - if you are not using *vcpkg* - you need to make sure, that the *glfw* shared library gets copied over to the build directory. To do this, extent the `FOREACH` loop at the bottom of your file:

```cmake
FOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES} glfw)
  # ...
ENDFOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES} glfw)
```

Re-configure your project and edit the *main.h* and *main.cpp* files and copy the following code to it:

```cxx
#pragma once

#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>   // Alternatively you can include dx12.hpp here.
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
```

And to the *main.cpp* file:

```cxx
#include "main.h"

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Graphics;
using namespace LiteFX::Rendering::Backends;

class SimpleApp : public LiteFX::App {
public:
	String getName() const noexcept override { return "Simple App"; }
	AppVersion getVersion() const noexcept override { return AppVersion(1, 0, 0, 0); }

private:
    GLFWwindow* m_window;

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

	// The next lines are Vulkan-specific:
    UInt32 extensions = 0;
    const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
    Array<String> requiredExtensions(extensions);
    std::ranges::generate(requiredExtensions, [&extensionNames, i = 0]() mutable { return String(extensionNames[i++]); });

    try
    {
        App::build<SimpleApp>(window)
            .logTo<ConsoleSink>(LogLevel::Trace)
            .useBackend<VulkanBackend>(requiredExtensions)                   // Alternatively, you can use the DirectX12Backend here (and remove the required extensions).
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

**Note on Vulkan**: for the Vulkan backend, it is important to specify the mandatory extensions that are required to create a valid surface. We do this by calling `glfwGetRequiredInstanceExtensions` and passing the result to the `VulkanBackend`. It is, however, possible to manually specify those extensions, if GLFW is not used. For Windows, those extensions are: `VK_KHR_surface` and `VK_KHR_win32_surface`.

We then specify a log target (which is completely optional) to be a console window. and tell the app to initialize a `VulkanBackend` for rendering. The `.go()` calls cause the builder to perform the actual object initialization and you will see it frequently when using the fluent API. However, using the fluent builder syntax is also optional - you could in fact create all the instances on your own. All classes follow an [RAII](https://en.cppreference.com/w/cpp/language/raii) idiom, so it is clear from the constructors parameter, which objects are required to be initialized in which order.

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
while (!::glfwWindowShouldClose(m_window))
{
    ::glfwPollEvents();
	// TODO: draw frame.
}

// TODO: cleanup.

// Destroy the window.
::glfwDestroyWindow(m_window);
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

// For DX12 (and Vulkan under Windows):
auto surface = makeUnique<DirectX12Surface>(::glfwGetWin32Window(m_window));
```

With the surface and adapter, we can now proceed to creating our device. Creating a device automatically initializes the *Swap Chain*, which we will talk about in detail later. We can simply create it with a default extent, but it is more efficient to directly tell the swap chain how large the surface is from the beginning. This way, we prevent it from beeing re-created after the window first gets drawn to. In order to do this, we can request the frame buffer size from *GLFW*. Note that the frame buffer size is not always equal to the window size, depending on the monitor. High DPI monitors use a more coarse window coordinate system. You can read about it in more detail [here](https://www.glfw.org/docs/3.3/group__window.html#ga0e2637a4161afb283f5300c7f94785c9).

With the adapter, surface and frame buffer extent, we can go ahead to create our device. We also specify the output format (`Format::B8G8R8A8_SRGB`) and the number of frames, which we concurrently want to draw. This is commonly referred to as *frames in flight*, or *back buffers* throughout the engine, though there is a slight difference. Back buffers refer to the number of frame buffers in the swap chain, whilst frames in flight is a broader concept, that for example influences how many buffers or descriptor sets you want to allocate later.

```cxx
int width, height;
::glfwGetFramebufferSize(m_window, &width, &height);
m_device = backend->createDevice(*adapter, *surface, Format::B8G8R8A8_SRGB, Size2d(width, height), 3);
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
- A clear value vector, which contains the values that the render target will be cleared with when starting the render pass. For our *BGRA* image, we want to clear it with black and an alpha value of `0.0`.
- A boolean switch to enable or disable clearing the values, which we set to true, since we want to clear our image with the clear values specified earlier.
- A boolean switch to enable clearing for stencil buffers. This switch is only used, if the render target is a `DepthStencil` target and the format supports stencil values. It can be used to disable clearing stencil values and only clear depth values for depth/stencil targets.
- A boolean switch that states, if we want to preserve the contents of the image after the render pass has finished. Since we do not want to use our render target as input attachment for another render pass, we also set this value to `false`.

```cxx
m_renderPass = m_device->buildRenderPass()
	.renderTarget(RenderTargetType::Present, Format::B8G8R8A8_SRGB, { 0.f, 0.f, 0.f, 0.f }, true, false, false);
```

#### Creating a Render Pipeline

Next on our list is creating the render pipeline. The singular here is important, because in most applications you will have many pipelines created from one render pass. A pipeline contains the whole state that is used to render a buffer. It tells the GPU how to interpret inputs, how to store the outputs and which shader programs to use. More importantly, it tells the GPU about the shader program parameters and how they are layed out. A pipeline itself is a state object, that can be bound to a render pass. There can only be one pipeline active at a time and since there can also be only one render pass active at a time, there's only one active pipeline at each point in the drawing timeline. Changing the pipeline state may be an expensive operation and you should not do this frequently. Instead, group together objects that can be rendered with the same pipeline and draw all of them before switching to another pipeline.

The first important states, a pipeline stores are the *Viewport* and *Scissor* states. Both tell the GPU which pixels to draw to the output render area, however a scissor does not imply a viewport transform (that is a scaling of the image to fit the viewport). For our example, we set both (the viewport and the scissor) to the whole size of our frame buffer. Note that it is possible to have multiple viewports and scissors. In order to do this, we use the `width` and `height` we've requested earlier.

Furthermore, each pipeline can be assigned an ID, which must be unique for the render pass and a name, which is soley used for debugging purposes. Let's start by creating our viewport and scissor and then continue with building our pipeline state.

```cxx
m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

m_pipeline = m_renderPass->buildRenderPipeline("Basic Pipeline")
	.withViewport(m_viewport)
	.withScissor(m_scissor)
```

##### Input Assembler State

Next we tell our pipeline how to handle geometry inputs. Geometry is described by vertices and indices. A geometry has a certain topology, which is described by the `PrimitiveTopology` enumeration. Our geometry is represented as a list of triangles, so we set it to `TriangleList`. Since our geometry vertices must not be ordered, we use an index buffer to tell which vertices to use for triangle rendering. We set the index type to `UInt16`, which has enough space for `65535` vertices and is enough for our example.

Finally, we define out vertex buffer layout. This means, that we tell the input assembler state about the memory layout of a single vertex. We use the `Vertex` object, defined in the `LiteFX::Graphics` namespace, but it is possible to use a custom structure, too. Each property is of the vertex is mapped to an attribute, which describes the memory layout of the property and where to find it within the buffer. Furthermore, it tells the renderer, where the attribute should be bound to. For example, the `Position` property of the vertex object is a 3-component 32-bit float vector, so its format is `XYZ32F`. In the shader, the position is the first element of the vertex, so we set its location to `0`.

```cxx
    .inputAssembler(device->buildInputAssembler()
		.withTopology(PrimitiveTopology::TriangleList)
		.withIndexType(IndexType::UInt16)
		.vertexBuffer(sizeof(Vertex), 0)
			.withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
			.withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
			.add())
```

##### Rasterizer State

Next, we tell the pipeline about how those primitives (i.e. triangles in our example) should be drawn. We want to draw solid faces, so we set the `PolygonMode` to `Solid`. Another property of the rasterizer state is the face culling state. First, we set the order of vertices, which dictates which side of the primitive is interpreted as *front* and which one is the *back*. We set the `CullOrder` to `ClockWise` to tell the pipeline to treat this ordering as *front face*. Finally, we tell the pipeline  to draw both sides of a polygon, by setting the `CullMode` to `Disabled`.

```cxx
	.rasterizer(device->buildRasterizer()
		.withPolygonMode(PolygonMode::Solid)
		.withCullOrder(CullOrder::ClockWise)
		.withCullMode(CullMode::Disabled))
```

##### Render Pipeline Layout

Each pipeline is defined using a *Shader Program* and a *Pipeline Layout*. We start by defining the shader program, which in our simple example should contain two stages: *Vertex* and *Fragment* shaders (those are also called *Pixel* shaders in DirectX). A program is built from multiple modules, where each module type may only exist once within a program. The modules are loaded from files and must be in a compatible binary format. For Vulkan this format is *SPIR-V*, for DirectX it's *DXIL*. We define those shaders later, for now it is only important that they are written to the *shaders* directory and called *vs.spv* (vertex shader) and *fs.spv* (fragment shader).

```cxx
	.shaderProgram(device->buildShaderProgram()
		.withVertexShaderModule("shaders/vs.spv")		// .dxi for DXIL
		.withFragmentShaderModule("shaders/fs.spv"))
```

Finally we need to tell the pipeline layout about the buffers that are used by the shader. Buffers are grouped into descriptor sets. Each descriptor set can contain multiple buffers and is visible to a pre-defined range of shader stages. Each buffer is bound to a certain location within the descriptor set. It is a good pracitce to group buffers into descriptor sets, based on update frequency. We have two buffers in our example, that are updated in different frequencies:

- The `CameraBuffer`, which is only updated when the viewport changes (our example camera is static). The camera buffer will be bound to location `0` of set `0` and will be visible to the vertex and fragment shader stages.
- The `TransformBuffer`, which is updated every frame (we want to draw a rotating rectangle). The transform buffer will be bound to location `0` of set `1` and will only be visible to the vertex shader.

For now, we will only define the descriptor sets and take a look at the `CameraBuffer` and `TransformBuffer` objects later.

```cxx
	.layout(device->buildPipelineLayout()
		.descriptorSet(0, ShaderStage::Vertex | ShaderStage::Fragment)
			.withUniform(0, sizeof(CameraBuffer))
			.add()
		.descriptorSet(1, ShaderStage::Vertex)
			.withUniform(0, sizeof(TransformBuffer))
			.add())
```

For more details about buffers and descriptor sets, kindly refer to the [project wiki](https://github.com/crud89/LiteFX/wiki/Resource-Bindings) or read the API documentation about descriptor sets.

###### Defining and Building Shader Modules

We already told the pipeline to load the vertex and fragment shaders, however, we do not yet have defined them. Create two new files in the project directory: *vs.hlsl* for the vertex shader and *fs.hlsl* for the fragment shader. First, let's take a look at the vertex shader:

```hlsl
#pragma pack_matrix(row_major)

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct TransformData
{
    float4x4 Model;
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
ConstantBuffer<TransformData> transform : register(b0, space1);

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    float4 position = mul(float4(input.Position, 1.0), transform.Model);
    vertex.Position = mul(position, camera.ViewProjection);
    
    vertex.Color = input.Color;
 
    return vertex;
}
```

`VertexInput` corresponds to the definition we earlier passed to the input assembler. `VertexData` describes the output vertex of the vertex shader. The two constant buffers correspond to the descriptors we passed to the descriptor sets in the pipeline definition. Note how the descriptor set ID maps to the `space` in the shader. The main function is pretty straightforward, as it only performs the view/projection transform for the vertex and copies its color. Also note the `#pragma pack_matrix(row_major)`: since we are going to use GLM in this tutorial and GLM produces row-major matrices, this is important. If you are using another math library, you can simply change this line accordingly.

The fragment shader is also pretty rudimentary for the moment:

```hlsl
#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET0;
    float Depth : SV_DEPTH;
};

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = input.Color;

    return fragment;
}
```

Again, the `VertexData` contains the data we are passing from the vertex to the fragment shader stage. `FragmentData` holds the information about the fragment, where the `Color` property maps to the first render target and the `Depth` property maps to the depth/stencil target we defined earlier in the rasterizer state. The does not do any further computations, but copies the values from the vertex input.

Usually you have to manually compile the shaders before you can use them. LiteFX is capable to do this for you automatically and provides two helpers for CMake projects: `ADD_SHADER_MODULE` and `TARGET_LINK_SHADERS`. Let's head over to our *CMakeLists.txt* file and add the following lines below the `TARGET_LINK_LIBRARIES`:

```cmake
ADD_SHADER_MODULE(MyVertexShader SOURCE "vs.hlsl" LANGUAGE HLSL TYPE VERTEX COMPILE_AS SPIRV SHADER_MODEL 6_3 COMPILER DXC)
ADD_SHADER_MODULE(MyFragmentShader SOURCE "fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL 6_3 COMPILER DXC)
TARGET_LINK_SHADERS(MyLiteFXApp SHADERS MyVertexShader MyFragmentShader)
```

First, we define two targets `MyVertexShader` and `MyFragmentShader`, one for each shader module. The options behind specify the language, shader type, intermediate language (`SPIRV` for Vulkan, change it to `DXIL` for DirectX 12), the shader model and the compiler to use. `DXC` is recommended, unless you want to compile *GLSL* shaders, which can only be compiled using `GLSLC`. Note, however, that you cannot use GLSL shaders to target DirectX 12.

The shader helper attempts to find the *GLSLC* and *DXC* compilers automatically. If you have the Vulkan SDK installed, it looks for the compilers there. It prefers the Vulkan SDK, since this DXC distribution supports SPIR-V code generation and GLSLC is present too. If you only plan on using the DirectX 12 backend and don't have the Vulkan SDK installed, the helper falls back to the DXC distribution, installed with the Windows 10 SDK. Note, however, that this distribution does not support SPIR-V code generation. It is possible, however, to specify a custom location for each compiler, by setting the `BUILD_DXC_COMPILER` and/or `BUILD_GLSLC_COMPILER` variables.

Using `TARGET_LINK_SHADERS` we setup a dependency for between the shaders and our application, so that the shaders are copied to the build directory properly. Note that by default, the shaders are copied into a `shaders/` subdirectory. You can change this subdirectory by changing the `SHADER_DEFAULT_SUBDIR` variable. Keep in mind to also update the pipeline state definition, if you do change the directory.

**NOTE:** If you want to learn more about how to write portable shaders, refer to the [shader development guide](https://github.com/crud89/LiteFX/wiki/Shader-Development). For more information on how to use the helpers, refer to the [shader module targets](https://github.com/crud89/LiteFX/wiki/Shader-Module-Targets) page in the project wiki.

### Creating and Managing Buffers

Next, we need to pass data to the GPU for it to process. LiteFX supports different data management strategies, that you can choose from:

- Write once/Read once: This strategy is most common for buffers that are changing with each frame.
- Write once/Read multiple: This strategy requires you to create two buffers: one on the CPU and one on the GPU and issue a transfer between both. This is most efficient for static buffers and textures.

Other scenarios are also possible, however, they are all implemented using the techniques demonstrated by those two (and most common) scenarios.

#### Vertex- and Index Buffers

Vertex and index buffers are two examples of data, that is typically written once and read multiple times. We thus create a CPU-visible staging buffer for both, write the required data to it and transfer it to a GPU-visible resource. But first, let's define a vertex and index buffer structure:

```cxx
const Array<Vertex> vertices =
{
    { { 0.0f, 0.0f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
};

const Array<UInt16> indices = { 0, 1, 2 };
```

We define a vertex buffer with three vertices, each with a different color. The last two vectors represent the normal and texture coordinate, which we do not want to use yet. It is totally possible to use a custom `Vertex` object. Just keep in mind to change the input assembler state accordingly. We specify the index buffer to form a triangle from all three vertices. The order of the indices is specified by the `CullOrder` we defined in the rasterizer state. 

Next, let's transfer the buffers to the GPU. We start of by storing the input assembler reference (for easier access) and creating a command buffer to record all transfer commands. We do this right below the pipeline creation code:

```cxx
auto inputAssembler = m_pipeline->inputAssembler();
auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);
```

We then create a CPU visible vertex buffer and copy the vertex data into it:

```cxx
auto stagedVertices = m_device->factory().createVertexBuffer(inputAssembler->vertexBufferLayout(0), BufferUsage::Staging, vertices.size());
stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);
```

The `BufferUsage` defines where the buffer should be visible from. `Staging` corresponds to a CPU-only visible buffer, whilst `Resource` is used for GPU-only visible buffers. We will use another buffer type (`Dynamic`) later to represent *Write once/Read once* scenarios. Finally, we copy the data to the vertex buffer by calling `map`. After this, we can create the GPU-visible vertex buffer and issue a transfer command:

```cxx
m_vertexBuffer = m_device->factory().createVertexBuffer(inputAssembler->vertexBufferLayout(0), BufferUsage::Resource, vertices.size());
commandBuffer->transfer(*stagedVertices, *m_vertexBuffer, 0, 0, vertices.size());
```

We store the vertex buffer in a member variable. We then go ahead and repeat the same process for the index buffer:

```cxx
auto stagedIndices = m_device->factory().createIndexBuffer(inputAssembler->indexBufferLayout(), BufferUsage::Staging, indices.size());
stagedIndices->map(indices.data(), indices.size() * inputAssembler->indexBufferLayout().elementSize(), 0);

m_indexBuffer = m_device->factory().createIndexBuffer(inputAssembler->indexBufferLayout(), BufferUsage::Resource, indices.size());
commandBuffer->transfer(*stagedIndices, *m_indexBuffer, 0, 0, indices.size());
```

#### Constant/Uniform Buffers

The same memory management concepts as for vertex and index buffers apply to shader resources (i.e. constant buffers, samplers or textures). However, they have one more aspect to them: *descriptors*. Descriptors are basically GPU-pointers in a sense, that they point to a GPU-visible resource before a draw call is issued. They must, however, not change until the draw call has finished (i.e. the end of the frame). Managing descriptors manually can be quite challenging. Luckily, LiteFX already implements flexible descriptor management strategies and the way you interact with descriptors is always the same when using LiteFX.

Descriptors are grouped into *Descriptor Sets*. We already defined two descriptor sets when setting up our pipeline. For now, both are only contain one descriptor, a uniform buffer each. You can add as many descriptors to a set as you like, there are some things to keep in mind, though. Firstly, you must not mix samplers and images in one descriptor set. This rule is not directly enforced by the Vulkan backend, however it's a strong requirement for the DirectX 12 backend, so you should follow it anyway to keep your pipeline definitions consistent. Secondly, you should define descriptor sets based on the frequency they are updated. This is a good practice and generally helps to organize GPU workload.

Descriptor sets directly map to a GPU `space`. Within this space, a descriptor has a `binding`, which defines from which register the descriptor gets accessed by the shader. We defined both in the pipeline layout, as well as the shader code. Feel free to go back to those sections to ensure that you understood how descriptors are mapped to the shader. If you think you understood the relationship, go ahead with the next sections.

##### Static Buffers

We will first map a static `CameraBuffer` to the shader. This buffer contains the camera View/Projection matrix. In our example, the camera cannot move, making it a perfect fit for static buffers. As mentioned earlier, static buffers correspond to the *Write once/Read multiple* strategy and should be transferred to a GPU-visible resource. We will thus create two buffers, as we did for the vertices and indices again and issue a transfer command to copy the data to the GPU. We will then allocate a descriptor set and update it accordingly, so that the descriptor points to the GPU camera buffer. 

Let's start off by defining our camera buffer structure:

```cxx
struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;
```

Note that we are using *glm* to store the matrix here, but you can use any other representation that suits you. Keep in mind to specify the matrix order (`#pragma pack_matrix`) in the shader sources, if it is different to our example. If you want to follow this guide, you need to add an include for glm matrix transformations:

```cxx
#include <glm/gtc/matrix_transform.hpp>
```

Next, we create the two buffers that should store the camera data:

```cxx
auto& cameraBindingLayout = m_pipeline->layout()->descriptorSet(0);
auto& cameraBufferLayout = cameraBindingLayout.descriptor(0);
m_cameraStagingBuffer = m_device->factory().createConstantBuffer(cameraBufferLayout.type(), BufferUsage::Staging, cameraBufferLayout.elementSize(), 1);
m_cameraBuffer = m_device->factory().createConstantBuffer(cameraBufferLayout.type(), BufferUsage::Resource, cameraBufferLayout.elementSize(), 1);
```

First, we request a reference of the descriptor set layout (at space *0*), that contains the camera buffer descriptor layout (at binding point 0). We then create two constant buffers for and store them in a member variable, since we want to be able to update the camera buffer later (for example, if a resize-event occurs). The camera buffer is still static, since such events occur infrequently.

Let's move on and compute the view and projection matrix and pre-multiply them together:

```cxx
auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
camera.ViewProjection = projection * view;
```

In the last line, we pre-multiply the view/projection matrix and store it in the camera buffer, which we can now transfer to the GPU:

```cxx
m_cameraStagingBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
commandBuffer->transfer(*m_cameraStagingBuffer, *m_cameraBuffer);
```

The last thing we need to do is making the descriptor point to the GPU-visible camera buffer. We only need to do this once, since we do not change the buffer location on the GPU:

```cxx
m_cameraBindings = cameraBindingLayout.allocate();
m_cameraBindings->update(cameraBufferLayout.binding(), *m_cameraBuffer, 0);
```

Here we first allocate a descriptor set that holds our descriptor for the camera buffer. We then update the descriptor bound to register *0* to point to the GPU-visible camera buffer. Finally, with all the transfer commands being recorded to the command buffer, we can submit the buffer and wait for it to be executed:

```cxx
auto fence = m_device->bufferQueue().submit(*commandBuffer);
m_device->bufferQueue().waitFor(fence);
commandBuffer = nullptr;
stagedVertices = nullptr;
stagedIndices = nullptr;
```

We also explicitly release the temporary staging buffers and the transfer command buffer here, since we do not need them anymore and we need to ensure that they are released before we close the application.

##### Dynamic Buffers

We now want to look at a different memory management strategy: *Write once/Read once*. For resources that change frequently (i.e. every frame), this strategy is more efficient than record transfer commands and waiting for the transfer to happen. Instead we rely on the graphics queue to ensure that our buffer is transferred automatically, when it is needed.

Let's begin with defining our transform buffer structure:

```cxx
struct TransformBuffer {
    glm::mat4 World;
} transform;
```

Next, we create three `Dynamic` buffers and map them to the descriptor set at space *1* that holds the per-frame transform buffer descriptors. There are three buffers, since we have three *frames in flight*, i.e. three frames that are computed concurrently. This equals the number of back-buffers in the swap chain, we created earlier. Since we have three buffers, we also need three descriptor sets, each containing a descriptor that points to the buffer for the current frame. The three buffers are stored in one *buffer array* with three elements, so each descriptor points to an individual element in the transform buffer array.

```cxx
auto& transformBindingLayout = m_pipeline->layout()->descriptorSet(1);
auto& transformBufferLayout = transformBindingLayout.descriptor(0);
m_perFrameBindings = transformBindingLayout.allocateMultiple(3);
m_transformBuffer = m_device->factory().createConstantBuffer(transformBufferLayout.type(), BufferUsage::Dynamic, transformBufferLayout.elementSize(), 3);
std::ranges::for_each(m_perFrameBindings, [this, &transformBufferLayout, i = 0](const auto& descriptorSet) mutable { descriptorSet->update(transformBufferLayout.binding(), *m_transformBuffer, i++); });
```

### Drawing Frames

With everything setup so far, we can now start the actual drawing. Navigate to the main application loop (look for the `// TODO: draw frame.` comment) and start by swapping out the current back buffer:

```cxx
auto backBuffer = m_device->swapChain().swapBackBuffer();
```

The back buffer describes the resources that are used for the frame that is currently computed. This is done concurrently, so while a frame is still waiting to be drawn, future frames can already be recorded by the CPU. This ensures, the GPU is always busy with rendering.

Each frame is drawn in one or multiple sequential *render passes*. We already a single defined the render pass earlier, so all we need to do is tell the GPU to start on the current back buffer:

```cxx
m_renderPass->begin(backBuffer);
```

In order to draw something, we need to acquire a command buffer to record drawing commands to. Each render pass stores a set of command buffers within the current (active) frame buffer. The right frame buffer is selected when passing the `backBuffer` to the `begin` method. A frame buffer can store multiple command buffers in order to allow for multiple threads to record commands concurrently, however, in our example we only use one command buffer:

```cxx
auto& commandBuffer = m_renderPass->activeFrameBuffer().commandBuffer(0);
```

Next up, we want to handle drawing geometry. Each geometry draw call requires a certain *state* to let the GPU know, how to handle the data we pass to it. This state is contained the *pipeline* we defined earlier. In a real-world application, there may be many pipelines with different shaders, rasterizer and input assembler states. You should, however, always aim minimize the amount of pipeline switches. You can do this by pre-ordering the objects in your scene, so that you draw all objects that require the same pipeline state at the same time. In this example, however, we only have one pipeline state and we now tell the GPU to use it for the subsequent workload:

```cxx
commandBuffer.use(*m_pipeline);
```

Now it's time to update the transform buffer for our object. We want to animate a rotating triangle, so we can use a clock to dictate the amount of rotation. We use the duration since the beginning to compute a rotation matrix, that we use to update the transform buffer:

```cxx
static auto start = std::chrono::high_resolution_clock::now();

auto now = std::chrono::high_resolution_clock::now();
auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
m_transformBuffer->map(reinterpret_cast<const void*>(&transform), sizeof(transform), backBuffer);
```

Before we can record the draw call, we need to make sure, the shader sees the right resources by binding all descriptor sets:

```cxx
commandBuffer.bind(*m_vertexBuffer);
commandBuffer.bind(*m_indexBuffer);
commandBuffer.bind(*m_cameraBindings);
commandBuffer.bind(*m_perFrameBindings[backBuffer]);
```

Finally, we can record the actual draw call and end the render pass, which will cause the command buffer to be submitted to the graphics queue:

```cxx
commandBuffer.drawIndexed(m_indexBuffer->elements());
m_renderPass->end();
```

When you launch the app now, you should see a rotating triangle in all its beauty.

### Cleanup

Before we can close our application, we need to ensure that all resources are properly released. Whilst not absolutely mandatory in release builds, this will satisfy the validation or debug layer (depending on your back-end):

```cxx
// Shut down the device.
m_device->wait();

// Destroy all resources.
m_cameraBindings = nullptr;
m_perFrameBindings.clear();
m_cameraBuffer = nullptr;
m_cameraStagingBuffer = nullptr;
m_transformBuffer = nullptr;
m_vertexBuffer = nullptr;
m_indexBuffer = nullptr;

// Destroy the pipeline, render pass and the device.
m_pipeline = nullptr;
m_renderPass = nullptr;
m_device = nullptr;
```

First we wait for the device to finish drawing the remaining frames. This ensures, that we do not destroy resources, that are still accessed by the GPU in any submitted command buffers. We then first destroy all descriptors and buffers, before finally releasing the pipeline, render pass and device instances.

### Handling Resize-Events

If you resize the window, you might notice that the backend will return an error. This is caused by the swap chain rendering to an outdated back-buffer. In order to support window resize events, let's implement the `SimpleApp::resize` method:

```cxx
App::resize(width, height);

if (m_device == nullptr)
    return;
```

Since this method is inherited from the `App` base class, we first invoke the base class method. We then check, if the device has already been initialize, since resize-events may occur before any initialization has been done. If it is not initialized, there's no need for us to do anything else here. However, this during rendering it will be initialized, so let's continue with the implementation:

```cxx
m_device->wait();
auto surfaceFormat = m_device->swapChain().surfaceFormat();
auto renderArea = Size2d(width, height);
```

Again, we first wait for the device to finish all submitted work. This ensures that we do not destroy any back buffers, that might be still used by command buffers that are yet to be executed. Next we request the surface format from the current swap chain and initialize the new render area extent. We then can go ahead and re-create the swap chain, which causes the back buffers to be re-allocated with the new size and format. Furthermore, we can resize the frame buffers of our render pass. Note that you have to decide whether or not you want to do this, because you might have a render pass, that renders into a target that is deliberately at a different size than the swap chain back buffer. However, you almost certainly want to at least resize the frame buffer of the render pass that writes your present target.

```cxx
m_device->swapChain().reset(surfaceFormat, renderArea, 3);
m_renderPass->resizeFrameBuffers(renderArea);
```

We then also resize the viewport and scissor rectangles, so that the image is drawn over the whole area of our resized window:

```cxx
m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
```

If you launch the application now and resize the window, it already should work. You might, however, notice that the image appears stretched. This is caused, because we also need to adjust the aspect ratio in our view/projection matrix. To do this, we can use the code we've written earlier to compute the camera buffer and update the buffer once again:

```cxx
auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
projection[1][1] *= -1.f;   // Fix GLM clip coordinate scaling.
camera.ViewProjection = projection * view;

auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);
m_cameraStagingBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
commandBuffer->transfer(*m_cameraStagingBuffer, *m_cameraBuffer);
commandBuffer->end(true, true);
```

Note that we do not have to release the command buffer explicitly here, since it will go out of scope anyway and will be released automatically.

## Final Thoughts

This quick start covered the basics on how to interact with the engine to write a modern graphics application. For more in-depth information about the inner workings of the engine, head over to the [project wiki](https://github.com/crud89/LiteFX/wiki). If you have any problems or want to contribute to the development, feel free to open an [issue](https://github.com/crud89/LiteFX/issues) or create a [pull request](https://github.com/crud89/LiteFX/pulls).

Nevertheless, I hope you enjoy with the project. Happy coding! 👩‍💻👨‍💻