#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderTarget::RenderTargetImpl : public Implement<RenderTarget> {
public:
    friend class RenderTarget;

private:
    RenderTargetType m_type = RenderTargetType::Color;
    Format m_format = Format::B8G8R8A8_SRGB;
    RenderTargetFlags m_flags;
    Vector4f m_clearValues;
    UInt32 m_location;
    BlendState m_blendState;
    String m_name;

public:
    RenderTargetImpl(RenderTarget* parent, const String& name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
        base(parent), m_name(name), m_location(location), m_type(type), m_format(format), m_flags(flags), m_clearValues(clearValues), m_blendState(blendState)
    {
        if ((::hasDepth(format) || ::hasStencil(format)) && LITEFX_FLAG_IS_SET(flags, RenderTargetFlags::AllowStorage)) [[unlikely]]
            throw InvalidArgumentException("flags", "Depth/Stencil formats cannot be used for storage/unordered access.");

        if ((::hasDepth(format) || ::hasStencil(format)) && LITEFX_FLAG_IS_SET(flags, RenderTargetFlags::Shared)) [[unlikely]]
            throw InvalidArgumentException("flags", "Depth/Stencil formats cannot be used shared between queues.");
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget() noexcept :
    RenderTarget(0, RenderTargetType::Color, Format::None, RenderTargetFlags::None, Vector4f{0.f, 0.f, 0.f, 0.f}, BlendState{})
{
}

RenderTarget::RenderTarget(UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
    RenderTarget("", location, type, format, flags, clearValues, blendState)
{
}

RenderTarget::RenderTarget(const String& name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags, const Vector4f& clearValues, const BlendState& blendState) :
    m_impl(makePimpl<RenderTargetImpl>(this, name, location, type, format, flags, clearValues, blendState))
{
}

RenderTarget::RenderTarget(const RenderTarget& _other) noexcept :
    m_impl(makePimpl<RenderTargetImpl>(this, _other.name(), _other.location(), _other.type(), _other.format(), _other.flags(), _other.clearValues(), _other.blendState()))
{
}

RenderTarget::RenderTarget(RenderTarget&& _other) noexcept :
    m_impl(makePimpl<RenderTargetImpl>(this, std::move(_other.m_impl->m_name), std::move(_other.m_impl->m_location), std::move(_other.m_impl->m_type), std::move(_other.m_impl->m_format), std::move(_other.m_impl->m_flags), std::move(_other.m_impl->m_clearValues), std::move(_other.m_impl->m_blendState)))
{
}

RenderTarget::~RenderTarget() noexcept = default;

RenderTarget& RenderTarget::operator=(const RenderTarget& _other) noexcept
{
    m_impl->m_name = _other.m_impl->m_name;
    m_impl->m_location = _other.m_impl->m_location;
    m_impl->m_type = _other.m_impl->m_type;
    m_impl->m_format = _other.m_impl->m_format;
    m_impl->m_flags = _other.m_impl->m_flags;
    m_impl->m_clearValues = _other.m_impl->m_clearValues;
    m_impl->m_blendState = _other.m_impl->m_blendState;

    return *this;
}

RenderTarget& RenderTarget::operator=(RenderTarget&& _other) noexcept
{
    m_impl->m_name = std::move(_other.m_impl->m_name);
    m_impl->m_location = std::move(_other.m_impl->m_location);
    m_impl->m_type = std::move(_other.m_impl->m_type);
    m_impl->m_format = std::move(_other.m_impl->m_format);
    m_impl->m_flags = std::move(_other.m_impl->m_flags);
    m_impl->m_clearValues = std::move(_other.m_impl->m_clearValues);
    m_impl->m_blendState = std::move(_other.m_impl->m_blendState);
    
    return *this;
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

RenderTargetFlags RenderTarget::flags() const noexcept
{
    return m_impl->m_flags;
}

Format RenderTarget::format() const noexcept
{
    return m_impl->m_format;
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

bool RenderTarget::allowStorage() const noexcept
{
    return LITEFX_FLAG_IS_SET(m_impl->m_flags, RenderTargetFlags::AllowStorage);
}

bool RenderTarget::multiQueueAccess() const noexcept
{
    return LITEFX_FLAG_IS_SET(m_impl->m_flags, RenderTargetFlags::Shared);
}

const IRenderTarget::BlendState& RenderTarget::blendState() const noexcept
{
    return m_impl->m_blendState;
}