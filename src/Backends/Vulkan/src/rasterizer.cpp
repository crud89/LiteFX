#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(const VulkanRenderPipeline& pipeline, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth = 1.f, const bool& useDepthBias = false, const Float& depthBiasClamp = 1.f, const Float& depthBiasConstantFactor = 0.f, const Float& depthBiasSlopeFactor = 0.f) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, useDepthBias, depthBiasClamp, depthBiasConstantFactor, depthBiasSlopeFactor), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice())
{
}

VulkanRasterizer::VulkanRasterizer(const VulkanRenderPipeline& pipeline) noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice())
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRasterizerBuilder::VulkanRasterizerBuilderImpl : public Implement<VulkanRasterizerBuilder> {
public:
    friend class VulkanRasterizerBuilder;

private:
    PolygonMode m_polygonMode = PolygonMode::Solid;
    CullMode m_cullMode = CullMode::BackFaces;
    CullOrder m_cullOrder = CullOrder::CounterClockWise;
    Float m_lineWidth = 1.f;
    Float m_depthBiasClamp = 0.f, m_depthBiasConstantFactor = 0.f, m_depthBiasSlopeFactor = 0.f;
    bool m_depthBias = false;

public:
    VulkanRasterizerBuilderImpl(VulkanRasterizerBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder& VulkanRasterizerBuilder::go()
{
    this->instance()->polygonMode() = m_impl->m_polygonMode;
    this->instance()->cullMode() = m_impl->m_cullMode;
    this->instance()->cullOrder() = m_impl->m_cullOrder;
    this->instance()->lineWidth() = m_impl->m_lineWidth;
    this->instance()->useDepthBias() = m_impl->m_depthBias;
    this->instance()->depthBiasClamp() = m_impl->m_depthBiasClamp;
    this->instance()->depthBiasConstantFactor() = m_impl->m_depthBiasConstantFactor;
    this->instance()->depthBiasSlopeFactor() = m_impl->m_depthBiasSlopeFactor;

    return RasterizerBuilder::go();
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withPolygonMode(const PolygonMode& mode) noexcept
{
    m_impl->m_polygonMode = mode;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withCullMode(const CullMode& cullMode) noexcept
{
    m_impl->m_cullMode = cullMode;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withCullOrder(const CullOrder& cullOrder) noexcept
{
    m_impl->m_cullOrder = cullOrder;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withLineWidth(const Float& lineWidth) noexcept
{
    m_impl->m_lineWidth = lineWidth;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::enableDepthBias(const bool& enable) noexcept
{
    m_impl->m_depthBias = enable;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasClamp(const Float& clamp) noexcept
{
    m_impl->m_depthBiasClamp = clamp;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasConstantFactor(const Float& factor) noexcept
{
    m_impl->m_depthBiasConstantFactor = factor;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasSlopeFactor(const Float& factor) noexcept
{
    m_impl->m_depthBiasSlopeFactor = factor;
    return *this;
}