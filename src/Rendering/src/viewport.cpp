#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Viewport::ViewportImpl {
public:
    friend class Viewport;

private:
    RectF m_clientRect{};
    Float m_minDepth{ 0.f }, m_maxDepth{ 1.f };

public:
    ViewportImpl(RectF clientRect, Float minDepth, Float maxDepth) :
        m_clientRect(std::move(clientRect)), m_minDepth(minDepth), m_maxDepth(maxDepth)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Viewport::Viewport(const RectF& rect, Float minDepth, Float maxDepth) noexcept :
    m_impl(rect, minDepth, maxDepth)
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

Float Viewport::getMinDepth() const noexcept
{
    return m_impl->m_minDepth;
}

void Viewport::setMinDepth(Float depth) const noexcept
{
    m_impl->m_minDepth = depth;
}

Float Viewport::getMaxDepth() const noexcept
{
    return m_impl->m_maxDepth;
}

void Viewport::setMaxDepth(Float depth) const noexcept
{
    m_impl->m_maxDepth = depth;
}