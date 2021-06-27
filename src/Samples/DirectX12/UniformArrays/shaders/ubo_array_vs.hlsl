#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 PositionWS : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
}; 

struct VertexInput
{
    //[[vk::location(0)]] 
    float3 Position : POSITION;
    
    //[[vk::location(1)]]
    float4 Color : COLOR;
    
    //[[vk::location(2)]]
    float3 Normal : NORMAL;
};

struct CameraData
{
    float4x4 ViewProjection;
    float4 Position;
};

struct TransformData
{
    float4x4 Model;
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
ConstantBuffer<TransformData> transform : register(b0, space1);

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    vertex.PositionWS = mul(float4(input.Position, 1.0), transform.Model);
    vertex.Position = mul(vertex.PositionWS, camera.ViewProjection);
    vertex.Normal = mul(input.Normal, (float3x3)transform.Model);
    vertex.Color = input.Color;
 
    return vertex;
}