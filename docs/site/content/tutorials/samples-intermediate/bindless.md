# Bindless rendering

!!! abstract "Sample 08"

    [`Samples/Bindless`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Bindless) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md) and [uniform arrays](../samples-beginner/uniform-arrays.md).

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

The pipeline layout is reflected from the shader as usual. The array's size is not part of the layout; it is only decided when a descriptor
set for it is allocated:

```cpp
.layout(shaderProgram->reflectPipelineLayout())
```

## One descriptor for everything

The buffer holds all instances, and a single descriptor set covers the whole array. Its size is the first argument of `allocate`:

```cpp
auto& instanceBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::InstanceData);
auto instanceBuffer = m_device->factory().createBuffer("Instance Buffer", instanceBindingLayout, 0, ResourceHeap::Resource, sizeof(InstanceBuffer), NUM_INSTANCES);
auto instanceBinding = instanceBindingLayout.allocate(NUM_INSTANCES, { { 0, *instanceBuffer } });
```

The elements of the buffer are aligned to the GPU's requirements for structured buffers, so they are not necessarily packed as tightly as
the C++ array. The instances are therefore uploaded element by element, as in the [uniform
arrays](../samples-beginner/uniform-arrays.md) sample, which places each one at its aligned offset:

```cpp
auto instances = instanceData
    | std::views::transform([](const InstanceBuffer& instance) { return static_cast<const void*>(&instance); })
    | std::ranges::to<Array<const void*>>();
commandBuffer->transfer(instances, sizeof(InstanceBuffer), *instanceBuffer, 0);
```

Copying the whole array at once would only work as long as the element size happens to match the alignment, which differs between
backends and devices (see [buffer layout](../../guides/descriptor-bindings.md#buffer-layout)).

The array occupies a contiguous range of the global descriptor heap, so its size is only limited by the heap's capacity. The set is allocated
once and bound to every command buffer, because the instance buffer never changes. If it did, the sample would either need one set per frame
in flight, like the other samples, or contain each instance buffer once per frame in flight and select the current one in the shader.

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

    Shader model 6.6 offers an alternative that needs no array binding at all. The [dynamic descriptors](../samples-advanced/dynamic-descriptors.md) 
    tutorial shows the same sample with `ResourceDescriptorHeap`.
