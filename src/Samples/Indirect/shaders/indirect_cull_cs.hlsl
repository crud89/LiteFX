#pragma pack_matrix(row_major)

struct Object
{
    float4x4 Transform;
    float4 Color;
    float BoundingRadius;
    uint IndexCount;
    uint FirstIndex;
    int VertexOffset;
};

struct Camera
{
    float4x4 ViewProjection;
    float4x4 Projection;
    float4 Position;
    float4 Forward;
    float4 Up;
    float4 Right;
    float NearPlane;
    float FarPlane;
    float4 Frustum[6];
};

struct IndirectDrawCommand
{
    uint IndexCount;
    uint InstanceCount;
    uint FirstIndex;
    int VertexOffset;
    uint FirstInstance;
    uint3 Padding;
};

ConstantBuffer<Camera>                      camera       : register(b0, space0);
StructuredBuffer<Object>                    objects      : register(t0, space1);

// NOTE: Traditionally one would use an AppendStructuredBuffer here, however binding to the counter resource is somewhat convoluted and requires separate bindings
//       in Vulkan anyway. Using and RWByteAddressBuffer and explicitly calling `InterlockedAdd` is the more portable solution and should not result in performance
//       degradation on modern GPUs.
//       The `globallycoherent` storage class makes changes to the counter visible to all thread groups.
globallycoherent RWByteAddressBuffer        drawCounter  : register(u0, space2);
RWStructuredBuffer<IndirectDrawCommand>     drawCommands : register(u1, space2);

[numthreads(100, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint instanceId = NonUniformResourceIndex(id.x);
    Object object = objects.Load(instanceId);
    
    float3 center = transpose(object.Transform)[3].xyz; // Get the object translation.
    float radius = object.BoundingRadius; // Scale is ignored here at the moment...
    
    bool culled = false;
    
    // Cull against every frustum plane, except the far plane (which is the last one).
    [unroll(5)]
    for (int i = 0; i < 5; i++)
        culled = culled || dot(center, camera.Frustum[i].xyz) + radius < 0;
    
    if (!culled)
    {
        // Store the command.
        uint index;
        drawCounter.InterlockedAdd(0, 1, index);
        drawCommands[index].IndexCount = object.IndexCount;
        drawCommands[index].InstanceCount = 1;
        drawCommands[index].FirstIndex = object.FirstIndex;
        drawCommands[index].VertexOffset = object.VertexOffset;
        drawCommands[index].FirstInstance = instanceId;
    }
}