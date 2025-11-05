#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
#include <pix3.h>
#endif // defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)

#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPass::DirectX12RenderPassImpl {
public:
    friend class DirectX12RenderPassBuilder;
    friend class DirectX12RenderPass;
    using RenderPassContext = Tuple<Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC>, Optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC>>;

private:
    Array<RenderTarget> m_renderTargets;
    Array<RenderPassDependency> m_inputAttachments;
    Dictionary<const IFrameBuffer*, size_t> m_frameBufferTokens;
    Dictionary<const IFrameBuffer*, SharedPtr<DirectX12CommandBuffer>> m_beginCommandBuffers;
    Dictionary<const IFrameBuffer*, SharedPtr<DirectX12CommandBuffer>> m_endCommandBuffers;
    Dictionary<const IFrameBuffer*, Array<SharedPtr<DirectX12CommandBuffer>>> m_secondaryCommandBuffers;
    UInt32 m_secondaryCommandBufferCount = 0;
    SharedPtr<const DirectX12FrameBuffer> m_activeFrameBuffer = nullptr;
    RenderPassContext m_activeContext;
    const RenderTarget* m_presentTarget = nullptr;
    const RenderTarget* m_depthStencilTarget = nullptr;
    Optional<DescriptorBindingPoint> m_inputAttachmentSamplerBinding{ };
    SharedPtr<const DirectX12Device> m_device;
    SharedPtr<const DirectX12Queue> m_queue;
    bool m_onDefaultGraphicsQueue = false;
    UInt32 m_viewMask = 0x00;

public:
    DirectX12RenderPassImpl(const DirectX12Device& device, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
        m_secondaryCommandBufferCount(secondaryCommandBuffers), m_inputAttachmentSamplerBinding(inputAttachmentSamplerBinding), m_device(device.shared_from_this()), m_queue(queue.shared_from_this()), m_onDefaultGraphicsQueue(std::addressof(queue) == std::addressof(device.defaultQueue(QueueType::Graphics))), m_viewMask(viewMask)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);

        if (secondaryCommandBuffers == 0) [[unlikely]]
            LITEFX_WARNING(DIRECTX12_LOG, "Secondary command buffer count for this render pass is 0, which makes it prevents recording draw commands to this render pass.");
    }

    DirectX12RenderPassImpl(const DirectX12Device& device) :
        m_device(device.shared_from_this()), m_queue(device.defaultQueue(QueueType::Graphics).shared_from_this()), m_onDefaultGraphicsQueue(true)
    {
    }

    DirectX12RenderPassImpl(DirectX12RenderPassImpl&&) noexcept = default;
    DirectX12RenderPassImpl(const DirectX12RenderPassImpl&) = default;
    DirectX12RenderPassImpl& operator=(DirectX12RenderPassImpl&&) noexcept = default;
    DirectX12RenderPassImpl& operator=(const DirectX12RenderPassImpl&) = default;

    ~DirectX12RenderPassImpl() noexcept
    {
        // Stop listening to frame buffer events.
        for (auto [frameBuffer, token] : m_frameBufferTokens)
            frameBuffer->released -= token;
    }

public:
    void mapRenderTargets(Span<RenderTarget> renderTargets)
    {
        m_renderTargets.assign(std::begin(renderTargets), std::end(renderTargets));
        std::ranges::sort(m_renderTargets, [](const auto& a, const auto& b) { return a.location() < b.location(); });

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }); match != m_renderTargets.end())
            m_presentTarget = std::addressof(*match);
        else
            m_presentTarget = nullptr;

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }); match != m_renderTargets.end())
            m_depthStencilTarget = std::addressof(*match);
        else
            m_depthStencilTarget = nullptr;

        // TODO: If there is a present target, we need to check if the provided queue can actually present on the surface. Currently, 
        //       we simply check if the queue is the same as the swap chain queue (which is the default graphics queue).
        if (m_presentTarget != nullptr && !m_onDefaultGraphicsQueue) [[unlikely]]
            throw InvalidArgumentException("renderTargets", "A render pass with a present target must be executed on the default graphics queue.");
    }

    void mapInputAttachments(Span<RenderPassDependency> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
    }

    void registerFrameBuffer([[maybe_unused]] const DirectX12RenderPass& renderPass, const DirectX12Queue& queue, const DirectX12FrameBuffer& frameBuffer)
    {
        // If the frame buffer is not yet registered, do so by listening for its release.
        auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);

        if (!m_frameBufferTokens.contains(interfacePointer)) [[unlikely]]
        {
            m_frameBufferTokens[interfacePointer] = frameBuffer.released.add(std::bind(&DirectX12RenderPassImpl::onFrameBufferRelease, this, std::placeholders::_1, std::placeholders::_2));

            // Create begin command buffers.
            {
                auto commandBuffer = queue.createCommandBuffer(false);
#ifndef NDEBUG
                std::as_const(*commandBuffer).handle()->SetName(Widen(std::format("{0} Begin Commands {1}", renderPass.name(), m_beginCommandBuffers.size())).c_str());
#endif
                m_beginCommandBuffers[interfacePointer] = commandBuffer;
            }

            // Create end command buffers.
            {
                auto commandBuffer = queue.createCommandBuffer(false);
#ifndef NDEBUG
                std::as_const(*commandBuffer).handle()->SetName(Widen(std::format("{0} End Commands {1}", renderPass.name(), m_endCommandBuffers.size())).c_str());
#endif
                m_endCommandBuffers[interfacePointer] = commandBuffer;
            }

            // Create secondary command buffers.
            m_secondaryCommandBuffers[interfacePointer] = std::views::iota(0u, m_secondaryCommandBufferCount) |
                std::views::transform([&queue, &renderPass]([[maybe_unused]] UInt32 i) {
                    auto commandBuffer = queue.createCommandBuffer(false);
#ifdef NDEBUG
                    (void)renderPass;
#else
                    std::as_const(*commandBuffer).handle()->SetName(Widen(std::format("{0} Secondary Commands {1}", renderPass.name(), i)).c_str());
#endif
                    return commandBuffer;
                }) | std::ranges::to<Array<SharedPtr<DirectX12CommandBuffer>>>();
        }

        // Store the active frame buffer pointer.
        m_activeFrameBuffer = frameBuffer.shared_from_this();
    }

    void onFrameBufferRelease(const void* sender, const IFrameBuffer::ReleasedEventArgs& /*args*/)
    {
        // Obtain the interface pointer and release all resources bound to the frame buffer.
        auto interfacePointer = static_cast<const IFrameBuffer*>(sender);

        if (static_cast<const IFrameBuffer*>(m_activeFrameBuffer.get()) == interfacePointer) [[unlikely]]
            throw RuntimeException("A frame buffer that is currently in use on a render pass cannot be released.");

        m_beginCommandBuffers.erase(interfacePointer);
        m_endCommandBuffers.erase(interfacePointer);
        m_secondaryCommandBuffers.erase(interfacePointer);

        // Release the token.
        m_frameBufferTokens.erase(interfacePointer);
    }

    RenderPassContext& renderTargetContext(const DirectX12FrameBuffer& frameBuffer)
    {
        std::get<0>(m_activeContext) = m_renderTargets |
            std::views::filter([](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
            std::views::transform([&frameBuffer](const RenderTarget& renderTarget) {
                auto clearColor = std::array { renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() };
                CD3DX12_CLEAR_VALUE clearValue{ DX12::getFormat(renderTarget.format()), clearColor.data() };

                D3D12_RENDER_PASS_BEGINNING_ACCESS beginAccess = renderTarget.clearBuffer() ?
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ .Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, .Clear = { clearValue } } :
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
                
                D3D12_RENDER_PASS_ENDING_ACCESS endAccess = renderTarget.isVolatile() ?
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, {} } :
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };

                D3D12_RENDER_PASS_RENDER_TARGET_DESC renderTargetDesc{ frameBuffer.descriptorHandle(renderTarget), beginAccess, endAccess};

                return renderTargetDesc;
            }) | std::ranges::to<Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC>>();

        if (m_depthStencilTarget == nullptr)
            std::get<1>(m_activeContext) = std::nullopt;
        else
        {
            CD3DX12_CLEAR_VALUE clearValue{ DX12::getFormat(m_depthStencilTarget->format()), m_depthStencilTarget->clearValues().x(), static_cast<UInt8>(m_depthStencilTarget->clearValues().y()) };

            D3D12_RENDER_PASS_BEGINNING_ACCESS depthBeginAccess { D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS }, stencilBeginAccess { D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS };
            D3D12_RENDER_PASS_ENDING_ACCESS depthEndAccess { D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS }, stencilEndAccess { D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS };
            
            if (::hasDepth(m_depthStencilTarget->format()))
            {
                depthBeginAccess = m_depthStencilTarget->clearBuffer() ?
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ .Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, .Clear = { clearValue } } :
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
                depthEndAccess = m_depthStencilTarget->isVolatile() ?
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, { } } :
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, { } };
            }

            if (::hasStencil(m_depthStencilTarget->format()))
            {
                stencilBeginAccess = m_depthStencilTarget->clearStencil() ?
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ .Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, .Clear = { clearValue } } :
                    D3D12_RENDER_PASS_BEGINNING_ACCESS{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
                stencilEndAccess = m_depthStencilTarget->isVolatile() ?
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD, { } } :
                    D3D12_RENDER_PASS_ENDING_ACCESS{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, { } };
            }

            std::get<1>(m_activeContext) = D3D12_RENDER_PASS_DEPTH_STENCIL_DESC{ frameBuffer.descriptorHandle(*m_depthStencilTarget), depthBeginAccess, stencilBeginAccess, depthEndAccess, stencilEndAccess };
        }

        return m_activeContext;
    }

    inline SharedPtr<DirectX12CommandBuffer> getBeginCommandBuffer(const DirectX12FrameBuffer& frameBuffer)
    {
        return m_beginCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }

    inline SharedPtr<DirectX12CommandBuffer> getEndCommandBuffer(const DirectX12FrameBuffer& frameBuffer)
    {
        return m_endCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }

    inline Array<SharedPtr<DirectX12CommandBuffer>>& getSecondaryCommandBuffers(const DirectX12FrameBuffer& frameBuffer)
    {
        return m_secondaryCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    DirectX12RenderPass(device, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    DirectX12RenderPass(device, name, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    m_impl(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, const String& name, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    DirectX12RenderPass(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
    if (!name.empty())
        this->name() = name;
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device& device, const String& name) :
    m_impl(device)
{
    if (!name.empty())
        this->name() = name;
}

DirectX12RenderPass::~DirectX12RenderPass() noexcept = default;

const DirectX12Device& DirectX12RenderPass::device() const noexcept
{
    return *m_impl->m_device;
}

SharedPtr<const DirectX12FrameBuffer> DirectX12RenderPass::activeFrameBuffer() const noexcept
{
    return m_impl->m_activeFrameBuffer;
}

const DirectX12Queue& DirectX12RenderPass::commandQueue() const noexcept 
{
    return *m_impl->m_queue;
}

SharedPtr<const DirectX12CommandBuffer> DirectX12RenderPass::commandBuffer(UInt32 index) const
{
    if (m_impl->m_activeFrameBuffer == nullptr) [[unlikely]]
        throw RuntimeException("Unable to lookup command buffers on a render pass that has not been begun.");

    if (index >= m_impl->m_secondaryCommandBufferCount) [[unlikely]]
        throw ArgumentOutOfRangeException("index", std::make_pair(0u, m_impl->m_secondaryCommandBufferCount), index, "The render pass only contains {0} command buffers, but an index of {1} has been provided.", m_impl->m_secondaryCommandBufferCount, index);

    return m_impl->m_secondaryCommandBuffers.at(m_impl->m_activeFrameBuffer.get()).at(index);
}

Enumerable<SharedPtr<const DirectX12CommandBuffer>> DirectX12RenderPass::commandBuffers() const
{
    if (m_impl->m_secondaryCommandBufferCount == 0u || m_impl->m_activeFrameBuffer == nullptr)
    {
        static const Array<SharedPtr<const DirectX12CommandBuffer>> _empty { };
        return _empty;
    }
    else
    {
        return m_impl->m_secondaryCommandBuffers.at(m_impl->m_activeFrameBuffer.get());
    }
}

UInt32 DirectX12RenderPass::secondaryCommandBuffers() const noexcept
{
    return m_impl->m_secondaryCommandBufferCount;
}

const Array<RenderTarget>& DirectX12RenderPass::renderTargets() const noexcept
{
    return m_impl->m_renderTargets;
}

const RenderTarget& DirectX12RenderPass::renderTarget(UInt32 location) const
{
    if (auto match = std::ranges::find_if(m_impl->m_renderTargets, [&location](const RenderTarget& renderTarget) { return renderTarget.location() == location; }); match != m_impl->m_renderTargets.end())
        return *match;

    throw InvalidArgumentException("location", "No render target is mapped to location {0} in this render pass.", location);
}

bool DirectX12RenderPass::hasPresentTarget() const noexcept
{
    return m_impl->m_presentTarget != nullptr;
}

const Array<RenderPassDependency>& DirectX12RenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

const RenderPassDependency& DirectX12RenderPass::inputAttachment(UInt32 location) const 
{
    if (location >= m_impl->m_inputAttachments.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("location", std::make_pair(0uz, m_impl->m_inputAttachments.size()), static_cast<size_t>(location), "The render pass does not contain an input attachment at location {0}.", location);

    return m_impl->m_inputAttachments[location];
}

const Optional<DescriptorBindingPoint>& DirectX12RenderPass::inputAttachmentSamplerBinding() const noexcept 
{
    return m_impl->m_inputAttachmentSamplerBinding;
}

UInt32 DirectX12RenderPass::viewMask() const noexcept
{
    return m_impl->m_viewMask;
}

void DirectX12RenderPass::begin(const DirectX12FrameBuffer& frameBuffer) const
{
    // Only begin, if we are currently not running.
    if (m_impl->m_activeFrameBuffer != nullptr) [[unlikely]]
        throw RuntimeException("Unable to begin a render pass, that is already running. End the current pass first.");

    // Register the frame buffer.
    m_impl->registerFrameBuffer(*this, *m_impl->m_queue, frameBuffer);

    // Initialize the render pass context.
    auto& context = m_impl->renderTargetContext(frameBuffer);

    // Begin the command recording on the frame buffers command buffer. Before we can do that, we need to make sure it has not being executed anymore.
    auto beginCommandBuffer = m_impl->getBeginCommandBuffer(frameBuffer);
    beginCommandBuffer->begin();

    // Declare render pass input transition barriers for render targets and input attachments.
    DirectX12Barrier renderTargetBarrier(PipelineStage::None, PipelineStage::RenderTarget), depthStencilBarrier(PipelineStage::None, PipelineStage::DepthStencil);

    std::ranges::for_each(m_impl->m_renderTargets, [&renderTargetBarrier, &depthStencilBarrier, &frameBuffer](const RenderTarget& renderTarget) {
        auto& image = frameBuffer[renderTarget];

        if (renderTarget.type() == RenderTargetType::DepthStencil)
            depthStencilBarrier.transition(image, ResourceAccess::None, ResourceAccess::DepthStencilWrite, ImageLayout::Undefined, ImageLayout::DepthWrite);
        else //if (!renderTarget.multiQueueAccess())
            renderTargetBarrier.transition(image, ResourceAccess::None, ResourceAccess::RenderTarget, ImageLayout::Undefined, ImageLayout::RenderTarget);
        //else  // Resources with simultaneous access enabled don't need to be transitioned.
        //    renderTargetBarrier.transition(image, ResourceAccess::ShaderRead, ResourceAccess::RenderTarget, ImageLayout::Common);
    });

    DirectX12Barrier inputAttachmentBarrier(PipelineStage::None, PipelineStage::All);

    std::ranges::for_each(m_impl->m_inputAttachments, [&inputAttachmentBarrier, &frameBuffer](const RenderPassDependency& dependency) {
        inputAttachmentBarrier.transition(frameBuffer[dependency.renderTarget()], ResourceAccess::None, ResourceAccess::ShaderRead, ImageLayout::Undefined, ImageLayout::ShaderResource);
    });

    beginCommandBuffer->barrier(renderTargetBarrier);
    beginCommandBuffer->barrier(depthStencilBarrier);
    beginCommandBuffer->barrier(inputAttachmentBarrier);

    if (!this->name().empty())
        m_impl->m_queue->beginDebugRegion(std::format("{0} Render Pass", this->name()));

    // Begin a suspending render pass for the transition and a suspend-the-resume render pass on each command buffer of the frame buffer.
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    std::as_const(*beginCommandBuffer).handle()->BeginRenderPass(static_cast<UINT>(std::get<0>(context).size()), std::get<0>(context).data(), std::get<1>(context).has_value() ? &std::get<1>(context).value() : nullptr, D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS);
    std::as_const(*beginCommandBuffer).handle()->EndRenderPass();
    std::ranges::for_each(m_impl->getSecondaryCommandBuffers(frameBuffer), [&](auto& commandBuffer) { 
        commandBuffer->begin(); 
        std::as_const(*commandBuffer).handle()->BeginRenderPass(static_cast<UINT>(std::get<0>(context).size()), std::get<0>(context).data(), std::get<1>(context).has_value() ? &std::get<1>(context).value() : nullptr, D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS | D3D12_RENDER_PASS_FLAG_RESUMING_PASS);

        // Set the view mask.
        std::as_const(*commandBuffer).handle()->SetViewInstanceMask(m_impl->m_viewMask);
    });
    // NOLINTEND(bugprone-unchecked-optional-access)

    // Publish beginning event.
    this->beginning(this, { frameBuffer });
}

UInt64 DirectX12RenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    // Publish ending event.
    this->ending(this, { });

    auto& frameBuffer = *m_impl->m_activeFrameBuffer;
    const auto& swapChain = m_impl->m_device->swapChain();

    // Resume and end the render pass.
    const auto& context = m_impl->m_activeContext;
    auto endCommandBuffer = m_impl->getEndCommandBuffer(frameBuffer);
    std::ranges::for_each(m_impl->getSecondaryCommandBuffers(frameBuffer), [](auto& commandBuffer) { std::as_const(*commandBuffer).handle()->EndRenderPass(); });
    endCommandBuffer->begin();
    std::as_const(*endCommandBuffer).handle()->BeginRenderPass(static_cast<UInt32>(std::get<0>(context).size()), std::get<0>(context).data(), std::get<1>(context).has_value() ? &std::get<1>(context).value() : nullptr, D3D12_RENDER_PASS_FLAG_RESUMING_PASS); // NOLINT(bugprone-unchecked-optional-access)
    std::as_const(*endCommandBuffer).handle()->EndRenderPass();

    // If the present target is multi-sampled, we need to resolve it to the back buffer.
    const auto& backBufferImage = swapChain.image();
    bool requiresResolve{ this->hasPresentTarget() && frameBuffer[*m_impl->m_presentTarget].samples() > MultiSamplingLevel::x1 };

    // Transition the present and depth/stencil views.
    // NOTE: Ending the render pass implicitly barriers with legacy resource state?!
    DirectX12Barrier renderTargetBarrier(PipelineStage::RenderTarget, PipelineStage::None), depthStencilBarrier(PipelineStage::DepthStencil, PipelineStage::None),
        resolveBarrier(PipelineStage::RenderTarget, PipelineStage::Resolve), presentBarrier(PipelineStage::RenderTarget, PipelineStage::Transfer);
    std::ranges::for_each(m_impl->m_renderTargets, [&](const RenderTarget& renderTarget) {
        //if (renderTarget.multiQueueAccess())
        //    return;  // Resources with simultaneous access enabled don't need to be transitioned.

        switch (renderTarget.type())
        {
        default:
        case RenderTargetType::Color:
            renderTargetBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::None, ImageLayout::RenderTarget, ImageLayout::ShaderResource);
            break;
        case RenderTargetType::DepthStencil:
            depthStencilBarrier.transition(frameBuffer[renderTarget], ResourceAccess::DepthStencilWrite, ResourceAccess::None, ImageLayout::DepthWrite, ImageLayout::DepthRead);
            break;
        case RenderTargetType::Present:
            if (requiresResolve)
                resolveBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::ResolveRead, ImageLayout::RenderTarget, ImageLayout::ResolveSource);
            else
                presentBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::TransferRead, ImageLayout::RenderTarget, ImageLayout::CopySource);

            break;
        }
    });

    endCommandBuffer->barrier(renderTargetBarrier);
    endCommandBuffer->barrier(depthStencilBarrier);
    endCommandBuffer->barrier(presentBarrier);

    // Add another barrier for the back buffer image, if required.
    if (requiresResolve)
    {
        resolveBarrier.transition(backBufferImage, ResourceAccess::Common, ResourceAccess::ResolveWrite, ImageLayout::Common, ImageLayout::ResolveDestination);
        endCommandBuffer->barrier(resolveBarrier);

        auto& multiSampledImage = frameBuffer[*m_impl->m_presentTarget];
        std::as_const(*endCommandBuffer).handle()->ResolveSubresource(backBufferImage.handle().Get(), 0, multiSampledImage.handle().Get(), 0, DX12::getFormat(multiSampledImage.format()));

        // Transition the present target back to the present state.
        DirectX12Barrier backBufferBarrier(PipelineStage::Resolve, PipelineStage::Resolve);
        backBufferBarrier.transition(backBufferImage, ResourceAccess::ResolveWrite, ResourceAccess::Common, ImageLayout::ResolveDestination, ImageLayout::Present);
        backBufferBarrier.transition(multiSampledImage, ResourceAccess::ResolveRead, ResourceAccess::Common, ImageLayout::ResolveSource, ImageLayout::Common);
        endCommandBuffer->barrier(backBufferBarrier);
    }
    else if (this->hasPresentTarget())
    {
        DirectX12Barrier beginPresentBarrier(PipelineStage::None, PipelineStage::Transfer);
        beginPresentBarrier.transition(backBufferImage, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
        endCommandBuffer->barrier(beginPresentBarrier);

        auto& presentTarget = frameBuffer[*m_impl->m_presentTarget];
        endCommandBuffer->transfer(presentTarget, backBufferImage);

        DirectX12Barrier endPresentBarrier(PipelineStage::Transfer, PipelineStage::None);
        endPresentBarrier.transition(backBufferImage, ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::Present);
        endCommandBuffer->barrier(endPresentBarrier);
    }

    // If there is a present target, allow the swap chain to resolve queries for the current heap.
    if (m_impl->m_presentTarget != nullptr)
        swapChain.resolveQueryHeaps(*endCommandBuffer);

    // End the command buffer recording and submit all command buffers.
    // NOTE: In order to suspend/resume render passes, we need to pass them to the queue in one `ExecuteCommandLists` (i.e. submit) call. The order we pass them to the call is 
    //       important, since the first command list also gets executed first.
    auto commandBuffers = m_impl->getSecondaryCommandBuffers(frameBuffer);
    commandBuffers.insert(commandBuffers.begin(), m_impl->getBeginCommandBuffer(frameBuffer));
    commandBuffers.push_back(endCommandBuffer);

    // Submit and store the fence.
    auto fence = m_impl->m_queue->submit(commandBuffers | std::ranges::to<Enumerable<SharedPtr<const DirectX12CommandBuffer>>>());

    if (!this->name().empty())
        m_impl->m_queue->endDebugRegion();

    // NOTE: No need to wait for the fence here, since `Present` will wait for the back buffer to be ready. If we have multiple frames in flight, this will block until the first
    //       frame in the queue has been drawn and the back buffer can be written again.
    //       Instead of blocking, we could also use a wait-able swap chain (https://www.gamedev.net/forums/topic/677527-dx12-fences-and-swap-chain-present/).
    if (m_impl->m_presentTarget != nullptr)
        swapChain.present(fence);

    // Reset the frame buffer.
    m_impl->m_activeFrameBuffer = nullptr;

    // Return the last fence of the frame buffer.
    return fence;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(const DirectX12Device& device, const String& name) :
    DirectX12RenderPassBuilder(device, 1, name)
{
}

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(const DirectX12Device& device, UInt32 commandBuffers, const String& name) :
    RenderPassBuilder(DirectX12RenderPass::create(device, name))
{
    this->state().commandBufferCount = commandBuffers;
}

DirectX12RenderPassBuilder::~DirectX12RenderPassBuilder() noexcept = default;

void DirectX12RenderPassBuilder::build()
{
    auto instance = this->instance();

    if (this->state().commandQueue != nullptr)
        instance->m_impl->m_queue = this->state().commandQueue;

    instance->m_impl->mapRenderTargets(this->state().renderTargets);
    instance->m_impl->mapInputAttachments(this->state().inputAttachments);
    instance->m_impl->m_inputAttachmentSamplerBinding = this->state().inputAttachmentSamplerBinding;
    instance->m_impl->m_secondaryCommandBufferCount = this->state().commandBufferCount;
}

RenderPassDependency DirectX12RenderPassBuilder::makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget)
{
    return { renderTarget, binding };
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)