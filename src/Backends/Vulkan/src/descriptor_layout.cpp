#include <litefx/backends/vulkan.hpp>

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
    UniquePtr<IVulkanSampler> m_staticSampler;

public:
    VulkanDescriptorLayoutImpl(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors) :
        m_elementSize(elementSize), m_binding(binding), m_descriptors(descriptors), m_inputAttachmentIndex(0), m_descriptorType(type)
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

    VulkanDescriptorLayoutImpl(UniquePtr<IVulkanSampler>&& staticSampler, UInt32 binding) :
        VulkanDescriptorLayoutImpl(DescriptorType::Sampler, binding, 0, 1)
    {
        if (staticSampler == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("staticSampler", "The static sampler must be initialized.");

        m_staticSampler = std::move(staticSampler);
    }

    VulkanDescriptorLayoutImpl(UInt32 binding, UInt32 inputAttachmentIndex) :
        VulkanDescriptorLayoutImpl(DescriptorType::Sampler, binding, 0, 1)
    {
        m_inputAttachmentIndex = inputAttachmentIndex;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorLayout::VulkanDescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors) :
    m_impl(type, binding, elementSize, descriptors)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(UniquePtr<IVulkanSampler>&& staticSampler, UInt32 binding) :
    m_impl(std::move(staticSampler), binding)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(UInt32 binding, UInt32 inputAttachmentIndex) :
    m_impl(binding, inputAttachmentIndex)
{
}

VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDescriptorLayout&&) noexcept = default;
VulkanDescriptorLayout& VulkanDescriptorLayout::operator=(VulkanDescriptorLayout&&) noexcept = default;
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