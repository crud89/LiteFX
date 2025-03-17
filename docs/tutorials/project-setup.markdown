# Project Setup

This guide walks you through the steps required to start a new project using the LiteFX graphics engine. The easiest way to quickly get started is to use the [project template](https://github.com/crud89/LiteFX-Template). For a more elaborate project, take a look at the [sample project](https://github.com/crud89/LiteFX-Sample). The rest of the guide covers the steps required to setup a project manually, which may be helpful if you want to integrate the engine into an existing project. While there are several ways of consuming the project, the preferred way is using [CMake](https://cmake.org/) and [vcpkg](https://github.com/microsoft/vcpkg). The rest of this guide assumes that you are too using a similar build process. In case you want to use a different build system, you can, but you may have to adjust dependencies and build scripts accordingly. 

In any case, please note, that this guide also does not substitute a comprehensive tutorial about CMake or vcpkg. If you want to get to know the details about those, check out [this](https://cmake.org/cmake/help/latest/index.html) and [this](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started). Also this guide assumes that the project should be set up and built using Visual C++ on Windows. For other environments, you may have to perform some modifications to the sources provided below.

## Project Structure

The first thing to do is creating a bunch of files in an empty directory, that contain the build scripts, meta-data and project sources. The following list gives you an overview over the files that need to be created and about their function within the project. The contents of the files are detailled further down in this guide.

- *CMakeLists.txt*: contains the project build script.
- *CMakePresets.json*: contains build templates (e.g., common variables, definitions, etc.) for CMake to use.
- *main.cpp*: the project's main source file.
- *main.h*: the project's main header file.
- *vcpkg.json*: the project manifest, that stores meta-data and dependencies for the project.
- *vcpkg-configuration.json*: required for vcpkg to pick up the [LiteFX registry](https://github.com/crud89/LiteFX-Registry).

Apart from the files related to CMake and vcpkg, we are not going to write any code in this guide. The [next guide](md_docs_tutorials_quick_start.html) will walk you through the steps required to write a simple rendering application.

## Setting up vcpkg.

The first files we're going to initialize are the files related to vcpkg. The first file, *vcpkg.json* is called a *manifest file* and tells vcpkg about your project and its dependencies. Everything despite the list of dependencies is actually purely optional. Within the dependencies, we provide two libraries. The first one is the engine itself (after all, that's why you're here for) and the second one being [GLFW3](https://www.glfw.org/), a cross-platform window system, which we use to setup our app window with and which we can use to handle user input. If you don't want to use it, you can choose any alternative or even write your own window system. However, keep in mind that you might have to change some parts of the [quick start guide](md_docs_tutorials_quick_start.html) accordingly.

```json
{
  "name": "my-app",
  "version-string": "1.0",
  "supports": "windows & !arm",
  "dependencies": [
    "litefx",
    "glfw3"
  ]
}
```

The second file we need to setup is the *vcpkg-configuration.json* file. This file specifies where to find the engine package as well as some of its dependencies. This step is required, as the engine applies some customizations to official ports and provides ports that are currently not yet available in the default package registry. If you start a new project, just go ahead and copy the following contents into the *vcpkg-configuration.json* file:

```json
{
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/crud89/LiteFX-Registry",
      "reference": "litefx-registry",
      "baseline": "0",
      "packages": [ 
        "litefx", 
        "d3d12-memory-allocator",
        "directx12-agility",
        "directx-warp",
        "spdlog",
        "vulkan-memory-allocator",
        "winpixeventruntime"
      ]
    }
  ],
}
```

Finally, we need to install vcpkg. The straightforward option when you already put all your files inside a valid git repository, is to add it as a git submodule as described [here](https://learn.microsoft.com/en-us/vcpkg/concepts/continuous-integration#acquire-vcpkg). But of course any other installation method will work as well. It is strongly adviced to take your time and familiarize yourself with setting up vcpkg on your system depending on your needs. If you are not sure about what's the easiest option for you, then start by using the version integrated into Visual Studio.

## Setting up CMake.

Now that we've set up vcpkg, it's time to configure CMake to build your project. First, we need to setup a preset, that CMake uses for the build. The following simple *CMakePresets.json* file is sufficient as a starting point. Please modify the `VCPKG_ROOT` variable to point to your vcpkg installation directory. This may vary depending on your way of installation, as described earlier. In the provided example below it is assumed, that vcpkg got cloned as a submodule into a `modules/` directory in the project root. For more details about how to integrate vcpkg into a CMake build process in varying scenarios, refer to [this guide](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration).

```json
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
      "environment": {
        "VCPKG_ROOT": "${sourceDir}/modules/vcpkg/"
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

The next and final step is to setup the `CMakeLists.txt` file with a build script that describes how to perform a build of your app. The following file defines a new project called `MyApp` and sets up some build variables. First, it requires C++23 as a language standard. It also sets the `CMAKE_RUNTIME_OUTPUT_DIRECTORY` variable, which ensures that all dependencies are copied into a common *binaries* directory during the build. The `FIND_PACKAGE` lines make sure that the project knows about the engine and the aforementioned glfw framework. Using all this information it defines an executable project based on the two provided source files, which we implement in the [next guide](md_docs_tutorials_quick_start.html). The last line defines a dependency between the engine backends (here both available backends are used, but if you wish you can omit what you do not like to include), glfw and the executeable.

```cmake
CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
PROJECT(MyApp LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/binaries/>")

FIND_PACKAGE(LiteFX CONFIG REQUIRED)
FIND_PACKAGE(glfw3 CONFIG REQUIRED)

ADD_EXECUTABLE(MyApp
  "main.h"
  "main.cpp"
)

TARGET_LINK_LIBRARIES(MyApp PRIVATE LiteFX.Backends.Vulkan LiteFX.Backends.DirectX12 glfw)
```

## Performing a build.

The next step is to write the actual code and is described in [the quick start guide](md_docs_tutorials_quick_start.html). In order to compile your application after you've changed the code, you can run the following command. For now, however, this will produce errors, as we did not have defined any code yet.

```sh
cmake . --preset windows-msvc-x64-debug
cmake --build out/build/windows-msvc-x64-debug
```

LiteFX provides macros to include shaders and assets into your build process. If you want to use those, take a look at the [sample project](https://github.com/crud89/LiteFX-Sample) for a demonstration on how to use them.