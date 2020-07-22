#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class IndexBuffer::IndexBufferImpl : public Implement<IndexBuffer> {
public:
    friend class IndexBuffer;

private:
    const IIndexBufferLayout* m_layout;

public:
    IndexBufferImpl(IndexBuffer* parent, const IIndexBufferLayout* layout) :
        base(parent), m_layout(layout) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

IndexBuffer::IndexBuffer(const IIndexBufferLayout* layout, const UInt32& elements) :
    m_impl(makePimpl<IndexBufferImpl>(this, layout)), Buffer(elements, layout->getElementSize() * elements)
{
    if (layout == nullptr)
        throw std::invalid_argument("The index buffer layout must be initialized.");
}

IndexBuffer::~IndexBuffer() noexcept = default;

const IIndexBufferLayout* IndexBuffer::getLayout() const noexcept
{
    return m_impl->m_layout;
}