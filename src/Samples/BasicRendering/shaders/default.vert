float4x4 WorldViewProjection;

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

VertexData main(float3 position : POSITION0) 
{
    VertexData vertex;
    
    vertex.Position = mul(float4(position, 1.0), WorldViewProjection);
    //vertex.Position = float4(position, 1.0);
    vertex.Color = float4(1.0, 1.0, 1.0, 1.0);
 
    return vertex;
}