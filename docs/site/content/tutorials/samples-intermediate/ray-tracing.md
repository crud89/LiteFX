# Ray tracing

!!! abstract "Sample 11"

    [`Samples/RayTracing`](https://github.com/crud89/LiteFX/tree/main/src/Samples/RayTracing) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md) and [textures](../samples-beginner/textures.md).

Rasterization asks, for each triangle, which pixels it covers. Ray tracing asks the opposite: for each pixel, which geometry a ray hits.
This sample builds a scene of nine objects, traces rays into it, reflects them off one object and samples a skybox where a ray hits nothing.

## Acceleration structures

To find intersections quickly, the GPU needs the scene in a special structure. It has two levels:

- **Bottom level (BLAS):** the geometry itself, i.e. the triangles of an object.
- **Top level (TLAS):** instances of those objects, each with a transform.

Buffers used to build them have to be marked for it:

```cpp
auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource,
    static_cast<UInt32>(vertices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
```

The sample creates two bottom level structures, one opaque and one reflective, and puts the geometry into them:

```cpp
auto opaque = asShared(m_device->factory().createBottomLevelAccelerationStructure(AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
opaque->withTriangleMesh({ vertexBuffer, indexBuffer });
```

The top level structure then places instances of them in the scene. Each instance carries a transform and an index that selects its hit
group:

```cpp
auto tlas = m_device->factory().createTopLevelAccelerationStructure("TLAS", AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory);
tlas->withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, -3.0f, 0.0f))), 0)
    .withInstance(opaque, /* ... */, 1)
    // ... seven more instances ...
    .withInstance(reflective, /* ... */, 8);
```

Structures need memory of their own, which the sample computes and allocates before building them:

```cpp
m_device->computeAccelerationStructureSizes(*opaque, opaqueSize, opaqueScratchSize);
auto blasBuffer = m_device->factory().createBuffer("BLAS", BufferType::AccelerationStructure, ResourceHeap::Resource, size, 1u, ResourceUsage::AllowWrite);
```

Between uploading the geometry and building the structures, a barrier makes the buffers readable for the build. The build reads them like a
shader does, so the barrier waits for the transfer writes and makes the buffers available for shader reads:

```cpp
auto barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::AccelerationStructureBuild);
barrier->transition(*vertexBuffer, ResourceAccess::TransferWrite, ResourceAccess::ShaderRead);
barrier->transition(*indexBuffer, ResourceAccess::TransferWrite, ResourceAccess::ShaderRead);
commandBuffer->barrier(*barrier);
```

## The ray tracing pipeline

A ray tracing program consists of several shader types:

- **Ray generation:** runs once per pixel and starts the rays.
- **Closest hit:** runs where a ray hits geometry.
- **Miss:** runs where a ray hits nothing.

```cpp
SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
    .withRayGenerationShaderModule("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
    .withClosestHitShaderModule("shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER,
        DescriptorBindingPoint { .Register = 0, .Space = std::to_underlying(DescriptorSets::GeometryData) })
    .withMissShaderModule("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER);
```

Which shader runs for which geometry is decided by the *shader record collection*. Each record can carry its own data, which is how the two
instances get different properties without a buffer:

```cpp
UniquePtr<RayTracingPipeline> rayTracingPipeline = device->buildRayTracingPipeline("RT Geometry",
    shaderProgram->buildShaderRecordCollection()
        .withShaderRecord("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
        .withShaderRecord("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER)
        .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, GeometryData { .Index = 0, .Reflective = 0 })
        .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, GeometryData { .Index = 1, .Reflective = 1 }))
    .maxBounces(16)                       // Important: If changed, the closest hit shader also needs to be updated!
    .maxPayloadSize(sizeof(Float) * 5)    // See HitInfo in raytracing_common.hlsli
    .maxAttributeSize(sizeof(Float) * 2); // See Attributes in raytracing_common.hlsli
```

The limits matter: the *payload* is the data a ray carries between shaders, the *attributes* are what the intersection provides, and
`maxBounces` limits how often a ray may be traced recursively.

The records are collected in a *shader binding table*, a buffer the GPU uses to find the right shader for each hit:

```cpp
auto stagingSBT = geometryPipeline.allocateShaderBindingTable(m_offsets);
auto shaderBindingTable = m_device->factory().createBuffer("Shader Binding Table", BufferType::ShaderBindingTable, ResourceHeap::Resource,
    stagingSBT->elementSize(), stagingSBT->elements(), ResourceUsage::TransferDestination);
commandBuffer->transfer(std::move(stagingSBT), *shaderBindingTable, 0, 0, shaderBindingTable->elements());
```

## Rendering without a render pass

Ray tracing writes into an image rather than drawing into render targets, so there is no render pass. The sample binds its descriptor sets,
traces the rays, and copies the result into the swap chain image itself:

```cpp
commandBuffer->use(geometryPipeline);
commandBuffer->bind({ &staticDataBindings, &outputBindings, &materialBindings, &samplerBindings });
commandBuffer->traceRays(static_cast<UInt32>(m_device->swapChain().renderArea().width()),
    static_cast<UInt32>(m_device->swapChain().renderArea().height()), 1, m_offsets, shaderBindingTable, &shaderBindingTable, &shaderBindingTable);
```

`traceRays` starts one ray generation thread per pixel, much like a compute dispatch. As in the [compute](compute.md) sample, barriers and a
transfer bring the result into the swap chain image, and the frame is presented explicitly.

!!! tip "Without a pipeline"

    Rays can also be traced from a regular fragment or compute shader, without a ray tracing pipeline and shader binding table. The
    [ray queries](ray-queries.md) tutorial shows that variant.
