#include "raytracing_common.hlsli"

[shader("closesthit")]
void main(inout HitInfo payload, in Attributes attributes)
{
    payload.Color = float3(1.0, 1.0, 0.0);
    payload.Distance = RayTCurrent();
}
