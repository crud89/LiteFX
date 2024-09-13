#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;
using Instance = ITopLevelAccelerationStructure::Instance;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12TopLevelAccelerationStructure::DirectX12TopLevelAccelerationStructureImpl : public Implement<DirectX12TopLevelAccelerationStructure> {
public:
    friend class DirectX12TopLevelAccelerationStructure;

private:
    Array<Instance> m_instances { };
    AccelerationStructureFlags m_flags;
    SharedPtr<const IDirectX12Buffer> m_buffer;
    UniquePtr<IDirectX12Buffer> m_postBuildBuffer, m_postBuildResults;
    UInt64 m_offset { }, m_size { };

public:
    DirectX12TopLevelAccelerationStructureImpl(DirectX12TopLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }

public:
    Array<D3D12_RAYTRACING_INSTANCE_DESC> buildInfo() const
    {
        return m_instances | std::views::transform([](const Instance& instance) {
            const auto& blasBuffer = instance.BottomLevelAccelerationStructure->buffer();

            auto desc = D3D12_RAYTRACING_INSTANCE_DESC {
                .InstanceID = instance.Id,
                .InstanceMask = instance.Mask,
                .InstanceContributionToHitGroupIndex = instance.HitGroupOffset,
                .Flags = static_cast<UINT>(instance.Flags),
                .AccelerationStructure = blasBuffer == nullptr ? 0ull : blasBuffer->virtualAddress() + instance.BottomLevelAccelerationStructure->offset()
            };

            std::memcpy(desc.Transform, instance.Transform.elements(), sizeof(Float) * 12);
            return desc;
        }) | std::ranges::to<Array<D3D12_RAYTRACING_INSTANCE_DESC>>();
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

DirectX12TopLevelAccelerationStructure::DirectX12TopLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    m_impl(makePimpl<DirectX12TopLevelAccelerationStructureImpl>(this, flags)), StateResource(name)
{
}

DirectX12TopLevelAccelerationStructure::~DirectX12TopLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags DirectX12TopLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

SharedPtr<const IDirectX12Buffer> DirectX12TopLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer;
}

UInt64 DirectX12TopLevelAccelerationStructure::offset() const noexcept
{
    return m_impl->m_offset;
}

UInt64 DirectX12TopLevelAccelerationStructure::size() const noexcept
{
    UInt64 size = m_impl->m_size;

    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        m_impl->m_postBuildResults->map(&size, sizeof(UInt64), 0, false);

    return size;
}

void DirectX12TopLevelAccelerationStructure::build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
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
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0uz, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

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

void DirectX12TopLevelAccelerationStructure::update(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
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
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0uz, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

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

void DirectX12TopLevelAccelerationStructure::copy(const DirectX12CommandBuffer& commandBuffer, DirectX12TopLevelAccelerationStructure& destination, bool compress, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, bool copyBuildInfo) const
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
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

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
        destination.m_impl->m_instances = m_impl->m_instances;
}

const Array<Instance>& DirectX12TopLevelAccelerationStructure::instances() const noexcept
{
    return m_impl->m_instances;
}

void DirectX12TopLevelAccelerationStructure::addInstance(const Instance& instance)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    m_impl->m_instances.push_back(instance);
}

void DirectX12TopLevelAccelerationStructure::clear() noexcept
{
    m_impl->m_instances.clear();
}

bool DirectX12TopLevelAccelerationStructure::remove(const Instance& instance) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_instances, [&instance](const auto& e) { return std::addressof(e) == std::addressof(instance); }); match != m_impl->m_instances.end())
    {
        m_impl->m_instances.erase(match);
        return true;
    }

    return false;
}

Array<D3D12_RAYTRACING_INSTANCE_DESC> DirectX12TopLevelAccelerationStructure::buildInfo() const
{
    return m_impl->buildInfo();
}

SharedPtr<const IBuffer> DirectX12TopLevelAccelerationStructure::getBuffer() const noexcept
{
    return std::static_pointer_cast<const IBuffer>(m_impl->m_buffer);
}

void DirectX12TopLevelAccelerationStructure::doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->build(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(scratchBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, maxSize);
}

void DirectX12TopLevelAccelerationStructure::doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->update(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(scratchBuffer), std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, maxSize);
}

void DirectX12TopLevelAccelerationStructure::doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const
{
    this->copy(dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer), dynamic_cast<DirectX12TopLevelAccelerationStructure&>(destination), compress, std::dynamic_pointer_cast<const IDirectX12Buffer>(buffer), offset, copyBuildInfo);
}