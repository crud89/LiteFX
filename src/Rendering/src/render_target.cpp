#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderTarget::RenderTargetImpl final {
public:
    friend class RenderTarget;

private:
    RenderTargetType m_type;
    RenderTargetFlags m_flags;
    Vector4f m_clearValues;
    UInt32 m_location;
    BlendState m_blendState;
    String m_name;
    UInt64 m_identifier;
    Format m_format;

public:
    RenderTargetImpl(StringView name, UInt64 uid, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
        m_type(type), m_flags(flags), m_clearValues(clearValues), m_location(location), m_blendState(blendState), m_name(name), m_identifier(uid), m_format(format)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget(UInt64 uid, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
    m_impl("", uid, location, type, format, flags, clearValues, blendState)
{
}

RenderTarget::RenderTarget(StringView name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
    m_impl(name, hash(name), location, type, format, flags, clearValues, blendState)
{
}

RenderTarget::RenderTarget(const RenderTarget& _other) noexcept = default;
RenderTarget::RenderTarget(RenderTarget&& _other) noexcept = default;
RenderTarget& RenderTarget::operator=(const RenderTarget& _other) noexcept = default;
RenderTarget& RenderTarget::operator=(RenderTarget&& _other) noexcept = default;
RenderTarget::~RenderTarget() noexcept = default;

UInt64 RenderTarget::identifier() const noexcept 
{
    return m_impl->m_identifier;
}

const String& RenderTarget::name() const noexcept
{
    return m_impl->m_name;
}

UInt32 RenderTarget::location() const noexcept
{
    return m_impl->m_location;
}

RenderTargetType RenderTarget::type() const noexcept
{
    return m_impl->m_type;
}

Format RenderTarget::format() const noexcept
{
    return m_impl->m_format;
}

RenderTargetFlags RenderTarget::flags() const noexcept
{
    return m_impl->m_flags;
}

bool RenderTarget::clearBuffer() const noexcept
{
    return LITEFX_FLAG_IS_SET(m_impl->m_flags, RenderTargetFlags::Clear);
}

bool RenderTarget::clearStencil() const noexcept
{
    return LITEFX_FLAG_IS_SET(m_impl->m_flags, RenderTargetFlags::ClearStencil);
}

const Vector4f& RenderTarget::clearValues() const noexcept
{
    return m_impl->m_clearValues;
}

bool RenderTarget::isVolatile() const noexcept
{
    return LITEFX_FLAG_IS_SET(m_impl->m_flags, RenderTargetFlags::Volatile);
}

const IRenderTarget::BlendState& RenderTarget::blendState() const noexcept
{
    return m_impl->m_blendState;
}