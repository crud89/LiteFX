#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12IndexBufferLayout::DirectX12IndexBufferLayoutImpl : public Implement<DirectX12IndexBufferLayout> {
public:
    friend class DirectX12IndexBufferLayout;

private:
    IndexType m_type;

public:
    DirectX12IndexBufferLayoutImpl(DirectX12IndexBufferLayout* parent, const IndexType& type) : base(parent), m_type(type) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12IndexBufferLayout::DirectX12IndexBufferLayout(const DirectX12InputAssembler& inputAssembler, const IndexType& type) :
    m_impl(makePimpl<DirectX12IndexBufferLayoutImpl>(this, type)), DirectX12RuntimeObject(inputAssembler.getDevice())
{
}

DirectX12IndexBufferLayout::~DirectX12IndexBufferLayout() noexcept = default;

size_t DirectX12IndexBufferLayout::getElementSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_type) >> 3;
}

UInt32 DirectX12IndexBufferLayout::getBinding() const noexcept
{
    return 0;
}

BufferType DirectX12IndexBufferLayout::getType() const noexcept
{
    return BufferType::Index;
}

const IndexType& DirectX12IndexBufferLayout::getIndexType() const noexcept
{
    return m_impl->m_type;
}