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
    UniquePtr<IVulkanSampler> m_staticSampler;

public:
    VulkanDescriptorLayoutImpl(VulkanDescriptorLayout* parent, DescriptorType type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors) :
        base(parent), m_descriptorType(type), m_binding(binding), m_elementSize(elementSize), m_descriptors(descriptors)
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
        default:
            m_bufferType = BufferType::Other;
            break;
        }
    }

    VulkanDescriptorLayoutImpl(VulkanDescriptorLayout* parent, UniquePtr<IVulkanSampler>&& staticSampler, const UInt32& binding) :
        VulkanDescriptorLayoutImpl(parent, DescriptorType::Sampler, binding, 0, 1)
    {
        if (staticSampler == nullptr)
            throw ArgumentNotInitializedException("The static sampler must be initialized.");

        m_staticSampler = std::move(staticSampler);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorLayout::VulkanDescriptorLayout(DescriptorType type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors) :
    m_impl(makePimpl<VulkanDescriptorLayoutImpl>(this, type, binding, elementSize, descriptors))
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(UniquePtr<IVulkanSampler>&& staticSampler, const UInt32& binding) :
    m_impl(makePimpl<VulkanDescriptorLayoutImpl>(this, std::move(staticSampler), binding))
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