const static float4 Positions[] =
{
    float4(-1.0,  3.0, 0.0, 1.0),
    float4( 3.0, -1.0, 0.0, 1.0),
    float4(-1.0, -1.0, 0.0, 1.0)
};

float4 main(in uint vertexId : SV_VertexID) : SV_Position
{
    return Positions[vertexId];
}