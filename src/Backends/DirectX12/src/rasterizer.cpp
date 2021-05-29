#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Rasterizer::DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const bool& useDepthBias, const Float& depthBiasClamp, const Float& depthBiasConstantFactor, const Float& depthBiasSlopeFactor) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, useDepthBias, depthBiasClamp, depthBiasConstantFactor, depthBiasSlopeFactor), DirectX12RuntimeObject<DirectX12RenderPipeline>(pipeline, pipeline.getDevice())
{
}

DirectX12Rasterizer::DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline) noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise), DirectX12RuntimeObject<DirectX12RenderPipeline>(pipeline, pipeline.getDevice())
{
}

DirectX12Rasterizer::~DirectX12Rasterizer() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RasterizerBuilder::DirectX12RasterizerBuilderImpl : public Implement<DirectX12RasterizerBuilder> {
public:
    friend class DirectX12RasterizerBuilder;

private:
    PolygonMode m_polygonMode = PolygonMode::Solid;
    CullMode m_cullMode = CullMode::BackFaces;
    CullOrder m_cullOrder = CullOrder::CounterClockWise;
    Float m_lineWidth = 1.f;
    Float m_depthBiasClamp = 0.f, m_depthBiasConstantFactor = 0.f, m_depthBiasSlopeFactor = 0.f;
    bool m_depthBias = false;

public:
    DirectX12RasterizerBuilderImpl(DirectX12RasterizerBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RasterizerBuilder::DirectX12RasterizerBuilder(DirectX12RenderPipelineBuilder & parent) noexcept :
    m_impl(makePimpl<DirectX12RasterizerBuilderImpl>(this)), RasterizerBuilder(parent, SharedPtr<DirectX12Rasterizer>(new DirectX12Rasterizer(*std::as_const(parent).instance())))
{
}

DirectX12RasterizerBuilder::~DirectX12RasterizerBuilder() noexcept = default;

DirectX12RenderPipelineBuilder& DirectX12RasterizerBuilder::go()
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

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withPolygonMode(const PolygonMode & mode) noexcept
{
    m_impl->m_polygonMode = mode;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withCullMode(const CullMode & cullMode) noexcept
{
    m_impl->m_cullMode = cullMode;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withCullOrder(const CullOrder & cullOrder) noexcept
{
    m_impl->m_cullOrder = cullOrder;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withLineWidth(const Float & lineWidth) noexcept
{
    m_impl->m_lineWidth = lineWidth;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::enableDepthBias(const bool& enable) noexcept
{
    m_impl->m_depthBias = enable;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasClamp(const Float & clamp) noexcept
{
    m_impl->m_depthBiasClamp = clamp;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasConstantFactor(const Float & factor) noexcept
{
    m_impl->m_depthBiasConstantFactor = factor;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasSlopeFactor(const Float & factor) noexcept
{
    m_impl->m_depthBiasSlopeFactor = factor;
    return *this;
}