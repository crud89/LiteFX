#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl {
private:
    UniquePtr<IRenderPipelineLayout> m_layout;

public:
    VulkanRenderPipelineImpl(UniquePtr<IRenderPipelineLayout>&& layout) noexcept :
        m_layout(std::move(layout))
    {
    }

    VulkanRenderPipelineImpl() noexcept :
        m_layout(makeUnique<VulkanRenderPipelineLayout>())
    {
    }

public:
    const IRenderPipelineLayout* getLayout() const noexcept 
    {
        return m_layout.get();
    }

    void setLayout(UniquePtr<IRenderPipelineLayout>&& layout)
    {
        m_layout = std::move(layout);
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline() noexcept :
    m_impl(makePimpl<VulkanRenderPipelineImpl>())
{
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept = default;

const IRenderPipelineLayout* VulkanRenderPipeline::getLayout() const noexcept
{
    return m_impl->getLayout();
}

void VulkanRenderPipeline::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
    m_impl->setLayout(std::move(layout));
}