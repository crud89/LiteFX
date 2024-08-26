#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl : public Implement<VulkanRenderPass> {
public:
    friend class VulkanRenderPassBuilder;
    friend class VulkanRenderPass;

private:
    Array<RenderTarget> m_renderTargets;
    Array<RenderPassDependency> m_inputAttachments;
    Dictionary<const IFrameBuffer*, size_t> m_frameBufferTokens;
    Array<size_t> m_swapChainTokens;
    Dictionary<const IFrameBuffer*, SharedPtr<VulkanCommandBuffer>> m_primaryCommandBuffers;
    Dictionary<const IFrameBuffer*, Array<SharedPtr<VulkanCommandBuffer>>> m_secondaryCommandBuffers;
    Dictionary<const IVulkanImage*, VkImageView> m_swapChainViews;
    UInt32 m_secondaryCommandBufferCount = 0;
    const VulkanFrameBuffer* m_activeFrameBuffer = nullptr;
    const RenderTarget* m_presentTarget = nullptr;
    const RenderTarget* m_depthStencilTarget = nullptr;
    Optional<DescriptorBindingPoint> m_inputAttachmentSamplerBinding{ };
    const VulkanDevice& m_device;
    const VulkanSwapChain& m_swapChain;
    const VulkanQueue* m_queue;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers) :
        base(parent), m_device(device), m_swapChain(m_device.swapChain()), m_queue(&queue), m_inputAttachmentSamplerBinding(inputAttachmentSamplerBinding), m_secondaryCommandBufferCount(secondaryCommandBuffers)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);

        if (secondaryCommandBuffers == 0) [[unlikely]]
            LITEFX_WARNING(VULKAN_LOG, "Secondary command buffer count for this render pass is 0, which makes it prevents recording draw commands to this render pass.");
    }

    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device) :
        base(parent), m_device(device), m_swapChain(m_device.swapChain()), m_queue(&device.defaultQueue(QueueType::Graphics))
    {
    }

    ~VulkanRenderPassImpl()
    {
        // Stop listening to frame buffer events.
        for (auto [frameBuffer, token] : m_frameBufferTokens)
            frameBuffer->released -= token;

        // Stop listening to swap chain events.
        for (auto token : m_swapChainTokens)
            m_swapChain.reseted -= token;

        // Release swap chain image views if there are any.
        for (auto view : m_swapChainViews | std::views::values)
            ::vkDestroyImageView(m_device.handle(), view, nullptr);
    }

public:
    void mapRenderTargets(Span<RenderTarget> renderTargets)
    {
        m_renderTargets.assign(std::begin(renderTargets), std::end(renderTargets));
        std::ranges::sort(m_renderTargets, [this](const auto& a, const auto& b) { return a.location() < b.location(); });

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }); match != m_renderTargets.end())
            m_presentTarget = match._Ptr;
        else
            m_presentTarget = nullptr;

        if (auto match = std::ranges::find_if(m_renderTargets, [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }); match != m_renderTargets.end())
            m_depthStencilTarget = match._Ptr;
        else
            m_depthStencilTarget = nullptr;

        // TODO: If there is a present target, we need to check if the provided queue can actually present on the surface. Currently, 
        //       we simply check if the queue is the same as the swap chain queue (which is the default graphics queue).
        if (m_presentTarget != nullptr && m_queue != std::addressof(m_device.defaultQueue(QueueType::Graphics))) [[unlikely]]
            throw InvalidArgumentException("renderTargets", "A render pass with a present target must be executed on the default graphics queue.");

        // Listen to swap chain resets in order to clear back buffer image views.
        if (m_presentTarget != nullptr)
            m_swapChainTokens.push_back(m_swapChain.reseted.add(std::bind(&VulkanRenderPassImpl::onSwapChainReset, this, std::placeholders::_1, std::placeholders::_2)));
    }

    void mapInputAttachments(Span<RenderPassDependency> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
    }

    void registerFrameBuffer(const VulkanFrameBuffer& frameBuffer)
    {
        // If the frame buffer is not yet registered, do so by listening for its release.
        auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);

        if (!m_frameBufferTokens.contains(interfacePointer)) [[unlikely]]
        {
            m_frameBufferTokens[interfacePointer] = frameBuffer.released.add(std::bind(&VulkanRenderPassImpl::onFrameBufferRelease, this, std::placeholders::_1, std::placeholders::_2));

            // Create primary command buffers.
            {
                auto commandBuffer = m_queue->createCommandBuffer(false);
#ifndef NDEBUG
                m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*commandBuffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 
                    std::format("{0} Primary Commands {1}", m_parent->name(), m_primaryCommandBuffers.size()).c_str());
#endif
                m_primaryCommandBuffers[interfacePointer] = commandBuffer;
            }

            // Create secondary command buffers.
            m_secondaryCommandBuffers[interfacePointer] = std::views::iota(0u, m_secondaryCommandBufferCount) |
                std::views::transform([this](UInt32 i) {
                    auto commandBuffer = m_queue->createCommandBuffer(false, true);
#ifndef NDEBUG
                    m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*commandBuffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 
                        std::format("{0} Secondary Commands {1}", m_parent->name(), i).c_str());
#endif
                    return commandBuffer;
                }) | std::ranges::to<Array<SharedPtr<VulkanCommandBuffer>>>();
        }

        // Store the active frame buffer pointer.
        m_activeFrameBuffer = &frameBuffer;
    }

    void onFrameBufferRelease(const void* sender, IFrameBuffer::ReleasedEventArgs args)
    {
        // Obtain the interface pointer and release all resources bound to the frame buffer.
        auto interfacePointer = reinterpret_cast<const IFrameBuffer*>(sender);

        if (static_cast<const IFrameBuffer*>(m_activeFrameBuffer) == interfacePointer) [[unlikely]]
            throw RuntimeException("A frame buffer that is currently in use on a render pass cannot be released.");

        m_primaryCommandBuffers.erase(interfacePointer);
        m_secondaryCommandBuffers.erase(interfacePointer);

        // Release the token.
        m_frameBufferTokens.erase(interfacePointer);
    }

    void onSwapChainReset(const void* sender, ISwapChain::ResetEventArgs args)
    {
        // Release swap chain image views if there are any, so that they need to be re-created with the next context.
        for (auto view : m_swapChainViews | std::views::values)
            ::vkDestroyImageView(m_device.handle(), view, nullptr);

        m_swapChainViews.clear();
    }

    Array<VkRenderingAttachmentInfo> colorTargetContext(const VulkanFrameBuffer& frameBuffer)
    {
        return m_renderTargets | std::views::filter([](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
            std::views::transform([this, &frameBuffer](const RenderTarget& renderTarget) {
                // Create an attachment info.
                VkRenderingAttachmentInfo attachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = frameBuffer.imageView(renderTarget),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = renderTarget.clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = { renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() }
                };

                // Get the image and check if we need to resolve it.
                if (renderTarget.type() == RenderTargetType::Present && frameBuffer[renderTarget].samples() > MultiSamplingLevel::x1)
                {
                    auto getImageView = [&](const IVulkanImage& image) -> VkImageView {
                        VkImageViewCreateInfo createInfo = {
                            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                            .image = std::as_const(image).handle(),
                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                            .format = Vk::getFormat(image.format()),
                            .components = VkComponentMapping {
                                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .a = VK_COMPONENT_SWIZZLE_IDENTITY
                            },
                            .subresourceRange = VkImageSubresourceRange {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .levelCount = 1,
                                .layerCount = 1
                            }
                        };

                        VkImageView imageView;
                        raiseIfFailed(::vkCreateImageView(m_device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view for swap chain back buffer.");
                        return imageView;
                    };

                    // Create an image view if we don't already have one for the swap chain image.
                    auto& backBuffer = m_swapChain.image();

                    if (!m_swapChainViews.contains(&backBuffer))
                        m_swapChainViews[&backBuffer] = getImageView(backBuffer);

                    attachmentInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                    attachmentInfo.resolveImageView = m_swapChainViews.at(&backBuffer);
                    attachmentInfo.resolveImageLayout = Vk::getImageLayout(ImageLayout::ResolveDestination);
                }

                return attachmentInfo;
            }) | std::ranges::to<Array<VkRenderingAttachmentInfo>>();
    }

    Optional<VkRenderingAttachmentInfo> depthTargetContext(const VulkanFrameBuffer& frameBuffer)
    {
        if (m_depthStencilTarget == nullptr || !::hasDepth(m_depthStencilTarget->format()))
            return std::nullopt;

        return VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = frameBuffer.imageView(*m_depthStencilTarget),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = m_depthStencilTarget->clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = m_depthStencilTarget->isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { m_depthStencilTarget->clearValues().x(), m_depthStencilTarget->clearValues().x(), m_depthStencilTarget->clearValues().x(), m_depthStencilTarget->clearValues().x() }
        };
    }

    Optional<VkRenderingAttachmentInfo> stencilTargetContext(const VulkanFrameBuffer& frameBuffer)
    {
        if (m_depthStencilTarget == nullptr || !::hasStencil(m_depthStencilTarget->format()))
            return std::nullopt;

        return VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = frameBuffer.imageView(*m_depthStencilTarget),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = m_depthStencilTarget->clearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = m_depthStencilTarget->isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { m_depthStencilTarget->clearValues().y(), m_depthStencilTarget->clearValues().y(), m_depthStencilTarget->clearValues().y(), m_depthStencilTarget->clearValues().y() }
        };
    }

    inline SharedPtr<VulkanCommandBuffer> getPrimaryCommandBuffer(const VulkanFrameBuffer& frameBuffer)
    {
        return m_primaryCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }

    inline Array<SharedPtr<VulkanCommandBuffer>>& getSecondaryCommandBuffers(const VulkanFrameBuffer& frameBuffer)
    {
        return m_secondaryCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers) :
    VulkanRenderPass(device, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers) :
    VulkanRenderPass(device, name, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers))
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers) :
    VulkanRenderPass(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers)
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name) noexcept :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device))
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::~VulkanRenderPass() noexcept = default;

const VulkanDevice& VulkanRenderPass::device() const noexcept
{
    return m_impl->m_device;
}

const VulkanFrameBuffer& VulkanRenderPass::activeFrameBuffer() const
{
    if (m_impl->m_activeFrameBuffer == nullptr) [[unlikely]]
        throw RuntimeException("No frame buffer is active, since the render pass has not begun.");

    return *m_impl->m_activeFrameBuffer;
}

const VulkanQueue& VulkanRenderPass::commandQueue() const noexcept
{
    return *m_impl->m_queue;
}

SharedPtr<const VulkanCommandBuffer> VulkanRenderPass::commandBuffer(UInt32 index) const
{
    if (m_impl->m_activeFrameBuffer == nullptr) [[unlikely]]
        throw RuntimeException("Unable to lookup command buffers on a render pass that has not been begun.");

    if (index >= m_impl->m_secondaryCommandBufferCount) [[unlikely]]
        throw ArgumentOutOfRangeException("index", std::make_pair(0u, m_impl->m_secondaryCommandBufferCount), index, "The render pass only contains {0} command buffers, but an index of {1} has been provided.", m_impl->m_secondaryCommandBufferCount, index);

    return m_impl->m_secondaryCommandBuffers[m_impl->m_activeFrameBuffer][index];
}

Enumerable<SharedPtr<const VulkanCommandBuffer>> VulkanRenderPass::commandBuffers() const noexcept
{
    if (m_impl->m_secondaryCommandBufferCount == 0u || m_impl->m_activeFrameBuffer == nullptr)
        return { };

    return m_impl->m_secondaryCommandBuffers[m_impl->m_activeFrameBuffer];
}

UInt32 VulkanRenderPass::secondaryCommandBuffers() const noexcept
{
    return m_impl->m_secondaryCommandBufferCount;
}

const Array<RenderTarget>& VulkanRenderPass::renderTargets() const noexcept
{
    return m_impl->m_renderTargets;
}

const RenderTarget& VulkanRenderPass::renderTarget(UInt32 location) const
{
    if (auto match = std::ranges::find_if(m_impl->m_renderTargets, [&location](const RenderTarget& renderTarget) { return renderTarget.location() == location; }); match != m_impl->m_renderTargets.end()) [[likely]]
        return *match;

    throw InvalidArgumentException("location", "No render target is mapped to location {0} in this render pass.", location);
}

bool VulkanRenderPass::hasPresentTarget() const noexcept
{
    return std::ranges::any_of(m_impl->m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; });
}

const Array<RenderPassDependency>& VulkanRenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

const RenderPassDependency& VulkanRenderPass::inputAttachment(UInt32 location) const
{
    if (location >= m_impl->m_inputAttachments.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("location", std::make_pair(0ull, (UInt64)m_impl->m_inputAttachments.size()), location, "The render pass does not contain an input attachment at location {0}.", location);

    return m_impl->m_inputAttachments[location];
}

const Optional<DescriptorBindingPoint>& VulkanRenderPass::inputAttachmentSamplerBinding() const noexcept
{
    return m_impl->m_inputAttachmentSamplerBinding;
}

void VulkanRenderPass::begin(const VulkanFrameBuffer& frameBuffer) const
{
    // Only begin, if we are currently not running.
    if (m_impl->m_activeFrameBuffer != nullptr) [[unlikely]]
        throw RuntimeException("Unable to begin a render pass, that is already running. End the current pass first.");

    // Register the frame buffer.
    m_impl->registerFrameBuffer(frameBuffer);

    // Initialize the render pass context.
    auto colorTargetInfos  = m_impl->colorTargetContext(frameBuffer);
    auto depthTargetInfo   = m_impl->depthTargetContext(frameBuffer);
    auto stencilTargetInfo = m_impl->stencilTargetContext(frameBuffer);

    // Build up the rendering info to begin the render pass.
    VkRect2D renderArea = { 0, 0, static_cast<UInt32>(frameBuffer.size().width()), static_cast<UInt32>(frameBuffer.size().height()) };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
        .renderArea = renderArea,
        .layerCount = 1,
        .colorAttachmentCount = static_cast<UInt32>(colorTargetInfos.size()),
        .pColorAttachments = colorTargetInfos.data(),
        .pDepthAttachment = depthTargetInfo.has_value() ? &depthTargetInfo.value() : nullptr,
        .pStencilAttachment = stencilTargetInfo.has_value() ? &stencilTargetInfo.value() : nullptr
    };

    // Begin the command recording on the frame buffers command buffer. Before we can do that, we need to make sure it has not being executed anymore.
    auto primaryCommandBuffer = m_impl->getPrimaryCommandBuffer(frameBuffer);
    primaryCommandBuffer->begin();

    // Declare render pass input transition barriers for render targets and input attachments.
    VulkanBarrier renderTargetBarrier(PipelineStage::None, PipelineStage::RenderTarget), depthStencilBarrier(PipelineStage::None, PipelineStage::DepthStencil);

    std::ranges::for_each(m_impl->m_renderTargets, [&renderTargetBarrier, &depthStencilBarrier, &frameBuffer](const RenderTarget& renderTarget) {
        auto& image = frameBuffer[renderTarget];

        if (renderTarget.type() == RenderTargetType::DepthStencil)
            depthStencilBarrier.transition(image, ResourceAccess::None, ResourceAccess::DepthStencilWrite, ImageLayout::DepthRead, ImageLayout::DepthWrite);
        else
            renderTargetBarrier.transition(image, ResourceAccess::None, ResourceAccess::RenderTarget, ImageLayout::ShaderResource, ImageLayout::RenderTarget);
    });

    // If the present target is multi-sampled, transition the back buffer image into resolve state.
    const auto& backBufferImage = m_impl->m_swapChain.image();
    bool requiresResolve = this->hasPresentTarget() && frameBuffer[*m_impl->m_presentTarget].samples() > MultiSamplingLevel::x1;

    if (requiresResolve)
    {
        VulkanBarrier resolveBarrier(PipelineStage::None, PipelineStage::Resolve);
        resolveBarrier.transition(backBufferImage, ResourceAccess::None, ResourceAccess::ResolveWrite, ImageLayout::Undefined, ImageLayout::ResolveDestination);
        primaryCommandBuffer->barrier(resolveBarrier);
    }

    primaryCommandBuffer->barrier(renderTargetBarrier);
    primaryCommandBuffer->barrier(depthStencilBarrier);
    
    if (!this->name().empty())
        m_impl->m_queue->beginDebugRegion(std::format("{0} Render Pass", this->name()));

    // Begin the render pass on the primary command buffer.
    ::vkCmdBeginRendering(std::as_const(*primaryCommandBuffer).handle(), &renderingInfo);
    std::ranges::for_each(m_impl->getSecondaryCommandBuffers(frameBuffer), [this](auto& commandBuffer) { commandBuffer->begin(*this); });

    // Publish beginning event.
    this->beginning(this, { frameBuffer });
}

UInt64 VulkanRenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    // Publish ending event.
    this->ending(this, { });

    // Get frame buffer and swap chain references.
    auto& frameBuffer = *m_impl->m_activeFrameBuffer;
    const auto& swapChain = m_impl->m_swapChain;

    // End secondary command buffers and end rendering.
    auto primaryCommandBuffer = m_impl->getPrimaryCommandBuffer(frameBuffer);
    auto secondaryHandles = m_impl->getSecondaryCommandBuffers(frameBuffer) |
        std::views::transform([](auto commandBuffer) { commandBuffer->end(); return std::as_const(*commandBuffer).handle(); }) |
        std::ranges::to<Array<VkCommandBuffer>>();
    ::vkCmdExecuteCommands(std::as_const(*primaryCommandBuffer).handle(), static_cast<UInt32>(secondaryHandles.size()), secondaryHandles.data());
    ::vkCmdEndRendering(std::as_const(*primaryCommandBuffer).handle());

    // If the present target is multi-sampled, we need to resolve it to the back buffer.
    const auto& backBufferImage = m_impl->m_swapChain.image();
    bool requiresResolve = this->hasPresentTarget() && frameBuffer[*m_impl->m_presentTarget].samples() > MultiSamplingLevel::x1;

    // Transition the present and depth/stencil views.
    VulkanBarrier renderTargetBarrier(PipelineStage::RenderTarget, PipelineStage::None), depthStencilBarrier(PipelineStage::DepthStencil, PipelineStage::None),
        resolveBarrier(PipelineStage::RenderTarget, PipelineStage::None), presentBarrier(PipelineStage::RenderTarget, PipelineStage::Transfer);
    std::ranges::for_each(m_impl->m_renderTargets, [&](const RenderTarget& renderTarget) {
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
                resolveBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::None, ImageLayout::RenderTarget, ImageLayout::ShaderResource);
            else
                presentBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::TransferRead, ImageLayout::RenderTarget, ImageLayout::CopySource);

            break;
        }
    });

    primaryCommandBuffer->barrier(renderTargetBarrier);
    primaryCommandBuffer->barrier(depthStencilBarrier);
    primaryCommandBuffer->barrier(presentBarrier);
    primaryCommandBuffer->barrier(resolveBarrier);

    // Add another barrier for the back buffer image, if required.
    if (requiresResolve)
    {
        // Transition the resolved swap chain back buffer image into a present state.
        VulkanBarrier presentBarrier(PipelineStage::Resolve, PipelineStage::None);
        presentBarrier.transition(backBufferImage, ResourceAccess::ResolveWrite, ResourceAccess::None, ImageLayout::ResolveDestination, ImageLayout::Present);
        primaryCommandBuffer->barrier(presentBarrier);
    }
    else if (this->hasPresentTarget())
    {
        // Copy the contents from the frame buffer image into the swap chain back buffer.
        VulkanBarrier beginPresentBarrier(PipelineStage::None, PipelineStage::Transfer);
        beginPresentBarrier.transition(backBufferImage, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
        primaryCommandBuffer->barrier(beginPresentBarrier);

        auto& presentTarget = frameBuffer[*m_impl->m_presentTarget];
        primaryCommandBuffer->transfer(presentTarget, backBufferImage);

        VulkanBarrier endPresentBarrier(PipelineStage::Transfer, PipelineStage::None);
        endPresentBarrier.transition(presentTarget, ResourceAccess::TransferRead, ResourceAccess::None, ImageLayout::CopySource, ImageLayout::ShaderResource);
        endPresentBarrier.transition(backBufferImage, ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::Present);
        primaryCommandBuffer->barrier(endPresentBarrier);
    }
    
    // Submit and store the fence.
    auto fence = m_impl->m_queue->submit(primaryCommandBuffer);

    if (!this->name().empty())
        m_impl->m_queue->endDebugRegion();

    // If one render target is a present target, we can present it on the swap chain.
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

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const String& name) noexcept :
    VulkanRenderPassBuilder(device, 1, name)
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name) noexcept :
    RenderPassBuilder(UniquePtr<VulkanRenderPass>(new VulkanRenderPass(device, name)))
{
    m_state.commandBufferCount = commandBuffers;
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

void VulkanRenderPassBuilder::build()
{
    auto instance = this->instance();
    
    if (m_state.commandQueue != nullptr)
        instance->m_impl->m_queue = m_state.commandQueue;
    
    instance->m_impl->mapRenderTargets(m_state.renderTargets);
    instance->m_impl->mapInputAttachments(m_state.inputAttachments);
    instance->m_impl->m_inputAttachmentSamplerBinding = m_state.inputAttachmentSamplerBinding;
    instance->m_impl->m_secondaryCommandBufferCount = m_state.commandBufferCount;
}

RenderPassDependency VulkanRenderPassBuilder::makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget)
{
    return RenderPassDependency(renderTarget, binding);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)