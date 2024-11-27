#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class BufferAttribute::BufferAttributeImpl {
public:
    friend class BufferAttribute;

private:
    UInt32 m_location{ 0 }, m_offset{ 0 }, m_semanticIndex{ 0 };
    BufferFormat m_format{ BufferFormat::None };
    AttributeSemantic m_semantic{ AttributeSemantic::Unknown };

public:
    BufferAttributeImpl() noexcept = default;

    BufferAttributeImpl(UInt32 location, UInt32 offset, BufferFormat format, AttributeSemantic semantic, UInt32 semanticIndex) noexcept :
        m_location(location), m_offset(offset), m_semanticIndex(semanticIndex), m_format(format), m_semantic(semantic)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

BufferAttribute::BufferAttribute() noexcept :
    m_impl()
{
}

BufferAttribute::BufferAttribute(UInt32 location, UInt32 offset, BufferFormat format, AttributeSemantic semantic, UInt32 semanticIndex) noexcept :
    m_impl(location, offset, format, semantic, semanticIndex)
{
}

BufferAttribute::BufferAttribute(const BufferAttribute& _other) noexcept = default;
BufferAttribute::BufferAttribute(BufferAttribute&& _other) noexcept = default;
BufferAttribute& BufferAttribute::operator=(const BufferAttribute& _other) noexcept = default;
BufferAttribute& BufferAttribute::operator=(BufferAttribute&& _other) noexcept = default;
BufferAttribute::~BufferAttribute() noexcept = default;

UInt32 BufferAttribute::location() const noexcept
{
    return m_impl->m_location;
}

BufferFormat BufferAttribute::format() const noexcept
{
    return m_impl->m_format;
}

UInt32 BufferAttribute::offset() const noexcept
{
    return m_impl->m_offset;
}

AttributeSemantic BufferAttribute::semantic() const noexcept
{
    return m_impl->m_semantic;
}

UInt32 BufferAttribute::semanticIndex() const noexcept
{
    return m_impl->m_semanticIndex;
}