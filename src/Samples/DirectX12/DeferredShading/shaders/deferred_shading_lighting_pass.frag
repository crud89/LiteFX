#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float2 TextureCoordinate : TEXCOORD0;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET0;
};

Texture2D gDiffuse : register(t0, space0);
Texture2D gDepth : register(t1, space0);
SamplerState gBuffer : register(s0);

FragmentData main(VertexData input)
{
    FragmentData fragment;

    fragment.Color = gDiffuse.Sample(gBuffer, input.TextureCoordinate).rgba;
    
    return fragment;
}