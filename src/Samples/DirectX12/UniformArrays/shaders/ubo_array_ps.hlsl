#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 PositionWS : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
}; 

struct FragmentData
{
    float4 Color : SV_TARGET;
    float Depth : SV_DEPTH;
};

struct CameraData
{
    float4x4 ViewProjection;
    float4 Position;
};

struct LightData
{
    float4 Position;
    float4 Color;
    float4 Properties;
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
ConstantBuffer<LightData>     lights[8] : register(b1, space0);

float3 BlinnPhong(float3 P, float3 N, float3 V)
{
    float3 result = float3(0.0, 0.0, 0.0);

    const float kD = 0.8;
    const float kS = 0.1;
    const float s = 32;

    for (int l = 0; l < 8; l++)
    {
        // Compute vector P -> Light Source.
        float3 L = lights[l].Position.xyz - P;

        //const float attenuation = saturate(1.0 - ((length(L) * length(L)) / (lights[l].Properties.x * lights[l].Properties.x)));
        const float attenuation = saturate(1.0 - ((length(L) * length(L)) / (lights[l].Properties.x * lights[l].Properties.x)));
        const float3 C = lights[l].Color.xyz * lights[l].Properties.y;

        L = normalize(L);
        V = normalize(V);
        N = normalize(N);

        float NdotL = saturate(dot(N, L));
        result += kD * (attenuation * NdotL * C);

        const float3 H = normalize(V + L);
        const float spec = pow(saturate(dot(N, H)), s);
        result += kS * (attenuation * spec * lights[l].Color.xyz);
    }

    return result;
}

FragmentData main(VertexData input)
{
    FragmentData fragment;
    const float kA = 1.0;
    
    float3 P = input.PositionWS.xyz;
    float3 V = camera.Position.xyz - P;
    float3 color = BlinnPhong(P, input.Normal, V);

    fragment.Depth = input.Position.z;
    fragment.Color = kA * input.Color * float4(color, 1.0);
    
    return fragment;
}