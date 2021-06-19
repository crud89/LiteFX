#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float2 TextureCoordinate : TEXCOORD0;
}; 

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinate : TEXCOORD0;
};

VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    vertex.Position = float4(input.Position.xy, 0.0, 1.0);
    vertex.TextureCoordinate = input.TextureCoordinate;
 
    return vertex;
}