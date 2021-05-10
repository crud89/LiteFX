#include <litefx/backends/vulkan.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl : public Implement<VulkanRenderPass> {
public:
    friend class VulkanRenderPassBuilder;
    friend class VulkanRenderPass;

private:
    Array<UniquePtr<IRenderPipeline>> m_pipelines;
    const VulkanSwapChain* m_swapChain{ nullptr };
    const VulkanQueue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    Array<VkClearValue> m_clearValues;
    Array<VkFramebuffer> m_frameBuffers;
    Optional<VkAttachmentReference> m_presentAttachment;
    Array<UniquePtr<VulkanCommandBuffer>> m_commandBuffers;
    UInt32 m_backBuffer{ 0 };
    Array<VkSemaphore> m_semaphores;
    const VulkanRenderPass* m_dependency{ nullptr };

    /// <summary>
    /// Stores the images for all attachments (except the present attachment, which is a swap-chain image) and maps them to the frame buffer index.
    /// </summary>
    Dictionary<UInt32, Array<UniquePtr<IImage>>> m_attachmentImages;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent) : 
        base(parent)
    {
    }

private:
    void cleanup()
    {
        m_attachmentImages.clear();
        m_clearValues.clear();

        ::vkDestroyRenderPass(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);

        std::for_each(std::begin(m_semaphores), std::end(m_semaphores), [&](const auto& semaphore) {
            ::vkDestroySemaphore(m_parent->getDevice()->handle(), semaphore, nullptr);
        });

        m_semaphores.clear();

        std::for_each(std::begin(m_frameBuffers), std::end(m_frameBuffers), [&](VkFramebuffer& frameBuffer) {
            ::vkDestroyFramebuffer(m_parent->getDevice()->handle(), frameBuffer, nullptr);
        });

        m_backBuffer = 0;
    }

    UniquePtr<IImage> makeImageView(const IRenderTarget* target)
    {
        return m_parent->getDevice()->createAttachment(target->getFormat(), m_parent->getDevice()->getSwapChain()->getBufferSize());
    }

public:
    VkRenderPass initialize()
    {
        // Store swap chain and graphics queue.
        m_swapChain = dynamic_cast<const VulkanSwapChain*>(m_parent->getDevice()->getSwapChain());

        if (m_swapChain == nullptr)
            throw std::invalid_argument("The device swap chain is not a valid Vulkan swap chain.");
        
        m_queue = dynamic_cast<const VulkanQueue*>(m_parent->getDevice()->graphicsQueue());

        if (m_queue == nullptr)
            throw std::invalid_argument("The device queue is not a valid Vulkan command queue.");

        // Get the render targets of the render pass dependency, if there is any.
        Array<const IRenderTarget*> dependencyTargets;

        if (m_dependency != nullptr)
            dependencyTargets = m_dependency->getTargets();

        // Setup the attachments.
        Array<VkAttachmentDescription> attachments;
        Array<VkAttachmentReference> colorAttachments;
        Array<VkAttachmentReference> inputAttachments;
        Optional<VkAttachmentReference> depthAttachment;

        // Map input attachments.
        // TODO: We need to check if there's some better way to map dependency outputs to input attachments, since simply assuming the right order
        //       might actually be tricky in situations where there are more than two render passes or where one render pass is executed multiple
        //       times.
        std::for_each(std::begin(dependencyTargets), std::end(dependencyTargets), [&, i = 0](const auto& target) mutable {
            VkAttachmentDescription attachment{};
            attachment.format = getFormat(target->getFormat());
            attachment.samples = getSamples(target->getSamples());
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            // Add a clear value, so that the indexing stays valid.
            m_clearValues.push_back(VkClearValue{ });

            switch (target->getType())
            {
            case RenderTargetType::Color:
                attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                inputAttachments.push_back({ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                attachments.push_back(attachment);
                break;
            case RenderTargetType::Depth:
                attachment.initialLayout = attachment.finalLayout = ::hasStencil(target->getFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

                inputAttachments.push_back({ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                attachments.push_back(attachment);
                break;
            case RenderTargetType::Present:
                LITEFX_WARNING(VULKAN_LOG, "The render pass dependency defines a present render target, which can not be used as input attachment.");
                break;
            }
        });

        // Create attachments for each render target.
        std::for_each(std::begin(m_targets), std::end(m_targets), [&, i = inputAttachments.size()](const auto& target) mutable {
            if ((target->getType() == RenderTargetType::Depth && depthAttachment.has_value()) || (target->getType() == RenderTargetType::Present && presentAttachment.has_value()))
                throw ArgumentOutOfRangeException("Invalid render target {0}: only one target attachment of type {1} is allowed.", i, target->getType());
            else
            {
                VkAttachmentDescription attachment{};
                attachment.format = getFormat(target->getFormat());
                attachment.samples = getSamples(target->getSamples());
                attachment.loadOp = target->getClearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilLoadOp = target->getClearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.storeOp = target->getVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // Add a clear values (even if it's unused).
                if (target->getClearBuffer() || target->getClearStencil())
                    m_clearValues.push_back(VkClearValue{ target->getClearValues().x(), target->getClearValues().y(), target->getClearValues().z(), target->getClearValues().w() });
                else
                    m_clearValues.push_back(VkClearValue{ });

                switch (target->getType())
                {
                case RenderTargetType::Color:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    colorAttachments.push_back({ static_cast<UInt32>(i++), attachment.finalLayout });
                    break;
                case RenderTargetType::Depth:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = ::hasStencil(target->getFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

                    depthAttachment = VkAttachmentReference { static_cast<UInt32>(i++), attachment.finalLayout };
                    break;
                case RenderTargetType::Present:
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                    m_presentAttachment = VkAttachmentReference { static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
                    colorAttachments.push_back(m_presentAttachment.value());
                    break;
                }

                attachments.push_back(attachment);
            }
        });

        // Setup the sub-pass.
        VkSubpassDescription subPass{};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = static_cast<UInt32>(colorAttachments.size());
        subPass.pColorAttachments = colorAttachments.data();
        subPass.pDepthStencilAttachment = depthAttachment.has_value() ? &depthAttachment.value() : nullptr;
        subPass.inputAttachmentCount = static_cast<UInt32>(inputAttachments.size());
        subPass.pInputAttachments = inputAttachments.data();

        // Define an external sub-pass dependency.
        Array<VkSubpassDependency> dependencies;

        if (m_dependency != nullptr)
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

        if (::vkCreateRenderPass(m_parent->getDevice()->handle(), &renderPassState, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass.");

        // Initialize the frame buffer.
        this->createFramebuffer(renderPass);

        // Create a command buffer.
        if (m_commandBuffers.empty())
        {
            m_commandBuffers.resize(m_frameBuffers.size());
            std::generate(std::begin(m_commandBuffers), std::end(m_commandBuffers), [&]() mutable { return makeUnique<VulkanCommandBuffer>(*dynamic_cast<const VulkanQueue*>(m_parent->getDevice()->graphicsQueue())); });
        }

        // Create a semaphore that signals if the render pass has finished.
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        m_semaphores.resize(m_frameBuffers.size());
        std::generate(std::begin(m_semaphores), std::end(m_semaphores), [&]() mutable {
            VkSemaphore semaphore;

            if (::vkCreateSemaphore(m_parent->getDevice()->handle(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
                throw std::runtime_error("Unable to create signal semaphore.");

            return semaphore;
        });

        // Return the render pass.
        return renderPass;
    }

    void createFramebuffer(const VkRenderPass renderPass)
    {
        std::for_each(std::begin(m_frameBuffers), std::end(m_frameBuffers), [&](VkFramebuffer& frameBuffer) {
            ::vkDestroyFramebuffer(m_parent->getDevice()->handle(), frameBuffer, nullptr);
        });

        Array<const IRenderTarget*> dependencyTargets;

        if (m_dependency != nullptr)
            dependencyTargets = m_dependency->getTargets();

        // Initialize frame buffers.
        auto frames = m_swapChain->getFrames();
        Array<VkFramebuffer> frameBuffers(frames.size());

        LITEFX_TRACE(VULKAN_LOG, "Initializing {0} frame buffers...", frames.size());

        std::generate(std::begin(frameBuffers), std::end(frameBuffers), [&, i = 0]() mutable {
            Array<VkImageView> attachmentViews;
            Array<UniquePtr<IImage>> attachmentImages;

            std::for_each(std::begin(dependencyTargets), std::end(dependencyTargets), [&, a = 0](const auto& target) mutable {
                auto inputAttachmentImage = dynamic_cast<const IVulkanImage*>(m_dependency->m_impl->m_attachmentImages[i][a++].get());

                if (inputAttachmentImage == nullptr)
                    throw std::invalid_argument("An input attachment of the render pass dependency is not a valid Vulkan image.");

                attachmentViews.push_back(inputAttachmentImage->getImageView());
            });

            std::for_each(std::begin(m_targets), std::end(m_targets), [&, a = attachmentViews.size()](const auto& target) mutable {
                if (m_presentAttachment.has_value() && a++ == m_presentAttachment->attachment)
                {
                    // Acquire an image view from the swap chain.
                    auto swapChainImage = dynamic_cast<const IVulkanImage*>(frames[i]);

                    if (swapChainImage == nullptr)
                        throw std::invalid_argument("A frame of the provided swap chain is not a valid Vulkan texture.");

                    attachmentViews.push_back(swapChainImage->getImageView());
                }
                else
                {
                    // Create an image view for the render target.
                    auto image = this->makeImageView(target.get());
                    attachmentViews.push_back(dynamic_cast<const IVulkanImage*>(image.get())->getImageView());
                    attachmentImages.push_back(std::move(image));
                }
            });

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = renderPass;
            frameBufferInfo.attachmentCount = static_cast<UInt32>(attachmentViews.size());
            frameBufferInfo.pAttachments = attachmentViews.data();
            frameBufferInfo.width = m_parent->getDevice()->getSwapChain()->getBufferSize().width();
            frameBufferInfo.height = m_parent->getDevice()->getSwapChain()->getBufferSize().height();
            frameBufferInfo.layers = 1;

            VkFramebuffer frameBuffer;

            if (::vkCreateFramebuffer(m_parent->getDevice()->handle(), &frameBufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
                throw std::runtime_error("Unable to create frame buffer from swap chain frame.");

            m_attachmentImages[i++] = std::move(attachmentImages);

            return frameBuffer;
        });

        // Store the buffers.
        m_frameBuffers = frameBuffers;
    }

    void begin()
    {
        // Swap out the back buffer, if the render pass has a present target. Otherwise increment the current frame buffer anyways.
        // NOTE: Maybe this can be refactored to a boolean parameter `swapBackBuffer` which defaults to `false`?
        m_backBuffer = m_dependency == nullptr ? m_swapChain->swapBackBuffer() : m_dependency->m_impl->m_backBuffer;

        // Get current command buffer.
        auto commandBuffer = this->getCurrentCommandBuffer();
        commandBuffer->begin();

        // Begin the render pass.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_parent->handle();
        renderPassInfo.framebuffer = m_frameBuffers[m_backBuffer];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = static_cast<UInt32>(m_parent->getDevice()->getBufferWidth());
        renderPassInfo.renderArea.extent.height = static_cast<UInt32>(m_parent->getDevice()->getBufferHeight());
        renderPassInfo.clearValueCount = m_clearValues.size();
        renderPassInfo.pClearValues = m_clearValues.data();

        ::vkCmdBeginRenderPass(commandBuffer->handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void end(const bool present = false)
    {
        auto commandBuffer = this->getCurrentCommandBuffer();
        ::vkCmdEndRenderPass(commandBuffer->handle());
        commandBuffer->end();

        // Submit the command buffer.

        if (!present)
            commandBuffer->submit({}, {});
        else
        {
            Array<VkSemaphore> waitForSemaphores = { m_swapChain->getCurrentSemaphore() };
            Array<VkPipelineStageFlags> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
            Array<VkSemaphore> signalSemaphores = { this->getCurrentSemaphore() };

            commandBuffer->submit(waitForSemaphores, waitForStages, signalSemaphores, false);

            // Draw the frame, if the result of the render pass it should be presented to the swap chain.
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = signalSemaphores.size();
            presentInfo.pWaitSemaphores = signalSemaphores.data();
            presentInfo.pImageIndices = &m_backBuffer;
            presentInfo.pResults = nullptr;

            VkSwapchainKHR swapChains[] = { m_swapChain->handle() };
            presentInfo.pSwapchains = swapChains;
            presentInfo.swapchainCount = 1;

            if (::vkQueuePresentKHR(m_queue->handle(), &presentInfo) != VK_SUCCESS)
                throw std::runtime_error("Unable to present swap chain.");
        }
    }

public:
    const VulkanCommandBuffer* getCurrentCommandBuffer() const noexcept
    {
        return m_commandBuffers[m_backBuffer].get();
    }

    const VkSemaphore& getCurrentSemaphore() const noexcept
    {
        return m_semaphores[m_backBuffer];
    }

    void addTarget(UniquePtr<IRenderTarget>&& target) 
    {
        m_targets.push_back(std::move(target));
    }
    
    Array<const IRenderTarget*> getTargets() const noexcept 
    {
        Array<const IRenderTarget*> targets(m_targets.size());
        std::generate(std::begin(targets), std::end(targets), [&, i = 0]() mutable { return m_targets[i++].get(); });

        return targets;
    }
    
    UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) 
    {
        auto it = std::find_if(std::begin(m_targets), std::end(m_targets), [target](const UniquePtr<IRenderTarget>& t) { return t.get() == target; });

        if (it == m_targets.end())
            return UniquePtr<IRenderTarget>();
        else
        {
            auto result = std::move(*it);
            m_targets.erase(it);

            return std::move(result);
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPass::VulkanRenderPass(const IGraphicsDevice& device) : 
    VulkanRenderPass(dynamic_cast<const VulkanDevice&>(device)) 
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this)), VulkanRuntimeObject<VulkanDevice>(device, &device), IResource<VkRenderPass>(nullptr)
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept
{
    m_impl->cleanup();
}

const VulkanCommandBuffer* VulkanRenderPass::getVkCommandBuffer() const noexcept
{
    return m_impl->getCurrentCommandBuffer();
}

const ICommandBuffer* VulkanRenderPass::getCommandBuffer() const noexcept
{
    return m_impl->getCurrentCommandBuffer();
}

const UInt32 VulkanRenderPass::getCurrentBackBuffer() const
{
    return m_impl->m_backBuffer;
}

void VulkanRenderPass::addTarget(UniquePtr<IRenderTarget>&& target)
{
    m_impl->addTarget(std::move(target));
}

const Array<const IRenderTarget*> VulkanRenderPass::getTargets() const noexcept
{
    return m_impl->getTargets();
}

UniquePtr<IRenderTarget> VulkanRenderPass::removeTarget(const IRenderTarget* target)
{
    return m_impl->removeTarget(target);
}

Array<const IRenderPipeline*> VulkanRenderPass::getPipelines() const noexcept
{
    Array<const IRenderPipeline*> pipelines(m_impl->m_pipelines.size());
    std::generate(std::begin(pipelines), std::end(pipelines), [&, i = 0]() mutable { return m_impl->m_pipelines[i++].get(); });
    
    return pipelines;
}

const IRenderPipeline* VulkanRenderPass::getPipeline(const UInt32& id) const noexcept
{
    auto match = std::find_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; });

    return match == m_impl->m_pipelines.end() ? nullptr : match->get();
}

void VulkanRenderPass::addPipeline(UniquePtr<IRenderPipeline>&& pipeline)
{
    if (pipeline == nullptr)
        throw ArgumentNotInitializedException("The pipeline must be initialized.");

    auto id = pipeline->id();
    auto match = std::find_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; });

    if (match != m_impl->m_pipelines.end())
        throw InvalidArgumentException("Another pipeline with the ID {0} already has been registered. Pipeline IDs must be unique within a render pass.", id);

    m_impl->m_pipelines.push_back(std::move(pipeline));
}

void VulkanRenderPass::removePipeline(const UInt32& id)
{
    m_impl->m_pipelines.erase(std::remove_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; }), std::end(m_impl->m_pipelines));
}

void VulkanRenderPass::setDependency(const IRenderPass* renderPass)
{
    auto dependency = dynamic_cast<const VulkanRenderPass*>(renderPass);

    if (dependency == nullptr)
        throw std::invalid_argument("The render pass dependency must be a valid Vulkan render pass.");

    m_impl->m_dependency = dependency;
}

const IRenderPass* VulkanRenderPass::getDependency() const noexcept
{
    return m_impl->m_dependency;
}

void VulkanRenderPass::begin() const
{
    m_impl->begin();
}

void VulkanRenderPass::end(const bool& present)
{
    m_impl->end(present);
}

void VulkanRenderPass::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
    ::vkCmdDraw(m_impl->getCurrentCommandBuffer()->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanRenderPass::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
    ::vkCmdDrawIndexed(m_impl->getCurrentCommandBuffer()->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

const IImage* VulkanRenderPass::getAttachment(const UInt32& attachmentId) const
{
    if (m_impl->m_attachmentImages[m_impl->m_backBuffer].size() <= attachmentId)
        throw std::invalid_argument(fmt::format("Invalid attachment index ({0}, but expected {1} or less).", attachmentId, m_impl->m_attachmentImages[m_impl->m_backBuffer].size() - 1));

    return m_impl->m_attachmentImages[m_impl->m_backBuffer][attachmentId].get();
}

void VulkanRenderPass::resetFramebuffer()
{
    m_impl->createFramebuffer(this->handle());
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassBuilder::VulkanRenderPassBuilder(UniquePtr<VulkanRenderPass>&& instance) :
    RenderPassBuilder(std::move(instance))
{
}

VulkanRenderPassBuilder::~VulkanRenderPassBuilder() noexcept = default;

UniquePtr<VulkanRenderPass> VulkanRenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize();

    return RenderPassBuilder::go();
}

void VulkanRenderPassBuilder::use(UniquePtr<IRenderTarget>&& target)
{
    this->instance()->addTarget(std::move(target));
}

void VulkanRenderPassBuilder::use(UniquePtr<IRenderPipeline>&& pipeline)
{
    this->instance()->addPipeline(std::move(pipeline));
}

VulkanRenderPipelineBuilder VulkanRenderPassBuilder::addPipeline(const UInt32& id, const String& name)
{
    return this->make<VulkanRenderPipeline>(id, name);
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clear, bool clearStencil, bool isVolatile)
{
    UniquePtr<IRenderTarget> target = makeUnique<RenderTarget>();
    target->setType(type);
    target->setFormat(format);
    target->setSamples(samples);
    target->setClearBuffer(clear);
    target->setClearStencil(clearStencil);
    target->setVolatile(isVolatile);
    target->setClearValues(clearValues);

    this->use(std::move(target));

    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::dependsOn(const IRenderPass* renderPass)
{
    if (renderPass == nullptr)
        throw std::invalid_argument("The render pass must be initialized.");

    this->instance()->setDependency(renderPass);

    return *this;
}