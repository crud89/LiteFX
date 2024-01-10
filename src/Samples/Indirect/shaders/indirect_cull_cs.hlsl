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
    float Frustum[4];
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
    
    // Implementation following niagara approach, but without occlusion culling and level of detail: https://github.com/zeux/niagara/blob/master/src/shaders/math.h.
    float4 center = mul(float4(0.0, 0.0, 0.0, 1.0), object.Transform);
    float radius = object.BoundingRadius; // Scale is ignored here at the moment...

    bool visible = true;
    visible = visible && center.z * camera.Frustum[1] - abs(center.x) * camera.Frustum[0] > -radius;
    visible = visible && center.z * camera.Frustum[3] - abs(center.y) * camera.Frustum[2] > -radius;
    visible = visible && center.z + radius > camera.NearPlane && center.z - radius < camera.FarPlane;
    
    if (visible)
    {
        IndirectDrawCommand command;
        command.IndexCount = object.IndexCount;
        command.InstanceCount = 1;
        command.FirstIndex = object.FirstIndex;
        command.VertexOffset = object.VertexOffset;
        command.FirstInstance = instanceId;
        //command.Padding.x = 0;
        drawCommands.Append(command);
    }
}