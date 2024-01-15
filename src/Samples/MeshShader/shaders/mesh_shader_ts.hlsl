#pragma pack_matrix(row_major)

// NOTE: Due to a bug, you can currently only put one variable into the payload when compiling with DXC (see: https://github.com/microsoft/DirectXShaderCompiler/issues/5981).
struct MeshData
{
    uint FaceIndex;
};

groupshared MeshData Data;

[numthreads(1, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID)
{
    Data.FaceIndex = threadId.x;
    DispatchMesh(1, 1, 1, Data);
}