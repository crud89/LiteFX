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
    AccelerationStructureFlags m_flags;

public:
    VulkanTopLevelAccelerationStructureImpl(VulkanTopLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags) :
    m_impl(makePimpl<VulkanTopLevelAccelerationStructureImpl>(this, flags))
{
}

VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags VulkanTopLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

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