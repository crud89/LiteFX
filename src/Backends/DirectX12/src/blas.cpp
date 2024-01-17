#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;
using TriangleMesh  = IBottomLevelAccelerationStructure::TriangleMesh;
using BoundingBoxes = IBottomLevelAccelerationStructure::BoundingBoxes;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12BottomLevelAccelerationStructure::DirectX12BottomLevelAccelerationStructureImpl : public Implement<DirectX12BottomLevelAccelerationStructure> {
public:
    friend class DirectX12BottomLevelAccelerationStructure;

private:
    Array<TriangleMesh>  m_triangleMeshes { };
    Array<BoundingBoxes> m_boundingBoxes  { };
    AccelerationStructureFlags m_flags;

public:
    DirectX12BottomLevelAccelerationStructureImpl(DirectX12BottomLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }

public:
    Array<D3D12_RAYTRACING_GEOMETRY_DESC> build() const 
    {
        return [this]() -> std::generator<D3D12_RAYTRACING_GEOMETRY_DESC> {
            // Build up mesh descriptions.
            for (UInt32 i{ 0 }; auto& mesh : m_triangleMeshes)
            {
                // Find the position attribute.
                auto attributes = mesh.VertexBuffer->layout().attributes();
                auto positionAttribute = std::ranges::find_if(attributes, [](const BufferAttribute* attribute) { return attribute->semantic() == AttributeSemantic::Position; });

                if (positionAttribute == attributes.end()) [[unlikely]]
                    throw RuntimeException("A vertex buffer must contain a position attribute to be used in a bottom-level acceleration structure.");

                if ((*positionAttribute)->offset() != 0) [[unlikely]]
                    throw RuntimeException("The position attribute must not have a non-zero offset in the vertex buffer layout.");

                co_yield D3D12_RAYTRACING_GEOMETRY_DESC {
                    .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
                    .Flags = std::bit_cast<D3D12_RAYTRACING_GEOMETRY_FLAGS>(mesh.Flags),
                    .Triangles = D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC {
                        .Transform3x4 = mesh.TransformBuffer == nullptr ? 0 : mesh.TransformBuffer->virtualAddress(),
                        .IndexFormat = mesh.IndexBuffer == nullptr ? DXGI_FORMAT_UNKNOWN : (mesh.IndexBuffer->layout().indexType() == IndexType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT),
                        .VertexFormat = DX12::getFormat((*positionAttribute)->format()),
                        .IndexCount = mesh.IndexBuffer == nullptr ? 0 : mesh.IndexBuffer->elements(),
                        .VertexCount = mesh.VertexBuffer->elements(),
                        .IndexBuffer = mesh.IndexBuffer == nullptr ? 0 : mesh.IndexBuffer->virtualAddress(),
                        .VertexBuffer = {
                            mesh.VertexBuffer->virtualAddress(),
                            mesh.VertexBuffer->alignedElementSize()
                        }
                    }
                };
            }

            // Build up AABB descriptions.
            for (UInt32 i{ 0 }; auto& bb : m_boundingBoxes)
            {
                if (bb.Buffer == nullptr) [[unlikely]]
                    throw RuntimeException("Cannot build bottom-level acceleration structure from uninitialized bounding boxes.");

                co_yield D3D12_RAYTRACING_GEOMETRY_DESC {
                    .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS,
                    .Flags = std::bit_cast<D3D12_RAYTRACING_GEOMETRY_FLAGS>(bb.Flags),
                    .AABBs = D3D12_RAYTRACING_GEOMETRY_AABBS_DESC {
                        .AABBCount = bb.Buffer->elements(),
                        .AABBs = { bb.Buffer->virtualAddress(), bb.Buffer->alignedElementSize() }
                    }
                };
            }
        }() | std::ranges::to<Array<D3D12_RAYTRACING_GEOMETRY_DESC>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12BottomLevelAccelerationStructure::DirectX12BottomLevelAccelerationStructure(AccelerationStructureFlags flags) :
    m_impl(makePimpl<DirectX12BottomLevelAccelerationStructureImpl>(this, flags))
{
}

DirectX12BottomLevelAccelerationStructure::~DirectX12BottomLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags DirectX12BottomLevelAccelerationStructure::flags() const noexcept 
{
    return m_impl->m_flags;
}

const Array<TriangleMesh>& DirectX12BottomLevelAccelerationStructure::triangleMeshes() const noexcept
{
    return m_impl->m_triangleMeshes;
}

void DirectX12BottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh& mesh)
{
    m_impl->m_triangleMeshes.push_back(mesh);
}

const Array<BoundingBoxes>& DirectX12BottomLevelAccelerationStructure::boundingBoxes() const noexcept
{
    return m_impl->m_boundingBoxes;
}

void DirectX12BottomLevelAccelerationStructure::addBoundingBox(const BoundingBoxes& aabb)
{
    m_impl->m_boundingBoxes.push_back(aabb);
}

void DirectX12BottomLevelAccelerationStructure::clear(bool meshes, bool boundingBoxes)
{
    if (meshes)
        m_impl->m_triangleMeshes.clear();

    if (boundingBoxes)
        m_impl->m_boundingBoxes.clear();
}

Array<D3D12_RAYTRACING_GEOMETRY_DESC> DirectX12BottomLevelAccelerationStructure::buildInfo() const
{
    return m_impl->build();
}