# Indirect rendering

!!! abstract "Sample 13"

    [`Samples/Indirect`](https://github.com/crud89/LiteFX/tree/main/src/Samples/Indirect) · builds on
    [bindless](bindless.md) and [compute](compute.md).

Normally the CPU decides what to draw: one `drawIndexed` call per object, with the arguments known on the CPU. *Indirect* rendering moves
that decision to the GPU. The draw arguments live in a buffer that a compute shader fills, and the draw call only points at it. The sample
places 163,840 objects in the scene, culls them against the camera frustum in a compute pass, and draws only those that survive.

## The indirect buffers

Two buffers are needed: one with the draw arguments, and one with the number of draw calls to execute:

```cpp
auto indirectCommandsBuffer = m_device->factory().createBuffer("Indirect Commands", BufferType::Indirect, ResourceHeap::Resource,
    sizeof(IndirectIndexedBatch) * NUM_INSTANCES, 3, ResourceUsage::AllowWrite);
auto indirectCounterBuffer = m_device->factory().createBuffer("Indirect Counter", BufferType::Indirect, ResourceHeap::Resource,
    sizeof(UInt32), 4, ResourceUsage::Default | ResourceUsage::AllowWrite);
```

`IndirectIndexedBatch` is the engine's structure for the arguments of an indexed draw call: index count, instance count, the first index, the
vertex offset and the first instance. Both buffers exist once per frame in flight, since each frame culls on its own.

Indirect drawing is an optional device feature:

```cpp
m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, Size2d(width, height), 3, false,
    GraphicsDeviceFeatures { .DrawIndirect = true }));
```

## Culling on the GPU

The camera buffer carries the six planes of the view frustum, which the application computes each frame. The compute shader tests each object
against them and appends the surviving ones to the command buffer:

```hlsl
ConstantBuffer<Camera>                  camera       : register(b0, space0);
StructuredBuffer<Object>                objects      : register(t0, space1);
globallycoherent RWByteAddressBuffer    drawCounter  : register(u0, space2);
RWStructuredBuffer<IndirectDrawCommand> drawCommands : register(u1, space2);

[numthreads(128, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    Object object = objects[id.x];
    float3 center = object.Transform[3].xyz; // Get the object translation.

    [unroll(5)]
    for (uint i = 0; i < 6; ++i)
        culled = culled || dot(center, camera.Frustum[i].xyz) + radius < 0;

    if (!culled)
    {
        uint index;
        drawCounter.InterlockedAdd(0, 1, index);

        drawCommands[index].IndexCount = object.IndexCount;
        drawCommands[index].InstanceCount = 1;
        drawCommands[index].FirstIndex = object.FirstIndex;
        // ... vertex offset and first instance ...
    }
}
```

`InterlockedAdd` reserves a slot in the buffer for each visible object. It is what makes the append work even though thousands of threads run
at the same time, and `globallycoherent` makes the counter visible across all thread groups.

The pass is dispatched like any other compute workload, with one thread per object:

```cpp
cullCommands->dispatch({ NUM_INSTANCES / 128, 1, 1 });
```

## The indirect draw call

The draw call then refers to both buffers instead of taking its arguments directly:

```cpp
commandBuffer->drawIndexedIndirect(indirectCommandsBuffer, indirectCounterBuffer,
    backBuffer * indirectCommandsBuffer.alignedElementSize(), backBuffer * indirectCounterBuffer.alignedElementSize());
```

The offsets select the region of the current frame. The GPU reads the counter, then executes that many draw calls from the command buffer.
Since the CPU never learns how many objects were visible, it does not have to wait for the culling result, which is the main benefit of this
approach.

The counter has to be reset to zero before each culling pass, and barriers order the two passes: the culling must have finished writing
before the draw call reads.

!!! note "Differences between the backends"

    The structure of indirect commands differs between Vulkan and DirectX 12, and DirectX 12 needs a *command signature*. The engine hides
    both behind `IndirectIndexedBatch` and `drawIndexedIndirect`, using shader model 6.8 to resolve the remaining differences in the shader.
