# LiteFX

An extensible, descriptive, modern computer graphics and rendering engine, written in C++20.

<p align="center">
  <img src="https://raw.githubusercontent.com/crud89/LiteFX/main/docs/img/banner_m.jpg?token=AEMKYX75E4UF4U6GAT5ZTIDA3C4GU">
</p>

[![GitHub](https://img.shields.io/github/license/crud89/litefx)](https://github.com/crud89/LiteFX/blob/main/LICENSE) [![GitHub Workflow Status](https://img.shields.io/github/workflow/status/crud89/litefx/Weekly)](https://github.com/crud89/LiteFX/actions/workflows/weekly.yml) [![Latest release](https://img.shields.io/github/release/crud89/litefx.svg)](https://github.com/crud89/LiteFX/releases) [![Released at](https://img.shields.io/github/release-date/crud89/litefx.svg)](https://github.com/crud89/LiteFX/tags) [![Issues](https://img.shields.io/github/issues/crud89/LiteFX)](https://github.com/crud89/LiteFX/issues) [![Pull Requests](https://img.shields.io/github/issues-pr/crud89/LiteFX)](https://github.com/crud89/LiteFX/pulls) [![Documentation](https://img.shields.io/badge/docs-gh--pages-70dcf4.svg)](https://litefx.crudolph.io/docs/)

## About

[LiteFX](https://litefx.crudolph.io) is a computer graphics engine, that can be used to quick-start developing applications using Vulkan 🌋 and/or DirectX 12 ❎ rendering APIs. It provides a flexible abstraction layer over modern graphics pipelines. Furthermore, it can easily be build and integrated using CMake. It naturally extents build scripts with functions that can be used to handle assets and compile shaders † and model dependencies to both.

The engine design follows an descriptive approach, which means that an application focuses on configuring what it needs and the engine then takes care of handling those requirements. To support this, the API also provides a fluent interface. Here is an example of how to easily setup a render pass graphics pipeline with a few lines of code:

```cxx
auto renderPass = device->buildRenderPass()
    .renderTarget(RenderTargetType::Present, Format::B8G8R8A8_UNORM, MultiSamplingLevel::x1, { 0.f, 0.f, 0.f, 1.f }, true, false)
    .renderTarget(RenderTargetType::DepthStencil, Format::D32_SFLOAT, MultiSamplingLevel::x1, { 1.f, 0.f, 0.f, 0.f }, true, false)
    .go();

auto pipeline = renderPass->makePipeline(0, "Basic Pipeline")
    .withViewport(viewport)
    .withScissor(scissor)
    .layout()
        .shaderProgram()
            .addVertexShaderModule("shaders/vs.dxi")
            .addFragmentShaderModule("shaders/ps.dxi")
            .go()
        .addDescriptorSet(0, ShaderStage::Vertex | ShaderStage::Fragment)
            .addUniform(0, sizeof(CameraBuffer))
            .addImage(1)
            .go()
        .addDescriptorSet(1, ShaderStage::Fragment)
            .addSampler(0)
            .go()
        .go()
    .rasterizer()
        .withPolygonMode(PolygonMode::Solid)
        .withCullMode(CullMode::BackFaces)
        .withCullOrder(CullOrder::ClockWise)
        .withLineWidth(lineWidth)
        .go()
    .inputAssembler()
        .withTopology(PrimitiveTopology::TriangleList)
        .withIndexType(IndexType::UInt16)
        .addVertexBuffer(sizeof(Vertex), binding)
            .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
            .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
            .go()
        .go()
    .go();
```

LiteFX is written in modern C++20, following established design patterns to make it easy to learn and adapt. Its focus is make the performance of modern graphics APIs easily accessible, whilst retaining full flexibility.

† Shaders can be built using *glslc* or *DXC*. *glslc* can be used to compile HLSL and GLSL shaders into SPIR-V for the Vulkan backend. *DXC* can only compile HLSL, but can target SPIR-V and DXIL, that's why it is preferred over *glslc*.

### Key Features

- **Fluent API**: the fluent API can help you to organize creation of your render context. Instances, render passes and pipeline states typically require a lot of code to set them up. LiteFX hides this code behind an flexible builder architecture. However, if you don't like it, you can still create all objects on your own.
- **State of the Art**: the engine makes use of some of the most recent techniques introduced to the supported APIs in order to help you to fully utilize the latest hardware. For example it implements support for [DirectX 12 render passes](https://docs.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-render-passes) or [Vulkan timeline semaphores](https://www.khronos.org/blog/vulkan-timeline-semaphores) for improved thread synchronization.
- **Multi-Threading Support**: LiteFX comes with support for multi-threaded render passes in order to maximize throughput and prevent GPU stalls. It takes care of the necessary synchronization between threads and exposes interfaces for manual synchronization. 
- **Descriptor and Memory Management**: descriptors and buffers can be hard to get around. LiteFX abstracts them away in a way that is both efficient and easy to adapt.
- **CMake Integration**: you can use CMake to integrate the engine into your project. Furthermore, it exports scripts that allow you to integrate assets and shaders into your build process. It can be installed using *vcpkg*, making project setup quick and painless.
- **Much More**: If you want to learn what else you can do, check out the [guides](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html) and [wiki](https://github.com/crud89/LiteFX/wiki).

## Installation

If you just want to start using LiteFX, you can acquire binaries of the latest version from the [releases page](https://github.com/crud89/LiteFX/releases) and follow the [project setup](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html) and [quick start](https://litefx.crudolph.io/docs/md_docs_tutorials_quick_start.html) guides.

### Using vcpkg

If you are using vcpkg, you can use the [registry](https://github.com/crud89/LiteFX-Registry) to install the engine directly.

### Manual Builds

You can also build the sources on your own. Currently only MSVC builds under Windows are officially supported. However, the engine does use CMake and (besides the DirectX 12 backend) no Windows-specific features, so porting the Vulkan backend and engine architecture should be absolutely possible (pull requests are much appreciated!).

#### Prerequisites

In order for the project to be built, there are a few prerequisites that need to be present on your environment:

- [CMake](https://cmake.org/download/) (version 3.16 or higher).
- Optional: [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.2.148.0 or later (required to build the Vulkan backend).
- Optional: Custom [DXC](https://github.com/microsoft/DirectXShaderCompiler) build (required to build shaders for DirectX backend). †
- Optional: Windows 10 SDK 10.0.19041.0 or later (required to build DirectX backend).

† Note that the LunarG Vulkan SDK (1.2.141.0 and above) ships with a pre-built DXC binary, that supports DXIL and SPIR-V code generation and thus should be favored over the DXC binary shipped with the Windows SDK, which only supports DXIL.

#### Cloning the Repository

Create a new directory from where you want to build the sources. Then open your shell and clone the repository:

```sh
git clone https://github.com/crud89/LiteFX.git . --recurse-submodules
```

#### Performing a Build

There are multiple ways of creating a build from scratch. In general, all *CMake*-based build systems are supported.

##### From Command Line

Building from command line is the most straightforward way and is typically sufficient, if you only want to consume a fresh build.

```sh
cmake src/ -B out/build/
cmake --build out/build/ --target install --config Release
```

##### Using Visual Studio

From Visual Studio open the folder where you just checked out the contents of the repository. In the *Project Explorer* change the view to *CMake Targets*. Right click *LiteFX* and select *Install*.

#### Build Customization

You can customize the engine build, according to your specific needs. From Visual Studio, you can simply edit the `src/CMakeSettings.json` file to do this. The usual CMake process is similar. All customizable options have the `BUILD_` prefix and are described in detail below:

- `BUILD_VULKAN_BACKEND` (default: `ON`): builds the Vulkan 🌋 backend (requires [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.2.148.0 or later to be installed on your system).
- `BUILD_DX12_BACKEND` (default: `ON`): builds the DirectX 12 ❎ backend.
- `BUILD_WITH_GLM` (default: `ON`): adds [glm](https://glm.g-truc.net/0.9.9/index.html) converters to math types. †
- `BUILD_WITH_DIRECTX_MATH` (default: `ON`): adds [DirectX Math](https://github.com/microsoft/DirectXMath) converters to math types. †
- `BUILD_HLSL_SHADER_MODEL` (default: `6_3`): specifies the default HLSL shader model.
- `BUILD_EXAMPLES` (default: `ON`): builds the examples. Depending on which backends are built, some may be omitted.
- `BUILD_EXAMPLES_DX12_PIX_LOADER` (default: `ON`): enables code that attempts to load the latest version of the [PIX GPU capturer](https://devblogs.microsoft.com/pix/) in the DirectX 12 samples, if available (and if the command line argument `--load-pix=true` is specified).

† Note that *glm* and *DirectX Math* are installed using *vcpkg* automatically. If one of those options gets disabled, no converters will be generated and the dependency will not be exported. Note that both can be used for DirectX 12 and Vulkan.

### Dependencies

All dependencies are automatically installed using *vcpkg*, when performing a manual build. The engine only has two hard dependencies:

- [spdlog](https://github.com/gabime/spdlog): Lightweight logging library.
- [{fmt}](https://github.com/fmtlib/fmt): String formatting library and implicit dependency of *spdlog*.

Depending on which rendering backends are build, the following dependencies are required:

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/): Required by the Vulkan backend
- [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/): Required by the Vulkan backend. Handles memory allocations.
- [DirectX Headers](https://github.com/microsoft/DirectX-Headers): Required by the DirectX 12 backend.
- [D3D12 Memory Allocator](https://gpuopen.com/d3d12-memory-allocator/): Required by the DirectX 12 backend. Handles memory allocations.

The math module can optionally be built with converters for the following math and linear algebra libraries:

- [glm](https://github.com/g-truc/glm): Cross-platform math library.
- [DirectX Math](https://github.com/microsoft/DirectXMath): Windows-specific math library.

Furthermore, the samples also use some libraries for convenience. Those dependencies are not exported and are not required by your application. You can use whatever replacement suits you best instead.

- [CLI11](https://cliutils.github.io/CLI11/book/): Command line parser.
- [glfw3](https://www.glfw.org/): Cross-platform window manager.
- [stb](https://github.com/nothings/stb): Lightweight image loading and processing library.

## Getting Started

For a [quick-start guide](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html), a collection of tutorials and more in-depth information on how to use the engine and work with the code base, take a look at the [documentation](https://litefx.crudolph.io/docs/) and the [project wiki](https://github.com/crud89/LiteFX/wiki).

## Contribute

If you are having trouble using the engine, found a bug or have suggestions, just drop an [issue](https://github.com/crud89/LiteFX/issues). Keep in mind that this project is developed in my free time and I might not be able to provide any advanced support. If you want to, feel free to provide improvements by creating a pull request.

## Projects using LiteFX

<!-- Currently none, lol -->

Want to add yours? Feel free to [contact](mailto:litefx@crudolph.io?subject=[GitHub]%20LiteFX%20Project%20List) me!

## License

LiteFX is licensed under the permissive [MIT license](./LICENSE). The documentation (i.e. the contents of the `docs` folder of this repository, especially the LiteFX logo, banner and icon) is licensed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/).

If you want to use LiteFX in your projects, linking to [project website](https://litefx.crudolph.io/) and/or putting the logo in your project description is much appreciated.
