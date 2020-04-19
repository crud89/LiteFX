#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class Viewport::ViewportImpl {
private:
    Array<RectF> m_scissors;
    RectF m_clientRect;

public:
    ViewportImpl(const RectF& clientRect) : 
        m_clientRect(clientRect) { }

public:
    const RectF& getClientRect() const noexcept
    {
        return m_clientRect;
    }

    void setClientRect(const RectF& rect) noexcept
    {
        m_clientRect = rect;
    }

    Array<RectF>& getScissors() noexcept
    {
        return m_scissors;
    }
};

Viewport::Viewport(const RectF& rect) noexcept :
    m_impl(makePimpl<ViewportImpl>(rect))
{
}

Viewport::~Viewport() noexcept = default;

RectF Viewport::getRectangle() const noexcept
{
    return m_impl->getClientRect();
}

void Viewport::setRectangle(const RectF& rectangle) noexcept
{
    m_impl->setClientRect(rectangle);
}

const Array<RectF>& Viewport::getScissors() const noexcept
{
    return m_impl->getScissors();
}

Array<RectF>& Viewport::getScissors() noexcept
{
    return m_impl->getScissors();
}