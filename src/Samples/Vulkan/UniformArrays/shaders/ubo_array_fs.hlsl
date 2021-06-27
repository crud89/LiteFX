#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET;
    float Depth : SV_DEPTH;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct LightData
{
    float4 Position;
    float4 Color;
    float4 Properties;
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
ConstantBuffer<LightData>     lights[8] : register(b1, space0);

FragmentData main(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = input.Color;

    return fragment;
}