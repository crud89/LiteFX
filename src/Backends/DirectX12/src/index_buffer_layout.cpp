#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12IndexBufferLayout::DirectX12IndexBufferLayoutImpl : public Implement<DirectX12IndexBufferLayout> {
public:
    friend class DirectX12IndexBufferLayout;

private:
    IndexType m_indexType;
    UInt32 m_binding{ 0 };
    BufferType m_bufferType{ BufferType::Index };

public:
    DirectX12IndexBufferLayoutImpl(DirectX12IndexBufferLayout* parent, IndexType type) : 
        base(parent), m_indexType(type) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12IndexBufferLayout::DirectX12IndexBufferLayout(IndexType type) :
    m_impl(makePimpl<DirectX12IndexBufferLayoutImpl>(this, type))
{
}

DirectX12IndexBufferLayout::~DirectX12IndexBufferLayout() noexcept = default;

size_t DirectX12IndexBufferLayout::elementSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_indexType) >> 3;
}

UInt32 DirectX12IndexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

BufferType DirectX12IndexBufferLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

IndexType DirectX12IndexBufferLayout::indexType() const noexcept
{
    return m_impl->m_indexType;
}