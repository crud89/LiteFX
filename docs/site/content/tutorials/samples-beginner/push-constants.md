# Push constants

!!! abstract "Sample 03"

    [`Samples/PushConstants`](https://github.com/crud89/LiteFX/tree/main/src/Samples/PushConstants) · builds on
    [basic rendering](basic-rendering.md).

Descriptor sets are the right tool for data that many draw calls share, but they are heavy for small values that change with every draw call.
*Push constants* are the alternative: a small block of data that is written directly into the command buffer, with no buffer, no descriptor
and no synchronization. The sample draws nine copies of the same object, each with its own transform and color.

!!! note "Size limits"

    Push constants are limited: Vulkan guarantees at least 128 bytes, DirectX 12 works with a budget of 64 DWORDs for the whole pipeline
    layout. They are meant for a handful of values, such as an index, a transform or a color.

## Declaring the range

In the shader, push constants are a constant buffer like any other. The sample puts the transform and the color of an object into it:

```hlsl
struct TransformData
{
    float4x4 Model;
    float4 Color;
};

ConstantBuffer<CameraData> camera : register(b0, space0);

#ifdef SPIRV
[[vk::push_constant]] ConstantBuffer<TransformData> transform;
#elif DXIL
ConstantBuffer<TransformData> transform : register(b0, space1);
#endif
```

The declaration differs between the backends: SPIR-V marks the buffer with an attribute, while DXIL uses a regular binding. The
`SPIRV` and `DXIL` defines are set by the engine's shader targets, so the same source compiles for both.

Shader reflection cannot tell a push constant range from a regular constant buffer in every case, so the pipeline layout is told which
binding is a push constant range:

```cpp
.layout(shaderProgram->reflectPipelineLayout(std::array { PipelineBindingHint::pushConstants(1u, 0u) }))
```

The arguments are the space and the register of the range, matching `space1` and `b0` in the shader.

## Drawing with push constants

The per-object data is filled on the stack and pushed before each draw call. Only the camera is bound as a descriptor set:

```cpp
commandBuffer->bind(cameraBindings);
commandBuffer->bind(vertexBuffer);
commandBuffer->bind(indexBuffer);

for (int i(0); i < 9; ++i)
{
    ObjectBuffer buffer {
        .World = glm::translate(glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f)), translations[i]),
        .Color = colors[i]
    };

    commandBuffer->pushConstants(*geometryPipeline.layout()->pushConstants(), &buffer);
    commandBuffer->drawIndexed(indexBuffer.elements());
}
```

Each `pushConstants` call records a copy of the data into the command buffer, so the next draw call sees its own values. The per-frame
transform buffer and its descriptor sets from the basic rendering sample are no longer needed.

Because the objects are drawn at different positions and can overlap, the pipeline enables the depth test:

```cpp
.depthState(DepthStencilState::DepthState { .Operation = CompareOperation::LessEqual })
```
