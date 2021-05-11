#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class InputAttachmentMapping::InputAttachmentMappingImpl : public Implement<InputAttachmentMapping> {
public:
    friend class InputAttachmentMapping;

private:
    const RenderTarget& m_renderTarget;
    UInt32 m_location;

public:
    InputAttachmentMappingImpl(InputAttachmentMapping* parent, const UInt32& location, const RenderTarget& renderTarget) :
        base(parent), m_location(location), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

InputAttachmentMapping::InputAttachmentMapping(const RenderTarget& renderTarget, const UInt32& location) :
    m_impl(makePimpl<InputAttachmentMappingImpl>(this, location, renderTarget))
{
}

InputAttachmentMapping::InputAttachmentMapping(const InputAttachmentMapping& _other) noexcept :
    m_impl(makePimpl<InputAttachmentMappingImpl>(this, _other.location(), _other.renderTarget()))
{
}

InputAttachmentMapping::InputAttachmentMapping(InputAttachmentMapping&& _other) noexcept :
    m_impl(makePimpl<InputAttachmentMappingImpl>(this, std::move(_other.m_impl->m_location), std::move(_other.m_impl->m_renderTarget)))
{
}

InputAttachmentMapping::~InputAttachmentMapping() noexcept = default;

const UInt32& InputAttachmentMapping::location() const noexcept
{
    return m_impl->m_location;
}

const RenderTarget& InputAttachmentMapping::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}