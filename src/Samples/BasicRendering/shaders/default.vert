struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct VertexInput
{
    //[[vk::location(0)]] 
    float3 Position : POSITION;
    
    //[[vk::location(1)]]
    float4 Color : COLOR;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct TransformData
{
    float4x4 Model;
};

ConstantBuffer<CameraData> camera       : register(b0);
ConstantBuffer<TransformData> transform : register(b1);

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    float4 position = mul(float4(input.Position, 1.0), transform.Model);
    vertex.Position = mul(position, camera.ViewProjection);
    
    vertex.Color = input.Color;
 
    return vertex;
}