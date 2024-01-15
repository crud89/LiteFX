#pragma pack_matrix(row_major)

struct VertexData
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
};

Texture2D<float4> FrameBuffer : register(t0, space0);

float4 main(VertexData vertex) : SV_TARGET
{
    int width, height;
    FrameBuffer.GetDimensions(width, height);
    
    return FrameBuffer.Load(int3(vertex.TexCoord * float2(width, height), 0));
}