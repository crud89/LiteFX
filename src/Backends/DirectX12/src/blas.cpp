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
    UniquePtr<IDirectX12Buffer> m_buffer;
    UInt64 m_scratchBufferSize { };

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
            for (auto& mesh : m_triangleMeshes)
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
            for (auto& bb : m_boundingBoxes)
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

DirectX12BottomLevelAccelerationStructure::DirectX12BottomLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    m_impl(makePimpl<DirectX12BottomLevelAccelerationStructureImpl>(this, flags)), StateResource(name)
{
}

DirectX12BottomLevelAccelerationStructure::~DirectX12BottomLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags DirectX12BottomLevelAccelerationStructure::flags() const noexcept 
{
    return m_impl->m_flags;
}

UInt64 DirectX12BottomLevelAccelerationStructure::requiredScratchMemory() const noexcept
{
    return m_impl->m_scratchBufferSize;
}

const IDirectX12Buffer* DirectX12BottomLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer.get();
}

void DirectX12BottomLevelAccelerationStructure::allocateBuffer(const DirectX12Device& device)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("The buffer for this acceleration structure has already been allocated.");

    // Compute buffer sizes.
    UInt64 bufferSize{ };
    device.computeAccelerationStructureSizes(*this, bufferSize, m_impl->m_scratchBufferSize);

    // Allocate the buffer.
    m_impl->m_buffer = device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, bufferSize, 1, ResourceUsage::AllowWrite);
}

const Array<TriangleMesh>& DirectX12BottomLevelAccelerationStructure::triangleMeshes() const noexcept
{
    return m_impl->m_triangleMeshes;
}

void DirectX12BottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh& mesh)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_boundingBoxes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_triangleMeshes.push_back(mesh);
}

const Array<BoundingBoxes>& DirectX12BottomLevelAccelerationStructure::boundingBoxes() const noexcept
{
    return m_impl->m_boundingBoxes;
}

void DirectX12BottomLevelAccelerationStructure::addBoundingBox(const BoundingBoxes& aabb)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_triangleMeshes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_boundingBoxes.push_back(aabb);
}

Array<D3D12_RAYTRACING_GEOMETRY_DESC> DirectX12BottomLevelAccelerationStructure::buildInfo() const
{
    return m_impl->build();
}

void DirectX12BottomLevelAccelerationStructure::makeBuffer(const IGraphicsDevice& device)
{
    this->allocateBuffer(dynamic_cast<const DirectX12Device&>(device));
}