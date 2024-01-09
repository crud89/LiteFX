#pragma pack_matrix(row_major)

struct VertexData 
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

struct MeshData
{
    uint FaceIndex;
};

struct CameraData
{
    float4x4 ViewProjection;
};

struct TransformData
{
    float4x4 Model;
};

ConstantBuffer<CameraData> camera       : register(b0, space0);
ConstantBuffer<TransformData> transform : register(b0, space1);

static const float4 Positions[] =
{
    float4(-0.5, -0.5,  0.5, 1.0),
    float4( 0.5,  0.5,  0.5, 1.0),
    float4(-0.5,  0.5, -0.5, 1.0), 
    float4( 0.5, -0.5, -0.5, 1.0)
};

static const float4 Colors[] =
{
    float4(1.0, 0.0, 0.0, 1.0),
    float4(0.0, 1.0, 0.0, 1.0),
    float4(0.0, 0.0, 1.0, 1.0),
    float4(1.0, 1.0, 1.0, 1.0)
};

static const uint3 Indices[] =
{
    uint3(0, 2, 1),
    uint3(0, 1, 3),
    uint3(0, 3, 2),
    uint3(1, 2, 3)
};

[numthreads(8, 1, 1)]
[outputtopology("triangle")]
void main(in payload MeshData meshData, out indices uint3 triangles[1], out vertices VertexData vertices[4])
{
    triangles[0] = Indices[meshData.FaceIndex];
    
    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        float4 position = mul(mul(Positions[i], transform.Model), camera.ViewProjection);
        
#ifdef SPIRV
        // NOTE: DXC does not (yet) support y-inversion for mesh shaders, so we have to do this manually (see: https://github.com/microsoft/DirectXShaderCompiler/issues/3154).
        position.y = -position.y;
#endif
      
        // NOTE: The DXC SPIR-V codegen for mesh and task shaders is a total mess. Expect bugs when attempting to read from `out` variables.
        vertices[i].Position = position;
        vertices[i].Color = Colors[i];
    }
    
    SetMeshOutputCounts(4, 1);
}