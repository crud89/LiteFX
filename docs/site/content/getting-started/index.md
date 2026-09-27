# Getting started

LiteFX is a C++23 rendering engine for Windows with backends for Vulkan and DirectX 12. It is built with [CMake](https://cmake.org/), and its
dependencies are managed with [vcpkg](https://vcpkg.io/). This section helps you set up a project that uses the engine.

<div class="grid cards" markdown>

-   :material-download-outline:{ .lg .middle } __Installation__

    ---

    Install the engine with vcpkg (recommended), build it as part of your project using `FetchContent`, or install it manually.

    [:octicons-arrow-right-24: Install LiteFX](installation.md)

-   :material-folder-cog-outline:{ .lg .middle } __Project setup__

    ---

    Create a new CMake project that uses LiteFX, from the vcpkg manifest to the first build.

    [:octicons-arrow-right-24: Set up a project](project-setup.md)

-   :material-triangle-outline:{ .lg .middle } __Your first triangle__

    ---

    Write a small application that renders a triangle with both backends, and learn the core concepts of the engine along the way.

    [:octicons-arrow-right-24: Start the tutorial](../tutorials/first-triangle/index.md)

-   :material-code-braces:{ .lg .middle } __Samples__

    ---

    Browse the samples, from basic rendering to ray tracing and mesh shaders.

    [:octicons-arrow-right-24: Samples on GitHub](https://github.com/crud89/LiteFX/tree/main/src/Samples/)

</div>

!!! note "Work in progress"

    This guide describes is developed along the engine and may reflect changes that are not yet available in the current release. For a 
    working reference, please check the [project template](https://github.com/crud89/LiteFX-Template).

## Requirements

To build applications with LiteFX, you need:

- **Windows 10 or 11** (64-bit).
- **A C++23 compiler:** Visual C++ from Visual Studio 2022 or newer, or Clang/clang-cl.
- **CMake 3.23 or newer** and **vcpkg**. Both are part of the *Desktop development with C++* workload of Visual Studio.
- **A GPU with driver support** for Vulkan 1.3 and/or DirectX 12.
- **The [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)** (only required to target Vulkan).

## Starting points

If you want to start right away, use the [project template](https://github.com/crud89/LiteFX-Template), which contains a ready-to-build project that 
can be used as a starting point for your own project. For a more complete example, take a look at the [sample project](https://github.com/crud89/LiteFX-Sample).

The guides in this section explain the same setup step by step, which is helpful if you want to integrate the engine into an existing project.
They are not a replacement for the [CMake documentation](https://cmake.org/cmake/help/latest/index.html) and the
[vcpkg documentation](https://learn.microsoft.com/vcpkg/get_started/overview), which cover those tools in depth.
