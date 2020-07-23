#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinate : TEXCOORD0;
};

Texture2D diffuseMap : register(t0, space2);
SamplerState diffuse : register(s1, space2);

float4 main(VertexData input) : SV_TARGET0
{
    return diffuseMap.Sample(diffuse, input.TextureCoordinate) * input.Color;
}