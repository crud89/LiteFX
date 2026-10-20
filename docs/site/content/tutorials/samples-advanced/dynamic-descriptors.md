# Dynamic descriptors

!!! abstract "Sample 14"

    [`Samples/DynamicDescriptors`](https://github.com/crud89/LiteFX/tree/main/src/Samples/DynamicDescriptors) · builds on
    [bindless](../samples-intermediate/bindless.md).

Shader model 6.6 introduced a second way to do bindless rendering: instead of binding an array of resources, the shader indexes the GPU's
global descriptor heap directly, with `ResourceDescriptorHeap`. The type of a descriptor is then only decided when it is bound, not when the
pipeline is created. The sample draws the same 100,000 instances as the bindless sample.

## Enabling the feature

Dynamic descriptors are an optional device feature. Enabling it checks that the device supports shader model 6.6, and on Vulkan also enables
the `VK_KHR_shader_untyped_pointers` extension, which the heap indexing relies on:

```cpp
m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM,
    m_viewport->getRectangle().extent(), 3, false, GraphicsDeviceFeatures { .DynamicDescriptors = true }));
```

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

- **The instance buffer is no longer declared.** The shader reaches it through the heap, so the pipeline layout only contains the camera
  and the draw data.
- **`SV_StartInstanceLocation`** provides the offset of the first descriptor, which the draw call passes in. Adding it to the instance ID
  gives the descriptor index of the instance.

## Allocating descriptors on the heap

Since the instance buffer is not part of the pipeline layout, there is no descriptor set to bind it to. Instead, the sample allocates a
*descriptor range*: a block of descriptors directly on the device's global descriptor heap.

```cpp
m_instanceDescriptors = m_device->allocateGlobalDescriptors(DescriptorHeapType::Resource, NUM_INSTANCES);
```

The buffer is created without a layout, and written into the range much like into a descriptor set:

```cpp
auto instanceBuffer = m_device->factory().createBuffer("Instance Buffer", BufferType::Storage, ResourceHeap::Resource, sizeof(InstanceBuffer), NUM_INSTANCES);
m_instanceDescriptors->update(0u, DescriptorType::StructuredBuffer, *instanceBuffer, 0u, NUM_INSTANCES);
m_instanceBaseIndex = m_instanceDescriptors->globalIndex();
```

`update` writes one descriptor per element of the buffer, starting at the first descriptor of the range. The descriptor type is passed here
rather than taken from a pipeline layout, which is what makes these descriptors *dynamic*. `globalIndex` returns the position of the range's
first descriptor on the heap, which is the index the shader needs.

The range owns its descriptors, and releasing it frees them. The sample keeps it in a member and resets it when the backend stops, before
the device is released:

```cpp
auto stopCallback = [this]<typename TBackend>(TBackend* backend) {
    m_instanceDescriptors.reset();
    backend->releaseDevice("Default");
};
```

The pipeline layout needs no hint anymore and is reflected as usual:

```cpp
.layout(shaderProgram->reflectPipelineLayout())
```

## Passing the base index

The base index reaches the shader through the draw call, as the first instance. The descriptor range itself is not bound: its descriptors
are already part of the global heap, which the shader indexes directly.

```cpp
commandBuffer->bind({ &cameraBindings, &drawDataBindings });
commandBuffer->bind(vertexBuffer);
commandBuffer->bind(indexBuffer);
commandBuffer->drawIndexed(indexBuffer.elements(), NUM_INSTANCES, 0u, 0u, m_instanceBaseIndex);
```

The GPU adds it to each instance ID, so the shader reads the right descriptors without any extra buffer or push constant.

## When to use which

| | [Bindless arrays](../samples-intermediate/bindless.md) | Dynamic descriptors |
|---|---|---|
| Shader | Declares the array with its binding | Indexes `ResourceDescriptorHeap` |
| Descriptor type | Fixed by the pipeline layout | Chosen when writing the range |
| Descriptors | Descriptor set, bound to the pipeline | Descriptor range on the global heap, not bound |
| Requirements | None beyond the engine's base requirements | Shader model 6.6 and the `DynamicDescriptors` device feature |
| Fits | A known set of resource types | Mixed resources, or resources known only at runtime |
