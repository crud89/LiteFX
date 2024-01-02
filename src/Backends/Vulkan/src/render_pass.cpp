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
    Array<UniquePtr<VulkanRenderPipeline>> m_pipelines;
    Array<RenderTarget> m_renderTargets;
    Array<VulkanInputAttachmentMapping> m_inputAttachments;
    Array<UniquePtr<VulkanFrameBuffer>> m_frameBuffers;
    Array<SharedPtr<VulkanCommandBuffer>> m_primaryCommandBuffers;
    VulkanFrameBuffer* m_activeFrameBuffer = nullptr;
    SharedPtr<const VulkanCommandBuffer> m_activeCommandBuffer;
    Array<VkClearValue> m_clearValues;
    UInt32 m_backBuffer{ 0 };
    MultiSamplingLevel m_samples;
    const VulkanDevice& m_device;
    const VulkanQueue* m_queue;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, MultiSamplingLevel samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
        base(parent), m_samples(samples), m_device(device), m_queue(&queue)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);
    }

    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device) :
        base(parent), m_device(device), m_queue(&device.defaultQueue(QueueType::Graphics))
    {
    }

public:
    void mapRenderTargets(Span<RenderTarget> renderTargets)
    {
        m_renderTargets.assign(std::begin(renderTargets), std::end(renderTargets));
        //std::ranges::sort(m_renderTargets, [this](const RenderTarget& a, const RenderTarget& b) { return a.location() < b.location(); });
        std::sort(std::begin(m_renderTargets), std::end(m_renderTargets), [](const RenderTarget& a, const RenderTarget& b) { return a.location() < b.location(); });

        // TODO: If there is a present target, we need to check if the provided queue can actually present on the surface. Currently, 
        //       we simply check if the queue is the same as the swap chain queue (which is the default graphics queue).
        if (std::ranges::any_of(m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }) &&
            m_queue != std::addressof(m_device.defaultQueue(QueueType::Graphics))) [[unlikely]]
            throw InvalidArgumentException("renderTargets", "A render pass with a present target must be executed on the default graphics queue.");
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
        Optional<VkAttachmentDescription> presentResolveAttachment;

        // Map input attachments.
        std::ranges::for_each(m_inputAttachments, [&, i = 0](const VulkanInputAttachmentMapping& inputAttachment) mutable {
            UInt32 currentIndex = i++;

            if (inputAttachment.location() != currentIndex) [[unlikely]]
                throw InvalidArgumentException("inputAttachments", "No input attachment is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

            VkAttachmentDescription attachment{};
            attachment.format = Vk::getFormat(inputAttachment.renderTarget().format());
            attachment.samples = Vk::getSamples(inputAttachment.inputAttachmentSource()->multiSamplingLevel());
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            // Add a clear value, so that the indexing stays valid.
            m_clearValues.push_back(VkClearValue{ });

            switch (inputAttachment.renderTarget().type()) 
            {
            case RenderTargetType::Present: [[unlikely]]
                throw InvalidArgumentException("inputAttachments", "The render pass input attachment at location {0} maps to a present render target, which can not be used as input attachment.", currentIndex);
            case RenderTargetType::Color:
                attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                inputAttachments.push_back({ static_cast<UInt32>(currentIndex), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                attachments.push_back(attachment);
                break;
            case RenderTargetType::DepthStencil:
                if (::hasDepth(inputAttachment.renderTarget().format()) && ::hasStencil(inputAttachment.renderTarget().format())) [[likely]]
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (::hasDepth(inputAttachment.renderTarget().format()))
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (::hasStencil(inputAttachment.renderTarget().format()))
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }
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

            if (renderTarget.location() != currentIndex) [[unlikely]]
                throw InvalidArgumentException("renderTargets", "No render target is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

            if ((renderTarget.type() == RenderTargetType::DepthStencil && depthTarget.has_value())) [[unlikely]]
                throw InvalidArgumentException("renderTargets", "The depth/stencil target at location {0} cannot be mapped. Another depth/stencil target is already bound to location {1} and only one is allowed.", renderTarget.location(), depthTarget->attachment);
            else if (renderTarget.type() == RenderTargetType::Present && presentTarget.has_value()) [[unlikely]]
                throw InvalidArgumentException("renderTargets", "The present target at location {0} cannot be mapped. Another present target is already bound to location {1} and only one is allowed.", renderTarget.location(), presentTarget->attachment);
            else [[likely]]
            {
                VkAttachmentDescription attachment{};
                attachment.format = Vk::getFormat(renderTarget.format());
                attachment.samples = Vk::getSamples(m_samples);
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
                    attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                    outputAttachments.push_back({ static_cast<UInt32>(currentIndex + inputAttachments.size()), attachment.finalLayout });
                    break;
                case RenderTargetType::DepthStencil:
                    if (::hasDepth(renderTarget.format()) || ::hasStencil(renderTarget.format())) [[likely]]
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    else if (::hasDepth(renderTarget.format()))
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                    else if (::hasStencil(renderTarget.format()))
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
                    else [[unlikely]]
                    {
                        LITEFX_WARNING(VULKAN_LOG, "The depth/stencil render target at location {0} does not have a valid depth/stencil format ({1}). Falling back to VK_IMAGE_LAYOUT_GENERAL.", currentIndex, renderTarget.format());
                        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }

                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    depthTarget = VkAttachmentReference{ static_cast<UInt32>(currentIndex + inputAttachments.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
                    break;
                case RenderTargetType::Present:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                    // If we have a multi-sampled present attachment, we also need to attach a resolve attachment for it.
                    if (m_samples == MultiSamplingLevel::x1)
                        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    else
                    {
                        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                        presentResolveAttachment = VkAttachmentDescription{};
                        presentResolveAttachment->format = attachment.format;
                        presentResolveAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
                        presentResolveAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        presentResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        presentResolveAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        presentResolveAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        presentResolveAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        presentResolveAttachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    }

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
        subPass.pResolveAttachments = nullptr;

        // Add the resolve attachment.
        VkAttachmentReference presentResolveReference = { static_cast<UInt32>(attachments.size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        if (presentResolveAttachment.has_value())
        {
            subPass.pResolveAttachments = &presentResolveReference;
            attachments.push_back(presentResolveAttachment.value());
        }

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
        raiseIfFailed(::vkCreateRenderPass(m_device.handle(), &renderPassState, nullptr, &renderPass), "Unable to create render pass.");

#ifndef NDEBUG
        m_device.setDebugName(*reinterpret_cast<const UInt64*>(&renderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, m_parent->name());
#endif

        return renderPass;
    }

    void initializeFrameBuffers(UInt32 commandBuffers)
    {
        // Initialize the frame buffers.
        this->m_frameBuffers.resize(this->m_device.swapChain().buffers());
        std::ranges::generate(this->m_frameBuffers, [this, &commandBuffers, i = 0]() mutable { 
            auto frameBuffer = makeUnique<VulkanFrameBuffer>(*m_parent, i++, this->m_device.swapChain().renderArea(), commandBuffers);

#ifndef NDEBUG
            m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*frameBuffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, fmt::format("Framebuffer {0}-{1}", m_parent->name(), i));

            auto images = frameBuffer->images();
            int renderTarget = 0;

            for (auto& image : images)
                if (renderTarget < m_renderTargets.size())  // Resolve target is not included in render targets, but appended to the image list in the frame buffer.
                    m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*image).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_renderTargets[renderTarget++].name());
                else
                    m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*image).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Multisampling Resolve");


            auto secondaryCommandBuffers = frameBuffer->commandBuffers();
            int commandBuffer = 0;

            for (auto& buffer : secondaryCommandBuffers)
                m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*buffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, fmt::format("Command Buffer {0}-{1}", m_parent->name(), commandBuffer++));
#endif

            return frameBuffer;
        });

        // Initialize the primary command buffers, that are used to record begin and end commands for the render pass on each frame buffer.
        m_primaryCommandBuffers.resize(this->m_device.swapChain().buffers());
        std::ranges::generate(m_primaryCommandBuffers, [&, i = 0]() mutable {
            auto commandBuffer = m_queue->createCommandBuffer(false);

#ifndef NDEBUG
            m_device.setDebugName(*reinterpret_cast<const UInt64*>(&std::as_const(*commandBuffer).handle()), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, fmt::format("{0} Primary Command Buffer {1}", m_parent->name(), i++));
#endif

            return commandBuffer;
        });
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    VulkanRenderPass(device, device.defaultQueue(QueueType::Graphics), renderTargets, commandBuffers, samples, inputAttachments)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    VulkanRenderPass(device, name, device.defaultQueue(QueueType::Graphics), renderTargets, commandBuffers, samples, inputAttachments)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device, queue, renderTargets, samples, inputAttachments)), Resource<VkRenderPass>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
    m_impl->initializeFrameBuffers(commandBuffers);
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    VulkanRenderPass(device, queue, renderTargets, commandBuffers, samples, inputAttachments)
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name) noexcept :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device)), Resource<VkRenderPass>(VK_NULL_HANDLE)
{
    if (!name.empty())
        this->name() = name;
}

VulkanRenderPass::~VulkanRenderPass() noexcept
{
    ::vkDestroyRenderPass(m_impl->m_device.handle(), this->handle(), nullptr);
}

const VulkanFrameBuffer& VulkanRenderPass::frameBuffer(UInt32 buffer) const
{
    if (buffer >= m_impl->m_frameBuffers.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0u, static_cast<UInt32>(m_impl->m_frameBuffers.size()), buffer, "The buffer {0} does not exist in this render pass. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

    return *m_impl->m_frameBuffers[buffer].get();
}

const VulkanDevice& VulkanRenderPass::device() const noexcept
{
    return m_impl->m_device;
}

const VulkanFrameBuffer& VulkanRenderPass::activeFrameBuffer() const
{
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("No frame buffer is active, since the render pass has not begun.");

    return *m_impl->m_activeFrameBuffer;
}

const VulkanQueue& VulkanRenderPass::commandQueue() const noexcept
{
    return *m_impl->m_queue;
}

Enumerable<const VulkanFrameBuffer*> VulkanRenderPass::frameBuffers() const noexcept
{
    return m_impl->m_frameBuffers | std::views::transform([](const UniquePtr<VulkanFrameBuffer>& frameBuffer) { return frameBuffer.get(); });
}

Enumerable<const VulkanRenderPipeline*> VulkanRenderPass::pipelines() const noexcept
{
    return m_impl->m_pipelines | std::views::transform([](const UniquePtr<VulkanRenderPipeline>& pipeline) { return pipeline.get(); });
}

const RenderTarget& VulkanRenderPass::renderTarget(UInt32 location) const
{
    if (auto match = std::ranges::find_if(m_impl->m_renderTargets, [&location](const RenderTarget& renderTarget) { return renderTarget.location() == location; }); match != m_impl->m_renderTargets.end()) [[likely]]
        return *match;

    throw InvalidArgumentException("location", "No render target is mapped to location {0} in this render pass.", location);
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

MultiSamplingLevel VulkanRenderPass::multiSamplingLevel() const noexcept
{
    return m_impl->m_samples;
}

void VulkanRenderPass::begin(UInt32 buffer)
{
    // Only begin, if we are currently not running.
    if (m_impl->m_activeFrameBuffer != nullptr) [[unlikely]]
        throw RuntimeException("Unable to begin a render pass, that is already running. End the current pass first.");

    // Select the active frame buffer.
    if (buffer >= m_impl->m_frameBuffers.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0u, static_cast<UInt32>(m_impl->m_frameBuffers.size()), buffer, "The frame buffer {0} is out of range. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

    auto frameBuffer = m_impl->m_activeFrameBuffer = m_impl->m_frameBuffers[buffer].get();
    auto commandBuffer = m_impl->m_activeCommandBuffer = m_impl->m_primaryCommandBuffers[buffer];
    m_impl->m_backBuffer = buffer;

    // Begin the command recording on the frame buffers command buffer. Before we can do that, we need to make sure it has not being executed anymore.
    m_impl->m_queue->waitFor(frameBuffer->lastFence());
    commandBuffer->begin();

    // Begin the render pass.
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->handle();
    renderPassInfo.framebuffer = std::as_const(*frameBuffer).handle();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent.width = static_cast<UInt32>(frameBuffer->getWidth());
    renderPassInfo.renderArea.extent.height = static_cast<UInt32>(frameBuffer->getHeight());
    renderPassInfo.clearValueCount = m_impl->m_clearValues.size();
    renderPassInfo.pClearValues = m_impl->m_clearValues.data();

    ::vkCmdBeginRenderPass(std::as_const(*commandBuffer).handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    // Begin the frame buffer command buffers.
    std::ranges::for_each(frameBuffer->commandBuffers(), [this](auto commandBuffer) { commandBuffer->begin(*this); });

    // Publish beginning event.
    this->beginning(this, { buffer });
}

UInt64 VulkanRenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr) [[unlikely]]
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    // Publish ending event.
    this->ending(this, { });

    auto frameBuffer = m_impl->m_activeFrameBuffer;
    auto commandBuffer = m_impl->m_activeCommandBuffer;

    // End the render pass and the command buffer recording.
    auto secondaryBuffers = frameBuffer->commandBuffers();
    auto secondaryHandles = secondaryBuffers |
        std::views::transform([](auto commandBuffer) { commandBuffer->end(); return commandBuffer->handle(); }) |
        std::ranges::to<Array<VkCommandBuffer>>();
    ::vkCmdExecuteCommands(std::as_const(*commandBuffer).handle(), static_cast<UInt32>(secondaryHandles.size()), secondaryHandles.data());
    ::vkCmdEndRenderPass(std::as_const(*commandBuffer).handle());

    // Submit the command buffer.
    if (!this->hasPresentTarget())
        frameBuffer->lastFence() = m_impl->m_queue->submit(commandBuffer);
    else
    {
        // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        std::array<VkSemaphore, 1> waitForSemaphores = { m_impl->m_device.swapChain().semaphore() };
        std::array<VkPipelineStageFlags, 1> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        std::array<VkSemaphore, 1> signalSemaphores = { frameBuffer->semaphore() };
        frameBuffer->lastFence() = m_impl->m_queue->submit(commandBuffer, waitForSemaphores, waitForStages, signalSemaphores);

        // Present the swap chain.
        m_impl->m_device.swapChain().present(*frameBuffer);
    }

    // Reset the frame buffer.
    m_impl->m_activeFrameBuffer = nullptr;
    m_impl->m_activeCommandBuffer = nullptr;

    // Return the last fence.
    return frameBuffer->lastFence();
}

void VulkanRenderPass::resizeFrameBuffers(const Size2d& renderArea)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr) [[unlikely]]
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<VulkanFrameBuffer>& frameBuffer) { frameBuffer->resize(renderArea); });
}

void VulkanRenderPass::changeMultiSamplingLevel(MultiSamplingLevel samples)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr) [[unlikely]]
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    m_impl->m_samples = samples;
    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<VulkanFrameBuffer>& frameBuffer) { frameBuffer->resize(frameBuffer->size()); });
}

void VulkanRenderPass::updateAttachments(const VulkanDescriptorSet& descriptorSet) const
{
    const auto backBuffer = m_impl->m_backBuffer;

    std::ranges::for_each(m_impl->m_inputAttachments, [&descriptorSet, &backBuffer](const VulkanInputAttachmentMapping& inputAttachment) {
#ifndef NDEBUG
        if (inputAttachment.inputAttachmentSource() == nullptr) [[unlikely]]
            throw RuntimeException("No source render pass has been specified for the input attachment mapped to location {0}.", inputAttachment.location());
#endif

        descriptorSet.attach(inputAttachment.location(), inputAttachment.inputAttachmentSource()->frameBuffer(backBuffer).image(inputAttachment.renderTarget().location()));
    });
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const String& name) noexcept :
    VulkanRenderPassBuilder(device, 1, MultiSamplingLevel::x1, name)
{
}

constexpr VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, MultiSamplingLevel samples, const String& name) noexcept :
    VulkanRenderPassBuilder(device, 1, samples, name)
{
}

constexpr VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, const String& name) noexcept :
    VulkanRenderPassBuilder(device, commandBuffers, MultiSamplingLevel::x1, name)
{
}

constexpr VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, UInt32 commandBuffers, MultiSamplingLevel samples, const String& name) noexcept :
    RenderPassBuilder(UniquePtr<VulkanRenderPass>(new VulkanRenderPass(device, name)))
{
    m_state.commandBufferCount = commandBuffers;
    m_state.multiSamplingLevel = samples;
}

constexpr VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

void VulkanRenderPassBuilder::build()
{
    auto instance = this->instance();
    
    if (m_state.commandQueue != nullptr)
        instance->m_impl->m_queue = m_state.commandQueue;
    
    instance->m_impl->mapRenderTargets(m_state.renderTargets);
    instance->m_impl->mapInputAttachments(m_state.inputAttachments);
    instance->m_impl->m_samples = m_state.multiSamplingLevel;
    instance->handle() = instance->m_impl->initialize();
    instance->m_impl->initializeFrameBuffers(m_state.commandBufferCount);
}

VulkanInputAttachmentMapping VulkanRenderPassBuilder::makeInputAttachment(UInt32 inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget)
{
    return VulkanInputAttachmentMapping(renderPass, renderTarget, inputLocation);
}
#endif // defined(BUILD_DEFINE_BUILDERS)