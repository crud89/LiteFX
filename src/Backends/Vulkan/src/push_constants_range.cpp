#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsRange::VulkanPushConstantsRangeImpl {
public:
    friend class VulkanPushConstantsRange;

private:
    ShaderStage m_stage;
    UInt32 m_offset, m_size, m_space, m_binding;

public:
    VulkanPushConstantsRangeImpl(ShaderStage shaderStage, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
        m_stage(shaderStage), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
        if (offset % 4 != 0)
            throw InvalidArgumentException("offset", "The push constants range offset must be a multiple of 4 bytes.");

        if (size % 4 != 0)
            throw InvalidArgumentException("size", "The push constants range size must be a multiple of 4 bytes.");

        if (!(std::to_underlying(shaderStage) && !(std::to_underlying(shaderStage) & (std::to_underlying(shaderStage) - 1))))
            throw InvalidArgumentException("shaderStage", "A push constant range is only allowed to be associated with one shader stage.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsRange::VulkanPushConstantsRange(ShaderStage shaderStage, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
    m_impl(shaderStage, offset, size, space, binding)
{
}

VulkanPushConstantsRange::VulkanPushConstantsRange(VulkanPushConstantsRange&&) noexcept = default;
VulkanPushConstantsRange::VulkanPushConstantsRange(const VulkanPushConstantsRange&) noexcept = default;
VulkanPushConstantsRange& VulkanPushConstantsRange::operator=(VulkanPushConstantsRange&&) noexcept = default;
VulkanPushConstantsRange& VulkanPushConstantsRange::operator=(const VulkanPushConstantsRange&) noexcept = default;
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

ShaderStage VulkanPushConstantsRange::stage() const noexcept
{
    return m_impl->m_stage;
}