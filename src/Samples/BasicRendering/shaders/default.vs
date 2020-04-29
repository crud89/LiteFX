struct VertexData {
    float4 Position : POSITION;
    float4 Color : COLOR;
}; 

float4 main(VertexData input) : SV_TARGET
{
    return input.Color;
}