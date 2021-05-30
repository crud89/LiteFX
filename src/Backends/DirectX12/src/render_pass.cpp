#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPass::DirectX12RenderPassImpl : public Implement<DirectX12RenderPass> {
public:
    friend class DirectX12RenderPassBuilder;
    friend class DirectX12RenderPass;

private:
    Array<UniquePtr<DirectX12RenderPipeline>> m_pipelines;
    Array<RenderTarget> m_renderTargets;
    Array<DirectX12InputAttachmentMapping> m_inputAttachments;
    Array<UniquePtr<DirectX12FrameBuffer>> m_frameBuffers;
    const DirectX12FrameBuffer* m_activeFrameBuffer = nullptr;
    UInt32 m_backBuffer{ 0 };

public:
    DirectX12RenderPassImpl(DirectX12RenderPass* parent, Span<RenderTarget> renderTargets, Span<DirectX12InputAttachmentMapping> inputAttachments) :
        base(parent)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);
    }

    DirectX12RenderPassImpl(DirectX12RenderPass* parent) :
        base(parent)
    {
    }

public:
    void mapRenderTargets(Span<RenderTarget> renderTargets)
    {
        m_renderTargets.assign(std::begin(renderTargets), std::end(renderTargets));
        //std::ranges::sort(m_renderTargets, [this](const RenderTarget& a, const RenderTarget& b) { return a.location() < b.location(); });
        std::sort(std::begin(m_renderTargets), std::end(m_renderTargets), [](const RenderTarget& a, const RenderTarget& b) { return a.location() < b.location(); });
    }

    void mapInputAttachments(Span<DirectX12InputAttachmentMapping> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
        //std::ranges::sort(m_inputAttachments, [this](const DirectX12InputAttachmentMapping& a, const DirectX12InputAttachmentMapping& b) { return a.location() < b.location(); });
        std::sort(std::begin(m_inputAttachments), std::end(m_inputAttachments), [](const DirectX12InputAttachmentMapping& a, const DirectX12InputAttachmentMapping& b) { return a.location() < b.location(); });
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<DirectX12InputAttachmentMapping> inputAttachments) :
    m_impl(makePimpl<DirectX12RenderPassImpl>(this, renderTargets, inputAttachments)), DirectX12RuntimeObject<DirectX12Device>(device, &device)
{
    // Initialize the frame buffers.
    m_impl->m_frameBuffers.resize(this->getDevice()->swapChain().buffers());
    std::ranges::generate(m_impl->m_frameBuffers, [this, i = 0]() mutable { return makeUnique<DirectX12FrameBuffer>(*this, i++, this->parent().swapChain().renderArea()); });
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device) noexcept :
    m_impl(makePimpl<DirectX12RenderPassImpl>(this)), DirectX12RuntimeObject<DirectX12Device>(device, &device)
{
}

DirectX12RenderPass::~DirectX12RenderPass() noexcept = default;

const DirectX12FrameBuffer& DirectX12RenderPass::frameBuffer(const UInt32& buffer) const
{
    if (buffer >= m_impl->m_frameBuffers.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("The buffer {0} does not exist in this render pass. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

    return *m_impl->m_frameBuffers[buffer].get();
}

const DirectX12FrameBuffer& DirectX12RenderPass::activeFrameBuffer() const
{
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("No frame buffer is active, since the render pass has not begun.");

    return *m_impl->m_activeFrameBuffer;
}

Array<const DirectX12FrameBuffer*> DirectX12RenderPass::frameBuffers() const noexcept
{
    return m_impl->m_frameBuffers |
        std::views::transform([](const UniquePtr<DirectX12FrameBuffer>& frameBuffer) { return frameBuffer.get(); }) |
        ranges::to<Array<const DirectX12FrameBuffer*>>();
}

const DirectX12RenderPipeline& DirectX12RenderPass::pipeline(const UInt32& id) const
{
    if (auto match = std::ranges::find_if(m_impl->m_pipelines, [&id](const UniquePtr<DirectX12RenderPipeline>& pipeline) { return pipeline->id() == id; }); match != m_impl->m_pipelines.end())
        return *match->get();

    throw InvalidArgumentException("No render pipeline with the ID {0} is contained by this render pass.", id);
}

Array<const DirectX12RenderPipeline*> DirectX12RenderPass::pipelines() const noexcept
{
    return m_impl->m_pipelines |
        std::views::transform([](const UniquePtr<DirectX12RenderPipeline>& pipeline) { return pipeline.get(); }) | ranges::to<Array<const DirectX12RenderPipeline*>>() |
        ranges::to<Array<const DirectX12RenderPipeline*>>();
}

const RenderTarget& DirectX12RenderPass::renderTarget(const UInt32& location) const
{
    if (auto match = std::ranges::find_if(m_impl->m_renderTargets, [&location](const RenderTarget& renderTarget) { return renderTarget.location() == location; }); match != m_impl->m_renderTargets.end())
        return *match;

    throw ArgumentOutOfRangeException("No render target is mapped to location {0} in this render pass.", location);
}

Span<const RenderTarget> DirectX12RenderPass::renderTargets() const noexcept
{
    return m_impl->m_renderTargets;
}

bool DirectX12RenderPass::hasPresentTarget() const noexcept
{
    return std::ranges::any_of(m_impl->m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; });
}

Span<const DirectX12InputAttachmentMapping> DirectX12RenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

void DirectX12RenderPass::begin(const UInt32& buffer)
{
    throw;
}

void DirectX12RenderPass::end() const
{
    throw;
}

void DirectX12RenderPass::resizeFrameBuffers(const Size2d& renderArea)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<DirectX12FrameBuffer>& frameBuffer) { frameBuffer->resize(renderArea); });
}

void DirectX12RenderPass::updateAttachments(const DirectX12DescriptorSet& descriptorSet) const
{
    const auto backBuffer = m_impl->m_backBuffer;

    std::ranges::for_each(m_impl->m_inputAttachments, [&descriptorSet, &backBuffer](const DirectX12InputAttachmentMapping& inputAttachment) {
#ifndef NDEBUG
        if (inputAttachment.inputAttachmentSource() == nullptr)
            throw RuntimeException("No source render pass has been specified for the input attachment mapped to location {0}.", inputAttachment.location());
#endif

        descriptorSet.attach(inputAttachment.location(), inputAttachment.inputAttachmentSource()->frameBuffer(backBuffer).image(inputAttachment.renderTarget().location()));
    });
}

DirectX12RenderPipelineBuilder DirectX12RenderPass::makePipeline(const UInt32& id, const String& name) const noexcept
{
    return DirectX12RenderPipelineBuilder(*this, id, name);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPassBuilder::DirectX12RenderPassBuilderImpl : public Implement<DirectX12RenderPassBuilder> {
public:
    friend class DirectX12RenderPassBuilder;
    friend class DirectX12RenderPass;

private:
    Array<UniquePtr<DirectX12RenderPipeline>> m_pipelines;
    Array<DirectX12InputAttachmentMapping> m_inputAttachments;
    Array<RenderTarget> m_renderTargets;

public:
    DirectX12RenderPassBuilderImpl(DirectX12RenderPassBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(const DirectX12Device& device) noexcept :
    m_impl(makePimpl<DirectX12RenderPassBuilderImpl>(this)), RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass>(UniquePtr<DirectX12RenderPass>(new DirectX12RenderPass(device)))
{
}

DirectX12RenderPassBuilder::~DirectX12RenderPassBuilder() noexcept = default;

UniquePtr<DirectX12RenderPass> DirectX12RenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->mapRenderTargets(m_impl->m_renderTargets);
    instance->m_impl->mapInputAttachments(m_impl->m_inputAttachments);

    // Initialize the frame buffers.
    instance->m_impl->m_frameBuffers.resize(instance->getDevice()->swapChain().buffers());
    std::ranges::generate(instance->m_impl->m_frameBuffers, [&instance, i = 0]() mutable { return makeUnique<DirectX12FrameBuffer>(*instance, i++, instance->parent().swapChain().renderArea()); });

    return RenderPassBuilder::go();
}

void DirectX12RenderPassBuilder::use(RenderTarget&& target)
{
    m_impl->m_renderTargets.push_back(std::move(target));
}

void DirectX12RenderPassBuilder::use(DirectX12InputAttachmentMapping&& attachment)
{
    m_impl->m_inputAttachments.push_back(std::move(attachment));
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, samples, clearValues, clear, clearStencil, isVolatile);
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    m_impl->m_renderTargets.push_back(RenderTarget(location, type, format, clear, clearValues, clearStencil, samples, isVolatile));
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(output, static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, samples, clearValues, clear, clearStencil, isVolatile);
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    auto renderTarget = RenderTarget(location, type, format, clear, clearValues, clearStencil, samples, isVolatile);
    output = std::move(DirectX12InputAttachmentMapping(*this->instance(), renderTarget, location));
    m_impl->m_renderTargets.push_back(renderTarget);
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::inputAttachment(const DirectX12InputAttachmentMapping& inputAttachment)
{
    m_impl->m_inputAttachments.push_back(inputAttachment);
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const UInt32& outputLocation)
{
    m_impl->m_inputAttachments.push_back(DirectX12InputAttachmentMapping(renderPass, renderPass.renderTarget(outputLocation), inputLocation));
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget)
{
    m_impl->m_inputAttachments.push_back(DirectX12InputAttachmentMapping(renderPass, renderTarget, inputLocation));
    return *this;
}