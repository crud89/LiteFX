#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

float4 main(VertexData input) : SV_TARGET0
{
    return input.Color;
}