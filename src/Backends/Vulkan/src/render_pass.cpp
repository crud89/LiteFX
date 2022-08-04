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
    const VulkanFrameBuffer* m_activeFrameBuffer = nullptr;
    Array<VkClearValue> m_clearValues;
    UInt32 m_backBuffer{ 0 };
    MultiSamplingLevel m_samples;
    UniquePtr<VulkanCommandBuffer> m_primaryCommandBuffer;
    const VulkanDevice& m_device;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device, Span<RenderTarget> renderTargets, const MultiSamplingLevel& samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
        base(parent), m_samples(samples), m_device(device)
    {
        this->mapRenderTargets(renderTargets);
        this->mapInputAttachments(inputAttachments);
    }

    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
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
        Optional<VkAttachmentDescription> presentResolveAttachment;

        // Map input attachments.
        std::ranges::for_each(m_inputAttachments, [&, i = 0](const VulkanInputAttachmentMapping& inputAttachment) mutable {
            UInt32 currentIndex = i++;

            if (inputAttachment.location() != currentIndex)
                throw InvalidArgumentException("No input attachment is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

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
                    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    outputAttachments.push_back({ static_cast<UInt32>(currentIndex + inputAttachments.size()), attachment.finalLayout });
                    break;
                case RenderTargetType::DepthStencil:
                    if (::hasDepth(renderTarget.format()) || ::hasStencil(renderTarget.format())) [[likely]]
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
        raiseIfFailed<RuntimeException>(::vkCreateRenderPass(m_device.handle(), &renderPassState, nullptr, &renderPass), "Unable to create render pass.");

#ifndef NDEBUG
        m_device.setDebugName(*reinterpret_cast<const UInt64*>(&renderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, m_parent->name());
#endif

        return renderPass;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, const UInt32& commandBuffers, const MultiSamplingLevel& samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device, renderTargets, samples, inputAttachments)), Resource<VkRenderPass>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();

    // Initialize the frame buffers.
    m_impl->m_frameBuffers.resize(m_impl->m_device.swapChain().buffers());
    std::ranges::generate(m_impl->m_frameBuffers, [this, &commandBuffers, i = 0]() mutable { return makeUnique<VulkanFrameBuffer>(*this, i++, m_impl->m_device.swapChain().renderArea(), commandBuffers); });

    // Initialize the command buffers.
    m_impl->m_primaryCommandBuffer = m_impl->m_device.graphicsQueue().createCommandBuffer(false);
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, const UInt32& commandBuffers, const MultiSamplingLevel& samples, Span<VulkanInputAttachmentMapping> inputAttachments) :
    VulkanRenderPass(device, renderTargets, commandBuffers, samples, inputAttachments)
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

const VulkanFrameBuffer& VulkanRenderPass::frameBuffer(const UInt32& buffer) const
{
    if (buffer >= m_impl->m_frameBuffers.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("The buffer {0} does not exist in this render pass. The render pass only contains {1} frame buffers.", buffer, m_impl->m_frameBuffers.size());

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

Array<const VulkanFrameBuffer*> VulkanRenderPass::frameBuffers() const noexcept
{
    return m_impl->m_frameBuffers | 
        std::views::transform([](const UniquePtr<VulkanFrameBuffer>& frameBuffer) { return frameBuffer.get(); }) |
        ranges::to<Array<const VulkanFrameBuffer*>>();
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

const MultiSamplingLevel& VulkanRenderPass::multiSamplingLevel() const noexcept
{
    return m_impl->m_samples;
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

    // Begin the command recording on the frame buffers command buffer. Before we can do that, we need to make sure it has not being executed anymore.
    m_impl->m_device.graphicsQueue().waitFor(frameBuffer->lastFence());
    m_impl->m_primaryCommandBuffer->begin();

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

    ::vkCmdBeginRenderPass(std::as_const(*m_impl->m_primaryCommandBuffer).handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    // Begin the frame buffer command buffers.
    std::ranges::for_each(frameBuffer->commandBuffers(), [this](const VulkanCommandBuffer* commandBuffer) { commandBuffer->begin(*this); });
}

void VulkanRenderPass::end() const
{
    // Check if we are running.
    if (m_impl->m_activeFrameBuffer == nullptr)
        throw RuntimeException("Unable to end a render pass, that has not been begun. Start the render pass first.");

    auto frameBuffer = m_impl->m_activeFrameBuffer;

    // End the render pass and the command buffer recording.
    auto secondaryBuffers = frameBuffer->commandBuffers();
    auto secondaryHandles = secondaryBuffers |
        std::views::transform([](const VulkanCommandBuffer* commandBuffer) { commandBuffer->end(); return commandBuffer->handle(); }) |
        ranges::to<Array<VkCommandBuffer>>();
    ::vkCmdExecuteCommands(std::as_const(*m_impl->m_primaryCommandBuffer).handle(), static_cast<UInt32>(secondaryHandles.size()), secondaryHandles.data());
    ::vkCmdEndRenderPass(std::as_const(*m_impl->m_primaryCommandBuffer).handle());

    // Submit the command buffer.
    if (!this->hasPresentTarget())
        frameBuffer->lastFence() = m_impl->m_device.graphicsQueue().submit(*m_impl->m_primaryCommandBuffer);
    else
    {
        // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        std::array<VkSemaphore, 1> waitForSemaphores = { m_impl->m_device.swapChain().semaphore() };
        std::array<VkPipelineStageFlags, 1> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        std::array<VkSemaphore, 1> signalSemaphores = { frameBuffer->semaphore() };
        frameBuffer->lastFence() = m_impl->m_device.graphicsQueue().submit(*m_impl->m_primaryCommandBuffer, waitForSemaphores, waitForStages, signalSemaphores);

        // Present the swap chain.
        m_impl->m_device.swapChain().present(*frameBuffer);
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

void VulkanRenderPass::changeMultiSamplingLevel(const MultiSamplingLevel& samples)
{
    // Check if we're currently running.
    if (m_impl->m_activeFrameBuffer != nullptr)
        throw RuntimeException("Unable to reset the frame buffers while the render pass is running. End the render pass first.");

    m_impl->m_samples = samples;
    std::ranges::for_each(m_impl->m_frameBuffers, [&](UniquePtr<VulkanFrameBuffer>& frameBuffer) { frameBuffer->resize(frameBuffer->size()); });
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

#if defined(BUILD_DEFINE_BUILDERS)
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
    MultiSamplingLevel m_samples;
    UInt32 m_commandBuffers;

public:
    VulkanRenderPassBuilderImpl(VulkanRenderPassBuilder* parent, const MultiSamplingLevel& samples, const UInt32& commandBuffers) :
        base(parent), m_samples(samples), m_commandBuffers(commandBuffers)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const String& name) noexcept :
    VulkanRenderPassBuilder(device, 1, MultiSamplingLevel::x1, name)
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const String& name) noexcept :
    VulkanRenderPassBuilder(device, commandBuffers, MultiSamplingLevel::x1, name)
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const MultiSamplingLevel& samples, const String& name) noexcept :
    VulkanRenderPassBuilder(device, 1, samples, name)
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const VulkanDevice& device, const UInt32& commandBuffers, const MultiSamplingLevel& samples, const String& name) noexcept :
    m_impl(makePimpl<VulkanRenderPassBuilderImpl>(this, samples, commandBuffers)), RenderPassBuilder<VulkanRenderPassBuilder, VulkanRenderPass>(UniquePtr<VulkanRenderPass>(new VulkanRenderPass(device, name)))
{
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

void VulkanRenderPassBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->mapRenderTargets(m_impl->m_renderTargets);
    instance->m_impl->mapInputAttachments(m_impl->m_inputAttachments);
    instance->m_impl->m_samples = std::move(m_impl->m_samples);
    instance->handle() = instance->m_impl->initialize();

    // Initialize the frame buffers.
    instance->m_impl->m_frameBuffers.resize(instance->device().swapChain().buffers());
    std::ranges::generate(instance->m_impl->m_frameBuffers, [this, &instance, i = 0]() mutable { return makeUnique<VulkanFrameBuffer>(*instance, i++, instance->device().swapChain().renderArea(), m_impl->m_commandBuffers); });

    // Initialize the command buffers.
    instance->m_impl->m_primaryCommandBuffer = instance->device().graphicsQueue().createCommandBuffer(false);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::commandBuffers(const UInt32& count)
{
    m_impl->m_commandBuffers = count;
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::multiSamplingLevel(const MultiSamplingLevel& samples)
{
    m_impl->m_samples = samples;
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, clearValues, clear, clearStencil, isVolatile);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    m_impl->m_renderTargets.push_back(RenderTarget(location, type, format, clear, clearValues, clearStencil, isVolatile));
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(VulkanInputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    // TODO: This might be invalid, if another target is already defined with a custom location, however in this case we have no guarantee that the location range will be contiguous
    //       until the render pass is initialized, so we silently ignore this for now.
    return this->renderTarget(output, static_cast<UInt32>(m_impl->m_renderTargets.size()), type, format, clearValues, clear, clearStencil, isVolatile);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::renderTarget(VulkanInputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    auto renderTarget = RenderTarget(location, type, format, clear, clearValues, clearStencil, isVolatile);
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
#endif // defined(BUILD_DEFINE_BUILDERS)