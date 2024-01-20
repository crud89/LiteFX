#include "raytracing_common.hlsli"

[shader("miss")]
void main(inout HitInfo payload : SV_RayPayload)
{
    payload.Color = float3(0.2, 0.2, 0.8);
    payload.Distance = -1.0;
}