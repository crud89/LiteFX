#include "raytracing_common.hlsli"

#define MAX_RAY_DEPTH 16

struct GeometryData
{
    uint Index;
    uint Reflective;
    //uint2 Padding;
};

struct MaterialData
{
    float4 Color;
};

RaytracingAccelerationStructure scene           : register(t1, space0);
StructuredBuffer<MaterialData> materials[]      : register(t0, space2);

[[vk::shader_record_ext]]
ConstantBuffer<GeometryData> geometryProperties : register(b0, space3);

// NOTE: Since our geometry is always the same we can use pre-computed normals here. In a real-world scenario, you might want to pass the normals in a 
//       buffer instead.
static float3 Normals[] =
{
    float3(0.0,  1.0, 0.0), float3(0.0,  1.0, 0.0), // Front
    float3(0.0, -1.0, 0.0), float3(0.0, -1.0, 0.0), // Back
    float3( 1.0, 0.0, 0.0), float3( 1.0, 0.0, 0.0), // Right
    float3(-1.0, 0.0, 0.0), float3(-1.0, 0.0, 0.0), // Left
    float3(0.0, 0.0, -1.0), float3(0.0, 0.0, -1.0), // Bottom
    float3(0.0, 0.0,  1.0), float3(0.0, 0.0,  1.0)  // Top
};

[shader("closesthit")]
void main(inout HitInfo payload, in Attributes attributes)
{
    // NOTE: For DXR 1.1 enabled hardware no shader-local data is required for this. Instead the intrinsic `GeometryIndex()` can be called.
    uint instanceId = InstanceID();
    MaterialData materialData = materials[instanceId].Load(0);
    float4 materialColor = materialData.Color;
    
    // If we're at the maximum depth (currently constant of 16) return the current color. Otherwise reflect and continue.
    if (payload.Depth >= MAX_RAY_DEPTH || geometryProperties.Reflective == 0)
    {   
        payload.Color = materialColor.rgb;
        payload.Distance = RayTCurrent();
    }
    else
    {
        // Compute the surface normal.
        float3 normal = Normals[PrimitiveIndex()];
        normal = normalize(mul(normal, (float3x3)ObjectToWorld4x3()));
        
        // Create a new ray.
        RayDesc ray;
        ray.Origin    = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        ray.Direction = reflect(normalize(WorldRayDirection()), normal);
        ray.TMin      = 0.1;
        ray.TMax      = 1000.0;
        
        // Update the payload.
        payload.Depth += 1;
        payload.Color += materialColor.rgb * (1.0 / float(MAX_RAY_DEPTH));
     
        // Trace new ray.
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
    }
}