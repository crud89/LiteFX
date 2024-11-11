#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DepthStencilState::DepthStencilStateImpl : public Implement<DepthStencilState> {
public:
    friend class DepthStencilState;

private:
    DepthState m_depthState;
    DepthBias m_depthBias;
    StencilState m_stencilState;

public:
    DepthStencilStateImpl(DepthStencilState* parent, const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) :
        base(parent), m_depthState(depthState), m_depthBias(depthBias), m_stencilState(stencilState)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DepthStencilState::DepthStencilState(const DepthState& depthState, const DepthBias& depthBias, const StencilState& stencilState) noexcept :
    m_impl(makePimpl<DepthStencilStateImpl>(this, depthState, depthBias, stencilState))
{
}

DepthStencilState::DepthStencilState() noexcept :
    m_impl(makePimpl<DepthStencilStateImpl>(this, DepthState{}, DepthBias{}, StencilState{}))
{
}

DepthStencilState::DepthStencilState(const DepthStencilState& _other) noexcept :
    m_impl(makePimpl<DepthStencilStateImpl>(this, _other.depthState(), _other.depthBias(), _other.stencilState()))
{
}

DepthStencilState::DepthStencilState(DepthStencilState&& _other) noexcept :
    m_impl(makePimpl<DepthStencilStateImpl>(this, _other.depthState(), _other.depthBias(), _other.stencilState()))
{
}

DepthStencilState::~DepthStencilState() noexcept = default;

DepthStencilState& DepthStencilState::operator=(const DepthStencilState& _other) noexcept
{
    m_impl->m_depthState = _other.depthState();
    m_impl->m_depthBias = _other.depthBias();
    m_impl->m_stencilState = _other.stencilState();

    return *this;
}

DepthStencilState& DepthStencilState::operator=(DepthStencilState&& _other) noexcept
{
    m_impl->m_depthState = std::move(_other.m_impl->m_depthState);
    m_impl->m_depthBias = std::move(_other.m_impl->m_depthBias);
    m_impl->m_stencilState = std::move(_other.m_impl->m_stencilState);

    return *this;
}

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