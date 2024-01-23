#include "raytracing_common.hlsli"

RWTexture2D<float4> Output : register(u0, space0);
RaytracingAccelerationStructure SceneBVH : register(t1, space0);

[shader("raygeneration")]
void main()
{
    HitInfo payload;
    payload.Color = float3(0.0, 0.0, 0.0);
    payload.Distance = 1.0;
    
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dimensions = float2(DispatchRaysDimensions().xy);
    float2 origin = ((launchIndex.xy + 0.5) / dimensions.xy) * 2.0 - 1.0;
    
    RayDesc ray;
    ray.Origin = float3(origin.x, origin.y, 1.0);
    ray.Direction = float3(0.0, 0.0, -1.0);
    ray.TMin = 0.0;
    ray.TMax = 100000.0;
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    Output[launchIndex] = float4(payload.Color, 1.f);
}
