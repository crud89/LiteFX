#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class BufferAttribute::BufferAttributeImpl : public Implement<BufferAttribute> {
public:
    friend class BufferAttribute;

private:
    UInt32 m_location, m_offset, m_semanticIndex;
    BufferFormat m_format;
    AttributeSemantic m_semantic;

public:
    BufferAttributeImpl(BufferAttribute* parent, const UInt32& location, const UInt32& offset, BufferFormat format, AttributeSemantic semantic, const UInt32& semanticIndex) :
        base(parent), m_location(location), m_offset(offset), m_format(format), m_semantic(semantic), m_semanticIndex(semanticIndex) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

BufferAttribute::BufferAttribute() :
    m_impl(makePimpl<BufferAttributeImpl>(this, 0, 0, BufferFormat::None, AttributeSemantic::Unknown, 0))
{
}

BufferAttribute::BufferAttribute(const UInt32& location, const UInt32& offset, BufferFormat format, AttributeSemantic semantic, const UInt32& semanticIndex) :
    m_impl(makePimpl<BufferAttributeImpl>(this, location, offset, format, semantic, semanticIndex))
{
}

BufferAttribute::BufferAttribute(const BufferAttribute& _other) :
    m_impl(makePimpl<BufferAttributeImpl>(this, _other.location(), _other.offset(), _other.format(), _other.semantic(), _other.semanticIndex()))
{
}

BufferAttribute::BufferAttribute(BufferAttribute&& _other) noexcept :
    m_impl(std::move(_other.m_impl))
{
    m_impl->m_parent = this;
}

BufferAttribute::~BufferAttribute() noexcept = default;

const UInt32& BufferAttribute::location() const noexcept
{
    return m_impl->m_location;
}

BufferFormat BufferAttribute::format() const noexcept
{
    return m_impl->m_format;
}

const UInt32& BufferAttribute::offset() const noexcept
{
    return m_impl->m_offset;
}

AttributeSemantic BufferAttribute::semantic() const noexcept
{
    return m_impl->m_semantic;
}

const UInt32& BufferAttribute::semanticIndex() const noexcept
{
    return m_impl->m_semanticIndex;
}