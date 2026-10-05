# Mesh shaders

!!! abstract "Sample 10"

    [`Samples/MeshShader`](https://github.com/crud89/LiteFX/tree/main/src/Samples/MeshShader) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md).

The traditional pipeline reads vertices and indices from buffers, which the input assembler feeds into the vertex shader. *Mesh shaders*
replace that fixed part with two programmable stages: a *task* (or amplification) shader decides how much geometry to generate, and a *mesh*
shader produces the vertices and primitives. The sample builds its geometry entirely in shaders, without vertex or index buffers.

## Enabling the feature

Mesh shaders are an optional device feature, so the device is created with it:

```cpp
m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM,
    m_viewport->getRectangle().extent(), 3, false, GraphicsDeviceFeatures { .MeshShaders = true }));
```

## The shader program

Instead of a vertex shader, the program has a task and a mesh shader:

```cpp
SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
    .withTaskShaderModule("shaders/mesh_shader_ts." + FileExtensions<TRenderBackend>::SHADER)
    .withMeshShaderModule("shaders/mesh_shader_ms." + FileExtensions<TRenderBackend>::SHADER)
    .withFragmentShaderModule("shaders/mesh_shader_fs." + FileExtensions<TRenderBackend>::SHADER);
```

The input assembler only keeps the topology, as there are no vertex buffer layouts and no index type anymore:

```cpp
SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
    .topology(PrimitiveTopology::TriangleList);
```

## Task shader

The task shader runs once per dispatched group and decides how many mesh shader groups to start. It passes a *payload* to them, here the
index of the face to build:

```hlsl
struct MeshData
{
    uint FaceIndex;
};

groupshared MeshData Data;

[numthreads(1, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID)
{
    Data.FaceIndex = threadId.x;
    DispatchMesh(1, 1, 1, Data);
}
```

## Mesh shader

The mesh shader declares how many vertices and primitives it outputs at most, and writes them into two output arrays:

```hlsl
[numthreads(8, 1, 1)]
[outputtopology("triangle")]
void main(in payload MeshData meshData, out indices uint3 triangles[1], out vertices VertexData vertices[4])
{
    SetMeshOutputCounts(4, 1);
    // ... compute the vertices of the face and write triangles[0] ...
}
```

`SetMeshOutputCounts` must be called before the arrays are written, as it tells the GPU how much of them is used.

## Drawing

Without vertex and index buffers, the draw call becomes a dispatch. Its arguments are the number of task shader groups:

```cpp
commandBuffer->dispatchMesh(4, 1, 1); // This will create 4 faces.
```

Each group builds one face of the tetrahedron, and the mesh shader turns it into vertices and a triangle.

!!! note "Where this pays off"

    Generating a fixed shape is a small example. In practice, mesh shaders are used to split large meshes into *meshlets* and to cull them
    on the GPU before any vertex is transformed, which the task shader is made for.
