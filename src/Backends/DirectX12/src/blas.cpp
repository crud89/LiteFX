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
    UniquePtr<IDirectX12Buffer> m_postBuildBuffer, m_postBuildResults;
    UInt64 m_offset { }, m_size { };

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

    inline void queuePostbuildInfoCommands(const DirectX12CommandBuffer& commandBuffer, bool afterCopy = false) 
    {
        auto& device = static_cast<const DirectX12Queue&>(commandBuffer.queue()).device();

        if (m_postBuildBuffer == nullptr) [[unlikely]]
        {
            m_postBuildBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, sizeof(UInt64), 1, ResourceUsage::TransferSource | ResourceUsage::AllowWrite);

            auto barrier = device.makeBarrier(PipelineStage::None, PipelineStage::AccelerationStructureCopy);
            barrier->transition(*m_postBuildBuffer, ResourceAccess::None, ResourceAccess::ShaderReadWrite);
            commandBuffer.barrier(*barrier);
        }

        if (m_postBuildResults == nullptr) [[unlikely]]
            m_postBuildResults = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Readback, sizeof(UInt64), 1, ResourceUsage::TransferDestination);

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC postBuildInfo = {
            .DestBuffer = m_postBuildBuffer->virtualAddress(),
            .InfoType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE
        };

        // Transition the buffer into UAV state. We create  manual barriers here, as the special access flag is only required in this specific situation.
        CD3DX12_BUFFER_BARRIER preBarrier[1] = {
            CD3DX12_BUFFER_BARRIER(afterCopy ? D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE : D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO, D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE, D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ, std::as_const(*m_buffer).handle().Get()),
            //CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO, D3D12_BARRIER_ACCESS_NO_ACCESS, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS, std::as_const(*m_postBuildBuffer).handle().Get()),
        };
        auto preBarrierGroup = CD3DX12_BARRIER_GROUP(1, preBarrier);
        commandBuffer.handle()->Barrier(1, &preBarrierGroup);

        // Emit the
        auto address = m_buffer->virtualAddress() + m_offset;
        commandBuffer.handle()->EmitRaytracingAccelerationStructurePostbuildInfo(&postBuildInfo, 1, &address);

        // Transition the buffer into copy state.
        auto postBarrier = CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS, D3D12_BARRIER_ACCESS_COPY_SOURCE, std::as_const(*m_postBuildBuffer).handle().Get());
        auto postBarrierGroup = CD3DX12_BARRIER_GROUP(1, &postBarrier);
        commandBuffer.handle()->Barrier(1, &postBarrierGroup);

        // Transfer the data into the host-visible buffer.
        commandBuffer.transfer(*m_postBuildBuffer, *m_postBuildResults);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12BottomLevelAccelerationStructure::DirectX12BottomLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    StateResource(name), m_impl(makePimpl<DirectX12BottomLevelAccelerationStructureImpl>(this, flags))
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

UInt64 DirectX12BottomLevelAccelerationStructure::size() const noexcept
{
    UInt64 size = m_impl->m_size;

    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        m_impl->m_postBuildResults->map(&size, sizeof(UInt64), 0, false);

    return size;
}

void DirectX12BottomLevelAccelerationStructure::build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the arguments.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const DirectX12Queue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    if ((offset % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to build the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer && m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0ull, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0ull, (UInt64)buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the build.
    commandBuffer.buildAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
    m_impl->m_size = requiredMemory;

    // If the acceleration structure allows for compaction, create a query pool in order to query the compacted size later.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        m_impl->queuePostbuildInfoCommands(commandBuffer);
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
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory, true);

    if ((offset % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to update the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0ull, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0ull, (UInt64)buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the update.
    commandBuffer.updateAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
    m_impl->m_size = requiredMemory;

    // If the acceleration structure allows for compaction, create a query pool in order to query the compacted size later.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        m_impl->queuePostbuildInfoCommands(commandBuffer);
}

void DirectX12BottomLevelAccelerationStructure::copy(const DirectX12CommandBuffer& commandBuffer, DirectX12BottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, bool copyBuildInfo) const
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (compress && !LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow compaction. Specify `AccelerationStructureFlags::AllowCompaction` during creation.");

    if ((offset % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

    // Get the amount of memory required. Note that in DirectX it is not possible to query the availability of size info, so we have to rely on external synchronization anyway.
    UInt64 requiredMemory = this->size();
    auto& device = static_cast<const DirectX12Queue&>(commandBuffer.queue()).device();

    // Validate the input arguments.
    if (buffer == nullptr)
        buffer = destination.m_impl->m_buffer->size() >= requiredMemory ? destination.m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0ull, (UInt64)buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Store the buffer and the offset.
    destination.m_impl->m_offset = offset;
    destination.m_impl->m_buffer = buffer;
    destination.m_impl->m_size = requiredMemory;

    // Perform the update.
    commandBuffer.copyAccelerationStructure(*this, destination, compress);

    // If the destination structure also allows for compaction, emit post-build commands for it.
    if (LITEFX_FLAG_IS_SET(destination.flags(), AccelerationStructureFlags::AllowCompaction))
        destination.m_impl->queuePostbuildInfoCommands(commandBuffer);

    // Copy build data, if requested.
    if (copyBuildInfo)
    {
        destination.m_impl->m_boundingBoxes = m_impl->m_boundingBoxes;
        destination.m_impl->m_triangleMeshes = m_impl->m_triangleMeshes;
    }
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

void DirectX12BottomLevelAccelerationStructure::doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const
{
    this->copy(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), dynamic_cast<DirectX12BottomLevelAccelerationStructure&>(destination), compress, std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, copyBuildInfo);
}