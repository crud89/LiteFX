#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(const VulkanRenderPipeline& pipeline, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice())
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
    DepthStencilState::DepthBias m_depthBias;
    DepthStencilState::DepthState m_depthState;
    DepthStencilState::StencilState m_stencilState;

public:
    VulkanRasterizerBuilderImpl(VulkanRasterizerBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizerBuilder::VulkanRasterizerBuilder(VulkanRenderPipelineBuilder& parent) noexcept :
    m_impl(makePimpl<VulkanRasterizerBuilderImpl>(this)), RasterizerBuilder(parent, SharedPtr<VulkanRasterizer>(new VulkanRasterizer(*std::as_const(parent).instance())))
{
}

VulkanRasterizerBuilder::~VulkanRasterizerBuilder() noexcept = default;

VulkanRenderPipelineBuilder& VulkanRasterizerBuilder::go()
{
    this->instance()->polygonMode() = m_impl->m_polygonMode;
    this->instance()->cullMode() = m_impl->m_cullMode;
    this->instance()->cullOrder() = m_impl->m_cullOrder;
    this->instance()->lineWidth() = m_impl->m_lineWidth;
    this->instance()->depthStencilState().depthBias() = m_impl->m_depthBias;
    this->instance()->depthStencilState().depthState() = m_impl->m_depthState;
    this->instance()->depthStencilState().stencilState() = m_impl->m_stencilState;

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

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBias(const DepthStencilState::DepthBias& depthBias) noexcept
{
    m_impl->m_depthBias = depthBias;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthState(const DepthStencilState::DepthState& depthState) noexcept
{
    m_impl->m_depthState = depthState;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withStencilState(const DepthStencilState::StencilState& stencilState) noexcept
{
    m_impl->m_stencilState = stencilState;
    return *this;
}