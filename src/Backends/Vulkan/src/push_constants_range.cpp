#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsRange::VulkanPushConstantsRangeImpl : public Implement<VulkanPushConstantsRange> {
public:
    friend class VulkanPushConstantsRange;

private:
    ShaderStage m_stages;
    UInt32 m_offset, m_size, m_space, m_binding;

public:
    VulkanPushConstantsRangeImpl(VulkanPushConstantsRange* parent, const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
        base(parent), m_stages(shaderStages), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsRange::VulkanPushConstantsRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
    m_impl(makePimpl<VulkanPushConstantsRangeImpl>(this, shaderStages, offset, size, space, binding))
{
}

VulkanPushConstantsRange::~VulkanPushConstantsRange() noexcept = default;

const UInt32& VulkanPushConstantsRange::space() const noexcept
{
    return m_impl->m_space;
}

const UInt32& VulkanPushConstantsRange::binding() const noexcept
{
    return m_impl->m_binding;
}

const UInt32& VulkanPushConstantsRange::offset() const noexcept
{
    return m_impl->m_offset;
}

const UInt32& VulkanPushConstantsRange::size() const noexcept
{
    return m_impl->m_size;
}

const ShaderStage& VulkanPushConstantsRange::stages() const noexcept
{
    return m_impl->m_stages;
}