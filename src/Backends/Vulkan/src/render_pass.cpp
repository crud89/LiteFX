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
    Array<VulkanRenderPassDependency> m_inputAttachments;
    Array<UniquePtr<VulkanFrameBuffer>> m_frameBuffers;
    Array<SharedPtr<VulkanCommandBuffer>> m_primaryCommandBuffers;
    VulkanFrameBuffer* m_activeFrameBuffer = nullptr;
    SharedPtr<const VulkanCommandBuffer> m_activeCommandBuffer;
    Array<VkClearValue> m_clearValues;
    UInt32 m_backBuffer{ 0 };
    DescriptorBindingPoint m_inputAttachmentSamplerBinding{ };
    MultiSamplingLevel m_samples;
    const VulkanDevice& m_device;
    const VulkanQueue* m_queue;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, MultiSamplingLevel samples, Span<VulkanRenderPassDependency> inputAttachments, DescriptorBindingPoint inputAttachmentSamplerBinding) :
        base(parent), m_samples(samples), m_device(device), m_queue(&queue), m_inputAttachmentSamplerBinding(inputAttachmentSamplerBinding)
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
        std::ranges::sort(m_renderTargets, [this](const auto& a, const auto& b) { return a.location() < b.location(); });

        // TODO: If there is a present target, we need to check if the provided queue can actually present on the surface. Currently, 
        //       we simply check if the queue is the same as the swap chain queue (which is the default graphics queue).
        if (std::ranges::any_of(m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }) &&
            m_queue != std::addressof(m_device.defaultQueue(QueueType::Graphics))) [[unlikely]]
            throw InvalidArgumentException("renderTargets", "A render pass with a present target must be executed on the default graphics queue.");
    }

    void mapInputAttachments(Span<VulkanRenderPassDependency> inputAttachments)
    {
        m_inputAttachments.assign(std::begin(inputAttachments), std::end(inputAttachments));
        std::ranges::sort(m_inputAttachments, [this](const auto& a, const auto& b) { return a.location() < b.location(); });
    }

public:
    VkRenderPass initialize()
    {
        // Setup the attachments.
        Array<VkAttachmentDescription2> attachments;
        Array<VkAttachmentReference2> inputAttachments;
        Array<VkAttachmentReference2> outputAttachments; // Contains all output attachments, except the depth/stencil target.
        Optional<VkAttachmentReference2> depthTarget, presentTarget;
        Optional<VkAttachmentDescription2> presentResolveAttachment;

        // Map input attachments.
        std::ranges::for_each(m_inputAttachments, [&, i = 0](const VulkanRenderPassDependency& inputAttachment) mutable {
            UInt32 currentIndex = i++;

            if (inputAttachment.location() != currentIndex) [[unlikely]]
                throw InvalidArgumentException("inputAttachments", "No input attachment is mapped to location {0}. The locations must be within a contiguous domain.", currentIndex);

            VkAttachmentDescription2 attachment = {
                .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
                .format = Vk::getFormat(inputAttachment.renderTarget().format()),
                .samples = Vk::getSamples(inputAttachment.inputAttachmentSource()->multiSamplingLevel()),
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
            };
            
            // Add a clear value, so that the indexing stays valid.
            m_clearValues.push_back(VkClearValue { });

            switch (inputAttachment.renderTarget().type()) 
            {
            case RenderTargetType::Color:
            {
                attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                inputAttachments.push_back({
                    .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                    .attachment = static_cast<UInt32>(currentIndex),
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                });

                attachments.push_back(attachment);
                break;
            }
            case RenderTargetType::DepthStencil:
            {
                VkImageAspectFlags aspectMask { };

                if (::hasDepth(inputAttachment.renderTarget().format()) && ::hasStencil(inputAttachment.renderTarget().format())) [[likely]]
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                }
                else if (::hasDepth(inputAttachment.renderTarget().format()))
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                }
                else if (::hasStencil(inputAttachment.renderTarget().format()))
                {
                    attachment.initialLayout = inputAttachment.renderTarget().attachment() ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                    aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                }
                else [[unlikely]]
                {
                    LITEFX_WARNING(VULKAN_LOG, "The depth/stencil input attachment at location {0} does not have a valid depth/stencil format ({1}). Falling back to VK_IMAGE_LAYOUT_GENERAL.", currentIndex, inputAttachment.renderTarget().format());
                    attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                }

                inputAttachments.push_back({
                    .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                    .attachment = static_cast<UInt32>(currentIndex), 
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .aspectMask = aspectMask
                });

                attachments.push_back(attachment);
                break;
            }
            case RenderTargetType::Present: [[unlikely]]
                throw InvalidArgumentException("inputAttachments", "The render pass input attachment at location {0} maps to a present render target, which can not be used as input attachment.", currentIndex);
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
                VkAttachmentDescription2 attachment = {
                    .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
                    .format = Vk::getFormat(renderTarget.format()),
                    .samples = Vk::getSamples(m_samples),
                    .loadOp = renderTarget.clearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = renderTarget.clearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = renderTarget.isVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE
                };

                // Add a clear values (even if it's unused).
                if (renderTarget.clearBuffer() || renderTarget.clearStencil())
                    m_clearValues.push_back(VkClearValue{ renderTarget.clearValues().x(), renderTarget.clearValues().y(), renderTarget.clearValues().z(), renderTarget.clearValues().w() });
                else
                    m_clearValues.push_back(VkClearValue{ });

                switch (renderTarget.type())
                {
                case RenderTargetType::Color:
                {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                    outputAttachments.push_back({
                        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                        .attachment = static_cast<UInt32>(currentIndex + inputAttachments.size()),
                        .layout = attachment.finalLayout,
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                    });

                    break;
                }
                case RenderTargetType::DepthStencil:
                {
                    VkImageAspectFlags aspectMask { };

                    if (::hasDepth(renderTarget.format()) || ::hasStencil(renderTarget.format())) [[likely]]
                    {
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                    else if (::hasDepth(renderTarget.format()))
                    {
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    }
                    else if (::hasStencil(renderTarget.format()))
                    {
                        attachment.finalLayout = renderTarget.attachment() ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
                        aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                    else [[unlikely]]
                    {
                        LITEFX_WARNING(VULKAN_LOG, "The depth/stencil render target at location {0} does not have a valid depth/stencil format ({1}). Falling back to VK_IMAGE_LAYOUT_GENERAL.", currentIndex, renderTarget.format());
                        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }

                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    depthTarget = VkAttachmentReference2 { 
                        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                        .attachment = static_cast<UInt32>(currentIndex + inputAttachments.size()), 
                        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        .aspectMask = aspectMask 
                    };

                    break;
                }
                case RenderTargetType::Present:
                {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                    // If we have a multi-sampled present attachment, we also need to attach a resolve attachment for it.
                    if (m_samples == MultiSamplingLevel::x1)
                    {
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
                        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#else
                        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif
                    }
                    else
                    {
                        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                        presentResolveAttachment = VkAttachmentDescription2 {
                            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
                            .format = attachment.format,
                            .samples = VK_SAMPLE_COUNT_1_BIT,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
                            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
#else
                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
#endif
                        };
                    }

                    presentTarget = VkAttachmentReference2 { 
                        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                        .attachment = static_cast<UInt32>(currentIndex + inputAttachments.size()), 
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                    };

                    outputAttachments.push_back(presentTarget.value());
                    break;
                }
                }

                attachments.push_back(attachment);
            }
        });

        // Setup the sub-pass.
        VkSubpassDescription2 subPass = {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<UInt32>(inputAttachments.size()),
            .pInputAttachments = inputAttachments.data(),
            .colorAttachmentCount = static_cast<UInt32>(outputAttachments.size()),
            .pColorAttachments = outputAttachments.data(),
            .pDepthStencilAttachment = depthTarget.has_value() ? &depthTarget.value() : nullptr
        };

        // Add the resolve attachment.
        VkAttachmentReference2 presentResolveReference = { 
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .attachment = static_cast<UInt32>(attachments.size()), 
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
        };

        if (presentResolveAttachment.has_value())
        {
            subPass.pResolveAttachments = &presentResolveReference;
            attachments.push_back(presentResolveAttachment.value());
        }

        // Define an external sub-pass dependency, if there are input attachments to synchronize with.
        Array<VkSubpassDependency2> dependencies;

        if (!m_inputAttachments.empty())
        {
            dependencies.push_back({
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
            });
        }

        // Setup render pass state.
        VkRenderPassCreateInfo2 renderPassState = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
            .attachmentCount = static_cast<UInt32>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subPass,
            .dependencyCount = static_cast<UInt32>(dependencies.size()),
            .pDependencies = dependencies.data()
        };

        // Create the render pass.
        VkRenderPass renderPass;
        raiseIfFailed(::vkCreateRenderPass2(m_device.handle(), &renderPassState, nullptr, &renderPass), "Unable to create render pass.");

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

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanRenderPassDependency> inputAttachments, DescriptorBindingPoint inputAttachmentSamplerBinding) :
    VulkanRenderPass(device, device.defaultQueue(QueueType::Graphics), renderTargets, commandBuffers, samples, inputAttachments, inputAttachmentSamplerBinding)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanRenderPassDependency> inputAttachments, DescriptorBindingPoint inputAttachmentSamplerBinding) :
    VulkanRenderPass(device, name, device.defaultQueue(QueueType::Graphics), renderTargets, commandBuffers, samples, inputAttachments, inputAttachmentSamplerBinding)
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanRenderPassDependency> inputAttachments, DescriptorBindingPoint inputAttachmentSamplerBinding) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this, device, queue, renderTargets, samples, inputAttachments, inputAttachmentSamplerBinding)), Resource<VkRenderPass>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
    m_impl->initializeFrameBuffers(commandBuffers);
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, UInt32 commandBuffers, MultiSamplingLevel samples, Span<VulkanRenderPassDependency> inputAttachments, DescriptorBindingPoint inputAttachmentSamplerBinding) :
    VulkanRenderPass(device, queue, renderTargets, commandBuffers, samples, inputAttachments, inputAttachmentSamplerBinding)
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

Span<const VulkanRenderPassDependency> VulkanRenderPass::inputAttachments() const noexcept
{
    return m_impl->m_inputAttachments;
}

MultiSamplingLevel VulkanRenderPass::multiSamplingLevel() const noexcept
{
    return m_impl->m_samples;
}

const DescriptorBindingPoint& VulkanRenderPass::inputAttachmentSamplerBinding() const noexcept
{
    return m_impl->m_inputAttachmentSamplerBinding;
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
    frameBuffer->lastFence() = m_impl->m_queue->submit(commandBuffer);

    // Present the swap chain.
    if (this->hasPresentTarget())
        m_impl->m_device.swapChain().present(*frameBuffer);

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

    std::ranges::for_each(m_impl->m_inputAttachments, [&descriptorSet, &backBuffer](const VulkanRenderPassDependency& inputAttachment) {
#ifndef NDEBUG
        if (inputAttachment.inputAttachmentSource() == nullptr) [[unlikely]]
            throw RuntimeException("No source render pass has been specified for the input attachment mapped to location {0}.", inputAttachment.location());
#endif

        descriptorSet.attach(inputAttachment.location(), inputAttachment.inputAttachmentSource()->frameBuffer(backBuffer).image(inputAttachment.renderTarget().location()));
    });
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
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
    instance->m_impl->m_inputAttachmentSamplerBinding = m_state.inputAttachmentSamplerBinding;
}

VulkanRenderPassDependency VulkanRenderPassBuilder::makeInputAttachment(UInt32 inputLocation, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget)
{
    return VulkanRenderPassDependency(renderPass, renderTarget, inputLocation);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)