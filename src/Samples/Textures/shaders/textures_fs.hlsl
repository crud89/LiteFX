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

Texture2D diffuseMap : register(t1, space0);
SamplerState diffuse : register(s2, space0);

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = diffuseMap.Sample(diffuse, input.TextureCoordinate) * input.Color;

    // Apply gamma correction.
    fragment.Color = pow(fragment.Color, 1.0 / 2.2);

    return fragment;
}