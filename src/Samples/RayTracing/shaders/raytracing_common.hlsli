struct HitInfo
{
    float3 Color;
    float Distance;
    uint Depth;
};

struct Attributes
{
    float2 HitPosition; // Barycentric coordinates.
};
