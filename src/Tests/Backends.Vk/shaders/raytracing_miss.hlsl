#include "raytracing_common.hlsli"

Texture2D skybox           : register(t2, space0);
SamplerState skyboxSampler : register(s0, space4);

float2 equirectangular(float3 direction)
{   
    static const float2 invAtan = float2(0.1591, 0.3183);
    return (float2(atan2(direction.y, -direction.x), asin(-direction.z)) * invAtan) + float2(0.5, 0.5);
}

[shader("miss")]
void main(inout HitInfo payload : SV_RayPayload)
{
    // Sample skybox and apply gamma correction.
    float4 color = skybox.SampleLevel(skyboxSampler, equirectangular(normalize(WorldRayDirection())), 0.0);
    
    // Alternative: Compute the sky color and apply gamma correction.
    //float3 colorTop = float3(0.4000, 0.7686, 1.0);
    //float3 colorBot = float3(0.8196, 0.9294, 1.0);
    //float3 color = lerp(colorTop, colorBot, (-WorldRayDirection().z + 1.0) * 0.5);
    
    payload.Color = pow(color, 1.0 / 2.2).rgb;
    payload.Distance = -1.0;
}