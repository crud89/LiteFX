---
title: "Fluent Interface"
weight: 2
---

The explicit nature of modern graphics APIs can be pretty obtrusive. LiteFX offers a fluent builder interface that helps you to quickly configure commonly used objects in a domain language. Here is an example:

```cxx
SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
    .topology(PrimitiveTopology::TriangleList)
    .indexType(IndexType::UInt16)
    .vertexBuffer(sizeof(Vertex), 0)
        .withAttribute(0, BufferFormat::XYZ32F, 
            offsetof(Vertex, Position), AttributeSemantic::Position)
        .withAttribute(1, BufferFormat::XYZW32F,
            offsetof(Vertex, Color), AttributeSemantic::Color)
        .add();

UniquePtr<RenderPass> renderPass = device->
    buildRenderPass("Render Pass")
    .renderTarget(RenderTargetType::Present, 
        Format::B8G8R8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.f })

UniquePtr<RenderPipeline> renderPipeline = 
    device->buildRenderPipeline(*renderPass, "Render Pipeline")
    .viewport(viewport)
    .scissor(scissor)
    .inputAssembler(inputAssembler)
```