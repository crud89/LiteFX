float4x4 g_mWorldViewProjection;    // World * View * Projection 

struct VertexData {
    float4 Position : POSITION;
    float4 Color : COLOR;
}; 

VertexData main(float4 position : POSITION) 
{
    VertexData vertex;
    
    vertex.Position = mul(position, g_mWorldViewProjection);
    vertex.Color = float4(1.0, 1.0, 1.0, 1.0);
 
    return vertex;
}