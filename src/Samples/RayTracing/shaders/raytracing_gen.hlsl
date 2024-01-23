#include "raytracing_common.hlsli"

RWTexture2D<float4> Output : register(u0, space0);
RaytracingAccelerationStructure SceneBVH : register(t1, space0);

[shader("raygeneration")]
void main()
{
    HitInfo payload;
    payload.Color = float3(0.0, 0.0, 0.0);
    payload.Distance = 1.0;
    
    uint3 rayIndex = DispatchRaysIndex();

    Output[rayIndex.xy] = float4(payload.Color, 1.f);
}
