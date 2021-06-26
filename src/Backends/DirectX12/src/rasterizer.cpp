#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Rasterizer::DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState), DirectX12RuntimeObject<DirectX12RenderPipeline>(pipeline, pipeline.getDevice())
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
    DepthStencilState::DepthBias m_depthBias;
    DepthStencilState::DepthState m_depthState;
    DepthStencilState::StencilState m_stencilState;

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
    this->instance()->depthStencilState().depthBias() = m_impl->m_depthBias;
    this->instance()->depthStencilState().depthState() = m_impl->m_depthState;
    this->instance()->depthStencilState().stencilState() = m_impl->m_stencilState;

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

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBias(const DepthStencilState::DepthBias & depthBias) noexcept
{
    m_impl->m_depthBias = depthBias;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthState(const DepthStencilState::DepthState & depthState) noexcept
{
    m_impl->m_depthState = depthState;
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withStencilState(const DepthStencilState::StencilState & stencilState) noexcept
{
    m_impl->m_stencilState = stencilState;
    return *this;
}