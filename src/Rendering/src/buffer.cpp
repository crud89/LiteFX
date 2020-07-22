#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Buffer::BufferImpl : public Implement<Buffer> {
public:
    friend class Buffer;

private:
    UInt32 m_elements, m_size;

public:
    BufferImpl(Buffer* parent, const UInt32& elements, const UInt32& size) : 
        base(parent), m_elements(elements), m_size(size) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Buffer::Buffer(const UInt32& elements, const UInt32& size) : m_impl(makePimpl<BufferImpl>(this, elements, size))
{
}

Buffer::~Buffer() noexcept = default;

UInt32 Buffer::getElements() const noexcept
{
    return m_impl->m_elements;
}

size_t Buffer::getSize() const noexcept
{
    return m_impl->m_size;
}