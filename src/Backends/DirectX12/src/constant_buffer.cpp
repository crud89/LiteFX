#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorLayout::DirectX12DescriptorLayoutImpl : public Implement<DirectX12DescriptorLayout> {
public:
    friend class DirectX12DescriptorLayout;

private:
    const IDescriptorSetLayout* m_setLayout;
    size_t m_elementSize;
    UInt32 m_binding;
    DescriptorType m_type;

public:
    DirectX12DescriptorLayoutImpl(DirectX12DescriptorLayout* parent, const DirectX12DescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize) :
        base(parent), m_setLayout(&descriptorSetLayout), m_type(type), m_binding(binding), m_elementSize(elementSize) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorLayout::DirectX12DescriptorLayout(const DirectX12DescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize) :
    m_impl(makePimpl<DirectX12DescriptorLayoutImpl>(this, descriptorSetLayout, type, binding, elementSize)), DirectX12RuntimeObject(descriptorSetLayout.getDevice())
{
}

DirectX12DescriptorLayout::~DirectX12DescriptorLayout() noexcept = default;

size_t DirectX12DescriptorLayout::getElementSize() const noexcept
{
    return m_impl->m_elementSize;
}

UInt32 DirectX12DescriptorLayout::getBinding() const noexcept
{
    return m_impl->m_binding;
}

BufferType DirectX12DescriptorLayout::getType() const noexcept
{
    switch (m_impl->m_type)
    {
    case DescriptorType::Uniform: return BufferType::Uniform;
    case DescriptorType::Storage: return BufferType::Storage;
    default: return BufferType::Other;
    }
}

const IDescriptorSetLayout* DirectX12DescriptorLayout::getDescriptorSet() const noexcept
{
    return m_impl->m_setLayout;
}

DescriptorType DirectX12DescriptorLayout::getDescriptorType() const noexcept
{
    return m_impl->m_type;
}