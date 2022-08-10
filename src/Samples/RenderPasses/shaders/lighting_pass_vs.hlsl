#pragma pack_matrix(row_major)

// For DXIL we need to define a root signature, in order for shader reflection to properly pick up the push constants.
#define ROOT_SIGNATURE \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "SRV(t0, space = 0, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE), " \
    "SRV(t1, space = 0, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE), " \
    "StaticSampler(s2, filter = FILTER_MIN_MAG_MIP_LINEAR)"

struct VertexData 
{
    float4 Position : SV_POSITION;
    float2 TextureCoordinate : TEXCOORD0;
}; 

struct VertexInput
{
    //[[vk::location(0)]] 
    float3 Position : POSITION;
    
    //[[vk::location(1)]]
    float4 Color : COLOR;
    
    //[[vk::location(2)]]
    float2 TextureCoordinate : TEXCOORD0;
};

[RootSignature(ROOT_SIGNATURE)]
VertexData main(in VertexInput input)
{
    VertexData vertex;
    
    vertex.Position = float4(input.Position.xy, 0.0, 1.0);
    vertex.TextureCoordinate = input.TextureCoordinate;
 
    return vertex;
}