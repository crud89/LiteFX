#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DepthStencilState::DepthStencilStateImpl {
public:
    friend class DepthStencilState;

private:
    DepthState m_depthState{};
    DepthBias m_depthBias{};
    StencilState m_stencilState{};

public:
    DepthStencilStateImpl() noexcept = default;

    DepthStencilStateImpl(const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) noexcept :
        m_depthState(depthState), m_depthBias(depthBias), m_stencilState(stencilState)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DepthStencilState::DepthStencilState() noexcept = default;

DepthStencilState::DepthStencilState(const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) noexcept :
    m_impl(depthState, depthBias, stencilState)
{
}

DepthStencilState::DepthStencilState(const DepthStencilState& _other) noexcept = default;
DepthStencilState::DepthStencilState(DepthStencilState&& _other) noexcept = default;
DepthStencilState& DepthStencilState::operator=(const DepthStencilState& _other) noexcept = default;
DepthStencilState& DepthStencilState::operator=(DepthStencilState&& _other) noexcept = default;
DepthStencilState::~DepthStencilState() noexcept = default;

DepthStencilState::DepthState& DepthStencilState::depthState() const noexcept
{
    return m_impl->m_depthState;
}

DepthStencilState::DepthBias& DepthStencilState::depthBias() const noexcept
{
    return m_impl->m_depthBias;
}

DepthStencilState::StencilState& DepthStencilState::stencilState() const noexcept
{
    return m_impl->m_stencilState;
}