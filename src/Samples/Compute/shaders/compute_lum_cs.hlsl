#pragma pack_matrix(row_major)

RWTexture2D<float4> FrameBuffer : register(u0, space0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // Read the color at the current position and compute luminosity.
    float3 color = FrameBuffer.Load(id.xy).rgb;
    float Y = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    
    FrameBuffer[id.xy] = float4(Y, Y, Y, 1.0);
}