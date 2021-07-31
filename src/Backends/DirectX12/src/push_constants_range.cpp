#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PushConstantsRange::DirectX12PushConstantsRangeImpl : public Implement<DirectX12PushConstantsRange> {
public:
    friend class DirectX12PushConstantsRange;

private:
    ShaderStage m_stages;
    UInt32 m_offset, m_size, m_space, m_binding;

public:
    DirectX12PushConstantsRangeImpl(DirectX12PushConstantsRange* parent, const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
        base(parent), m_stages(shaderStages), m_offset(offset), m_size(size), m_space(space), m_binding(binding)
    {
        if (offset % 4 != 0)
            throw InvalidArgumentException("The push constants range offset must be a multiple of 4 bytes.");

        if (size % 4 != 0)
            throw InvalidArgumentException("The push constants range size must be a multiple of 4 bytes.");
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12PushConstantsRange::DirectX12PushConstantsRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) :
    m_impl(makePimpl<DirectX12PushConstantsRangeImpl>(this, shaderStages, offset, size, space, binding))
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

const ShaderStage& DirectX12PushConstantsRange::stages() const noexcept
{
    return m_impl->m_stages;
}