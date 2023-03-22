# Project Setup

This guide demonstrates the steps that are required to setup a simple application that uses the LiteFX engine. It also shows you how to integrate shaders and assets into your build process.

## Runtime and Dependency Installation

LiteFX makes heavy use of [CMake](https://cmake.org/) for building applications. Using the engine is straightforward, if your application uses CMake too. This guide will only focus on explaining how to setup a new project using CMake. Other build systems might work similar. In fact, all required sources are available from the binary release or manual build. For details about how to perform a manual build, read the [installation notes](index.html#autotoc_md14).

Before we start, we need to make sure that your application can access the engine and it's dependencies. Start of by downloading the [latest release](https://github.com/crud89/LiteFX/releases) and extract and copy it to a location you prefer. 

The engine on itself does, however, require you to install its dependencies too. There are several ways to do this:

- Manual dependency installation: take a look at the [dependency list](index.html#autotoc_md22) and download all the required dependencies and extract or install them into a common folder. Ideally you can use the same directory you already used for the LiteFX release. All downloads may reside in their own respective sub-directories, though. Note that, if you are using a release version (and not a manual build), all optional dependencies are required, thus you also have to install them. The only exception are the dependencies used by the samples, though in the following guide [glfw3](https://www.glfw.org/) is utilized, so you might want to install it too.
- Using CMake find scripts: more advanced users may use `FindXXX.cmake` files to locate the dependencies. However, this process is convoluted, tedious and error-prone and thus not further described in this guide.
- Use [vcpkg](https://github.com/microsoft/vcpkg): the engine itself uses *vcpkg* to install dependencies, so you could too. *vcpkg* makes dependency management easy, so this is the preferred way.

## Setup Project using CMake

Let's start creating our project. Create a new directory and within it, create the following files:

- *CMakeLists.txt*: contains the project build script.
- *main.h*: the project's main header file.
- *main.cpp*: the project's main source file.

Apart from the *CMakeLists.txt* file, we are not going to write any code in this guide. The [next guide](md_docs_tutorials_quick_start.html) will walk you through the steps required to write a simple rendering application.

Go ahead and open the *CMakeLists.txt* file with any text editor. Copy the following code into the file:

```cmake
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(MyLiteFXApp LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_PREFIX_PATH "...")
FIND_PACKAGE(LiteFX 1.0 CONFIG REQUIRED)

ADD_EXECUTABLE(MyLiteFXApp
  "main.h"
  "main.cpp"
)

TARGET_LINK_LIBRARIES(MyLiteFXApp PRIVATE LiteFX.Backends.Vulkan)   # For DirectX 12 use: LiteFX.Backends.DirectX12
```

Replace the `...` in line 5 with the directory, you've installed all your dependencies and the engine to. If you have installed LiteFX to another directory, add the following line below `SET(CMAKE_PREFIX_PATH "...")`:

```cmake
SET(LiteFX_DIR "...")
```

Again, replace the `...` with the directory, you've placed the LiteFX release to.

Next, we have to ensure, that the dynamic library files (i.e. the DLL-files) for all dependencies are copied to the build directory. Otherwise your app will not run properly. LiteFX provides a list of all dependencies in the `LITEFX_DEPENDENCIES` variable. Add the following code to the bottom of your *CMakeLists.txt* file to add a copy command for the build:

```cmake
FOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES})
  GET_TARGET_PROPERTY(DEPENDENCY_LOCATION ${DEPENDENCY} IMPORTED_LOCATION)

  IF(NOT DEPENDENCY_LOCATION)
    GET_TARGET_PROPERTY(DEPENDENCY_LOCATION_DEBUG ${DEPENDENCY} IMPORTED_LOCATION_DEBUG)
    GET_TARGET_PROPERTY(DEPENDENCY_LOCATION_RELEASE ${DEPENDENCY} IMPORTED_LOCATION_RELEASE)
    SET(DEPENDENCY_LOCATION "$<$<CONFIG:DEBUG>:${DEPENDENCY_LOCATION_DEBUG}>$<$<CONFIG:RELEASE>:${DEPENDENCY_LOCATION_RELEASE}>")
  ENDIF(NOT DEPENDENCY_LOCATION)

  ADD_CUSTOM_COMMAND(TARGET MyLiteFXApp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEPENDENCY_LOCATION} $<TARGET_FILE_DIR:MyLiteFXApp>)
ENDFOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES})
```

This loop looks complicated at first, but all it does is to look for the right library file, depending on the build configuration (`Debug` or `Release`) and copy the proper file into the build directory in a post-build event.

### Setup Project using vcpkg

If you choose to use *vcpkg*, then the *CMakeLists.txt* file will look slightly different. If you want to go ahead with the manual installation, you can skip this section.

```cmake
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

SET(VCPKG_MODULE_DIRECTORY "...")
SET(CMAKE_TOOLCHAIN_FILE "${VCPKG_MODULE_DIRECTORY}/scripts/buildsystems/vcpkg.cmake")

PROJECT(MyLiteFXApp LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 20)
SET(LiteFX_DIR "...")
FIND_PACKAGE(LiteFX 1.0 CONFIG REQUIRED)

ADD_EXECUTABLE(MyLiteFXApp "main.h" "main.cpp")

TARGET_LINK_LIBRARIES(MyLiteFXApp PRIVATE LiteFX.Backends.Vulkan)   # For the DirectX 12 target use: LiteFX.Backends.DirectX12. You can also add both targets here.

FOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES})
  GET_TARGET_PROPERTY(DEPENDENCY_LOCATION ${DEPENDENCY} IMPORTED_LOCATION)

  IF(NOT DEPENDENCY_LOCATION)
    GET_TARGET_PROPERTY(DEPENDENCY_LOCATION_DEBUG ${DEPENDENCY} IMPORTED_LOCATION_DEBUG)
    GET_TARGET_PROPERTY(DEPENDENCY_LOCATION_RELEASE ${DEPENDENCY} IMPORTED_LOCATION_RELEASE)
    SET(DEPENDENCY_LOCATION "$<$<CONFIG:DEBUG>:${DEPENDENCY_LOCATION_DEBUG}>$<$<CONFIG:RELEASE>:${DEPENDENCY_LOCATION_RELEASE}>")
  ENDIF(NOT DEPENDENCY_LOCATION)

  ADD_CUSTOM_COMMAND(TARGET MyLiteFXApp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEPENDENCY_LOCATION} $<TARGET_FILE_DIR:MyLiteFXApp>)
ENDFOREACH(DEPENDENCY ${LITEFX_DEPENDENCIES})
```

Again, replace the `...` in line 4 with the path to the *vcpkg* installation and set the `LiteFX_DIR` in line 9 to the release location. Unfortunately, LiteFX does not currently have its dedicated *vcpkg*-port, that's why this directory needs to be specified. Before we continue, create another file in the project directory and call it *vcpkg.json*. This file is called *manifest file* and is used by *vcpkg* to find the dependencies and install them when the project is configured later. Copy the following code to the manifest file:

```json
{
  "name": "my-litefx-app",
  "version": "1.0",
  "dependencies": [
    "spdlog",
    "glm",
    "directxmath",
    "directx-headers",
    "vulkan",
    "glfw3"
  ]
}
```

### Running CMake

The next thing we need to do is letting CMake configure our project. Open a command line interface and navigate to your project directory. Run the following command to configure your project and write the build to the *out/build/* subdirectory:

```sh
cmake . -B out/build/
```

Note that there are other ways to do this step. For example, Visual Studio's CMake integration is very straightforward and should be used, if you are working with Visual Studio anyway.

### Performing Builds

The next step is to write the actual code and is described in [the quick start guide](md_docs_tutorials_quick_start.html). In order to compile your application after you've changed the code, you can run the following command:

```sh
cmake --build out/build/
```

Currently this build will fail, because we have not yet written any code.

### Adding Shader Sources

During development you naturally have to work with shaders. LiteFX allows you to easily integrate shader sources into your build tree using CMake. You can use the `ADD_SHADER_MODULE` function to define a shader. The `TARGET_LINK_SHADERS` function can then be used to define a dependency to the shader module for your project. For more information, refer to the [project wiki](https://github.com/crud89/LiteFX/wiki/Shader-Module-Targets).

### Defining Asset Directories

Similar to shaders, you may want to integrate assets into your build process. Using LiteFX, you can define asset directories using the `TARGET_ADD_ASSET_DIRECTORY` function. This function will define a sub-directory that can be used by your executable to locate assets. For more information, refer to the [project wiki](https://github.com/crud89/LiteFX/wiki/Asset-Directories).