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

const IRenderPipelineLayout* RenderPipeline::getLayout() const noexcept
{
    return m_impl->m_layout.get();
}

IRenderPipelineLayout* RenderPipeline::getLayout() noexcept
{
    return m_impl->m_layout.get();
}

const IRenderPass* RenderPipeline::getRenderPass() const noexcept
{
    return m_impl->m_renderPass.get();
}

IRenderPass* RenderPipeline::getRenderPass() noexcept
{
    return m_impl->m_renderPass.get();
}

void RenderPipeline::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The layout must be initialized.");

    m_impl->m_layout = std::move(layout);
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

UniquePtr<IBuffer> RenderPipeline::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding) const
{
    auto layout = m_impl->m_layout->getInputAssembler()->getVertexBufferLayout(binding);

    if (layout == nullptr)
        throw std::invalid_argument("No vertex buffer layout defined for the provided binding.");

    return m_impl->m_device->createBuffer(layout->getType(), usage, layout->getElementSize(), elements, binding);
}

UniquePtr<IBuffer> RenderPipeline::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
    auto layout = m_impl->m_layout->getInputAssembler()->getIndexBufferLayout();

    if (layout == nullptr)
        throw std::invalid_argument("No index buffer layout defined.");

    return m_impl->m_device->createBuffer(layout->getType(), usage, layout->getElementSize(), elements, layout->getBinding());
}

UniquePtr<IBufferPool> RenderPipeline::makeBufferPool(const BufferUsage& usage, const UInt32& setId) const
{
    //auto bufferSet = this->getLayout()->getInputAssembler()->getDescriptorSetLayout(setId);
    //
    //if (bufferSet == nullptr)
    //    throw std::runtime_error("The requested buffer set could not be found.");

    //return bufferSet->createBufferPool(usage);
    throw;
}

UniquePtr<ITexture> RenderPipeline::makeTexture(const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
    //if (levels < 1)
    //    throw std::invalid_argument("The number of mip-map levels must be at least 1.");

    //// TODO: Do we actually need to provide the binding here?
    //return m_impl->m_device->createTexture(format, size, 0, levels, samples);

    throw;
}