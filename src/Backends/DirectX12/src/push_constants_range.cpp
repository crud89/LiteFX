#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PushConstantsRange::DirectX12PushConstantsRangeImpl : public Implement<DirectX12PushConstantsRange> {
public:
    friend class DirectX12PushConstantsRange;

private:
    ShaderStage m_stage;
    UInt32 m_offset, m_size, m_space, m_binding, m_rootParameterIndex{ 0 };

public:
    DirectX12PushConstantsRangeImpl(DirectX12PushConstantsRange* parent, const ShaderStage& shaderStage, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
        base(parent), m_stage(shaderStage), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
        if (offset % 4 != 0)
            throw InvalidArgumentException("The push constants range offset must be a multiple of 4 bytes.");

        if (size % 4 != 0)
            throw InvalidArgumentException("The push constants range size must be a multiple of 4 bytes.");

        if (!(static_cast<UInt32>(shaderStage) && !(static_cast<UInt32>(shaderStage) & (static_cast<UInt32>(shaderStage) - 1))))
            throw InvalidArgumentException("A push constant range is only allowed to be associated with one shader stage.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12PushConstantsRange::DirectX12PushConstantsRange(const ShaderStage& shaderStage, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
    m_impl(makePimpl<DirectX12PushConstantsRangeImpl>(this, shaderStage, offset, size, space, binding))
{
}

DirectX12PushConstantsRange::~DirectX12PushConstantsRange() noexcept = default;

const UInt32& DirectX12PushConstantsRange::space() const noexcept
{
    return m_impl->m_space;
}

const UInt32& DirectX12PushConstantsRange::binding() const noexcept
{
    return m_impl->m_binding;
}

const UInt32& DirectX12PushConstantsRange::offset() const noexcept
{
    return m_impl->m_offset;
}

const UInt32& DirectX12PushConstantsRange::size() const noexcept
{
    return m_impl->m_size;
}

const ShaderStage& DirectX12PushConstantsRange::stage() const noexcept
{
    return m_impl->m_stage;
}

const UInt32& DirectX12PushConstantsRange::rootParameterIndex() const noexcept
{
    return m_impl->m_rootParameterIndex;
}

UInt32& DirectX12PushConstantsRange::rootParameterIndex() noexcept
{
    return m_impl->m_rootParameterIndex;
}