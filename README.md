# LiteFX

An extensible, descriptive, modern computer graphics and rendering engine, written in C++23.

<p align="center">
  <img src="https://raw.githubusercontent.com/crud89/LiteFX/main/docs/img/litefx_rm_banner.png">
</p>

<div align="center">

[![GitHub](https://img.shields.io/github/license/crud89/litefx?style=flat-square)](https://github.com/crud89/LiteFX/blob/main/LICENSE) [![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/crud89/litefx/weekly.yml?branch=main&style=flat-square)](https://github.com/crud89/LiteFX/actions/workflows/weekly.yml) [![Latest release](https://img.shields.io/github/release/crud89/litefx.svg?style=flat-square)](https://github.com/crud89/LiteFX/releases) [![Released at](https://img.shields.io/github/release-date/crud89/litefx.svg?style=flat-square)](https://github.com/crud89/LiteFX/tags) [![Issues](https://img.shields.io/github/issues/crud89/LiteFX?style=flat-square)](https://github.com/crud89/LiteFX/issues) [![Pull Requests](https://img.shields.io/github/issues-pr/crud89/LiteFX?style=flat-square)](https://github.com/crud89/LiteFX/pulls) [![Documentation](https://img.shields.io/badge/docs-gh--pages-70dcf4.svg?style=flat-square)](https://litefx.crudolph.io/docs/)

</div>

## About

[LiteFX](https://litefx.crudolph.io) is a computer graphics engine, that can be used to quick-start developing applications using Vulkan 🌋 and/or DirectX 12 ❎ rendering APIs. It provides a flexible abstraction layer over modern graphics pipelines. Furthermore, it can easily be build and integrated using CMake. It naturally extents build scripts with functions that can be used to handle assets and compile shaders † and model dependencies to both.

The engine design follows an descriptive approach, which means that an application focuses on configuring what it needs and the engine then takes care of handling those requirements. To support this, the API also provides a fluent builder interface. Here is an example of how to easily setup a render pass graphics pipeline with a few lines of code:

```cxx
UniquePtr<RenderPass> renderPass = device->buildRenderPass("Geometry")
    .renderTarget(RenderTargetType::Present, Format::B8G8R8A8_UNORM, MultiSamplingLevel::x1, RenderTargetFlags::Clear, { 0.f, 0.f, 0.f, 1.f })
    .renderTarget(RenderTargetType::DepthStencil, Format::D32_SFLOAT, MultiSamplingLevel::x1, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
    .inputAssembler(inputAssembler)
    .rasterizer(device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::BackFaces)
        .cullOrder(CullOrder::ClockWise)
        .lineWidth(1.f))
    .layout(device->buildPipelineLayout()
        .descriptorSet(DescriptorSets::Constant, ShaderStage::Vertex | ShaderStage::Fragment)
            .withUniform(0, sizeof(CameraBuffer))
            .add()
        .descriptorSet(DescriptorSets::PerFrame, ShaderStage::Vertex)
            .withUniform(0, sizeof(TransformBuffer))
            .add())
    .shaderProgram(device->buildShaderProgram()
        .withVertexShaderModule("shaders/basic_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/basic_fs." + FileExtensions<TRenderBackend>::SHADER));
```

LiteFX is written in modern C++23, following established design patterns to make it easy to learn and adapt. Its focus is make the performance of modern graphics APIs easily accessible, whilst retaining full flexibility.

† Shaders can be built using *glslc* or *DXC*. *glslc* can be used to compile HLSL and GLSL shaders into SPIR-V for the Vulkan backend. *DXC* can only compile HLSL, but can target SPIR-V and DXIL, that's why it is preferred over *glslc*.

### Key Features

- **State of the Art**: the engine makes use of some of the most recent techniques introduced to the supported APIs in order to help you to fully utilize the latest hardware. For example it uses features from the [DirectX 12 Agility SDK](https://devblogs.microsoft.com/directx/announcing-dx12agility/) and [Vulkan 1.3](https://www.khronos.org/news/press/vulkan-reduces-fragmentation-and-provides-roadmap-visibility-for-developers), such as *mesh shaders*, *ray tracing* and *enhanced barriers*.
- **Streamlined API**: low-level graphics APIs typically involve a lot of boilerplate code to set them up. LiteFX provides different techniques to make this code less verbose. *Fluent Builders* can be used to setup and configure render graphs. *Shader Reflection* can be used in both, Vulkan and DirectX 12, to create pipeline layouts from a single line of code.
- **Multi-Threading Support**: LiteFX comes with support for multi-threaded render passes in order to maximize throughput and prevent GPU stalls. It takes care of the necessary synchronization between threads and exposes interfaces for manual synchronization. 
- **Descriptor and Memory Management**: descriptors and buffers can be hard to get around. LiteFX abstracts them away in a way that is both efficient and accessible.
- **Runtime Backend Switching**: Switch between API backends at runtime without re-creating the window. The engine's app model supports flexible configuration and all domain types are abstracted in a way, that make it easy to provide different render paths.
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

- [C++23 compatible compiler](https://en.cppreference.com/w/cpp/compiler_support/23): At the moment only MSVC fully supports the required features. †
- [CMake](https://cmake.org/download/) (version 3.20 or higher). ‡
- Optional: [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.3.204.1 or later (required to build the Vulkan backend).
- Optional: Windows 10 SDK 10.0.19041.0 or later (required to build DirectX backend).

† Note that at least Visual Studio 17.10 or later is required.

‡ CMake 3.20 is part of Visual Studio 2022. When using other compilers, CMake needs to be installed manually.

#### Cloning the Repository

Create a new directory from where you want to build the sources. Then open your shell and clone the repository:

```sh
git clone --recursive https://github.com/crud89/LiteFX.git .
```

#### Performing a Build

There are multiple ways of creating a build from scratch. In general, all *CMake*-based build systems are supported.

##### From Command Line

Building from command line is the most straightforward way and is typically sufficient, if you only want to consume a fresh build.

```sh
cmake src/ --preset windows-x64-release
cmake --build out/build/windows-x64-release/ --target install
```

##### Using Visual Studio

From Visual Studio open the folder where you just checked out the contents of the repository. In the *Project Explorer* change the view to *CMake Targets*. Right click *LiteFX* and select *Install*.

#### Build Customization

You can customize the engine build, according to your specific needs. The most straightforward way is to use [*CMake presets*](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html). Create a file called *CMakeUserPresets.json* inside the *src/* directory and add the following content to it:

```json
{
  "version": 2,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 20,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "win-x64-custom-preset",
      "inherits": "windows-x64-release",
      "cacheVariables": {
      }
    }
  ]
}
```

Within the cache variables, you can override the build options, LiteFX exports. All customizable options have the `LITEFX_BUILD_` prefix and are described in detail below:

- `LITEFX_BUILD_VULKAN_BACKEND` (default: `ON`): builds the Vulkan 🌋 backend (requires [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.3.204.1 or later to be installed on your system).
- `LITEFX_BUILD_DX12_BACKEND` (default: `ON`): builds the DirectX 12 ❎ backend.
- `LITEFX_BUILD_DEFINE_BUILDERS` (default: `ON`): enables the [builder architecture](https://github.com/crud89/LiteFX/wiki/Builders) for backends.
- `LITEFX_BUILD_SUPPORT_DEBUG_MARKERS` (default: `OFF`): implements support for setting debug regions on device queues.
- `LITEFX_BUILD_WITH_GLM` (default: `ON`): adds [glm](https://glm.g-truc.net/0.9.9/index.html) converters to math types. †
- `LITEFX_BUILD_WITH_DIRECTX_MATH` (default: `ON`): adds [DirectX Math](https://github.com/microsoft/DirectXMath) converters to math types. †
- `LITEFX_BUILD_HLSL_SHADER_MODEL` (default: `6_5`): specifies the default HLSL shader model.
- `LITEFX_BUILD_EXAMPLES` (default: `ON`): builds the examples. Depending on which backends are built, some may be omitted.
- `LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER` (default: `ON`): enables code that attempts to load the latest version of the [PIX GPU capturer](https://devblogs.microsoft.com/pix/) in the DirectX 12 samples, if available (and if the command line argument `--load-pix=true` is specified).
- `LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER` (default: `OFF`): enables code in the samples, that loads the [RenderDoc](https://renderdoc.org/) runtime API, if the application is launched from within RenderDoc (and if the command line argument `--load-render-doc=true` is specified).
- `LITEFX_BUILD_TESTS` (default: `OFF`): builds tests for the project.

For example, if you only want to build the Vulkan backend and samples and don't want to use DirectX Math, a preset would look like this:

```json
{
  "version": 2,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 20,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "win-x64-vulkan-only",
      "inherits": "windows-x64-release",
      "cacheVariables": {
        "LITEFX_BUILD_DX12_BACKEND": "OFF",
        "LITEFX_BUILD_WITH_DIRECTX_MATH": "OFF"
      }
    }
  ]
}
```

You can build using this preset from command line like so:

```sh
cmake src/ --preset win-x64-vulkan-only
cmake --build out/build/win-x64-vulkan-only/ --target install --config Release
```

† Note that *glm* and *DirectX Math* are installed using *vcpkg* automatically. If one of those options gets disabled, no converters will be generated and the dependency will not be exported. Note that both can be used for DirectX 12 and Vulkan.

#### Troubleshooting

If you are having problems building the project, you may find answers [in the wiki](https://github.com/crud89/LiteFX/wiki/Troubleshooting). Otherwise, feel free to start a [discussion](https://github.com/crud89/LiteFX/discussions/categories/q-a) or open an [issue](https://github.com/crud89/LiteFX/issues).

### Dependencies

All dependencies are automatically installed using *vcpkg*, when performing a manual build. The engine core by itself only has one hard dependency:

- [spdlog](https://github.com/gabime/spdlog): Lightweight logging library.

Depending on which rendering backends are build, the following dependencies are additionally linked against:

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/): Required by the Vulkan backend.
- [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/): Required by the Vulkan backend. Handles memory allocations.
- [SPIR-V Reflect](https://github.com/KhronosGroup/SPIRV-Reflect): Required by the Vulkan backend. Used to implement shader reflection.
- [DirectX Headers](https://github.com/microsoft/DirectX-Headers): Required by the DirectX 12 backend.
- [DirectX Agility SDK](https://devblogs.microsoft.com/directx/directx12agility/): Required by the DirectX 12 backend.
- [WinPixEventRuntime](https://devblogs.microsoft.com/pix/winpixeventruntime/): Required by the DirectX 12 backend.
- [D3D12 Memory Allocator](https://gpuopen.com/d3d12-memory-allocator/): Required by the DirectX 12 backend. Handles memory allocations.
- [DirectX Shader Compiler (DXC)](https://github.com/microsoft/DirectXShaderCompiler): Optional for Vulkan backend, but required for the DirectX 12 backend.

The math module can optionally be built with converters for the following math and linear algebra libraries:

- [glm](https://github.com/g-truc/glm): Cross-platform math library.
- [DirectX Math](https://github.com/microsoft/DirectXMath): Windows-specific math library.

Furthermore, the samples also use some libraries for convenience. Those dependencies are not exported and are not required by your application. You can use whatever replacement suits you best instead.

- [CLI11](https://cliutils.github.io/CLI11/book/): Command line parser.
- [glfw3](https://www.glfw.org/): Cross-platform window manager.
- [stb](https://github.com/nothings/stb): Lightweight image loading and processing library.

Finally, the tests are using conformant software emulated rasterizers to keep them portable:

- [Windows Advanced Rasterization Platform (WARP)](https://learn.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp): Emulates D3D12 devices.
- [Mesa 3D 24.1 / Lavapipe](https://github.com/pal1000/mesa-dist-win): Emulates Vulkan devices and needs to be installed manually (see [Tests](/src/tests/readme.md) for more information).

## Getting Started

For a [quick-start guide](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html), a collection of tutorials and more in-depth information on how to use the engine and work with the code base, take a look at the [documentation](https://litefx.crudolph.io/docs/) and the [project wiki](https://github.com/crud89/LiteFX/wiki).

## Contribute

If you are having trouble using the engine, found a bug or have suggestions, just drop an [issue](https://github.com/crud89/LiteFX/issues). Keep in mind that this project is developed in my free time and I might not be able to provide any advanced support. If you want to, feel free to provide improvements by creating a pull request.

## Projects using LiteFX

<!-- Currently none, lol -->

Want to add yours? Feel free to [contact](mailto:litefx@crudolph.io?subject=[GitHub]%20LiteFX%20Project%20List) me!

## License

LiteFX is licensed under the permissive [MIT license](./LICENSE). The documentation (i.e. the contents of the `docs` folder of this repository, especially the LiteFX logo, banner and icon) is licensed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/). Parts of this software are from third parties and are licensed under different terms. By using or redistributing this software, you agree to the terms of the third-party licenses mentioned in the [NOTICE](./NOTICE) file, depending on the parts you use or re-distribute. Please refer to [the above list](#dependencies) to see which license terms you have to agree to.

If you want to use LiteFX in your projects, linking to [project website](https://litefx.crudolph.io/) and/or putting the logo in your project description is much appreciated.
