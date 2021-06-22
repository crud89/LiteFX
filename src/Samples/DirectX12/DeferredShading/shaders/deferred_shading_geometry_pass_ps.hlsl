#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET;
    float Depth : SV_DEPTH;
};

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = input.Color;

    return fragment;
}