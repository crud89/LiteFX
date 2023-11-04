#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorLayout::DirectX12DescriptorLayoutImpl : public Implement<DirectX12DescriptorLayout> {
public:
    friend class DirectX12DescriptorLayout;

private:
    size_t m_elementSize;
    UInt32 m_binding;
    DescriptorType m_descriptorType;
    BufferType m_bufferType;
    UInt32 m_descriptors;
    UniquePtr<IDirectX12Sampler> m_staticSampler;

public:
    DirectX12DescriptorLayoutImpl(DirectX12DescriptorLayout* parent, DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors) :
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
        }
    }

    DirectX12DescriptorLayoutImpl(DirectX12DescriptorLayout* parent, UniquePtr<IDirectX12Sampler>&& staticSampler, UInt32 binding) :
        DirectX12DescriptorLayoutImpl(parent, DescriptorType::Sampler, binding, 0, 1)
    {
        if (staticSampler == nullptr)
            throw ArgumentNotInitializedException("The static sampler must be initialized.");

        m_staticSampler = std::move(staticSampler);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorLayout::DirectX12DescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors) :
    m_impl(makePimpl<DirectX12DescriptorLayoutImpl>(this, type, binding, elementSize, descriptors))
{
}

DirectX12DescriptorLayout::DirectX12DescriptorLayout(UniquePtr<IDirectX12Sampler>&& staticSampler, UInt32 binding) :
    m_impl(makePimpl<DirectX12DescriptorLayoutImpl>(this, std::move(staticSampler), binding))
{
}

DirectX12DescriptorLayout::~DirectX12DescriptorLayout() noexcept = default;

size_t DirectX12DescriptorLayout::elementSize() const noexcept
{
    return m_impl->m_elementSize;
}

UInt32 DirectX12DescriptorLayout::descriptors() const noexcept
{
    return m_impl->m_descriptors;
}

UInt32 DirectX12DescriptorLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

BufferType DirectX12DescriptorLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

DescriptorType DirectX12DescriptorLayout::descriptorType() const noexcept
{
    return m_impl->m_descriptorType;
}

const IDirectX12Sampler* DirectX12DescriptorLayout::staticSampler() const noexcept
{
    return m_impl->m_staticSampler.get();
}