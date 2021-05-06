#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12Rasterizer::DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline) noexcept :
    Rasterizer(), DirectX12RuntimeObject(pipeline.getDevice())
{
}

DirectX12Rasterizer::~DirectX12Rasterizer() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withPolygonMode(const PolygonMode& mode) 
{
    this->instance()->setPolygonMode(mode);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withCullMode(const CullMode& cullMode) 
{
    this->instance()->setCullMode(cullMode);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withCullOrder(const CullOrder& cullOrder) 
{
    this->instance()->setCullOrder(cullOrder);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withLineWidth(const Float& lineWidth) 
{
    this->instance()->setLineWidth(lineWidth);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::enableDepthBias(const bool& enable) 
{
    this->instance()->setDepthBiasEnabled(enable);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasClamp(const Float& clamp) 
{
    this->instance()->setDepthBiasClamp(clamp);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasConstantFactor(const Float& factor) 
{
    this->instance()->setDepthBiasConstantFactor(factor);
    return *this;
}

DirectX12RasterizerBuilder& DirectX12RasterizerBuilder::withDepthBiasSlopeFactor(const Float& factor) 
{
    this->instance()->setDepthBiasSlopeFactor(factor);
    return *this;
}