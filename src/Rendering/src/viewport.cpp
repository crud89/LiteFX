#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class Viewport::ViewportImpl : public Implement<Viewport> {
public:
    friend class Viewport;

private:
    Array<RectF> m_scissors;
    RectF m_clientRect;

public:
    ViewportImpl(Viewport* parent, const RectF& clientRect) : base(parent), m_clientRect(clientRect) { }
};

Viewport::Viewport(const RectF& rect) :
    m_impl(makePimpl<ViewportImpl>(this, rect))
{
}

Viewport::~Viewport() noexcept = default;

RectF Viewport::getRectangle() const noexcept
{
    return m_impl->m_clientRect;
}

void Viewport::setRectangle(const RectF& rectangle) noexcept
{
    m_impl->m_clientRect = rectangle;
}

const Array<RectF>& Viewport::getScissors() const noexcept
{
    return m_impl->m_scissors;
}

Array<RectF>& Viewport::getScissors() noexcept
{
    return m_impl->m_scissors;
}