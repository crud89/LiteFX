#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

class BufferAttribute::BufferAttributeImpl : public Implement<BufferAttribute> {
public:
    friend class BufferAttribute;

private:
    UInt32 m_location, m_binding, m_offset;
    BufferFormat m_format;

public:
    BufferAttributeImpl(BufferAttribute* parent, const UInt32& location = 0, const UInt32& binding = 0, const UInt32& offset = 0, const BufferFormat& format = BufferFormat::None) :
        base(parent), m_location(location), m_binding(binding), m_offset(offset), m_format(format) { }
};

BufferAttribute::BufferAttribute() :
    m_impl(makePimpl<BufferAttributeImpl>(this))
{
}

BufferAttribute::BufferAttribute(const UInt32& location, const UInt32& binding, const UInt32& offset, const BufferFormat& format) :
    m_impl(makePimpl<BufferAttributeImpl>(this, location, binding, offset, format))
{
}

BufferAttribute::BufferAttribute(const BufferAttribute& _other) :
    m_impl(makePimpl<BufferAttributeImpl>(this, _other.getLocation(), _other.getBinding(), _other.getOffset(), _other.getFormat()))
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

const UInt32& BufferAttribute::getBinding() const noexcept
{
    return m_impl->m_binding;
}

const BufferFormat& BufferAttribute::getFormat() const noexcept
{
    return m_impl->m_format;
}

const UInt32& BufferAttribute::getOffset() const noexcept
{
    return m_impl->m_offset;
}