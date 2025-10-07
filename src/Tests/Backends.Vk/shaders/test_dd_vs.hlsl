#pragma pack_matrix(row_major)

struct VertexData
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

struct VertexInput
{
    //[[vk::location(0)]] 
	float3 Position : POSITION;
};

struct CameraData
{
	float4x4 ViewProjection;
};

struct InstanceData
{
	float4x4 Transform;
};

ConstantBuffer<CameraData> camera : register(b0, space1);
Texture2D texture : register(t0, space2);

VertexData main(in VertexInput input, uint id : SV_InstanceID)
{
	VertexData vertex;
    
	StructuredBuffer<InstanceData> instanceBuffer = ResourceDescriptorHeap[NonUniformResourceIndex(id)];
	InstanceData instance = instanceBuffer.Load(0);
	float4 position = mul(float4(input.Position, 1.0), instance.Transform);    
	vertex.Position = mul(position, camera.ViewProjection);
	
	SamplerState samplerState = SamplerDescriptorHeap[NonUniformResourceIndex(id)];
	vertex.Color = texture.SampleLevel(samplerState, 0.5.xx, 0.0);
 
	return vertex;
}