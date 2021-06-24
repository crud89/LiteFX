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
    const RenderTarget* m_presentTarget = nullptr;
    const RenderTarget* m_depthStencilTarget = nullptr;
    MultiSamplingLevel m_multiSamplingLevel{ MultiSamplingLevel::x1 };

public:
    DirectX12RenderPassImpl(DirectX12RenderPass* parent, Span<RenderTarget> renderTargets, const MultiSamplingLevel& samples, Span<DirectX12InputAttachmentMapping> inputAttachments) :
        base(parent), m_multiSamplingLevel(samples)
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

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }); match != m_renderTargets.end())
            m_presentTarget = match._Ptr;
        else
            m_presentTarget = nullptr;

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }); match != m_renderTargets.end())
            m_depthStencilTarget = match._Ptr;
        else
            m_depthStencilTarget = nullptr;
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

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, const MultiSamplingLevel& samples, Span<DirectX12InputAttachmentMapping> inputAttachments) :
    m_impl(makePimpl<DirectX12RenderPassImpl>(this, renderTargets, samples, inputAttachments)), DirectX12RuntimeObject<DirectX12Device>(device, &device)
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
    return m_impl->m_presentTarget != nullptr;
}

Span<const DirectX12InputAttachmentMapping> DirectX12RenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

const MultiSamplingLevel& DirectX12RenderPass::multiSamplingLevel() const noexcept
{
    return m_impl->m_multiSamplingLevel;
}

void DirectX12RenderPass::begin(const UInt32& buffer)
{
    // Only begin, if we are currently not running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to begin a render pass, that is already running. End the current pass first.");

    // Select the active frame buffer.
    if (buffer >= m_impl->m_frameBuffers.size())
        throw ArgumentOutOfRangeException("The frame buffer {0} is out of range. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

    auto frameBuffer = m_impl->m_activeFrameBuffer = m_impl->m_frameBuffers[buffer].get();
    m_impl->m_backBuffer = buffer;

    // Begin the command recording on the frame buffers command buffer.
    frameBuffer->commandBuffer().begin();

    // Bind the global descriptor heaps.
    // TODO: This is done once per render pass, since a frame buffer owns a command list. This may be ineffective and we may want to re-use command lists over multiple render passes.
    this->getDevice()->bindGlobalDescriptorHeaps(frameBuffer->commandBuffer());

    // Declare render pass input and output access and transition barriers.
    // TODO: This could possibly be pre-defined as a part of the frame buffer, but would it also safe much time?
    Array<D3D12_RESOURCE_BARRIER> transitionBarriers = m_impl->m_renderTargets |
        std::views::transform([&frameBuffer](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil ? frameBuffer->image(renderTarget.location()).transitionTo(D3D12_RESOURCE_STATE_RENDER_TARGET) : frameBuffer->image(renderTarget.location()).transitionTo(D3D12_RESOURCE_STATE_DEPTH_WRITE); }) | 
        ranges::to<Array<D3D12_RESOURCE_BARRIER>>();

    CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetView(frameBuffer->renderTargetHeap()->GetCPUDescriptorHandleForHeapStart(), 0, frameBuffer->renderTargetDescriptorSize());
    Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderTargets = m_impl->m_renderTargets |
        std::views::filter([](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
        std::views::transform([&frameBuffer, &renderTargetView](const RenderTarget& renderTarget) {
        Float clearColor[4] = { renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() };
        CD3DX12_CLEAR_VALUE clearValue{ ::getFormat(renderTarget.format()), clearColor };

        D3D12_RENDER_PASS_BEGINNING_ACCESS beginAccess = renderTarget.clearBuffer() ?
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } } :
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD, { } };

        D3D12_RENDER_PASS_ENDING_ACCESS endAccess = renderTarget.isVolatile() ?
            D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, {} } :
            D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };

        D3D12_RENDER_PASS_RENDER_TARGET_DESC renderTargetDesc{ renderTargetView, beginAccess, endAccess };
        renderTargetView = renderTargetView.Offset(frameBuffer->renderTargetDescriptorSize());

        return renderTargetDesc;
    }) | ranges::to<Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC>>();

    frameBuffer->commandBuffer().handle()->ResourceBarrier(transitionBarriers.size(), transitionBarriers.data());

    // Declare the depth/stencil render target access (if there is such a target) and begin the render pass.
    if (m_impl->m_depthStencilTarget == nullptr)
        frameBuffer->commandBuffer().handle()->BeginRenderPass(renderTargets.size(), renderTargets.data(), nullptr, D3D12_RENDER_PASS_FLAG_NONE);
    else
    {
        CD3DX12_CLEAR_VALUE clearValue{ ::getFormat(m_impl->m_depthStencilTarget->format()), m_impl->m_depthStencilTarget->clearValues().x(), static_cast<Byte>(m_impl->m_depthStencilTarget->clearValues().y()) };

        D3D12_RENDER_PASS_ENDING_ACCESS depthEndAccess, stencilEndAccess;
        D3D12_RENDER_PASS_BEGINNING_ACCESS depthBeginAccess = m_impl->m_depthStencilTarget->clearBuffer() && ::hasDepth(m_impl->m_depthStencilTarget->format()) ?
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } } :
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, { } };

        D3D12_RENDER_PASS_BEGINNING_ACCESS stencilBeginAccess = m_impl->m_depthStencilTarget->clearStencil() && ::hasStencil(m_impl->m_depthStencilTarget->format()) ?
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } } :
            D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, { } };

        if (depthBeginAccess.Type == D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS)
            depthEndAccess = D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, { } };
        else
            depthEndAccess = m_impl->m_depthStencilTarget->isVolatile() ?
                D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, { } } :
                D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, { } };

        if (stencilBeginAccess.Type == D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS)
            stencilEndAccess = D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, { } };
        else
            stencilEndAccess = m_impl->m_depthStencilTarget->isVolatile() ?
                D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, { } } :
                D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, { } };

        CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilView(frameBuffer->depthStencilTargetHeap()->GetCPUDescriptorHandleForHeapStart(), 0, frameBuffer->depthStencilTargetDescriptorSize());
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc{ depthStencilView, depthBeginAccess, stencilBeginAccess, depthEndAccess, stencilEndAccess };

        frameBuffer->commandBuffer().handle()->BeginRenderPass(renderTargets.size(), renderTargets.data(), &depthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
    }
}

void DirectX12RenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    auto frameBuffer = m_impl->m_activeFrameBuffer;
    const auto& swapChain = this->getDevice()->swapChain();

    // End the render pass and the command buffer recording.
    frameBuffer->commandBuffer().handle()->EndRenderPass();

    // Transition the present and depth/stencil views.
    Array<D3D12_RESOURCE_BARRIER> transitionBarriers = m_impl->m_renderTargets |
        std::views::transform([&frameBuffer](const RenderTarget& renderTarget) {
        const auto& renderTargetImage = frameBuffer->image(renderTarget.location());

        switch (renderTarget.type()) 
        {
        default:
        case RenderTargetType::Color:           return renderTargetImage.transitionTo(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        case RenderTargetType::Present:         return renderTargetImage.transitionTo(D3D12_RESOURCE_STATE_PRESENT);
        case RenderTargetType::DepthStencil:    return renderTargetImage.transitionTo(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ);
        }
    }) | ranges::to<Array<D3D12_RESOURCE_BARRIER>>();

    frameBuffer->commandBuffer().handle()->ResourceBarrier(transitionBarriers.size(), transitionBarriers.data());

    // End the command buffer recording and submit it.
    frameBuffer->commandBuffer().end(true);

    if (m_impl->m_presentTarget != nullptr)
        raiseIfFailed<RuntimeException>(swapChain.handle()->Present(0, swapChain.supportsVariableRefreshRate() ? DXGI_PRESENT_ALLOW_TEARING : 0), "Unable to present swap chain");

    // Reset the frame buffer.
    m_impl->m_activeFrameBuffer = nullptr;
}

void DirectX12RenderPass::resizeFrameBuffers(const Size2d& renderArea)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<DirectX12FrameBuffer>& frameBuffer) { frameBuffer->resize(renderArea); });
}

void DirectX12RenderPass::changeMultiSamplingLevel(const const MultiSamplingLevel& samples)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<DirectX12FrameBuffer>& frameBuffer) { frameBuffer->resize(frameBuffer->size()); });
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
    MultiSamplingLevel m_multiSamplingLevel;

public:
    DirectX12RenderPassBuilderImpl(DirectX12RenderPassBuilder* parent, const MultiSamplingLevel& samples) :
        base(parent), m_multiSamplingLevel(samples)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(const DirectX12Device& device, const MultiSamplingLevel& samples) noexcept :
    m_impl(makePimpl<DirectX12RenderPassBuilderImpl>(this, samples)), RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass>(UniquePtr<DirectX12RenderPass>(new DirectX12RenderPass(device)))
{
}

DirectX12RenderPassBuilder::~DirectX12RenderPassBuilder() noexcept = default;

UniquePtr<DirectX12RenderPass> DirectX12RenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->mapRenderTargets(m_impl->m_renderTargets);
    instance->m_impl->mapInputAttachments(m_impl->m_inputAttachments);
    instance->m_impl->m_multiSamplingLevel = std::move(m_impl->m_multiSamplingLevel);

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

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, clearValues, clear, clearStencil, isVolatile);
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    m_impl->m_renderTargets.push_back(RenderTarget(location, type, format, clear, clearValues, clearStencil, isVolatile));
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(output, static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, clearValues, clear, clearStencil, isVolatile);
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::renderTarget(DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    auto renderTarget = RenderTarget(location, type, format, clear, clearValues, clearStencil, isVolatile);
    output = std::move(DirectX12InputAttachmentMapping(*this->instance(), renderTarget, location));
    m_impl->m_renderTargets.push_back(renderTarget);
    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::setMultiSamplingLevel(const MultiSamplingLevel& samples)
{
    m_impl->m_multiSamplingLevel = samples;
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