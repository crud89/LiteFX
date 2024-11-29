#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Scissor::ScissorImpl {
public:
    friend class Scissor;

private:
    RectF m_rect;

public:
    ScissorImpl(RectF clientRect) : 
        m_rect(std::move(clientRect)) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Scissor::Scissor(const RectF& rect) noexcept :
    m_impl(rect)
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