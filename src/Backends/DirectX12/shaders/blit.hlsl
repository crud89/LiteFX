#include "blit.hlsli"

ConstantBuffer<BlitParameters> input	: register(b0);
Texture2D<float4>		parent			: register(t0);
RWTexture2D<float4>		result			: register(u0);
SamplerState			linearFilter	: register(s0);

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	float2 texcoords = input.TexelSize * (threadId.xy + 0.5);
	float4 color = parent.SampleLevel(linearFilter, texcoords, 0);
	result[threadId.xy] = packColor(color);
}