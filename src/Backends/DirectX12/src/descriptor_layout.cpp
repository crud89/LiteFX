#include <litefx/backends/dx12.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorLayout::DirectX12DescriptorLayoutImpl {
public:
    friend class DirectX12DescriptorLayout;

private:
    size_t m_elementSize;
    UInt32 m_binding;
    DescriptorType m_descriptorType;
    BufferType m_bufferType;
    UInt32 m_descriptors;
    SharedPtr<const IDirectX12Sampler> m_staticSampler{};
    bool m_local, m_unbounded;

public:
    DirectX12DescriptorLayoutImpl(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors, bool unbounded, bool local) :
        m_elementSize(elementSize), m_binding(binding), m_descriptorType(type), m_descriptors(descriptors), m_local(local), m_unbounded(unbounded)
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
        }
    }

    DirectX12DescriptorLayoutImpl(const IDirectX12Sampler& staticSampler, UInt32 binding, bool local) :
        DirectX12DescriptorLayoutImpl(DescriptorType::Sampler, binding, 0, 1, false, local)
    {
        m_staticSampler = DirectX12Sampler::copy(staticSampler);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorLayout::DirectX12DescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors, bool unbounded, bool local) :
    m_impl(type, binding, elementSize, descriptors, unbounded, local)
{
}

DirectX12DescriptorLayout::DirectX12DescriptorLayout(const IDirectX12Sampler& staticSampler, UInt32 binding, bool local) :
    m_impl(staticSampler, binding, local)
{
}

DirectX12DescriptorLayout::DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) noexcept = default;
DirectX12DescriptorLayout::DirectX12DescriptorLayout(const DirectX12DescriptorLayout&) = default;
DirectX12DescriptorLayout& DirectX12DescriptorLayout::operator=(DirectX12DescriptorLayout&&) noexcept = default;
DirectX12DescriptorLayout& DirectX12DescriptorLayout::operator=(const DirectX12DescriptorLayout&)= default;
DirectX12DescriptorLayout::~DirectX12DescriptorLayout() noexcept = default;

bool DirectX12DescriptorLayout::local() const noexcept
{
    return m_impl->m_local;
}

size_t DirectX12DescriptorLayout::elementSize() const noexcept
{
    return m_impl->m_elementSize;
}

UInt32 DirectX12DescriptorLayout::descriptors() const noexcept
{
    return m_impl->m_descriptors;
}

bool DirectX12DescriptorLayout::unbounded() const noexcept
{
    return m_impl->m_unbounded;
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