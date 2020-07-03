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

//struct TransformData
//{
//    float4x4 WorldViewProjection;
//};

//ConstantBuffer<TransformData> transform : register(b0);

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    //vertex.Position = mul(float4(input.Position, 1.0), transform.WorldViewProjection);
    vertex.Position = float4(input.Position, 1.0);
    vertex.Color = input.Color;
 
    return vertex;
}