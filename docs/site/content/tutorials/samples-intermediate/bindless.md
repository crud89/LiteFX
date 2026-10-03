# Bindless rendering

!!! abstract "Sample 08"

    [`Samples/Bindless`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Bindless) · builds on
    [basic rendering](basic-rendering.md) and [uniform arrays](uniform-arrays.md).

With one descriptor set per object, drawing many objects means binding many sets. *Bindless* rendering turns this around: all objects live in
one array that the shader indexes itself, and nothing has to be rebound between draw calls. The sample draws 100,000 instances of the same
object, each with its own transform, color and rotation axis.

## Unbounded arrays

The instance data is a *runtime array*, an array whose size the shader does not know:

```hlsl
struct InstanceData
{
    float4x4 Transform;
    float4 Color;
    float4 Axis;
};

ConstantBuffer<CameraData> camera  : register(b0, space0);
ConstantBuffer<DrawData> drawData  : register(b0, space1);
StructuredBuffer<InstanceData> instanceBuffer[] : register(t0, space2);
```

The vertex shader picks its instance by the index the GPU provides for each instance of a draw call:

```hlsl
VertexData main(in VertexInput input, uint id : SV_InstanceID)
{
    InstanceData instance = instanceBuffer[NonUniformResourceIndex(id)].Load(0);
    // ... transform the vertex with instance.Transform ...
}
```

`NonUniformResourceIndex` tells the GPU that neighboring threads may read different elements, which it must know to access the array
correctly.

Since the size is missing from the shader, reflection cannot determine it either, so the pipeline layout is given a hint:

```cpp
.layout(shaderProgram->reflectPipelineLayout(std::array { PipelineBindingHint::runtimeArray(DescriptorSets::InstanceData, 0u, NUM_INSTANCES) }))
```

## One descriptor for everything

The buffer holds all instances, and a single descriptor set covers the whole array:

```cpp
auto& instanceBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::InstanceData);
auto instanceBuffer = m_device->factory().createBuffer("Instance Buffer", instanceBindingLayout, 0, ResourceHeap::Resource, sizeof(InstanceBuffer), NUM_INSTANCES);
auto instanceBinding = instanceBindingLayout.allocate(NUM_INSTANCES, { { 0, *instanceBuffer } });
commandBuffer->transfer(static_cast<const void*>(&instanceData), sizeof(instanceData), *instanceBuffer, 0, NUM_INSTANCES);
```

## Instanced drawing

All instances are drawn with one call, whose second argument is the number of instances:

```cpp
commandBuffer->bind({ &cameraBindings, &drawDataBindings, &instanceBindings });
commandBuffer->bind(vertexBuffer);
commandBuffer->bind(indexBuffer);
commandBuffer->drawIndexed(indexBuffer.elements(), NUM_INSTANCES);
```

The animation is computed in the shader, so the instance data stays unchanged after the upload. Only the current time is written per frame,
in a small per-frame buffer:

```cpp
drawData.Time = time;
drawData.Speed = glm::radians(42.0f);
drawDataBuffer.map(static_cast<const void*>(&drawData), sizeof(drawData), backBuffer);
```

Each instance rotates around its own axis, so the shader builds the rotation matrix from `instance.Axis` and the elapsed time.

!!! tip "The other way around"

    Shader model 6.6 offers an alternative that needs no array binding at all. The [dynamic descriptors](dynamic-descriptors.md) tutorial
    shows the same sample with `ResourceDescriptorHeap`.
