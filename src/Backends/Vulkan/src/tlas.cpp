#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using Instance = ITopLevelAccelerationStructure::Instance;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructureImpl : public Implement<VulkanTopLevelAccelerationStructure> {
public:
    friend class VulkanTopLevelAccelerationStructure;

private:
    Array<Instance> m_instances { };

public:
    VulkanTopLevelAccelerationStructureImpl(VulkanTopLevelAccelerationStructure* parent) noexcept :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure() noexcept :
    m_impl(makePimpl<VulkanTopLevelAccelerationStructureImpl>(this))
{
}

VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure() noexcept = default;

const Array<Instance>& VulkanTopLevelAccelerationStructure::instances() const noexcept
{
    return m_impl->m_instances;
}

void VulkanTopLevelAccelerationStructure::addInstance(const Instance& instance)
{
    m_impl->m_instances.push_back(instance);
}

void VulkanTopLevelAccelerationStructure::clear()
{
    m_impl->m_instances.clear();
}