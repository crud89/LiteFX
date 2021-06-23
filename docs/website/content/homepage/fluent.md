---
title: "Fluent Interface"
weight: 2
---

The explicit nature of modern graphics APIs can be pretty obstrusive. LiteFX offers a fluent interface that helps you to quickly configure commonly used objects, such as render passes, pipeline state and devices.

```cxx
auto pipeline = renderPass->makePipeline(0, "Basic Pipeline")
    .layout()
        .shaderProgram()
            .addVertexShaderModule("shaders/vs.dxi")
            .addFragmentShaderModule("shaders/ps.dxi")
            .go()
        .addDescriptorSet(0, ShaderStage::Vertex | ShaderStage::Fragment)
            .addUniform(0, sizeof(CameraBuffer))
            .addImage(1)
            .go()
// ...
```