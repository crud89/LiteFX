#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Viewport::ViewportImpl : public Implement<Viewport> {
public:
    friend class Viewport;

private:
    RectF m_clientRect;
    float m_minDepth{ 0.f }, m_maxDepth{ 1.f };

public:
    ViewportImpl(Viewport* parent, const RectF& clientRect) : 
        base(parent), m_clientRect(clientRect) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

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

float Viewport::getMinDepth() const noexcept
{
    return m_impl->m_minDepth;
}

void Viewport::setMinDepth(const float& depth) const noexcept
{
    m_impl->m_minDepth = depth;
}

float Viewport::getMaxDepth() const noexcept
{
    return m_impl->m_maxDepth;
}

void Viewport::setMaxDepth(const float& depth) const noexcept
{
    m_impl->m_maxDepth = depth;
}