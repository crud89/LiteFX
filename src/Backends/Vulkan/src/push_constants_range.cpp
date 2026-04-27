#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsRange::VulkanPushConstantsRangeImpl {
public:
    friend class VulkanPushConstantsRange;

private:
    ShaderStage m_stageMask;
    UInt32 m_offset, m_size, m_space, m_binding;

public:
    VulkanPushConstantsRangeImpl(ShaderStage shaderStageMask, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
        m_stageMask(shaderStageMask), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
        if (offset % 4 != 0)
            throw InvalidArgumentException("offset", "The push constants range offset must be a multiple of 4 bytes.");

        if (size % 4 != 0)
            throw InvalidArgumentException("size", "The push constants range size must be a multiple of 4 bytes.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsRange::VulkanPushConstantsRange(ShaderStage shaderStageMask, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
    m_impl(shaderStageMask, offset, size, space, binding)
{
}

VulkanPushConstantsRange::VulkanPushConstantsRange(VulkanPushConstantsRange&&) noexcept = default;
VulkanPushConstantsRange::VulkanPushConstantsRange(const VulkanPushConstantsRange&) = default;
VulkanPushConstantsRange& VulkanPushConstantsRange::operator=(VulkanPushConstantsRange&&) noexcept = default;
VulkanPushConstantsRange& VulkanPushConstantsRange::operator=(const VulkanPushConstantsRange&) = default;
VulkanPushConstantsRange::~VulkanPushConstantsRange() noexcept = default;

UInt32 VulkanPushConstantsRange::space() const noexcept
{
    return m_impl->m_space;
}

UInt32 VulkanPushConstantsRange::binding() const noexcept
{
    return m_impl->m_binding;
}

UInt32 VulkanPushConstantsRange::offset() const noexcept
{
    return m_impl->m_offset;
}

UInt32 VulkanPushConstantsRange::size() const noexcept
{
    return m_impl->m_size;
}

ShaderStage VulkanPushConstantsRange::stageMask() const noexcept
{
    return m_impl->m_stageMask;
}