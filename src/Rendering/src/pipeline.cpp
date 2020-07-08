#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPipeline::RenderPipelineImpl : public Implement<RenderPipeline> {
public:
    friend class RenderPipeline;

private:
    UniquePtr<IRenderPass> m_renderPass;
    UniquePtr<IRenderPipelineLayout> m_layout;
    UniquePtr<IShaderProgram> m_program;
    const IGraphicsDevice* m_device;

public:
    RenderPipelineImpl(RenderPipeline* parent, UniquePtr<IRenderPipelineLayout>&& layout, const IGraphicsDevice* device) :
        base(parent), m_layout(std::move(layout)), m_device(device) { }

    RenderPipelineImpl(RenderPipeline* parent, const IGraphicsDevice* device) :
        base(parent), m_layout(makeUnique<RenderPipelineLayout>()), m_device(device) { }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderPipeline::RenderPipeline(const IGraphicsDevice* device) :
    m_impl(makePimpl<RenderPipelineImpl>(this, device))
{
    if (device == nullptr)
        throw std::invalid_argument("The graphics device must be initialized.");
}

RenderPipeline::RenderPipeline(const IGraphicsDevice* device, UniquePtr<IRenderPipelineLayout>&& layout) :
    m_impl(makePimpl<RenderPipelineImpl>(this, std::move(layout), device))
{
    if (device == nullptr)
        throw std::invalid_argument("The graphics device must be initialized.");
}

RenderPipeline::~RenderPipeline() noexcept = default;

const IGraphicsDevice* RenderPipeline::getDevice() const noexcept
{
    return m_impl->m_device;
}

const IRenderPipelineLayout* RenderPipeline::getLayout() const noexcept
{
    return m_impl->m_layout.get();
}

const IShaderProgram* RenderPipeline::getProgram() const noexcept
{
    return m_impl->m_program.get();
}

const IRenderPass* RenderPipeline::getRenderPass() const noexcept
{
    return m_impl->m_renderPass.get();
}

void RenderPipeline::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The layout must be initialized.");

    m_impl->m_layout = std::move(layout);
}

void RenderPipeline::use(UniquePtr<IShaderProgram>&& program)
{
    if (program == nullptr)
        throw std::invalid_argument("The program must be initialized.");

    m_impl->m_program = std::move(program);
}

void RenderPipeline::use(UniquePtr<IRenderPass>&& renderPass)
{
    if (renderPass == nullptr)
        throw std::invalid_argument("The render pass must be initialized.");

    m_impl->m_renderPass = std::move(renderPass);
}

void RenderPipeline::beginFrame() const
{
    m_impl->m_renderPass->begin();

    // TODO: In case there are multiple render passes the caller should be able to advance instead of handling only beginning/ending the entire frame.
}

void RenderPipeline::endFrame()
{
    m_impl->m_renderPass->end(true);
}

UniquePtr<IBuffer> RenderPipeline::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements) const
{
    return m_impl->m_device->createBuffer(BufferType::Vertex, usage, m_impl->m_layout->getInputAssembler()->getLayout(), elements);
}

UniquePtr<IBuffer> RenderPipeline::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
    switch (indexType)
    {
    case IndexType::UInt16:
        return m_impl->m_device->createBuffer(BufferType::Index, usage, 2, elements);       // 16 bit = 2 bytes per index.
    case IndexType::UInt32:
        return m_impl->m_device->createBuffer(BufferType::Index, usage, 4, elements);       // 32 bit = 4 bytes per index.
    default:
        throw std::invalid_argument("Unsupported index type.");
    }
}