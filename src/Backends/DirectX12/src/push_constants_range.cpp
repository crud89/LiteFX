#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PushConstantsRange::DirectX12PushConstantsRangeImpl {
public:
    friend class DirectX12PushConstantsRange;

private:
    ShaderStage m_stage;
    UInt32 m_offset, m_size, m_space, m_binding, m_rootParameterIndex{ 0 };

public:
    DirectX12PushConstantsRangeImpl(ShaderStage shaderStage, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
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

DirectX12PushConstantsRange::DirectX12PushConstantsRange(ShaderStage shaderStage, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) :
    m_impl(shaderStage, offset, size, space, binding)
{
}

DirectX12PushConstantsRange::DirectX12PushConstantsRange(DirectX12PushConstantsRange&&) noexcept = default;
DirectX12PushConstantsRange::DirectX12PushConstantsRange(const DirectX12PushConstantsRange&) noexcept = default;
DirectX12PushConstantsRange& DirectX12PushConstantsRange::operator=(DirectX12PushConstantsRange&&) noexcept = default;
DirectX12PushConstantsRange& DirectX12PushConstantsRange::operator=(const DirectX12PushConstantsRange&) noexcept = default;
DirectX12PushConstantsRange::~DirectX12PushConstantsRange() noexcept = default;

UInt32 DirectX12PushConstantsRange::space() const noexcept
{
    return m_impl->m_space;
}

UInt32 DirectX12PushConstantsRange::binding() const noexcept
{
    return m_impl->m_binding;
}

UInt32 DirectX12PushConstantsRange::offset() const noexcept
{
    return m_impl->m_offset;
}

UInt32 DirectX12PushConstantsRange::size() const noexcept
{
    return m_impl->m_size;
}

ShaderStage DirectX12PushConstantsRange::stage() const noexcept
{
    return m_impl->m_stage;
}

UInt32 DirectX12PushConstantsRange::rootParameterIndex() const noexcept
{
    return m_impl->m_rootParameterIndex;
}

UInt32& DirectX12PushConstantsRange::rootParameterIndex() noexcept
{
    return m_impl->m_rootParameterIndex;
}