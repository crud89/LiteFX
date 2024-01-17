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

public:
    DirectX12TopLevelAccelerationStructureImpl(DirectX12TopLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12TopLevelAccelerationStructure::DirectX12TopLevelAccelerationStructure(AccelerationStructureFlags flags) :
    m_impl(makePimpl<DirectX12TopLevelAccelerationStructureImpl>(this, flags))
{
}

DirectX12TopLevelAccelerationStructure::~DirectX12TopLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags DirectX12TopLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

const Array<Instance>& DirectX12TopLevelAccelerationStructure::instances() const noexcept
{
    return m_impl->m_instances;
}

void DirectX12TopLevelAccelerationStructure::addInstance(const Instance& instance)
{
    m_impl->m_instances.push_back(instance);
}

void DirectX12TopLevelAccelerationStructure::clear()
{
    m_impl->m_instances.clear();
}