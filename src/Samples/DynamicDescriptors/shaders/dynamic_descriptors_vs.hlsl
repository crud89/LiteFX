#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct VertexInput
{
    //[[vk::location(0)]] 
    float3 Position : POSITION;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct DrawData
{
    float Time;     // In seconds.
    float Speed;    // Radians per second.
};

struct InstanceData
{
    float4x4 Transform;
    float4 Color;    // Random color.
    float4 Axis;     // Random axis to rotate around.
};

ConstantBuffer<CameraData> camera  : register(b0, space1);
ConstantBuffer<DrawData> drawData  : register(b0, space2);

float4x4 angleAxis(float angle, float4 axis)
{
    float c, s, i;
    sincos(angle, s, c);
    i = 1 - c;

    return float4x4(
        i * axis.x * axis.x + c, i * axis.x * axis.y - s * axis.z, i * axis.x * axis.z + s * axis.y, 0,
        i * axis.x * axis.y + s * axis.z, i * axis.y * axis.y + c, i * axis.y * axis.z - s * axis.x, 0,
        i * axis.x * axis.z - s * axis.y, i * axis.y * axis.z + s * axis.x, i * axis.z * axis.z + c, 0,
        0, 0, 0, 1
    );
}

VertexData main(in VertexInput input, uint id : SV_InstanceID, uint baseId : SV_StartInstanceLocation)
{
    VertexData vertex;
    
    StructuredBuffer<InstanceData> instanceBuffer = ResourceDescriptorHeap[NonUniformResourceIndex(id + baseId)];
    InstanceData instance = instanceBuffer.Load(0);
    float4x4 rotation = angleAxis(drawData.Time * drawData.Speed, instance.Axis);
    
    // Double-rotation: first around object center, then around a common center.
    float4x4 transform = mul(rotation, mul(instance.Transform, rotation));
    float4 position = mul(float4(input.Position, 1.0), transform);
    
    vertex.Position = mul(position, camera.ViewProjection);
    vertex.Color = instance.Color;
 
    return vertex;
}