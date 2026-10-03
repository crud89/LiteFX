# Ray queries

!!! abstract "Sample 12"

    [`Samples/RayQueries`](https://github.com/crud89/LiteFX/tree/main/src/Samples/RayQueries) · builds on
    [ray tracing](ray-tracing.md).

A full ray tracing pipeline is a lot of machinery for effects that only need a few rays, such as shadows, reflections or ambient occlusion.
*Ray queries* (also called inline ray tracing) trace rays directly from an ordinary shader: no ray tracing pipeline, no shader binding table,
no separate shader stages. The sample renders the same scene as the ray tracing sample, but from a fragment shader.

## A pipeline without geometry

The rays are started per pixel, so the sample needs a fragment shader that covers the screen. It draws a single triangle large enough to fill
it, which needs neither vertex nor index buffers:

```cpp
SharedPtr<InputAssembler> pipelineInputAssembler = device->buildInputAssembler()
    .topology(PrimitiveTopology::TriangleStrip);

UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
    .inputAssembler(pipelineInputAssembler)
    .rasterizer(device->buildRasterizer()
        .polygonMode(PolygonMode::Solid)
        .cullMode(CullMode::Disabled))
    .layout(shaderProgram->reflectPipelineLayout())
    .shaderProgram(shaderProgram);
```

The vertex shader computes the three corners from the vertex index, so the draw call only states how many vertices to produce:

```hlsl
const static float4 Positions[] =
{
    float4(-1.0,  3.0, 0.0, 1.0),
    float4( 3.0, -1.0, 0.0, 1.0),
    float4(-1.0, -1.0, 0.0, 1.0)
};

float4 main(in uint vertexId : SV_VertexID) : SV_Position
{
    return Positions[vertexId];
}
```

Drawing the screen-filling triangle is then a single call: `commandBuffer->draw(3)`.

The acceleration structures are built exactly as in the [ray tracing](ray-tracing.md) sample. The difference is that the scene is bound as a
regular descriptor, next to the camera:

```hlsl
ConstantBuffer<CameraData> camera           : register(b0, space0);
RaytracingAccelerationStructure scene       : register(t1, space0);
```

## Tracing in the fragment shader

A ray query is an object in the shader. It is started with the scene, the ray flags and the ray itself:

```hlsl
RayQuery<RAY_FLAG_NONE> query;
query.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);
```

Unlike a pipeline, the traversal is driven by the shader: `Proceed` advances the query to the next candidate intersection, and the shader
decides what to do with it:

```hlsl
while (queryDepth++ < 15)
{
    // Proceed the query.
    query.Proceed();

    // If there was no hit, stop traversal.
    if (query.CommittedStatus() == COMMITTED_NOTHING)
        break;

    float depth = query.CommittedRayT();
    float3 normal = Normals[query.CommittedPrimitiveIndex()];
    normal = normalize(mul(normal, (float3x3)query.CommittedObjectToWorld4x3()));

    // ... shade the hit, then continue with a reflected ray ...
    ray.Origin = query.WorldRayOrigin() + query.WorldRayDirection() * query.CommittedRayT();
}
```

The query provides everything the hit shaders of a pipeline would get: the distance along the ray (`CommittedRayT`), which primitive and
geometry was hit, and the transform of the instance. Reflections are a loop in the shader, which traces a new ray from the hit point instead
of a recursive shader call.

## What this saves

Compared to the ray tracing sample, several pieces disappear entirely:

| Ray tracing pipeline | Ray queries |
|---|---|
| Ray generation, closest hit and miss shaders | One fragment shader |
| Shader record collection and shader binding table | — |
| `traceRays` and manual presentation | A regular render pass and `draw(3)` |
| Per-geometry data in shader records | Data from buffers, indexed by the query's results |

The trade-off is that the GPU cannot reorder and batch the work as it can for a ray tracing pipeline. For a few rays per pixel, ray queries
are usually the better choice; for path tracing with many bounces, the pipeline is.
