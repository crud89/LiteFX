#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class StateResource::StateResourceImpl {
public:
    friend class StateResource;

private:
    String m_name;

public:
    StateResourceImpl(StringView name) noexcept :
        m_name(name)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

StateResource::StateResource() noexcept :
    StateResource("Unnamed Resource"sv)
{
}

StateResource::StateResource(StringView name) noexcept :
    m_impl(name)
{
}

StateResource::StateResource(StateResource&&) noexcept = default;
StateResource& StateResource::operator=(StateResource&&) noexcept = default;
StateResource::~StateResource() noexcept = default;

String& StateResource::name() noexcept
{
    return m_impl->m_name;
}

const String& StateResource::name() const noexcept
{
    return m_impl->m_name;
}