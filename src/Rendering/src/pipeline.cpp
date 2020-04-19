#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPipeline::RenderPipelineImpl {
private:
    UniquePtr<IRenderPipelineLayout> m_layout;

public:
    RenderPipelineImpl(UniquePtr<IRenderPipelineLayout>&& layout) noexcept :
        m_layout(std::move(layout))
    {
    }

    RenderPipelineImpl() noexcept :
        m_layout(makeUnique<RenderPipelineLayout>())
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

RenderPipeline::RenderPipeline() noexcept :
    m_impl(makePimpl<RenderPipelineImpl>())
{
}

RenderPipeline::~RenderPipeline() noexcept = default;

const IRenderPipelineLayout* RenderPipeline::getLayout() const noexcept
{
    return m_impl->getLayout();
}

void RenderPipeline::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
    m_impl->setLayout(std::move(layout));
}