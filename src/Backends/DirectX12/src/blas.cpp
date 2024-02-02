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
    SharedPtr<const IDirectX12Buffer> m_buffer;
    UInt64 m_offset { };

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

SharedPtr<const IDirectX12Buffer> DirectX12BottomLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer;
}

UInt64 DirectX12BottomLevelAccelerationStructure::offset() const noexcept
{
    return m_impl->m_offset;
}

void DirectX12BottomLevelAccelerationStructure::build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the arguments.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const DirectX12Queue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to build the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer && m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", 0ull, maxSize, requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0ull, buffer->size(), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the build.
    commandBuffer.buildAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
}

void DirectX12BottomLevelAccelerationStructure::update(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowUpdate)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow updates. Specify `AccelerationStructureFlags::AllowUpdate` during creation.");

    // Validate the arguments and create the buffers if required.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const DirectX12Queue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to update the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", 0ull, maxSize, requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0ull, buffer->size(), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the update.
    commandBuffer.updateAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
}

const Array<TriangleMesh>& DirectX12BottomLevelAccelerationStructure::triangleMeshes() const noexcept
{
    return m_impl->m_triangleMeshes;
}

void DirectX12BottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh& mesh)
{
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
    if (!m_impl->m_triangleMeshes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_boundingBoxes.push_back(aabb);
}

void DirectX12BottomLevelAccelerationStructure::clear() noexcept
{
    m_impl->m_boundingBoxes.clear();
    m_impl->m_triangleMeshes.clear();
}

bool DirectX12BottomLevelAccelerationStructure::remove(const TriangleMesh& mesh) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_triangleMeshes, [&mesh](const auto& e) { return std::addressof(e) == std::addressof(mesh); }); match != m_impl->m_triangleMeshes.end())
    {
        m_impl->m_triangleMeshes.erase(match);
        return true;
    }

    return false;
}

bool DirectX12BottomLevelAccelerationStructure::remove(const BoundingBoxes& aabb) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_boundingBoxes, [&aabb](const auto& e) { return std::addressof(e) == std::addressof(aabb); }); match != m_impl->m_boundingBoxes.end())
    {
        m_impl->m_boundingBoxes.erase(match);
        return true;
    }

    return false;
}

Array<D3D12_RAYTRACING_GEOMETRY_DESC> DirectX12BottomLevelAccelerationStructure::buildInfo() const
{
    return m_impl->build();
}

SharedPtr<const IBuffer> DirectX12BottomLevelAccelerationStructure::getBuffer() const noexcept
{
    return std::static_pointer_cast<const IBuffer>(m_impl->m_buffer);
}

void DirectX12BottomLevelAccelerationStructure::doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->build(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(scratchBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, maxSize);
}

void DirectX12BottomLevelAccelerationStructure::doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->update(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(scratchBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, maxSize);
}