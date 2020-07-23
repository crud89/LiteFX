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
    float Depth : SV_DEPTH;
};

Texture2D diffuseMap : register(t0, space2);
SamplerState diffuse : register(s1, space2);

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = diffuseMap.Sample(diffuse, input.TextureCoordinate) * input.Color;

    return fragment;
}