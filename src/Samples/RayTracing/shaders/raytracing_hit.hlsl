#include "raytracing_common.hlsli"

struct GeometryData
{
    uint Index;
    //uint3 Padding;
};

struct MaterialData
{
    float4 Color;
};

StructuredBuffer<MaterialData> materials[]      : register(t0, space2);

[[vk::shader_record_ext]]
ConstantBuffer<GeometryData> geometryProperties : register(b0, space3);

[shader("closesthit")]
void main(inout HitInfo payload, in Attributes attributes)
{
    // NOTE: For DXR 1.1 enabled hardware no shader-local data is required for this. Instead the intrinsic `GeometryIndex()` can be called.
    uint geometryId = NonUniformResourceIndex(geometryProperties.Index);
    MaterialData materialData = materials[geometryId].Load(0);
    float4 materialColor = materialData.Color;
    
    payload.Color = materialColor.rgb;
    payload.Distance = RayTCurrent();
}
