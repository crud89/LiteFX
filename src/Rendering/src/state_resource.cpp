#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class StateResource::StateResourceImpl : public Implement<StateResource> {
public:
    friend class StateResource;

private:
    String m_name;

public:
    StateResourceImpl(StateResource* parent, StringView name) :
        base(parent), m_name(name)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

StateResource::StateResource() noexcept :
    StateResource("Unnamed Resource")
{
}

StateResource::StateResource(StringView name) :
    m_impl(makePimpl<StateResourceImpl>(this, name))
{
}

StateResource::~StateResource() noexcept = default;

String& StateResource::name() noexcept
{
    return m_impl->m_name;
}

const String& StateResource::name() const noexcept
{
    return m_impl->m_name;
}