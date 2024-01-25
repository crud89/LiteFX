#include "raytracing_common.hlsli"

Texture2D skybox           : register(t2, space0);
SamplerState skyboxSampler : register(s0, space4);

//static const float PI = 3.14159265;

float2 equirectangular(float3 direction)
{   
    static const float2 invAtan = float2(0.1591, 0.3183);
    return (float2(atan2(direction.y, direction.x), asin(-direction.z)) * invAtan) + 0.5;
}

[shader("miss")]
void main(inout HitInfo payload : SV_RayPayload)
{
    // Sample skybox and apply gamma correction.
    float4 color = skybox.SampleLevel(skyboxSampler, equirectangular(payload.Direction), 0.0);
    payload.Color = pow(color, 1.0 / 2.2).rgb;
    payload.Distance = -1.0;
}