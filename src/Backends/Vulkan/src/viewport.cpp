#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanViewport::VulkanViewportImpl {
private:
    Array<RectF> m_scissors;
    RectF m_clientRect;

public:
    VulkanViewportImpl(const RectF& clientRect) : 
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

VulkanViewport::VulkanViewport(const RectF& rect) noexcept :
    m_impl(makePimpl<VulkanViewportImpl>(rect))
{
}

VulkanViewport::~VulkanViewport() noexcept = default;

RectF VulkanViewport::getRectangle() const noexcept
{
    return m_impl->getClientRect();
}

void VulkanViewport::setRectangle(const RectF& rectangle) noexcept
{
    m_impl->setClientRect(rectangle);
}

const Array<RectF>& VulkanViewport::getScissors() const noexcept
{
    return m_impl->getScissors();
}

Array<RectF>& VulkanViewport::getScissors() noexcept
{
    return m_impl->getScissors();
}