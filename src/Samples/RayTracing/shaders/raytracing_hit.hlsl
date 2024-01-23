#include "raytracing_common.hlsli"

struct GeometryData
{
    uint Index;
};

struct MaterialData
{
    float4 Color;
};

StructuredBuffer<MaterialData> Materials[] : register(t2, space1);

[[vk::shader_record_ext]]
ConstantBuffer<GeometryData> GeometryProperties : register(b0, space2);

[shader("closesthit")]
void main(inout HitInfo payload, in Attributes attributes)
{
    // NOTE: For DXR 1.1 enabled hardware no shader-local data is required for this. Instead the intrinsic `GeometryIndex()` can be called.
    uint geometryId = GeometryProperties.Index;
    float4 materialColor = Materials[geometryId].Load(0).Color;
    
    payload.Color = materialColor.rgb;
    payload.Distance = RayTCurrent();
}
