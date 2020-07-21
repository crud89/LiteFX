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
    UInt32 m_binding;
    DescriptorType m_type;

public:
    VulkanDescriptorLayoutImpl(VulkanDescriptorLayout* parent, const DescriptorType& type, const UInt32& binding, const size_t& elementSize) : 
        base(parent), m_type(type), m_binding(binding), m_elementSize(elementSize) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorLayout::VulkanDescriptorLayout(const VulkanDescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize) :
    m_impl(makePimpl<VulkanDescriptorLayoutImpl>(this, type, binding, elementSize)), VulkanRuntimeObject(descriptorSetLayout.getDevice())
{
}

VulkanDescriptorLayout::~VulkanDescriptorLayout() noexcept = default;

size_t VulkanDescriptorLayout::getElementSize() const noexcept
{
    return m_impl->m_elementSize;
}

UInt32 VulkanDescriptorLayout::getBinding() const noexcept
{
    return m_impl->m_binding;
}

BufferType VulkanDescriptorLayout::getType() const noexcept
{
    return BufferType::Descriptor;
}

DescriptorType VulkanDescriptorLayout::getDescriptorType() const noexcept
{
    return m_impl->m_type;
}