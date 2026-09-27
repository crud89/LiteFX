# Project setup

This guide walks you through setting up a new CMake project that uses LiteFX, with vcpkg providing the engine. If you have not installed vcpkg
yet, follow the [installation guide](installation.md#vcpkg) first. The guide assumes a Windows system with Visual C++ and uses
[GLFW](https://www.glfw.org/) for the application window.

## Project structure

Start with an empty directory, which will contain the following files:

| File | Purpose |
|---|---|
| `vcpkg.json` | The *manifest*: your project's metadata and dependencies. |
| `vcpkg-configuration.json` | Tells vcpkg where to find the engine (the LiteFX registry). |
| `CMakePresets.json` | Build configurations, e.g. the compiler, build type and vcpkg integration. |
| `CMakeLists.txt` | The build script of your project. |
| `main.h`, `main.cpp` | The application sources, written in the [first tutorial](../tutorials/first-triangle/index.md). |

## vcpkg manifest { #vcpkg-manifest }

The manifest tells vcpkg which libraries your project depends on: the engine itself and GLFW, which creates the application window and
handles user input. You can use any other window library, or your own, but then some parts of the tutorials need to be adapted.

```json title="vcpkg.json"
{
  "name": "myapp",
  "version": "1.0",
  "supports": "windows & !arm",
  "dependencies": [
    "litefx",
    "glfw3"
  ]
}
```

Next to it, create the `vcpkg-configuration.json` file that adds the LiteFX registry, as described in the
[installation guide](installation.md#adding-the-litefx-registry).

## CMake presets { #cmake-presets }

A *preset* describes how CMake configures a build. The following preset builds a 64-bit debug version with Visual C++ and
[Ninja](https://ninja-build.org/). It loads vcpkg through its CMake *toolchain file*, which installs the dependencies from your manifest when
you configure the project.

```json title="CMakePresets.json"
{
  "version": 4,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 23,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "windows-msvc-x64-debug",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/out/build/${presetName}",
      "installDir": "${sourceDir}/out/install/${presetName}",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "architecture": {
        "value": "x64",
        "strategy": "external"
      },
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "VCPKG_TARGET_TRIPLET": "x64-windows"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "windows-msvc-x64-debug",
      "configurePreset": "windows-msvc-x64-debug"
    }
  ]
}
```

The toolchain file is located through the `VCPKG_ROOT` environment variable, which is set by Visual Studio or by you, depending on how you
[installed vcpkg](installation.md#installing-vcpkg). For more ways to integrate vcpkg, see the
[vcpkg CMake integration](https://learn.microsoft.com/vcpkg/users/buildsystems/cmake-integration) documentation.

## Build script

The build script defines your application and its dependencies:

```cmake title="CMakeLists.txt"
CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
PROJECT(MyApp LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the application and its runtime files into a common directory.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/binaries")

FIND_PACKAGE(LiteFX CONFIG REQUIRED)
FIND_PACKAGE(glfw3 CONFIG REQUIRED)

ADD_EXECUTABLE(MyApp
    "main.h"
    "main.cpp"
)

TARGET_LINK_LIBRARIES(MyApp PRIVATE LiteFX::Vulkan LiteFX::DirectX12 glfw)

# Copy the runtime files the engine requires next to the application.
LITEFX_DEPLOY_RUNTIME(MyApp)
```

Here is what the individual parts do:

- **C++23** is the language standard the engine requires.
- **`CMAKE_RUNTIME_OUTPUT_DIRECTORY`** places the application and the files it needs at runtime (e.g. shaders) into a *binaries* directory
  within the build directory.
- **`FIND_PACKAGE`** loads the engine and GLFW, which vcpkg installed.
- **`TARGET_LINK_LIBRARIES`** links the application with the backends it uses. This example uses both, but you can remove the one you do not
  need. Each backend links the core libraries of the engine automatically.
- **`LITEFX_DEPLOY_RUNTIME`** copies files the engine loads at runtime next to the application. For example, the DirectX 12 backend requires
  the [Agility SDK](https://devblogs.microsoft.com/directx/directx12agility/) runtime.

!!! tip "Requiring specific backends"

    To make sure the engine was built with the backends your application uses, list them as components:
    `FIND_PACKAGE(LiteFX CONFIG REQUIRED COMPONENTS Vulkan DirectX12)`. Configuring then fails with a clear message, if a backend is
    missing.

## Building the project

Configure and build the project on the command line (in the *Developer PowerShell*, if you use the vcpkg bundled with Visual Studio):

```powershell
cmake --preset windows-msvc-x64-debug
cmake --build --preset windows-msvc-x64-debug
```

When you configure the project for the first time, vcpkg builds the engine and its dependencies, which takes a while. Later runs use the
cached binaries.

In Visual Studio, open the project folder instead (*File* > *Open* > *Folder*). Visual Studio picks up the preset, and you can build
and debug the application from the toolbar.

For now, the build fails, because the application sources are still missing. Continue with the [first tutorial](../tutorials/first-triangle/index.md)
to write them.

## Next steps

The engine provides CMake helpers to integrate shaders and assets into your build:

- **`TARGET_ADD_SHADER_PROGRAM`** compiles shaders for all backends and copies them next to your application. The
  [tutorial](../tutorials/first-triangle/shaders.md) uses it for its first shaders.
- **`TARGET_ADD_ASSET_DIRECTORY`** copies assets, such as textures or models, next to your application.

The [sample project](https://github.com/crud89/LiteFX-Sample) demonstrates both.
