#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct VertexInput
{
    float3 Position : POSITION;
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

struct Object
{
    float4x4 Transform;
    float4 Color;
    float BoundingRadius;
    uint IndexCount;
    uint FirstIndex;
    int VertexOffset;
};

ConstantBuffer<Camera>   camera  : register(b0, space0);
StructuredBuffer<Object> objects : register(t0, space1);

VertexData main(in VertexInput input, in uint modelId : SV_InstanceID)
{
    VertexData vertex;
    Object object = objects.Load(modelId);
    
    float4 position = mul(float4(input.Position, 1.0), object.Transform);
    
    vertex.Position = mul(position, camera.ViewProjection);
    vertex.Color = object.Color;
 
    return vertex;
}