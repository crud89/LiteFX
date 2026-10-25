#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl {
public:
    friend class VulkanRenderPassBuilder;
    friend class VulkanRenderPass;

private:
    Array<RenderTarget> m_renderTargets;
    Array<RenderPassDependency> m_inputAttachments;
    Dictionary<const IFrameBuffer*, size_t> m_frameBufferTokens;
    Array<size_t> m_swapChainTokens;
    Dictionary<const IFrameBuffer*, Array<SharedPtr<VulkanCommandBuffer>>> m_secondaryCommandBuffers;
    SharedPtr<const VulkanCommandBuffer> m_activeCommandBuffer{};
    bool m_external{ false };
    Dictionary<const IVulkanImage*, VkImageView> m_swapChainViews;
    UInt32 m_secondaryCommandBufferCount = 0;
    SharedPtr<const VulkanFrameBuffer> m_activeFrameBuffer = nullptr;
    const RenderTarget* m_presentTarget = nullptr;
    const RenderTarget* m_depthStencilTarget = nullptr;
    Optional<DescriptorBindingPoint> m_inputAttachmentSamplerBinding{ };
    SharedPtr<const VulkanDevice> m_device;
    SharedPtr<const VulkanQueue> m_queue;
    bool m_onDefaultGraphicsQueue = false;
    UInt32 m_viewMask = 0x00;

public:
    VulkanRenderPassImpl(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
        m_secondaryCommandBufferCount(secondaryCommandBuffers), m_inputAttachmentSamplerBinding(inputAttachmentSamplerBinding), m_device(device.shared_from_this()), m_queue(queue.shared_from_this()), m_onDefaultGraphicsQueue(std::addressof(queue) == std::addressof(device.defaultQueue(QueueType::Graphics))), m_viewMask(viewMask)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);

        if (secondaryCommandBuffers == 0) [[unlikely]]
            LITEFX_WARNING(VULKAN_LOG, "Secondary command buffer count for this render pass is 0, which makes it prevents recording draw commands to this render pass.");
    }

    VulkanRenderPassImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this()), m_queue(device.defaultQueue(QueueType::Graphics).shared_from_this()), m_onDefaultGraphicsQueue(true)
    {
    }

    VulkanRenderPassImpl(VulkanRenderPassImpl&&) noexcept = default;
    VulkanRenderPassImpl(const VulkanRenderPassImpl&) = delete;
    VulkanRenderPassImpl& operator=(VulkanRenderPassImpl&&) noexcept = default;
    VulkanRenderPassImpl& operator=(const VulkanRenderPassImpl&) = delete;

    ~VulkanRenderPassImpl() noexcept
    {
        // Stop listening to frame buffer events.
        for (auto [frameBuffer, token] : m_frameBufferTokens)
            frameBuffer->released -= token;

        // Stop listening to swap chain events.
        for (auto token : m_swapChainTokens)
            m_device->swapChain().reseted -= token;

        // Release swap chain image views if there are any.
        for (auto view : m_swapChainViews | std::views::values)
            ::vkDestroyImageView(m_device->handle(), view, nullptr);
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

        // Listen to swap chain resets in order to clear back buffer image views.
        if (m_presentTarget != nullptr)
            m_swapChainTokens.push_back(m_device->swapChain().reseted.add(std::bind(&VulkanRenderPassImpl::onSwapChainReset, this, std::placeholders::_1, std::placeholders::_2)));
    }

    void mapInputAttachments(Span<RenderPassDependency> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
    }

    void registerFrameBuffer([[maybe_unused]] const VulkanRenderPass& renderPass, const VulkanFrameBuffer& frameBuffer)
    {
        // If the frame buffer is not yet registered, do so by listening for its release.
        auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);

        if (!m_frameBufferTokens.contains(interfacePointer)) [[unlikely]]
            m_frameBufferTokens[interfacePointer] = frameBuffer.released.add(std::bind(&VulkanRenderPassImpl::onFrameBufferRelease, this, std::placeholders::_1, std::placeholders::_2));

        // Store the active frame buffer pointer.
        m_activeFrameBuffer = frameBuffer.shared_from_this();
    }

    void onFrameBufferRelease(const void* sender, const IFrameBuffer::ReleasedEventArgs& /*args*/)
    {
        // Obtain the interface pointer and release all resources bound to the frame buffer.
        auto interfacePointer = static_cast<const IFrameBuffer*>(sender);

        if (static_cast<const IFrameBuffer*>(m_activeFrameBuffer.get()) == interfacePointer) [[unlikely]]
            throw RuntimeException("A frame buffer that is currently in use on a render pass cannot be released.");

        m_secondaryCommandBuffers.erase(interfacePointer);

        // Release the token.
        m_frameBufferTokens.erase(interfacePointer);
    }

    void onSwapChainReset([[maybe_unused]] const void* sender, const ISwapChain::ResetEventArgs& /*args*/)
    {
        // Release swap chain image views if there are any, so that they need to be re-created with the next context.
        for (auto view : m_swapChainViews | std::views::values)
            ::vkDestroyImageView(m_device->handle(), view, nullptr);

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
                    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                    .loadOp = renderTarget.clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = { .color = { .float32 = { renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() } } }
                };

                // Get the image and check if we need to resolve it.
                if (renderTarget.type() == RenderTargetType::Present && frameBuffer[renderTarget].samples() > MultiSamplingLevel::x1) // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
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

                        VkImageView imageView{};
                        raiseIfFailed(::vkCreateImageView(m_device->handle(), &createInfo, nullptr, &imageView), "Unable to create image view for swap chain back buffer.");
                        return imageView;
                    };

                    // Create an image view if we don't already have one for the swap chain image.
                    auto& backBuffer = m_device->swapChain().image();

                    if (!m_swapChainViews.contains(&backBuffer))
                        m_swapChainViews[&backBuffer] = getImageView(backBuffer);

                    attachmentInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                    attachmentInfo.resolveImageView = m_swapChainViews.at(&backBuffer);
                    attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
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
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = m_depthStencilTarget->clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = m_depthStencilTarget->isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { .depthStencil = { .depth = m_depthStencilTarget->clearValues().x(), .stencil = 0 } }
        };
    }

    Optional<VkRenderingAttachmentInfo> stencilTargetContext(const VulkanFrameBuffer& frameBuffer)
    {
        if (m_depthStencilTarget == nullptr || !::hasStencil(m_depthStencilTarget->format()))
            return std::nullopt;

        return VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = frameBuffer.imageView(*m_depthStencilTarget),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = m_depthStencilTarget->clearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = m_depthStencilTarget->isVolatile() ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { .depthStencil = { .depth = 0.0f, .stencil = static_cast<UInt32>(m_depthStencilTarget->clearValues().y()) } }
        };
    }

    inline Array<SharedPtr<VulkanCommandBuffer>>& getSecondaryCommandBuffers(const VulkanFrameBuffer& frameBuffer)
    {
        return m_secondaryCommandBuffers.at(static_cast<const IFrameBuffer*>(&frameBuffer));
    }

    void begin(const VulkanRenderPass& renderPass, const VulkanFrameBuffer& frameBuffer, const VulkanCommandBuffer& primaryCommandBuffer, bool external) 
    {
        // Only begin, if we are currently not running.
        if (m_activeFrameBuffer != nullptr) [[unlikely]]
            throw RuntimeException("Unable to begin a render pass, that is already running. End the current pass first.");

        // Register the frame buffer.
        registerFrameBuffer(renderPass, frameBuffer);

        try
        {
            // Initialize the render pass context.
            auto colorTargetInfos  = colorTargetContext(frameBuffer);
            auto depthTargetInfo   = depthTargetContext(frameBuffer);
            auto stencilTargetInfo = stencilTargetContext(frameBuffer);

            // Build up the rendering info to begin the render pass.
            VkRect2D renderArea = { { 0, 0 }, { static_cast<UInt32>(frameBuffer.size().width()), static_cast<UInt32>(frameBuffer.size().height()) } };

            VkRenderingInfo renderingInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
                .renderArea = renderArea,
                .layerCount = 1,
                .viewMask = m_viewMask,
                .colorAttachmentCount = static_cast<UInt32>(colorTargetInfos.size()),
                .pColorAttachments = colorTargetInfos.data(),
                .pDepthAttachment = depthTargetInfo.has_value() ? &depthTargetInfo.value() : nullptr,
                .pStencilAttachment = stencilTargetInfo.has_value() ? &stencilTargetInfo.value() : nullptr
            };

            // Declare render pass input transition barriers for render targets and input attachments.
            auto syncBefore = external ? PipelineStage::All : PipelineStage::None;
            auto accessBefore = external ? ResourceAccess::Common : ResourceAccess::None;
            auto layoutBefore = [external](bool clear) { return clear && !external ? ImageLayout::Undefined : ImageLayout::Common; };

            VulkanBarrier
                renderTargetBarrier(syncBefore, PipelineStage::RenderTarget),
                depthStencilBarrier(syncBefore, PipelineStage::DepthStencil),
                inputAttachmentBarrier(syncBefore, PipelineStage::Pixel);

            std::ranges::for_each(m_renderTargets, [&](const RenderTarget& renderTarget) {
                auto& image = frameBuffer[renderTarget]; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

                if (renderTarget.type() == RenderTargetType::DepthStencil) {
                    const auto withDepth = ::hasDepth(renderTarget.format());
            
                    if (withDepth)
                        depthStencilBarrier.transition(image, 0u, image.levels(), 0u, image.layers(), 0u, accessBefore, ResourceAccess::DepthStencil, layoutBefore(renderTarget.clearBuffer()), ImageLayout::DepthWrite);
                    if (::hasStencil(renderTarget.format()))
                        depthStencilBarrier.transition(image, 0u, image.levels(), 0u, image.layers(), withDepth ? 1u : 0u, accessBefore, ResourceAccess::DepthStencil, layoutBefore(renderTarget.clearStencil()), ImageLayout::DepthWrite);
                }
                else
                    renderTargetBarrier.transition(image, accessBefore, ResourceAccess::RenderTarget, layoutBefore(renderTarget.clearBuffer()), ImageLayout::RenderTarget);
            });

            std::ranges::for_each(m_inputAttachments, [&](const RenderPassDependency& dependency) {
                inputAttachmentBarrier.transition(frameBuffer[dependency.renderTarget()], accessBefore, ResourceAccess::ShaderRead, ImageLayout::Common, ImageLayout::ShaderResource); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            });

            primaryCommandBuffer.barrier(renderTargetBarrier);
            primaryCommandBuffer.barrier(depthStencilBarrier);
            primaryCommandBuffer.barrier(inputAttachmentBarrier);

            // If the present target is multi-sampled, transition the back buffer image into resolve state.
            const auto& backBufferImage = m_device->swapChain().image();
            bool requiresResolve{ renderPass.hasPresentTarget() && frameBuffer[*m_presentTarget].samples() > MultiSamplingLevel::x1 }; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

            if (requiresResolve)
            {
                VulkanBarrier resolveBarrier(PipelineStage::None, PipelineStage::Resolve);
                resolveBarrier.transition(backBufferImage, ResourceAccess::None, ResourceAccess::ResolveWrite, ImageLayout::Undefined, ImageLayout::ResolveDestination);
                primaryCommandBuffer.barrier(resolveBarrier);
            }
    
            // Create secondary command buffers.
            const auto frameBufferPointer = static_cast<const IFrameBuffer*>(&frameBuffer);
            m_secondaryCommandBuffers[frameBufferPointer] = std::views::iota(0u, m_secondaryCommandBufferCount)
                | std::views::transform([&](UInt32 i) { 
#ifndef NDEBUG
                        auto commandBuffer = VulkanCommandBuffer::create(*m_queue, renderPass);
                        m_device->setDebugName(std::as_const(*commandBuffer).handle(), VK_OBJECT_TYPE_COMMAND_BUFFER, std::format("{0} Secondary Commands {1}", renderPass.name(), i).c_str());
                        return commandBuffer;
#else
                        return VulkanCommandBuffer::create(*m_queue, renderPass);
#endif
                    })
                | std::ranges::to<Array<SharedPtr<VulkanCommandBuffer>>>();

            // Begin a debug region, if we're not using an external command buffer, since those are grouped per submission on the queue.
            if (!external && !renderPass.name().empty())
                m_queue->beginDebugRegion(std::format("{0} Render Pass", renderPass.name()));

            // Begin the render pass on the primary command buffer.
            ::vkCmdBeginRendering(primaryCommandBuffer.handle(), &renderingInfo);

            // Publish beginning event.
            renderPass.beginning(&renderPass, { frameBuffer });

            // Store the primary command buffer.
            m_activeCommandBuffer = primaryCommandBuffer.shared_from_this();
            m_external = external;
        }
        catch (...)
        {
            m_activeFrameBuffer.reset();
            m_external = false;
            throw;
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    VulkanRenderPass(device, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    VulkanRenderPass(device, name, device.defaultQueue(QueueType::Graphics), renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    m_impl(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding, UInt32 secondaryCommandBuffers, UInt32 viewMask) :
    VulkanRenderPass(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask)
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name) :
    m_impl(device)
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::~VulkanRenderPass() noexcept = default;

const VulkanDevice& VulkanRenderPass::device() const noexcept
{
    return *m_impl->m_device;
}

SharedPtr<const VulkanFrameBuffer> VulkanRenderPass::activeFrameBuffer() const noexcept
{
    return m_impl->m_activeFrameBuffer;
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

    return m_impl->m_secondaryCommandBuffers.at(m_impl->m_activeFrameBuffer.get()).at(index);
}

Enumerable<SharedPtr<const VulkanCommandBuffer>> VulkanRenderPass::commandBuffers() const
{
    if (m_impl->m_secondaryCommandBufferCount == 0u || m_impl->m_activeFrameBuffer == nullptr)
    {
        static const Array<SharedPtr<const VulkanCommandBuffer>> _empty { };
        return _empty;
    }
    else
    {
        return m_impl->m_secondaryCommandBuffers.at(m_impl->m_activeFrameBuffer.get());
    }
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
        throw ArgumentOutOfRangeException("location", std::make_pair(0uz, m_impl->m_inputAttachments.size()), static_cast<size_t>(location), "The render pass does not contain an input attachment at location {0}.", location);

    return m_impl->m_inputAttachments[location]; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
}

const Optional<DescriptorBindingPoint>& VulkanRenderPass::inputAttachmentSamplerBinding() const noexcept
{
    return m_impl->m_inputAttachmentSamplerBinding;
}

UInt32 VulkanRenderPass::viewMask() const noexcept
{
    return m_impl->m_viewMask;
}

void VulkanRenderPass::begin(const VulkanFrameBuffer& frameBuffer) const
{
    // Create the primary command buffer for the render pass.
    auto primaryCommandBuffer = m_impl->m_queue->createCommandBuffer(false);

#ifndef NDEBUG
    m_impl->m_device->setDebugName(std::as_const(*primaryCommandBuffer).handle(), VK_OBJECT_TYPE_COMMAND_BUFFER, std::format("{0} Primary Commands", this->name()).c_str());
#endif

    m_impl->begin(*this, frameBuffer, *primaryCommandBuffer, false);
}

void VulkanRenderPass::begin(const VulkanFrameBuffer& frameBuffer, const VulkanCommandBuffer& commandBuffer) const
{
    if (commandBuffer.secondary() || !commandBuffer.recording()) [[unlikely]]
        throw InvalidArgumentException("commandBuffer", "The command buffer must be a primary command buffer that is still recording.");

    if (commandBuffer.queue().get() != m_impl->m_queue.get()) [[unlikely]]
        throw InvalidArgumentException("commandBuffer", "The command buffer must be created from the render pass queue.");

    m_impl->begin(*this, frameBuffer, commandBuffer, true);
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
    const auto& swapChain = m_impl->m_device->swapChain();

    // End secondary command buffers and end rendering.
    auto primaryCommandBuffer = m_impl->m_activeCommandBuffer;
    primaryCommandBuffer->execute(m_impl->getSecondaryCommandBuffers(frameBuffer));
    ::vkCmdEndRendering(std::as_const(*primaryCommandBuffer).handle());

    // If the present target is multi-sampled, we need to resolve it to the back buffer.
    const auto& backBufferImage = swapChain.image();
    const auto* presentImage = this->hasPresentTarget() ? &frameBuffer[*m_impl->m_presentTarget] : nullptr; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    bool requiresResolve{ presentImage != nullptr && presentImage->samples() > MultiSamplingLevel::x1 }; 

    // Add another barrier for the back buffer image, if required.
    auto syncAfter = m_impl->m_external ? PipelineStage::All : PipelineStage::None;
    auto accessAfter = m_impl->m_external ? ResourceAccess::Common : ResourceAccess::None;

    if (requiresResolve)
    {
        // Transition the resolved swap chain back buffer image into a present state.
        VulkanBarrier backBufferBarrier(PipelineStage::Resolve, syncAfter);
        backBufferBarrier.transition(backBufferImage, ResourceAccess::ResolveWrite, accessAfter, ImageLayout::ResolveDestination, ImageLayout::Present);
        primaryCommandBuffer->barrier(backBufferBarrier);
    }
    else if (this->hasPresentTarget())
    {
        // Copy the contents from the frame buffer image into the swap chain back buffer.
        VulkanBarrier beginPresentBarrier(PipelineStage::RenderTarget, PipelineStage::Transfer);
        beginPresentBarrier.transition(*presentImage, ResourceAccess::RenderTarget, ResourceAccess::TransferRead, ImageLayout::RenderTarget, ImageLayout::CopySource);
        beginPresentBarrier.transition(backBufferImage, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
        primaryCommandBuffer->barrier(beginPresentBarrier);

        primaryCommandBuffer->transfer(*presentImage, backBufferImage);

        VulkanBarrier endPresentBarrier(PipelineStage::Transfer, syncAfter);
        endPresentBarrier.transition(*presentImage, ResourceAccess::TransferRead, accessAfter, ImageLayout::CopySource, ImageLayout::Common);
        endPresentBarrier.transition(backBufferImage, ResourceAccess::TransferWrite, accessAfter, ImageLayout::CopyDestination, ImageLayout::Present);
        primaryCommandBuffer->barrier(endPresentBarrier);
    }

    // The external path needs to insert a barrier here to make sure render target writes are synchronized with subsequent commands.
    UInt64 fence{ 0ull };

    if (m_impl->m_external)
    {
        VulkanBarrier endBarrier(PipelineStage::RenderTarget | PipelineStage::DepthStencil | PipelineStage::Fragment, PipelineStage::All);
        
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        std::ranges::for_each(m_impl->m_renderTargets, [&](const RenderTarget& renderTarget) {
            if (renderTarget.type() == RenderTargetType::DepthStencil)
                endBarrier.transition(frameBuffer[renderTarget], ResourceAccess::DepthStencilWrite, ResourceAccess::Common, ImageLayout::DepthWrite, ImageLayout::Common);
            else if (renderTarget.type() != RenderTargetType::Present || requiresResolve)
                endBarrier.transition(frameBuffer[renderTarget], ResourceAccess::RenderTarget, ResourceAccess::Common, ImageLayout::RenderTarget, ImageLayout::Common);
        });

        std::ranges::for_each(m_impl->m_inputAttachments, [&](const RenderPassDependency& dependency) {
            endBarrier.transition(frameBuffer[dependency.renderTarget()], ResourceAccess::ShaderRead, ResourceAccess::Common, ImageLayout::ShaderResource, ImageLayout::Common);
        });
        // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

        primaryCommandBuffer->barrier(endBarrier);
    }
    else
    {
        // NOTE: D3D12 transitions the render targets and input attachments here, but in Vulkan we don't need to do this, as the layout is already `Common` and accesses
        //       are synchronized through command buffer submission order (see the queue implementation for details).

        // Submit and store the fence.
        fence = m_impl->m_queue->submit(primaryCommandBuffer);

        if (!this->name().empty())
            m_impl->m_queue->endDebugRegion();

        // If one render target is a present target, we can present it on the swap chain.
        if (m_impl->m_presentTarget != nullptr)
            swapChain.present(fence);
    }

    // Reset the frame and active command buffers.
    m_impl->m_activeFrameBuffer = nullptr;
    m_impl->m_activeCommandBuffer = nullptr;
    m_impl->m_external = false;

    return fence;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const String& name) :
    VulkanRenderPassBuilder(device, 1, name)
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name) :
    RenderPassBuilder(VulkanRenderPass::create(device, name))
{
    this->state().commandBufferCount = commandBuffers;
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

void VulkanRenderPassBuilder::build()
{
    auto instance = this->instance();
    
    if (this->state().commandQueue != nullptr)
        instance->m_impl->m_queue = this->state().commandQueue;
    
    instance->m_impl->mapRenderTargets(this->state().renderTargets);
    instance->m_impl->mapInputAttachments(this->state().inputAttachments);
    instance->m_impl->m_inputAttachmentSamplerBinding = this->state().inputAttachmentSamplerBinding;
    instance->m_impl->m_secondaryCommandBufferCount = this->state().commandBufferCount;
    instance->m_impl->m_viewMask = this->state().viewMask;
}

RenderPassDependency VulkanRenderPassBuilder::makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget)
{
    return { renderTarget, binding };
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)