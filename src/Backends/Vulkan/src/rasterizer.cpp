#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRasterizer::VulkanRasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth, const DepthStencilState& depthStencilState) noexcept :
    Rasterizer(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState)
{
}

VulkanRasterizer::VulkanRasterizer() noexcept :
    Rasterizer(PolygonMode::Solid, CullMode::BackFaces, CullOrder::CounterClockWise)
{
}

VulkanRasterizer::~VulkanRasterizer() noexcept = default;

void VulkanRasterizer::updateLineWidth(const Float& lineWidth) noexcept
{
    this->lineWidth() = lineWidth;
}

#if defined(BUILD_DEFINE_BUILDERS)
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

VulkanRasterizerBuilder::VulkanRasterizerBuilder() noexcept :
    m_impl(makePimpl<VulkanRasterizerBuilderImpl>(this)), RasterizerBuilder(SharedPtr<VulkanRasterizer>(new VulkanRasterizer()))
{
}

VulkanRasterizerBuilder::~VulkanRasterizerBuilder() noexcept = default;

void VulkanRasterizerBuilder::build()
{
    this->instance()->polygonMode() = m_impl->m_polygonMode;
    this->instance()->cullMode() = m_impl->m_cullMode;
    this->instance()->cullOrder() = m_impl->m_cullOrder;
    this->instance()->lineWidth() = m_impl->m_lineWidth;
    this->instance()->depthStencilState().depthBias() = m_impl->m_depthBias;
    this->instance()->depthStencilState().depthState() = m_impl->m_depthState;
    this->instance()->depthStencilState().stencilState() = m_impl->m_stencilState;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::polygonMode(const PolygonMode& mode) noexcept
{
    m_impl->m_polygonMode = mode;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::cullMode(const CullMode& cullMode) noexcept
{
    m_impl->m_cullMode = cullMode;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::cullOrder(const CullOrder& cullOrder) noexcept
{
    m_impl->m_cullOrder = cullOrder;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::lineWidth(const Float& lineWidth) noexcept
{
    m_impl->m_lineWidth = lineWidth;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::depthBias(const DepthStencilState::DepthBias& depthBias) noexcept
{
    m_impl->m_depthBias = depthBias;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::depthState(const DepthStencilState::DepthState& depthState) noexcept
{
    m_impl->m_depthState = depthState;
    return *this;
}

VulkanRasterizerBuilder& VulkanRasterizerBuilder::stencilState(const DepthStencilState::StencilState& stencilState) noexcept
{
    m_impl->m_stencilState = stencilState;
    return *this;
}
#endif // defined(BUILD_DEFINE_BUILDERS)