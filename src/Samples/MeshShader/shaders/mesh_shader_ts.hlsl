#pragma pack_matrix(row_major)

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