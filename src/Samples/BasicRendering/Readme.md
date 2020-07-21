# LiteFX Samples - 01: Basic Rendering

A sample that implements the basic rendering functionality of the LiteFX engine. It draws a rotating, vertex-colored tetrahedron.

<p align="center">
  <img src="docs/samples/BasicRendering/screenshot.jpg">
</p>

For a detailed explanation of this sample, refer to the [project wiki](https://github.com/Aschratt/LiteFX/wiki/Sample-01:-Basic-Rendering).

## Covered aspects

The sample covers the following aspects:

- Defining and building an application instance, that uses GLFW and Vulkan.
- Defining a forward render pipeline.
- Defining and using vertex, index and uniform buffers.
- Using staging buffers to transfer data.
- Using basic vertex and fragment shader stages.
- Rebuilding the swap chain to resize frame buffers.
- Properly releasing resources.