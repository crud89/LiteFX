#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

class BufferAttribute::BufferAttributeImpl {
private:
    UInt32 m_location, m_binding, m_offset;
    BufferFormat m_format;

public:
    BufferAttributeImpl(const UInt32& location = 0, const UInt32& binding = 0, const UInt32& offset = 0, const BufferFormat& format = BufferFormat::None) noexcept :
        m_location(location), m_binding(binding), m_offset(offset), m_format(format) { }

public:
    const UInt32& getLocation() const noexcept
    {
        return m_location;
    }

    const UInt32& getBinding() const noexcept
    {
        return m_binding;
    }

    const BufferFormat& getFormat() const noexcept
    {
        return m_format;
    }

    const UInt32& getOffset() const noexcept
    {
        return m_offset;
    }
};

BufferAttribute::BufferAttribute() noexcept :
    m_impl(makePimpl<BufferAttributeImpl>())
{
}

BufferAttribute::BufferAttribute(const UInt32& location, const UInt32& binding, const UInt32& offset, const BufferFormat& format) noexcept :
    m_impl(makePimpl<BufferAttributeImpl>(location, binding, offset, format))
{
}

BufferAttribute::BufferAttribute(const BufferAttribute& _other) noexcept :
    m_impl(makePimpl<BufferAttributeImpl>(_other.getLocation(), _other.getBinding(), _other.getOffset(), _other.getFormat()))
{
}

BufferAttribute::BufferAttribute(BufferAttribute&& _other) noexcept :
    m_impl(std::move(_other.m_impl))
{
}

BufferAttribute::~BufferAttribute() noexcept = default;

const UInt32& BufferAttribute::getLocation() const noexcept
{
    return m_impl->getLocation();
}

const UInt32& BufferAttribute::getBinding() const noexcept
{
    return m_impl->getBinding();
}

const BufferFormat& BufferAttribute::getFormat() const noexcept
{
    return m_impl->getFormat();
}

const UInt32& BufferAttribute::getOffset() const noexcept
{
    return m_impl->getOffset();
}