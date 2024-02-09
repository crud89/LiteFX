#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float2 TextureCoordinate : TEXCOORD0;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET0;
    float Depth : SV_DEPTH;
};

#ifdef SPIRV
// NOTE: vk::input_attachment_index refers to the index of the render pass dependency in the order they have been added to the render pass whilst building or defining it.
[[vk::input_attachment_index(0)]] 
SubpassInput<float4> gDiffuse : register(t0, space0);
[[vk::input_attachment_index(1)]] 
SubpassInput<float>  gDepth   : register(t1, space0);
#elif DXIL
Texture2D gDiffuse   : register(t0, space0);
Texture2D gDepth     : register(t1, space0);
SamplerState gBuffer : register(s0, space1);
#endif

FragmentData main(VertexData input)
{
    FragmentData fragment;

    // NOTE: Unfortunately, specifying static samplers in the shader is currently not supported by dxc (see https://github.com/microsoft/DirectXShaderCompiler/issues/4137). This would
    //       allow us to use the same code path for both backends without where the input attachments are sampled. In this case the `RenderTargetFlags::Attachment` should not be provided
    //       and `SubpassLoad` should not be used for the Vulkan backend. As an alternative the static sampler can be provided from the render pipeline description instead of the root
    //       signature. However, in both cases whilst the code path would be the same, we would lose on the possible optimization from input attachment formats, which is only supported
    //       in Vulkan and demonstrated in this sample. Note that it is still possible to use input attachments with less optimal image layouts (i.e., without the `Attachment` flag).
    
#ifdef SPIRV  
    fragment.Color = gDiffuse.SubpassLoad();
    fragment.Depth = gDepth.SubpassLoad();
#elif DXIL
    fragment.Color = gDiffuse.Sample(gBuffer, input.TextureCoordinate).rgba;
    fragment.Depth = gDepth.Sample(gBuffer, input.TextureCoordinate).r;
#endif
    
    return fragment;
}