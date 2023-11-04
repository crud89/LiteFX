#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPushConstantsRange::VulkanPushConstantsRangeImpl : public Implement<VulkanPushConstantsRange> {
public:
    friend class VulkanPushConstantsRange;

private:
    ShaderStage m_stage;
    UInt32 m_offset, m_size, m_space, m_binding;

public:
    VulkanPushConstantsRangeImpl(VulkanPushConstantsRange* parent, ShaderStage shaderStage, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
        base(parent), m_stage(shaderStage), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
        if (offset % 4 != 0)
            throw InvalidArgumentException("The push constants range offset must be a multiple of 4 bytes.");

        if (size % 4 != 0)
            throw InvalidArgumentException("The push constants range size must be a multiple of 4 bytes.");

        if (!(std::to_underlying(shaderStage) && !(std::to_underlying(shaderStage) & (std::to_underlying(shaderStage) - 1))))
            throw InvalidArgumentException("A push constant range is only allowed to be associated with one shader stage.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanPushConstantsRange::VulkanPushConstantsRange(ShaderStage shaderStage, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
    m_impl(makePimpl<VulkanPushConstantsRangeImpl>(this, shaderStage, offset, size, space, binding))
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

ShaderStage VulkanPushConstantsRange::stage() const noexcept
{
    return m_impl->m_stage;
}