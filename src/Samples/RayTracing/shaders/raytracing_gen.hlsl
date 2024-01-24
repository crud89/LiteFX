#include "raytracing_common.hlsli"

struct CameraData
{
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 InverseProjection;
};

ConstantBuffer<CameraData> camera        : register(b0, space0);
RaytracingAccelerationStructure scene    : register(t1, space0);
RWTexture2D<float4> output               : register(u1, space1);

[shader("raygeneration")]
void main()
{
    HitInfo payload;
    payload.Color = float3(0.0, 0.0, 0.0);
    payload.Distance = 1.0;
    
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dimensions = float2(DispatchRaysDimensions().xy);
    float2 origin = ((launchIndex.xy + 0.5) / dimensions.xy) * 2.0 - 1.0;
    float4 target = mul(camera.InverseProjection, float4(origin.x, -origin.y, 1.0, 1.0));
    
    RayDesc ray;
    ray.Origin    = mul(camera.InverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = mul(camera.InverseView, float4(target.xyz, 0.0)).xyz;
    ray.TMin      = 0.0;
    ray.TMax      = 100000.0;
    
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    output[launchIndex] = float4(payload.Color, 1.f);
}
