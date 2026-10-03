# Dynamic descriptors

!!! abstract "Sample 14"

    [`Samples/DynamicDescriptors`](https://github.com/crud89/LiteFX/tree/main/src/Samples/DynamicDescriptors) · builds on
    [bindless](../samples-intermediate/bindless.md).

Shader model 6.6 introduced a second way to do bindless rendering: instead of binding an array of resources, the shader indexes the GPU's
global descriptor heap directly, with `ResourceDescriptorHeap`. The type of a descriptor is then only decided when it is bound, not when the
pipeline is created. The sample draws the same 100,000 instances as the bindless sample.

## Indexing the heap in the shader

The array binding disappears from the shader. What remains is the index, which the shader uses to fetch the buffer from the heap:

```hlsl
ConstantBuffer<CameraData> camera  : register(b0, space1);
ConstantBuffer<DrawData> drawData  : register(b0, space2);

VertexData main(in VertexInput input, uint id : SV_InstanceID, uint baseId : SV_StartInstanceLocation)
{
    StructuredBuffer<InstanceData> instanceBuffer = ResourceDescriptorHeap[NonUniformResourceIndex(id + baseId)];
    InstanceData instance = instanceBuffer.Load(0);
    // ...
}
```

Two things change compared to the [bindless](../samples-intermediate/bindless.md) sample:

- **The instance buffer is no longer declared**, so the camera and draw data move to the spaces after the heap's own space.
- **`SV_StartInstanceLocation`** provides the offset of the first descriptor, which the draw call passes in. Adding it to the instance ID
  gives the descriptor index of the instance.

## Binding to the heap

Instead of allocating a descriptor set over an array, the sample allocates an empty set and binds the buffer into the heap:

```cpp
auto& instanceBindingLayout = geometryPipeline.layout()->descriptorSet(0u);
auto instanceBuffer = m_device->factory().createBuffer("Instance Buffer", instanceBindingLayout, 0, ResourceHeap::Resource, sizeof(InstanceBuffer), NUM_INSTANCES);
auto instanceBinding = instanceBindingLayout.allocate();
m_instanceBaseIndex = instanceBinding->bindToHeap(DescriptorType::StructuredBuffer, 0u, *instanceBuffer, 0u, NUM_INSTANCES);
```

`bindToHeap` returns the index of the first descriptor it bound. That index is what the shader needs, and the descriptor type is passed here
rather than taken from the pipeline layout, which is what makes these descriptors *dynamic*.

The pipeline layout gets a matching hint:

```cpp
.layout(shaderProgram->reflectPipelineLayout(std::array { PipelineBindingHint::resourceHeap(0u, 0u, NUM_INSTANCES) }))
```

## Passing the base index

The base index reaches the shader through the draw call, as the first instance:

```cpp
commandBuffer->bind({ &cameraBindings, &instanceBindings, &drawDataBindings });
commandBuffer->bind(vertexBuffer);
commandBuffer->bind(indexBuffer);
commandBuffer->drawIndexed(indexBuffer.elements(), NUM_INSTANCES, 0u, 0u, m_instanceBaseIndex);
```

The GPU adds it to each instance ID, so the shader reads the right descriptors without any extra buffer or push constant.

## When to use which

| | [Bindless arrays](../samples-intermediate/bindless.md) | Dynamic descriptors |
|---|---|---|
| Shader | Declares the array with its binding | Indexes `ResourceDescriptorHeap` |
| Descriptor type | Fixed by the pipeline layout | Chosen when binding |
| Requirements | Descriptor indexing | Shader model 6.6 |
| Fits | A known set of resource types | Mixed resources, or resources known only at runtime |
