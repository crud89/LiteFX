# LiteFX

An extensible, descriptive, modern computer graphics and rendering engine.

<p align="center">
  <img src="/docs/img/banner_m.jpg">
</p>

![GitHub](https://img.shields.io/github/license/aschratt/litefx.svg) ![GitHub Workflow Status](https://img.shields.io/github/workflow/status/aschratt/litefx/release?label=Build) ![Latest release](https://img.shields.io/github/release/aschratt/litefx.svg) ![Released at](https://img.shields.io/github/release-date/aschratt/litefx.svg) [![Documentation](https://img.shields.io/badge/docs-gh--pages-70dcf4.svg)](https://litefx.github.io/docs/#/)

## About

LiteFX is a computer graphics engine, I've developed to learn about modern graphics APIs, like Vulkan 🌋 and DirectX 12 ❎. It follows an descriptive approach, which means that an application focuses on configuring what it needs and the engine then takes care of handling those requirements. To do this, the API provides a fluent interface. Here is an example of how to easily define a Vulkan graphics pipeline with a few lines of code:

```cxx
auto pipeline = device->build<VulkanRenderPipeline>()
    .defineLayout()
        .setRasterizer()
            .withCullMode(CullMode::BackFaces)
            .withCullOrder(CullOrder::ClockWise)
            .go()
        .setInputAssembler()
            .withTopology(PrimitiveTopology::TriangleList)
            .withIndexType(IndexType::UInt16)
            .make<VulkanVertexBufferLayout>(sizeof(MyVertex), 0)
                .addAttribute(0, BufferFormat::XYZ32F, offsetof(MyVertex, Position))
                .addAttribute(1, BufferFormat::XYZW32F, offsetof(MyVertex, Color))
                .go()
            .go()
        .setShaderProgram()
            .addVertexShaderModule("shaders/vertex.spv")
            .addFragmentShaderModule("shaders/fragment.spv")
            .addDescriptorSet(0, ShaderStage::Vertex | ShaderStage::Fragment)
                .addUniform(0, sizeof(MyBuffer))
                .addTexture(1)
                .addSampler(2)
                .go()
            .go()
        .addViewport()
            .withRectangle(RectF(0.f, 0.f, 800.f, 600.f))
            .go()
        .go()
    .defineRenderPass()
        .attachPresentTarget(true)
        .go()
    .go();
```

The core of the fluent interface is a flexible [builder architecture](https://github.com/Aschratt/LiteFX/wiki/builder-guide), that allows to easily extent built-in types provide custom implementations. Furthermore it also allows to provide whole custom rendering backends.

LiteFX is written in modern C++17, following established design patterns to make it easy to learn and adapt. It's focus is make the performance of modern graphics APIs easily accessible, whilst retaining full flexibility.

## Installation

If you just want to start using LiteFX, you can acquire binaries of the latest version from the [releases page](./releases/).

You can also build the sources on your own. Create a new directory from where you want to build the sources. Then open your shell and clone the repository:

    git clone https://github.com/Aschratt/LiteFX.git .

<!-- TODO: CMake & Build -->

## Getting Started

<!-- TODO: Samples and Wiki tutorials -->

## Contribute

<!-- TODO: bug reports, suggestions, pull requests -->

## Projects using LiteFX

<!-- Currently none, lol -->

## License

LiteFX is licensed under the permissive [MIT licsense](./LICENSE). The documentation (i.e. the contents of the `docs` folder of this repository, especially the LiteFX logo, banner and icon) is licensed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/).

If you want to use LiteFX in your projects, linking to the engine and/or putting the logo in your project description is much appreciated.
