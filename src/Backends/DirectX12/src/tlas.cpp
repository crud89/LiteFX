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
    UInt64 m_offset { };

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

void DirectX12TopLevelAccelerationStructure::build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer, SharedPtr<const IDirectX12Buffer> buffer, UInt64 offset, UInt64 maxSize)
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