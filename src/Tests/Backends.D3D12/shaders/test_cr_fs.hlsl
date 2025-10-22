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

FragmentData main(VertexData input, uint coverage : SV_InnerCoverage)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    
    // Apply input color only to fully inner pixels and color the remaining pixels gray-ish.
    // See: https://learn.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization#innercoverage-interaction
    if (coverage > 0)
        fragment.Color = input.Color;
    else // if (coverage == 0)
        fragment.Color = float4(0.6.xxx, 1.0f);

    return fragment;
}