struct BlitParameters
{
	float2 TexelSize;	// Size of a single texel in UV coordinates ([0..1]).
	float isSRGB;
	float Padding;
};

ConstantBuffer<BlitParameters> input	: register(b0, space0);
Texture2D<float4>		parent			: register(t1, space0);
RWTexture2D<float4>		result			: register(u2, space0);
SamplerState			linearFilter	: register(s0, space1);

float3 applySRGB(float3 x)
{
	// See: https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/GenerateMipsCS.hlsli#L55
	//return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float4 packColor(float4 color)
{
	if (input.isSRGB > 0.0)
		return float4(applySRGB(color.rgb), color.a);
	else
		return color;
}

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	float2 texcoords = input.TexelSize * (threadId.xy + 0.5);
	float4 color = parent.SampleLevel(linearFilter, texcoords, 0);
	result[threadId.xy] = packColor(color);
}