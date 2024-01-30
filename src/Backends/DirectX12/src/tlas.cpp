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
    UniquePtr<IDirectX12Buffer> m_buffer, m_instanceBuffer;
    UInt64 m_scratchBufferSize { };

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
            if (instance.BottomLevelAccelerationStructure->buffer() == nullptr) [[unlikely]]
                throw RuntimeException("The bottom-level acceleration structure for at least one instance has not yet been built.");

            auto desc = D3D12_RAYTRACING_INSTANCE_DESC {
                .InstanceID = instance.Id,
                .InstanceMask = instance.Mask,
                .InstanceContributionToHitGroupIndex = instance.HitGroupOffset,
                .Flags = static_cast<UINT>(instance.Flags),
                .AccelerationStructure = instance.BottomLevelAccelerationStructure->buffer()->virtualAddress()
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

UInt64 DirectX12TopLevelAccelerationStructure::requiredScratchMemory() const noexcept
{
    return m_impl->m_scratchBufferSize;
}

const IDirectX12Buffer* DirectX12TopLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer.get();
}

void DirectX12TopLevelAccelerationStructure::allocateBuffer(const DirectX12Device& device)
{
#ifndef LITEFX_BUILD_RAY_TRACING_SUPPORT
    throw RuntimeException("Unable to allocate acceleration structure buffer. Engine was not built with ray-tracing support enabled.");
#else
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("The buffer for this acceleration structure has already been allocated.");

    // Compute buffer sizes.
    UInt64 bufferSize{ };
    device.computeAccelerationStructureSizes(*this, bufferSize, m_impl->m_scratchBufferSize);

    // Allocate the buffers.
    m_impl->m_buffer = device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, bufferSize, 1, ResourceUsage::AllowWrite);
    m_impl->m_instanceBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Dynamic, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_impl->m_instances.size(), 1, ResourceUsage::AccelerationStructureBuildInput);

    // Map the instance buffer.
    auto buildInfo = m_impl->buildInfo();
    m_impl->m_instanceBuffer->map(buildInfo.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_impl->m_instances.size());
#endif
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

Array<D3D12_RAYTRACING_INSTANCE_DESC> DirectX12TopLevelAccelerationStructure::buildInfo() const
{
    return m_impl->buildInfo();
}

void DirectX12TopLevelAccelerationStructure::makeBuffer(const IGraphicsDevice& device)
{
    this->allocateBuffer(dynamic_cast<const DirectX12Device&>(device));
}

const IDirectX12Buffer* DirectX12TopLevelAccelerationStructure::instanceBuffer() const noexcept
{
    return m_impl->m_instanceBuffer.get();
}