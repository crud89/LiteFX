#pragma pack_matrix(row_major)

struct VertexData
{
	float4 Position : SV_POSITION;
	float4 Color    : COLOR;
	float2 TexCoord : TEXCOORD0;
};

static float4 Corners[] =
{
	float4(-1.0f, -1.0f, 0.0f, 1.0f),
	float4( 1.0f, -1.0f, 0.0f, 1.0f),
	float4(-1.0f,  1.0f, 0.0f, 1.0f),
	float4( 1.0f,  1.0f, 0.0f, 1.0f)
};

static float2 TexCoords[] =
{
	float2(0.0, 0.0),
	float2(1.0, 0.0),
	float2(0.0, 1.0),
	float2(1.0, 1.0)
};

VertexData main(uint id : SV_InstanceID)
{
	// This shader is intended to be called without any inputs, just 4 instances for which the corners of the screen quad are created.
	VertexData vertex;
	
	vertex.Position = Corners[id];
	vertex.TexCoord = TexCoords[id];
	vertex.Color = float4(0.0, 0.0, 0.0, 1.0);
	
	return vertex;
}