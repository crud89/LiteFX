#include <litefx/backends/vulkan.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorLayout::VulkanDescriptorLayoutImpl {
public:
    friend class VulkanDescriptorLayout;

private:
    size_t m_elementSize;
    UInt32 m_binding, m_descriptors, m_inputAttachmentIndex;
    DescriptorType m_descriptorType;
    BufferType m_bufferType;
    SharedPtr<const IVulkanSampler> m_staticSampler;
    bool m_unbounded;

public:
    VulkanDescriptorLayoutImpl(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors, bool unbounded) :
        m_elementSize(elementSize), m_binding(binding), m_descriptors(descriptors), m_inputAttachmentIndex(0), m_descriptorType(type), m_unbounded(unbounded)
    {
        switch (m_descriptorType)
        {
        case DescriptorType::ConstantBuffer:
            m_bufferType = BufferType::Uniform;
            break;
        case DescriptorType::RWStructuredBuffer:
        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWByteAddressBuffer:
        case DescriptorType::ByteAddressBuffer:
            m_bufferType = BufferType::Storage;
            break;
        case DescriptorType::RWBuffer:
        case DescriptorType::Buffer:
            m_bufferType = BufferType::Texel;
            break;
        case DescriptorType::AccelerationStructure:
            m_bufferType = BufferType::AccelerationStructure;
            break;
        default:
            m_bufferType = BufferType::Other;
            break;
        }
    }

    VulkanDescriptorLayoutImpl(const IVulkanSampler& staticSampler, UInt32 binding) :
        VulkanDescriptorLayoutImpl(DescriptorType::Sampler, binding, 0, 1, false)
    {
        m_staticSampler = VulkanSampler::copy(staticSampler);
    }

    VulkanDescriptorLayoutImpl(UInt32 binding, UInt32 inputAttachmentIndex) :
        VulkanDescriptorLayoutImpl(DescriptorType::Sampler, binding, 0, 1, false)
    {
        m_inputAttachmentIndex = inputAttachmentIndex;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorLayout::VulkanDescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors, bool unbounded) :
    m_impl(type, binding, elementSize, descriptors, unbounded)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(const IVulkanSampler& staticSampler, UInt32 binding) :
    m_impl(staticSampler, binding)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(UInt32 binding, UInt32 inputAttachmentIndex) :
    m_impl(binding, inputAttachmentIndex)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDescriptorLayout&&) noexcept = default;
VulkanDescriptorLayout::VulkanDescriptorLayout(const VulkanDescriptorLayout&) = default;
VulkanDescriptorLayout& VulkanDescriptorLayout::operator=(VulkanDescriptorLayout&&) noexcept = default;
VulkanDescriptorLayout& VulkanDescriptorLayout::operator=(const VulkanDescriptorLayout&) = default;
VulkanDescriptorLayout::~VulkanDescriptorLayout() noexcept = default;

size_t VulkanDescriptorLayout::elementSize() const noexcept
{
    return m_impl->m_elementSize;
}

UInt32 VulkanDescriptorLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

UInt32 VulkanDescriptorLayout::descriptors() const noexcept
{
    return m_impl->m_descriptors;
}

bool VulkanDescriptorLayout::unbounded() const noexcept
{
    return m_impl->m_unbounded;
}

BufferType VulkanDescriptorLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

DescriptorType VulkanDescriptorLayout::descriptorType() const noexcept
{
    return m_impl->m_descriptorType;
}

const IVulkanSampler* VulkanDescriptorLayout::staticSampler() const noexcept
{
    return m_impl->m_staticSampler.get();
}

UInt32 VulkanDescriptorLayout::inputAttachmentIndex() const noexcept
{
    return m_impl->m_inputAttachmentIndex;
}