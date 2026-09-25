#pragma pack_matrix(row_major)

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

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

struct TransformData
{
    float4x4 Model;
};

ConstantBuffer<CameraData> camera       : register(b0, space0);
ConstantBuffer<TransformData> transform : register(b0, space1);

VertexData VSMain(in VertexInput input)
{
    VertexData vertex;
    
    float4 position = mul(float4(input.Position, 1.0), transform.Model);
    vertex.Position = mul(position, camera.ViewProjection);
    
    vertex.Color = input.Color;
 
    return vertex;
}

FragmentData PSMain(VertexData input)
{
    FragmentData fragment;
    
    fragment.Depth = input.Position.z;
    fragment.Color = input.Color;

    return fragment;
}