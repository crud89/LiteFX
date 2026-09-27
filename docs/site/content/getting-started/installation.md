# Installation

There are three ways to use LiteFX in your project:

- **vcpkg (recommended):** vcpkg builds the engine and all of its dependencies for you. This is the easiest way, and the one the other guides
  use.
- **FetchContent:** CMake downloads the engine sources and builds them as part of your project. This is useful if you want to build the engine
  with your own settings, or work with an unreleased version.
- **Manual installation:** you build and install the engine yourself (or use a release package), and point your project to the installation.

!!! Tip

    The [project template](https://github.com/crud89/LiteFX-Template) provides a ready-to-go basis for new projects using the vcpkg installation
    method.

## Using vcpkg { #vcpkg }

### Installing vcpkg

=== "Visual Studio"

    Visual Studio comes with vcpkg included: it is part of the *C++ CMake tools for Windows*, which are installed with the *Desktop development
    with C++* workload. If it is missing, add the *vcpkg package manager* component in the Visual Studio Installer.

    Visual Studio sets the `VCPKG_ROOT` environment variable to the bundled vcpkg, both for CMake projects opened in the IDE and in the
    *Developer PowerShell*. No further setup is required.

=== "Standalone"

    If you do not use Visual Studio, or want to use a specific vcpkg version, clone and bootstrap vcpkg into a directory of your choice, and
    set the `VCPKG_ROOT` environment variable to it:

    ```powershell
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    C:\vcpkg\bootstrap-vcpkg.bat
    setx VCPKG_ROOT C:\vcpkg
    ```

    `setx` stores the variable permanently, but only for new terminal sessions. Open a new terminal before you continue.

Your project refers to vcpkg through `VCPKG_ROOT` (see [project setup](project-setup.md#cmake-presets)), so it works with both variants
without changes.

### Adding the LiteFX registry

LiteFX is provided through its own [vcpkg registry](https://github.com/crud89/LiteFX-Registry), which you add to your project in a
`vcpkg-configuration.json` file next to your `vcpkg.json` manifest. All other dependencies come from the official vcpkg registry:

```json title="vcpkg-configuration.json"
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/microsoft/vcpkg",
    "baseline": "9e593bb18ea69cc5095e012465dcd675a822ed0d"
  },
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/crud89/LiteFX-Registry",
      "baseline": "<latest commit of the LiteFX registry>",
      "packages": [ "litefx", "directx-warp" ]
    }
  ]
}
```

Each `baseline` pins the versions a registry provides:

- The **default registry** baseline is the vcpkg commit used by the current LiteFX release. You can use a newer commit, but staying on the
  same one avoids version conflicts.
- The **LiteFX registry** baseline must be a commit of the [LiteFX registry](https://github.com/crud89/LiteFX-Registry/commits/main/). Use the
  latest one to get the latest release of the engine.

### Adding the dependency

Add `litefx` to the dependencies in your `vcpkg.json` manifest. The [project setup](project-setup.md#vcpkg-manifest) guide shows a complete
manifest. By default, both backends are built. The following features are available:

| Feature | Default | Description |
|---|---|---|
| `vulkan` | :material-check: | Builds the Vulkan backend. |
| `dx12` | :material-check: | Builds the DirectX 12 backend. |
| `glm` | | Adds converters between the engine's math types and [glm](https://github.com/g-truc/glm). |
| `dx-math` | | Adds converters between the engine's math types and [DirectXMath](https://github.com/microsoft/DirectXMath). |
| `pix-support` | | Adds support for [PIX](https://devblogs.microsoft.com/pix/) markers in the DirectX 12 backend (x64 only). |
| `debug-markers` | | Emits debug markers in command buffers and queues, which are shown by graphics debuggers. |

For example, to build only the Vulkan backend with glm support:

```json
{
  "name": "litefx",
  "default-features": false,
  "features": [ "vulkan", "glm" ]
}
```

## Using FetchContent { #fetchcontent }

With [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html), CMake downloads the engine sources while configuring your
project and builds them as part of it:

```cmake title="CMakeLists.txt"
INCLUDE(FetchContent)

# Only build the engine itself, without samples and tests.
SET(LITEFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(LITEFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(LiteFX
    GIT_REPOSITORY https://github.com/crud89/LiteFX.git
    GIT_TAG        main   # Pin a release tag or commit for reproducible builds.
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  src    # The engine's build script is located in the src/ directory.
)

FetchContent_MakeAvailable(LiteFX)
```

The engine targets (e.g. `LiteFX::Vulkan`) and the CMake helpers (e.g. `LITEFX_DEPLOY_RUNTIME`) are then available, just as with a
package installed by vcpkg.

The engine's dependencies still have to be provided. vcpkg only reads the manifest of the top-level project, so add them to your own
`vcpkg.json`:

```json title="vcpkg.json"
{
  "name": "myapp",
  "version": "1.0",
  "builtin-baseline": "9e593bb18ea69cc5095e012465dcd675a822ed0d",
  "dependencies": [
    {
      "name": "spdlog",
      "default-features": false,
      "features": [ "tz-offset" ]
    },
    "directx-headers",
    "directx12-agility",
    "directx-dxc",
    "d3d12-memory-allocator",
    "vulkan",
    "vulkan-memory-allocator",
    "spirv-reflect",
    "directxmath",
    "glm",
    "glfw3"
  ]
}
```

!!! note

    - The engine uses `spdlog` with `std::format` instead of `fmt`, which is why its default features are disabled.
    - The engine's build script requires **CMake 4.0 or newer**.
    - Building the engine as part of your project increases your build times. If you do not need to change the engine itself, prefer vcpkg.

## Manual installation { #manual }

You can also build and install the engine yourself. Clone the repository, then configure, build and install it with one of its presets (see
`src/CMakePresets.json` for all of them):

```powershell
git clone https://github.com/crud89/LiteFX.git
cd LiteFX/src
cmake --preset windows-msvc-x64-release
cmake --build --preset windows-msvc-x64-release
cmake --install out/build/windows-msvc-x64-release --prefix C:\libs\LiteFX
```

The engine uses vcpkg for its own dependencies, so `VCPKG_ROOT` must be set (see [installing vcpkg](#installing-vcpkg)). Alternatively,
download a pre-built package from the [releases](https://github.com/crud89/LiteFX/releases), which contains the same files as the
installation directory.

To use the installation in your project, add it to the CMake search path, for example in your preset:

```json
"cacheVariables": {
  "CMAKE_PREFIX_PATH": "C:/libs/LiteFX"
}
```

!!! warning "Dependencies"

    The installed package still requires the engine's dependencies when you call `FIND_PACKAGE(LiteFX)`. Provide them with the same
    versions the engine was built with, for example by adding them to your vcpkg manifest as shown for [FetchContent](#fetchcontent). Also
    make sure that your project uses a compatible compiler and runtime library (e.g. the same Visual C++ version), as the engine is a C++
    library.
