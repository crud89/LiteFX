#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct TransformData
{
    float4x4 Model;
    float4 Color;
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
[[vk::push_constant]] ConstantBuffer<TransformData> transform;

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    float4 position = mul(float4(input.Position, 1.0), transform.Model);
    vertex.Position = mul(position, camera.ViewProjection);
    
    vertex.Color = transform.Color;
 
    return vertex;
}