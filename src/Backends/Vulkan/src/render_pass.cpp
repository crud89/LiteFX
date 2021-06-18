#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl : public Implement<VulkanRenderPass> {
public:
    friend class VulkanRenderPassBuilder;
    friend class VulkanRenderPass;

private:
    Array<UniquePtr<VulkanRenderPipeline>> m_pipelines;
    Array<RenderTarget> m_renderTargets;
    Array<VulkanInputAttachmentMapping> m_inputAttachments;
    Array<UniquePtr<VulkanFrameBuffer>> m_frameBuffers;
    const VulkanFrameBuffer* m_activeFrameBuffer = nullptr;
    Array<VkClearValue> m_clearValues;
    UInt32 m_backBuffer{ 0 };

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, Span<RenderTarget> renderTargets, Span<VulkanInputAttachmentMapping> inputAttachments) :
        base(parent)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);
    }

    VulkanRenderPassImpl(VulkanRenderPass* parent) :
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

    void mapInputAttachments(Span<VulkanInputAttachmentMapping> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
        //std::ranges::sort(m_inputAttachments, [this](const VulkanInputAttachmentMapping& a, const VulkanInputAttachmentMapping& b) { return a.location() < b.location(); });
        std::sort(std::begin(m_inputAttachments), std::end(m_inputAttachments), [](const VulkanInputAttachmentMapping& a, const VulkanInputAttachmentMapping& b) { return a.location() < b.location(); });
    }

public:
    VkRenderPass initialize()
    {
        // Setup the attachments.
        Array<VkAttachmentDescription> attachments;
        Array<VkAttachmentReference> inputAttachments;
        Array<VkAttachmentReference> outputAttachments; // Contains all output attachments, except the depth/stencil target.
        Optional<VkAttachmentReference> depthTarget, presentTarget;

        // Map input attachments.
        std::ranges::for_each(m_inputAttachments, [&, i = 0](const VulkanInputAttachmentMapping& inputAttachment) mutable {
            UInt32 currentIndex = i++;

            if (inputAttachment.location() != currentIndex)
                throw InvalidArgumentException("No input attachment is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

            VkAttachmentDescription attachment{};
            attachment.format = getFormat(inputAttachment.renderTarget().format());
            attachment.samples = getSamples(inputAttachment.renderTarget().samples());
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            // Add a clear value, so that the indexing stays valid.
            m_clearValues.push_back(VkClearValue{ });

            switch (inputAttachment.renderTarget().type()) 
            {
            case RenderTargetType::Present: [[unlikely]]
                throw InvalidArgumentException("The render pass input attachment at location {0} maps to a present render target, which can not be used as input attachment.", currentIndex);
            case RenderTargetType::Color:
                attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                inputAttachments.push_back({ static_cast<UInt32>(currentIndex), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                attachments.push_back(attachment);
                break;
            case RenderTargetType::DepthStencil:
                if (::hasDepth(inputAttachment.renderTarget().format()) && ::hasStencil(inputAttachment.renderTarget().format())) [[likely]]
                    attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                else if (::hasDepth(inputAttachment.renderTarget().format()))
                    attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                else if (::hasStencil(inputAttachment.renderTarget().format()))
                    attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                else [[unlikely]]
                {
                    LITEFX_WARNING(VULKAN_LOG, "The depth/stencil input attachment at location {0} does not have a valid depth/stencil format ({1}). Falling back to VK_IMAGE_LAYOUT_GENERAL.", currentIndex, inputAttachment.renderTarget().format());
                    attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                }

                inputAttachments.push_back({ static_cast<UInt32>(currentIndex), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                attachments.push_back(attachment);
                break;
            }
        });

        // Map the render targets.
        std::ranges::for_each(m_renderTargets, [&, i = 0](const RenderTarget& renderTarget) mutable {
            UInt32 currentIndex = i++;

            if (renderTarget.location() != currentIndex)
                throw InvalidArgumentException("No render target is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

            if ((renderTarget.type() == RenderTargetType::DepthStencil && depthTarget.has_value())) [[unlikely]]
                throw InvalidArgumentException("The depth/stencil target at location {0} cannot be mapped. Another depth/stencil target is already bound to location {1} and only one is allowed.", renderTarget.location(), depthTarget->attachment);
            else if (renderTarget.type() == RenderTargetType::Present && presentTarget.has_value()) [[unlikely]]
                throw InvalidArgumentException("The present target at location {0} cannot be mapped. Another present target is already bound to location {1} and only one is allowed.", renderTarget.location(), presentTarget->attachment);
            else [[likely]]
            {
                VkAttachmentDescription attachment{};
                attachment.format = getFormat(renderTarget.format());
                attachment.samples = getSamples(renderTarget.samples());
                attachment.loadOp = renderTarget.clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilLoadOp = renderTarget.clearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.storeOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilStoreOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;

                // Add a clear values (even if it's unused).
                if (renderTarget.clearBuffer() || renderTarget.clearStencil())
                    m_clearValues.push_back(VkClearValue{ renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() });
                else
                    m_clearValues.push_back(VkClearValue{ });

                switch (renderTarget.type())
                {
                case RenderTargetType::Color:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    outputAttachments.push_back({ static_cast<UInt32>(currentIndex + inputAttachments.size()), attachment.finalLayout });
                    break;
                case RenderTargetType::DepthStencil:
                    if (::hasDepth(renderTarget.format()) && ::hasStencil(renderTarget.format())) [[likely]]
                        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    else if (::hasDepth(renderTarget.format()))
                        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    else if (::hasStencil(renderTarget.format()))
                        attachment.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                    else [[unlikely]]
                    {
                        LITEFX_WARNING(VULKAN_LOG, "The depth/stencil render target at location {0} does not have a valid depth/stencil format ({1}). Falling back to VK_IMAGE_LAYOUT_GENERAL.", currentIndex, renderTarget.format());
                        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }

                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    depthTarget = VkAttachmentReference{ static_cast<UInt32>(currentIndex + inputAttachments.size()), attachment.finalLayout };
                    break;
                case RenderTargetType::Present:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    presentTarget = VkAttachmentReference { static_cast<UInt32>(currentIndex + inputAttachments.size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
                    outputAttachments.push_back(presentTarget.value());
                    break;
                }

                attachments.push_back(attachment);
            }
        });

        // Setup the sub-pass.
        VkSubpassDescription subPass{};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = static_cast<UInt32>(outputAttachments.size());
        subPass.pColorAttachments = outputAttachments.data();
        subPass.pDepthStencilAttachment = depthTarget.has_value() ? &depthTarget.value() : nullptr;
        subPass.inputAttachmentCount = static_cast<UInt32>(inputAttachments.size());
        subPass.pInputAttachments = inputAttachments.data();

        // Define an external sub-pass dependency, if there are input attachments to synchronize with.
        Array<VkSubpassDependency> dependencies;

        if (!m_inputAttachments.empty())
        {
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency.dstSubpass = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            dependencies.push_back(dependency);
        }

        // Setup render pass state.
        VkRenderPassCreateInfo renderPassState{};
        renderPassState.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassState.attachmentCount = static_cast<UInt32>(attachments.size());
        renderPassState.pAttachments = attachments.data();
        renderPassState.subpassCount = 1;
        renderPassState.pSubpasses = &subPass;
        renderPassState.dependencyCount = static_cast<UInt32>(dependencies.size());
        renderPassState.pDependencies = dependencies.data();

        // Create the render pass.
        VkRenderPass renderPass;
        raiseIfFailed<RuntimeException>(::vkCreateRenderPass(m_parent->getDevice()->handle(), &renderPassState, nullptr, &renderPass), "Unable to create render pass.");

        return renderPass;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<VulkanInputAttachmentMapping> inputAttachments) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, renderTargets, inputAttachments)), VulkanRuntimeObject<VulkanDevice>(device, &device), Resource<VkRenderPass>(nullptr)
{
    this->handle() = m_impl->initialize();

    // Initialize the frame buffers.
    m_impl->m_frameBuffers.resize(this->getDevice()->swapChain().buffers());
    std::ranges::generate(m_impl->m_frameBuffers, [this, i = 0]() mutable { return makeUnique<VulkanFrameBuffer>(*this, i++, this->parent().swapChain().renderArea()); });
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanRenderPassImpl>(this)), VulkanRuntimeObject<VulkanDevice>(device, &device), Resource<VkRenderPass>(nullptr)
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept
{
    ::vkDestroyRenderPass(this->getDevice()->handle(), this->handle(), nullptr);
}

const VulkanFrameBuffer& VulkanRenderPass::frameBuffer(const UInt32& buffer) const
{
    if (buffer >= m_impl->m_frameBuffers.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("The buffer {0} does not exist in this render pass. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

    return *m_impl->m_frameBuffers[buffer].get();
}

const VulkanFrameBuffer& VulkanRenderPass::activeFrameBuffer() const
{
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("No frame buffer is active, since the render pass has not begun.");

    return *m_impl->m_activeFrameBuffer;
}

Array<const VulkanFrameBuffer*> VulkanRenderPass::frameBuffers() const noexcept
{
    return m_impl->m_frameBuffers | 
        std::views::transform([](const UniquePtr<VulkanFrameBuffer>& frameBuffer) { return frameBuffer.get(); }) |
        ranges::to<Array<const VulkanFrameBuffer*>>();
}

const VulkanRenderPipeline& VulkanRenderPass::pipeline(const UInt32& id) const
{
    if (auto match = std::ranges::find_if(m_impl->m_pipelines, [&id](const UniquePtr<VulkanRenderPipeline>& pipeline) { return pipeline->id() == id; }); match != m_impl->m_pipelines.end())
        return *match->get();
    
    throw InvalidArgumentException("No render pipeline with the ID {0} is contained by this render pass.", id);
}

Array<const VulkanRenderPipeline*> VulkanRenderPass::pipelines() const noexcept
{
    return m_impl->m_pipelines | 
        std::views::transform([](const UniquePtr<VulkanRenderPipeline>& pipeline) { return pipeline.get(); }) | ranges::to<Array<const VulkanRenderPipeline*>>() |
        ranges::to<Array<const VulkanRenderPipeline*>>();
}

const RenderTarget& VulkanRenderPass::renderTarget(const UInt32& location) const
{
    if (auto match = std::ranges::find_if(m_impl->m_renderTargets, [&location](const RenderTarget& renderTarget) { return renderTarget.location() == location; }); match != m_impl->m_renderTargets.end())
        return *match;

    throw ArgumentOutOfRangeException("No render target is mapped to location {0} in this render pass.", location);
}

Span<const RenderTarget> VulkanRenderPass::renderTargets() const noexcept
{
    return m_impl->m_renderTargets;
}

bool VulkanRenderPass::hasPresentTarget() const noexcept
{
    return std::ranges::any_of(m_impl->m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; });
}

Span<const VulkanInputAttachmentMapping> VulkanRenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

void VulkanRenderPass::begin(const UInt32& buffer)
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

    // Begin the render pass.
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->handle();
    renderPassInfo.framebuffer = frameBuffer->handle();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent.width = static_cast<UInt32>(frameBuffer->getWidth());
    renderPassInfo.renderArea.extent.height = static_cast<UInt32>(frameBuffer->getHeight());
    renderPassInfo.clearValueCount = m_impl->m_clearValues.size();
    renderPassInfo.pClearValues = m_impl->m_clearValues.data();

    ::vkCmdBeginRenderPass(frameBuffer->commandBuffer().handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    auto frameBuffer = m_impl->m_activeFrameBuffer;
    
    // End the render pass and the command buffer recording.
    ::vkCmdEndRenderPass(frameBuffer->commandBuffer().handle());
    frameBuffer->commandBuffer().end(false);

    // Submit the command buffer.
    if (!this->hasPresentTarget())
        frameBuffer->commandBuffer().submit();
    else
    {
        Array<VkSemaphore> waitForSemaphores = { this->getDevice()->swapChain().semaphore() };
        Array<VkPipelineStageFlags> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        Array<VkSemaphore> signalSemaphores = { frameBuffer->semaphore() };
        frameBuffer->commandBuffer().submit(waitForSemaphores, waitForStages, signalSemaphores, false);

        // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = signalSemaphores.size();
        presentInfo.pWaitSemaphores = signalSemaphores.data();
        presentInfo.pImageIndices = &m_impl->m_backBuffer;
        presentInfo.pResults = nullptr;

        VkSwapchainKHR swapChains[] = { this->getDevice()->swapChain().handle() };
        presentInfo.pSwapchains = swapChains;
        presentInfo.swapchainCount = 1;

        raiseIfFailed<RuntimeException>(::vkQueuePresentKHR(this->getDevice()->graphicsQueue().handle(), &presentInfo), "Unable to present swap chain.");
    }

    // Reset the frame buffer.
    m_impl->m_activeFrameBuffer = nullptr;
}

void VulkanRenderPass::resizeFrameBuffers(const Size2d& renderArea)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<VulkanFrameBuffer>& frameBuffer) { frameBuffer->resize(renderArea); });
}

void VulkanRenderPass::updateAttachments(const VulkanDescriptorSet& descriptorSet) const
{
    const auto backBuffer = m_impl->m_backBuffer;

    std::ranges::for_each(m_impl->m_inputAttachments, [&descriptorSet, &backBuffer](const VulkanInputAttachmentMapping& inputAttachment) {
#ifndef NDEBUG
        if (inputAttachment.inputAttachmentSource() == nullptr)
            throw RuntimeException("No source render pass has been specified for the input attachment mapped to location {0}.", inputAttachment.location());
#endif

        descriptorSet.attach(inputAttachment.location(), inputAttachment.inputAttachmentSource()->frameBuffer(backBuffer).image(inputAttachment.renderTarget().location()));
    });
}

VulkanRenderPipelineBuilder VulkanRenderPass::makePipeline(const UInt32& id, const String& name) const noexcept
{
    return VulkanRenderPipelineBuilder(*this, id, name);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPassBuilder::VulkanRenderPassBuilderImpl : public Implement<VulkanRenderPassBuilder> {
public:
    friend class VulkanRenderPassBuilder;
    friend class VulkanRenderPass;

private:
    Array<UniquePtr<VulkanRenderPipeline>> m_pipelines;
    Array<VulkanInputAttachmentMapping> m_inputAttachments;
    Array<RenderTarget> m_renderTargets;

public:
    VulkanRenderPassBuilderImpl(VulkanRenderPassBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanRenderPassBuilderImpl>(this)), RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass>(UniquePtr<VulkanRenderPass>(new VulkanRenderPass(device)))
{
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

UniquePtr<VulkanRenderPass> VulkanRenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->mapRenderTargets(m_impl->m_renderTargets);
    instance->m_impl->mapInputAttachments(m_impl->m_inputAttachments);
    instance->handle() = instance->m_impl->initialize();

    // Initialize the frame buffers.
    instance->m_impl->m_frameBuffers.resize(instance->getDevice()->swapChain().buffers());
    std::ranges::generate(instance->m_impl->m_frameBuffers, [&instance, i = 0]() mutable { return makeUnique<VulkanFrameBuffer>(*instance, i++, instance->parent().swapChain().renderArea()); });

    return RenderPassBuilder::go();
}

void VulkanRenderPassBuilder::use(RenderTarget&& target)
{
    m_impl->m_renderTargets.push_back(std::move(target));
}

void VulkanRenderPassBuilder::use(VulkanInputAttachmentMapping&& attachment)
{
    m_impl->m_inputAttachments.push_back(std::move(attachment));
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, samples, clearValues, clear, clearStencil, isVolatile);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    m_impl->m_renderTargets.push_back(RenderTarget(location, type, format, clear, clearValues, clearStencil, samples, isVolatile));
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(VulkanInputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(output, static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, samples, clearValues, clear, clearStencil, isVolatile);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(VulkanInputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    auto renderTarget = RenderTarget(location, type, format, clear, clearValues, clearStencil, samples, isVolatile);
    output = std::move(VulkanInputAttachmentMapping(*this->instance(), renderTarget, location));
    m_impl->m_renderTargets.push_back(renderTarget);
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::inputAttachment(const VulkanInputAttachmentMapping& inputAttachment)
{
    m_impl->m_inputAttachments.push_back(inputAttachment);
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const UInt32& outputLocation)
{
    m_impl->m_inputAttachments.push_back(VulkanInputAttachmentMapping(renderPass, renderPass.renderTarget(outputLocation), inputLocation));
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::inputAttachment(const UInt32& inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget)
{
    m_impl->m_inputAttachments.push_back(VulkanInputAttachmentMapping(renderPass, renderTarget, inputLocation));
    return *this;
}