struct BlitParameters
{
	float2 TexelSize;	// Size of a single texel in UV coordinates ([0..1]).
	float  IsSRGB;		// True assumed for > 0.0.
};

float3 applySRGB(float3 x)
{
	// See: https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/GenerateMipsCS.hlsli#L55
	//return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float4 packColor(float4 color)
{
#ifdef CONVERT_TO_SRGB
	return float4(applySRGB(color.rgb), color.a);
#else
	return color;
#endif
}