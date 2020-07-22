#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VertexBuffer::VertexBufferImpl : public Implement<VertexBuffer> {
public:
    friend class VertexBuffer;

private:
    const IVertexBufferLayout* m_layout;

public:
    VertexBufferImpl(VertexBuffer* parent, const IVertexBufferLayout* layout) :
        base(parent), m_layout(layout) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VertexBuffer::VertexBuffer(const IVertexBufferLayout* layout, const UInt32& elements) : 
    m_impl(makePimpl<VertexBufferImpl>(this, layout)), Buffer(BufferType::Vertex, elements, layout->getElementSize() * elements)
{
    if (layout == nullptr)
        throw std::invalid_argument("The vertex buffer layout must be initialized.");
}

VertexBuffer::~VertexBuffer() noexcept = default;

const IVertexBufferLayout* VertexBuffer::getLayout() const noexcept
{
    return m_impl->m_layout;
}