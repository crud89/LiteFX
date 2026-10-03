# Adjacent Topics

This section contains adjacent topics that are not part of the core engine.

| # | Walkthrough | Topic |
|:-:|---|---|
| 1 | [Dear ImGui](imgui.md) | Integrating a user interface library using [Dear ImGui](https://github.com/ocornut/imgui) |

The [samples](https://github.com/crud89/LiteFX/tree/main/src/Samples/) in the repository build on each other: each one starts from the
*basic rendering* sample and adds one technique. These walkthroughs follow that structure and describe only what each sample changes, so
start with [basic rendering](basic-rendering.md) and then pick whatever you need.

## Running the samples

All samples share the same controls:

| Key | Action |
|---|---|
| ++f7++ | Toggle V-Sync |
| ++f8++ | Toggle full screen |
| ++f9++ | Switch to the Vulkan backend |
| ++f10++ | Switch to the DirectX 12 backend |
| ++esc++ | Exit the application |

++f11++ and ++f12++ are deliberately unused, as debuggers such as Visual Studio, RenderDoc and PIX reserve them.

To build the samples, configure the engine with `LITEFX_BUILD_EXAMPLES` enabled, which is the default. The executables and their shaders and
assets are placed in the `binaries` directory of your build.
