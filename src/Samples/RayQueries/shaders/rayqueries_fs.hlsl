struct FragmentData
{
    float4 Color : SV_TARGET;
    float Depth  : SV_DEPTH;
};

struct CameraData
{
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 InverseProjection;
    float2 ViewportSize;
};

struct MaterialData
{
    float4 Color;
};

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

ConstantBuffer<CameraData> camera          : register(b0, space0);
RaytracingAccelerationStructure scene      : register(t1, space0);
Texture2D skybox                           : register(t2, space0);
StructuredBuffer<MaterialData> materials[] : register(t0, space1);
SamplerState skyboxSampler                 : register(s0, space2);

float2 equirectangular(float3 direction)
{
    static const float2 invAtan = float2(0.1591, 0.3183);
    return (float2(atan2(direction.y, -direction.x), asin(-direction.z)) * invAtan) + float2(0.5, 0.5);
}

FragmentData main(in float4 screenPos : SV_Position)
{
    float2 origin = (screenPos.xy / camera.ViewportSize) * 2.0 - 1.0;
    float4 target = mul(camera.InverseProjection, float4(origin.x, -origin.y, 1.0, 1.0));
    
    // Build up a ray.
    RayDesc ray;
    ray.Origin    = mul(camera.InverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = mul(camera.InverseView, float4(target.xyz, 0.0)).xyz;
    ray.TMin      = 0.1;
    ray.TMax      = 1000.0;
    
    // Create a ray query and perform inline ray-tracing.
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);
    
    // Proceed the ray query as long as there's a hit.
    uint queryDepth = 0;
    float depth = 1.0;
    
    while (queryDepth++ < 15)
    {
        // Proceed the query.
        query.Proceed();
        
        // If there was no hit, stop traversal.
        if (query.CommittedStatus() == COMMITTED_NOTHING)
            break;
        
        // If there is a hit, handle it. Start by storing the depth if we're on the first iteration.
        if (queryDepth == 1)
            depth = query.CommittedRayT();
        
        // Check if the geometry is reflective and if not, stop traversal.
        if (query.CommittedGeometryIndex() == 0)
            break;
        
        // Compute the surface normal.
        float3 normal = Normals[query.CommittedPrimitiveIndex()];
        normal = normalize(mul(normal, (float3x3)query.CommittedObjectToWorld4x3()));
        
        // Create a new ray.
        RayDesc ray;
        ray.Origin = query.WorldRayOrigin() + query.WorldRayDirection() * query.CommittedRayT();
        ray.Direction = reflect(normalize(query.WorldRayDirection()), normal);
        ray.TMin = 0.1;
        ray.TMax = 1000.0;
        
        // Reset the query and continue traversal.
        query.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);
    }
    
    float4 color = float4(1.0, 1.0, 1.0, 1.0);
    
    switch (query.CommittedStatus())
    {
        case COMMITTED_NOTHING:
        {
            // If we're on a miss trajectory, sample the environment.
            color = skybox.SampleLevel(skyboxSampler, equirectangular(normalize(query.WorldRayDirection())), 0.0);
            color = float4(pow(color.rgb, 1.0 / 2.2), 1.0);
            break;
        }
        case COMMITTED_TRIANGLE_HIT:
        {
            // For a triangle hit, return the material color.
            uint instanceId = query.CommittedInstanceID();
            MaterialData materialData = materials[instanceId].Load(0);
            color = materialData.Color;
            break;
        }
        default:
        {
            // NOTE: no procedural geometry is contained by the TLAS yet, so this will never happen.
            break;
        }
    }
    
    // Setup the fragment data and return.
    FragmentData fragment;
    
    fragment.Depth = depth;
    fragment.Color = color;

    return fragment;
}