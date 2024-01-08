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

static float4 Positions[] =
{
    float4(-0.5, -0.5,  0.5, 1.0),
    float4( 0.5,  0.5,  0.5, 1.0),
    float4(-0.5,  0.5, -0.5, 1.0), 
    float4( 0.5, -0.5, -0.5, 1.0)
};

static float4 Colors[] =
{
    float4(1.0, 0.0, 0.0, 1.0),
    float4(0.0, 1.0, 0.0, 1.0),
    float4(0.0, 0.0, 1.0, 1.0),
    float4(1.0, 1.0, 1.0, 1.0)
};

static uint3 Indices[] =
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
    
    VertexData verts[4] = 
    {
        { mul(mul(Positions[0], transform.Model), camera.ViewProjection), Colors[0] },
        { mul(mul(Positions[1], transform.Model), camera.ViewProjection), Colors[1] },
        { mul(mul(Positions[2], transform.Model), camera.ViewProjection), Colors[2] },
        { mul(mul(Positions[3], transform.Model), camera.ViewProjection), Colors[3] }
    };
    
    vertices = verts;
    
    SetMeshOutputCounts(3, 1);
}