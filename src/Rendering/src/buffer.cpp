#include <litefx/rendering_pipelines.hpp>

using  namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Buffer::BufferImpl : public Implement<Buffer> {
public:
    friend class Buffer;

private:
    const IBufferLayout* m_layout;
    UInt32 m_elements;

public:
    BufferImpl(Buffer* parent, const IBufferLayout* layout, const UInt32& elements) : 
        base(parent), m_layout(layout), m_elements(elements) {}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

Buffer::Buffer(const IBufferLayout* layout, const UInt32& elements) :
    m_impl(makePimpl<BufferImpl>(this, layout, elements))
{
    if (layout == nullptr)
        throw std::invalid_argument("The buffer layout must be initialized.");
}

Buffer::~Buffer() noexcept = default;

const IBufferLayout* Buffer::getLayout() const noexcept
{
    return m_impl->m_layout;
}

UInt32 Buffer::getElements() const noexcept
{
    return m_impl->m_elements;
}

UInt32 Buffer::getSize() const noexcept
{
    return m_impl->m_layout->getElementSize() * m_impl->m_elements;
}