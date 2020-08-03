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

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    // TODO: Sample from input attachment.
    fragment.Color = float4(1.0, 1.0, 1.0, 1.0);

    return fragment;
}