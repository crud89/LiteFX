#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Scissor::ScissorImpl : public Implement<Scissor> {
public:
    friend class Scissor;

private:
    RectF m_rect;

public:
    ScissorImpl(Scissor* parent, const RectF& clientRect) : 
        base(parent), m_rect(clientRect) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Scissor::Scissor(const RectF& rect) noexcept:
    m_impl(makePimpl<ScissorImpl>(this, rect))
{
}

Scissor::~Scissor() noexcept = default;

RectF Scissor::getRectangle() const noexcept
{
    return m_impl->m_rect;
}

void Scissor::setRectangle(const RectF& rectangle) noexcept
{
    m_impl->m_rect = rectangle;
}