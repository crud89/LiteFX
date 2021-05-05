#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

class BufferAttribute::BufferAttributeImpl : public Implement<BufferAttribute> {
public:
    friend class BufferAttribute;

private:
    UInt32 m_location, m_offset, m_semanticIndex;
    BufferFormat m_format;
    AttributeSemantic m_semantic;

public:
    BufferAttributeImpl(BufferAttribute* parent, const UInt32& location, const UInt32& offset, const BufferFormat& format, const AttributeSemantic& semantic, const UInt32& semanticIndex) :
        base(parent), m_location(location), m_offset(offset), m_format(format), m_semantic(semantic), m_semanticIndex(semanticIndex) { }
};

BufferAttribute::BufferAttribute() :
    m_impl(makePimpl<BufferAttributeImpl>(this, 0, 0, BufferFormat::None, AttributeSemantic::Unknown, 0))
{
}

BufferAttribute::BufferAttribute(const UInt32& location, const UInt32& offset, const BufferFormat& format, const AttributeSemantic& semantic, const UInt32& semanticIndex) :
    m_impl(makePimpl<BufferAttributeImpl>(this, location, offset, format, semantic, semanticIndex))
{
}

BufferAttribute::BufferAttribute(const BufferAttribute& _other) :
    m_impl(makePimpl<BufferAttributeImpl>(this, _other.getLocation(), _other.getOffset(), _other.getFormat(), _other.getSemantic(), _other.getSemanticIndex()))
{
}

BufferAttribute::BufferAttribute(BufferAttribute&& _other) noexcept :
    m_impl(std::move(_other.m_impl))
{
    m_impl->m_parent = this;
}

BufferAttribute::~BufferAttribute() noexcept = default;

const UInt32& BufferAttribute::getLocation() const noexcept
{
    return m_impl->m_location;
}

const BufferFormat& BufferAttribute::getFormat() const noexcept
{
    return m_impl->m_format;
}

const UInt32& BufferAttribute::getOffset() const noexcept
{
    return m_impl->m_offset;
}

const AttributeSemantic& BufferAttribute::getSemantic() const noexcept
{
    return m_impl->m_semantic;
}

const UInt32& BufferAttribute::getSemanticIndex() const noexcept
{
    return m_impl->m_semanticIndex;
}