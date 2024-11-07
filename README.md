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

## Getting Started

If you just want to start using LiteFX, you can acquire binaries of the latest version from the [releases page](https://github.com/crud89/LiteFX/releases) and follow the [project setup](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html) and [quick start](https://litefx.crudolph.io/docs/md_docs_tutorials_quick_start.html) guides. If you want to perform a custom build, check out the [building guide](https://github.com/crud89/LiteFX/wiki/Building-Guide). For a collection of tutorials and more in-depth information on how to use the engine and work with the code base, take a look at the [documentation](https://litefx.crudolph.io/docs/) and the [project wiki](https://github.com/crud89/LiteFX/wiki).

## Dependencies

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

## Contribute

If you are having trouble using the engine, found a bug or have suggestions, just drop an [issue](https://github.com/crud89/LiteFX/issues). Keep in mind that this project is developed in my free time and I might not be able to provide any advanced support. If you want to, feel free to provide improvements by creating a pull request.

## Projects using LiteFX

<!-- Currently none, lol -->

Want to add yours? Feel free to [contact](mailto:litefx@crudolph.io?subject=[GitHub]%20LiteFX%20Project%20List) me!

## License

LiteFX is licensed under the permissive [MIT license](./LICENSE). The documentation (i.e. the contents of the `docs` folder of this repository, especially the LiteFX logo, banner and icon) is licensed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/). Parts of this software are from third parties and are licensed under different terms. By using or redistributing this software, you agree to the terms of the third-party licenses mentioned in the [NOTICE](./NOTICE) file, depending on the parts you use or re-distribute. Please refer to [the above list](#dependencies) to see which license terms you have to agree to.

If you want to use LiteFX in your projects, linking to [project website](https://litefx.crudolph.io/) and/or putting the logo in your project description is much appreciated.
