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

#ifdef SPIRV
// NOTE: Counter variable needs to be bound to a descriptor with index smaller than the actual buffer, since unbounded arrays must be the last descriptor int a set for Vulkan.
[[ vk::counter_binding(0) ]]
#endif
AppendStructuredBuffer<IndirectDrawCommand> drawCommands : register(u1, space2);

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
        IndirectDrawCommand command;
        command.IndexCount = object.IndexCount;
        command.InstanceCount = 1;
        command.FirstIndex = object.FirstIndex;
        command.VertexOffset = object.VertexOffset;
        command.FirstInstance = instanceId;
        command.Padding.xyz = uint3(0, 0, 0);
        drawCommands.Append(command);
    }
}