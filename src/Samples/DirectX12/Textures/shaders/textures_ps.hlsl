#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinate : TEXCOORD0;
};

struct FragmentData
{
    float4 Color : SV_TARGET;
};

Texture2D diffuseMap : register(t1, space0);
SamplerState diffuse : register(s0, space1);

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Color = diffuseMap.Sample(diffuse, input.TextureCoordinate) * input.Color;

    return fragment;
}