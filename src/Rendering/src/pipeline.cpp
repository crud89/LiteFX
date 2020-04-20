#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPipeline::RenderPipelineImpl {
private:
    Array<UniquePtr<IRenderPass>> m_renderPasses;
    UniquePtr<IRenderPipelineLayout> m_layout;
    UniquePtr<IShaderProgram> m_program;
    const IGraphicsDevice* m_device;

public:
    RenderPipelineImpl(UniquePtr<IRenderPipelineLayout>&& layout, const IGraphicsDevice* device) noexcept :
        m_layout(std::move(layout)), m_device(device) { }

    RenderPipelineImpl(const IGraphicsDevice* device) noexcept :
        m_layout(makeUnique<RenderPipelineLayout>()), m_device(device) { }

public:
    const IGraphicsDevice* getDevice() const noexcept
    {
        return m_device;
    }

    const IRenderPipelineLayout* getLayout() const noexcept
    {
        return m_layout.get();
    }

    void setLayout(UniquePtr<IRenderPipelineLayout>&& layout)
    {
        m_layout = std::move(layout);
    }

    const IShaderProgram* getProgram() const noexcept
    {
        return m_program.get();
    }

    void setProgram(UniquePtr<IShaderProgram>&& program)
    {
        m_program = std::move(program);
    }

    Array<const IRenderPass*> getRenderPasses() const noexcept
    {
        Array<const IRenderPass*> renderPasses(m_renderPasses.size());
        std::generate(std::begin(renderPasses), std::end(renderPasses), [&, i = 0]() mutable { return m_renderPasses[i++].get() });

        return renderPasses;
    }

    void addRenderPass(UniquePtr<IRenderPass>&& renderPass)
    {
        m_renderPasses.push_back(std::move(renderPass));
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderPipeline::RenderPipeline(const IGraphicsDevice* device) :
    m_impl(makePimpl<RenderPipelineImpl>(device))
{
    if (device == nullptr)
        throw std::invalid_argument("The graphics device must be initialized.");
}

RenderPipeline::RenderPipeline(const IGraphicsDevice* device, UniquePtr<IRenderPipelineLayout>&& layout) :
    m_impl(makePimpl<RenderPipelineImpl>(std::move(layout), device))
{
    if (device == nullptr)
        throw std::invalid_argument("The graphics device must be initialized.");
}

RenderPipeline::~RenderPipeline() noexcept = default;

const IGraphicsDevice* RenderPipeline::getDevice() const noexcept
{
    return m_impl->getDevice();
}

const IRenderPipelineLayout* RenderPipeline::getLayout() const noexcept
{
    return m_impl->getLayout();
}

const IShaderProgram* RenderPipeline::getProgram() const noexcept
{
    return m_impl->getProgram();
}

Array<const IRenderPass*> RenderPipeline::getRenderPasses() const noexcept
{
    return m_impl->getRenderPasses();
}

void RenderPipeline::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The layout must be initialized.");

    m_impl->setLayout(std::move(layout));
}

void RenderPipeline::use(UniquePtr<IShaderProgram>&& program)
{
    if (program == nullptr)
        throw std::invalid_argument("The program must be initialized.");

    m_impl->setProgram(std::move(program));
}

void RenderPipeline::use(UniquePtr<IRenderPass>&& renderPass)
{
    if (renderPass == nullptr)
        throw std::invalid_argument("The render pass must be initialized.");

    m_impl->addRenderPass(std::move(renderPass));
}