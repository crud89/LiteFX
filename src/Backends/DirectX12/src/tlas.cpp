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

public:
    DirectX12TopLevelAccelerationStructureImpl(DirectX12TopLevelAccelerationStructure* parent) noexcept :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12TopLevelAccelerationStructure::DirectX12TopLevelAccelerationStructure() noexcept :
    m_impl(makePimpl<DirectX12TopLevelAccelerationStructureImpl>(this))
{
}

DirectX12TopLevelAccelerationStructure::~DirectX12TopLevelAccelerationStructure() noexcept = default;

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