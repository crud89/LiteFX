# LiteFX

An extensible, descriptive, modern computer graphics and rendering engine.

<p align="center">
  <img src="/docs/img/banner_m.jpg">
</p>

![GitHub](https://img.shields.io/github/license/aschratt/litefx.svg) ![GitHub Workflow Status](https://img.shields.io/github/workflow/status/aschratt/litefx/release?label=Build) ![Latest release](https://img.shields.io/github/release/aschratt/litefx.svg) ![Released at](https://img.shields.io/github/release-date/aschratt/litefx.svg) [![Documentation](https://img.shields.io/badge/docs-gh--pages-70dcf4.svg)](https://litefx.github.io/docs/#/)

## About

LiteFX is a computer graphics engine, I've developed to learn about modern graphics APIs, like Vulkan 🌋 and DirectX 12 ❎. It follows an descriptive approach, which means that an application focuses on configuring what it needs and the engine then takes care of handling those requirements. To do this, the API provides a fluent interface. Here is an example of how to easily define a Vulkan graphics pipeline with a few lines of code:

```cxx
auto renderPass = device->buildRenderPass()
    .attachPresentTarget(true)
    .setPipeline()
        .defineLayout()
            .setRasterizer()
                .withPolygonMode(PolygonMode::Solid)
                .withCullMode(CullMode::BackFaces)
                .withCullOrder(CullOrder::ClockWise)
                .withLineWidth(1.f)
                .go()
            .setInputAssembler()
                .withTopology(PrimitiveTopology::TriangleList)
                .withIndexType(IndexType::UInt16)
                .addVertexBuffer(sizeof(Vertex), 0)
                    .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                    .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                    .go()
                .go()
            .addViewport()
                .withRectangle(RectF(0.f, 0.f, 600.f, 800.f))
                .addScissor(RectF(0.f, 0.f, 600.f, 800.,f))
                .go()
            .go()
        .go()
    .go();
```

The core of the fluent interface is a flexible [builder architecture](https://github.com/Aschratt/LiteFX/wiki/builder-guide), that allows to easily extent built-in types provide custom implementations. Furthermore it also allows to provide whole custom rendering backends.

LiteFX is written in modern C++17, following established design patterns to make it easy to learn and adapt. It's focus is make the performance of modern graphics APIs easily accessible, whilst retaining full flexibility.

## Installation

If you just want to start using LiteFX, you can acquire binaries of the latest version from the [releases page](https://github.com/Aschratt/LiteFX/releases).

### Manual Builds

You can also build the sources on your own. Currently only MSVC builds under Windows are supported. However, the engine does use CMake with the Ninja generator, so porting non platform-specific parts should be absolutely possible (pull requests are much appreciated!).

#### Prerequisites

In order for the project to be built, there are a few prerequisites that need to be present on your environment:

- [CMake](https://cmake.org/download/) (version 3.16 or higher).
- Optional: [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.2.148.0 or later (required to build the Vulkan backend).
- Optional: Custom [DXC](https://github.com/microsoft/DirectXShaderCompiler) build (required to build shaders for DirectX backend). †
- Optional: Windows 10 SDK 10.0.19041.0 or later (required to build DirectX backend).

Furthermore, the project uses [vcpkg](https://github.com/microsoft/vcpkg) to manage dependencies, so if you haven't already installed *vcpkg*, start by cloning and building it. Make sure to store the binary directory into a system environment variable `VCPKG_ROOT`. Alternatively you can also use the *vcpkg* [toolchain file](https://github.com/microsoft/vcpkg/blob/master/docs/examples/installing-and-using-packages.md#cmake).

† Note that the LunarG Vulkan SDK (1.2.141.0 and above) ships with a pre-built DXC binary, however it only contains the SPIR-V code generator. On the other hand, the default DXC build does only contain the DXIL code generator and cannot be used to compile HLSL shaders into SPIR-V. If you want to target both backends, i.e. Vulkan *and* DX12, then you have to use a custom DXC build, that contains both code generators. The process of building DXC is described in its [repository](https://github.com/microsoft/DirectXShaderCompiler).

#### Cloning the repository

Create a new directory from where you want to build the sources. Then open your shell and clone the repository:

    git clone https://github.com/Aschratt/LiteFX.git .

#### Building using Visual Studio

From Visual Studio open the folder where you just checked out the contents of the repository. In the *Project Explorer* change the view to *CMake Targets*. Right click *LiteFX* and select *Install*.

#### Build Customization

You can customize the engine build, according to your specific needs. From Visual Studio, you can simply edit the `src/CMakeSettings.json` file to do this. The usual CMake process is similar. All customizable options have the `BUILD_` prefix and are described in detail below:

- `BUILD_VULKAN_BACKEND` (default: `ON`): builds the Vulkan backend 🌋 (requires [LunarG Vulkan SDK](https://vulkan.lunarg.com/) 1.2.148.0 or later to be installed on your system).
- `BUILD_DX12_BACKEND` (default: `OFF`): builds the DirectX 12 backend ❎
- `BUILD_EXAMPLES` (default: `ON`): builds the examples. Depending on which backends are built, some may be omitted.
- `BUILD_WITH_GLM` (default: `ON`): adds [glm](https://glm.g-truc.net/0.9.9/index.html) converters to math types. †
- `BUILD_WITH_DIRECTX_MATH` (default: `ON`): adds [DirectX Math](https://github.com/microsoft/DirectXMath) converters to math types. †
- `BUILD_USE_GLSLC` (default: `OFF`): compiles shaders using *glslc* (requires LunarG Vulkan SDK to be installed on your system). **Can not be used together with `BUILD_USE_DXC`!** ‡
- `BUILD_USE_DXC` (default: `ON`): compiles shaders using *DXC* (requires LunarG Vulkan SDK when targeting only Vulkan, a pre-build of [DXC](https://github.com/microsoft/DirectXShaderCompiler/releases) when targeting only DirectX 12, or a custom DXC build when targeting both). **Can not be used together with `BUILD_USE_GLSLC`!** ‡
- `BUILD_DXC_DXIL` (default: `ON`): compiles HLSL shaders to DXIL when `BUILD_USE_DXC` is set to `ON`.
- `BUILD_DXC_SPIRV` (default: `ON`): compiles HLSL shaders to SPIR-V when `BUILD_USE_DXC` is set to `ON`.

† Note that *glm* will be installed from *vcpkg* automatically, if `BUILD_WITH_GLM` is enabled. In order to use *DirectX Math*, the latest Windows 10 SDK needs to be installed.

‡ *glslc* can be used to compile HLSL and GLSL shaders into SPIR-V for the Vulkan backend. *DXC* can only compile HLSL, but can target SPIR-V and DXIL, as long as a custom build with support for both is used.

## Getting Started

For a collection of tutorials and more in-depth information on how to use the engine and work with the code base, take a look at the [project wiki](https://github.com/Aschratt/LiteFX/wiki).

## Contribute

If you are having trouble using the engine, found a bug or have suggestions, just drop an [issue](https://github.com/Aschratt/LiteFX/issues). Keep in mind that this project is developed in my free time and I might not be able to provide any advanced support. If you want to, feel free to provide improvements by creating a pull request.

## Projects using LiteFX

<!-- Currently none, lol -->

Want to add yours? Feel free to [contact](mailto:litefx@aschratt.com?subject=[GitHub]%20LiteFX%20Project%20List) me!

## License

LiteFX is licensed under the permissive [MIT license](./LICENSE). The documentation (i.e. the contents of the `docs` folder of this repository, especially the LiteFX logo, banner and icon) is licensed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/).

If you want to use LiteFX in your projects, linking to the engine and/or putting the logo in your project description is much appreciated.
