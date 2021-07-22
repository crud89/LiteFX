#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorLayout::VulkanDescriptorLayoutImpl : public Implement<VulkanDescriptorLayout> {
public:
    friend class VulkanDescriptorLayout;

private:
    size_t m_elementSize;
    UInt32 m_binding, m_descriptors;
    DescriptorType m_descriptorType;
    BufferType m_bufferType;

public:
    VulkanDescriptorLayoutImpl(VulkanDescriptorLayout* parent, const DescriptorType& type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors) :
        base(parent), m_descriptorType(type), m_binding(binding), m_elementSize(elementSize), m_descriptors(descriptors)
    {
        switch (m_descriptorType)
        {
        case DescriptorType::Uniform:       m_bufferType = BufferType::Uniform; break;
        case DescriptorType::WritableStorage:
        case DescriptorType::Storage:       m_bufferType = BufferType::Storage; break;
        case DescriptorType::WritableBuffer:
        case DescriptorType::Buffer:        m_bufferType = BufferType::Texel; break;
        default:                            m_bufferType = BufferType::Other; break;
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorLayout::VulkanDescriptorLayout(const VulkanDescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors) :
    m_impl(makePimpl<VulkanDescriptorLayoutImpl>(this, type, binding, elementSize, descriptors)), VulkanRuntimeObject<VulkanDescriptorSetLayout>(descriptorSetLayout, descriptorSetLayout.getDevice())
{
}

VulkanDescriptorLayout::~VulkanDescriptorLayout() noexcept = default;

size_t VulkanDescriptorLayout::elementSize() const noexcept
{
    return m_impl->m_elementSize;
}

const UInt32& VulkanDescriptorLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

const UInt32& VulkanDescriptorLayout::descriptors() const noexcept
{
    return m_impl->m_descriptors;
}

const BufferType& VulkanDescriptorLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

const DescriptorType& VulkanDescriptorLayout::descriptorType() const noexcept
{
    return m_impl->m_descriptorType;
}