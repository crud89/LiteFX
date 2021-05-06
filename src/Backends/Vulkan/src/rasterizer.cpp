#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(const VulkanRenderPipeline& pipeline) noexcept :
    Rasterizer(), VulkanRuntimeObject(pipeline.getDevice())
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withPolygonMode(const PolygonMode& mode) 
{
    this->instance()->setPolygonMode(mode);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withCullMode(const CullMode& cullMode) 
{
    this->instance()->setCullMode(cullMode);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withCullOrder(const CullOrder& cullOrder) 
{
    this->instance()->setCullOrder(cullOrder);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withLineWidth(const Float& lineWidth) 
{
    this->instance()->setLineWidth(lineWidth);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::enableDepthBias(const bool& enable) 
{
    this->instance()->setDepthBiasEnabled(enable);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasClamp(const Float& clamp) 
{
    this->instance()->setDepthBiasClamp(clamp);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasConstantFactor(const Float& factor) 
{
    this->instance()->setDepthBiasConstantFactor(factor);
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::withDepthBiasSlopeFactor(const Float& factor) 
{
    this->instance()->setDepthBiasSlopeFactor(factor);
    return *this;
}