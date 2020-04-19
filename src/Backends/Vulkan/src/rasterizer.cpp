#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanRasterizer::VulkanRasterizerImpl {
private:
    PolygonMode m_polygonMode = PolygonMode::Solid;
    CullMode m_cullMode = CullMode::BackFaces;
    CullOrder m_cullOrder = CullOrder::CounterClockWise;
    Float m_lineWidth = 1.f;
    Float m_depthBiasClamp = 0.f, m_depthBiasConstantFactor = 0.f, m_depthBiasSlopeFactor = 0.f;
    bool m_depthBias = false;

public:
    VulkanRasterizerImpl() noexcept = default;
    
public:
    PolygonMode getPolygonMode() const noexcept
    {
        return m_polygonMode;
    }

    void setPolygonMode(const PolygonMode& mode) noexcept
    {
        m_polygonMode = mode;
    }

    CullMode getCullMode() const noexcept
    {
        return m_cullMode;
    }

    void setCullMode(const CullMode& mode) noexcept
    {
        m_cullMode = mode;
    }

    CullOrder getCullOrder() const noexcept
    {
        return m_cullOrder;
    }

    void setCullOrder(const CullOrder& order) noexcept
    {
        m_cullOrder = order;
    }

    Float getLineWidth() const noexcept
    {
        return m_lineWidth;
    }

    void setLineWidth(const Float& width) noexcept
    {
        m_lineWidth = width;
    }

    bool getDepthBiasEnabled() const noexcept
    {
        return m_depthBias;
    }

    void setDepthBiasEnabled(const bool& enable) noexcept
    {
        m_depthBias = enable;
    }

    float getDepthBiasClamp() const noexcept
    {
        return m_depthBiasClamp;
    }

    void setDepthBiasClamp(const Float& clamp) noexcept
    {
        m_depthBiasClamp = clamp;
    }

    float getDepthBiasConstantFactor() const noexcept
    {
        return m_depthBiasConstantFactor;
    }

    void setDepthBiasConstantFactor(const Float& factor) noexcept
    {
        m_depthBiasConstantFactor = factor;
    }

    float getDepthBiasSlopeFactor() const noexcept
    {
        return m_depthBiasSlopeFactor;
    }

    void setDepthBiasSlopeFactor(const Float& factor) noexcept
    {
        m_depthBiasSlopeFactor = factor;
    }
};


VulkanRasterizer::VulkanRasterizer() noexcept :
    m_impl(makePimpl<VulkanRasterizerImpl>())
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

PolygonMode VulkanRasterizer::getPolygonMode() const noexcept 
{
    return m_impl->getPolygonMode();
}

void VulkanRasterizer::setPolygonMode(const PolygonMode& mode) noexcept
{
    m_impl->setPolygonMode(mode);
}

CullMode VulkanRasterizer::getCullMode() const noexcept 
{
    return m_impl->getCullMode();
}

void VulkanRasterizer::setCullMode(const CullMode& mode) noexcept 
{
    m_impl->setCullMode(mode);
}

CullOrder VulkanRasterizer::getCullOrder() const noexcept 
{
    return m_impl->getCullOrder();
}

void VulkanRasterizer::setCullOrder(const CullOrder& order) noexcept 
{
    m_impl->setCullOrder(order);
}

Float VulkanRasterizer::getLineWidth() const noexcept 
{
    return m_impl->getLineWidth();
}

void VulkanRasterizer::setLineWidth(const Float& width) noexcept 
{
    m_impl->setLineWidth(width);
}

bool VulkanRasterizer::getDepthBiasEnabled() const noexcept 
{
    return m_impl->getDepthBiasEnabled();
}

void VulkanRasterizer::setDepthBiasEnabled(const bool& enable) noexcept
{
    m_impl->setDepthBiasEnabled(enable);
}

float VulkanRasterizer::getDepthBiasClamp() const noexcept 
{
    return m_impl->getDepthBiasClamp();
}

void VulkanRasterizer::setDepthBiasClamp(const Float& clamp) noexcept
{
    m_impl->setDepthBiasClamp(clamp);
}

float VulkanRasterizer::getDepthBiasConstantFactor() const noexcept 
{
    return m_impl->getDepthBiasConstantFactor();
}

void VulkanRasterizer::setDepthBiasConstantFactor(const Float& factor) noexcept
{
    m_impl->setDepthBiasConstantFactor(factor);
}

float VulkanRasterizer::getDepthBiasSlopeFactor() const noexcept
{
    return m_impl->getDepthBiasSlopeFactor();
}

void VulkanRasterizer::setDepthBiasSlopeFactor(const Float& factor) noexcept
{
    m_impl->setDepthBiasSlopeFactor(factor);
}