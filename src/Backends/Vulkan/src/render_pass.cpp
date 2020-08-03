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
    UniquePtr<IRenderPipeline> m_pipeline;
    const VulkanSwapChain* m_swapChain{ nullptr };
    const VulkanQueue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    Array<VkClearValue> m_clearValues;
    Array<VkFramebuffer> m_frameBuffers;
    Array<UniquePtr<VulkanCommandBuffer>> m_commandBuffers;
    UInt32 m_backBuffer{ 0 };
    Array<VkSemaphore> m_semaphores;
    const IRenderPass* m_dependency{ nullptr };
    bool m_present{ false };

    /// <summary>
    /// Stores the images for all attachments, except the present attachment.
    /// </summary>
    Array<UniquePtr<IImage>> m_attachmentImages;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent) : base(parent) { }

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
        
        m_queue = dynamic_cast<const VulkanQueue*>(m_parent->getDevice()->getGraphicsQueue());

        if (m_queue == nullptr)
            throw std::invalid_argument("The device queue is not a valid Vulkan command queue.");

        // Setup the attachments.
        Array<VkAttachmentDescription> attachments;
        Array<VkAttachmentReference> colorAttachments;
        Optional<VkAttachmentReference> depthAttachment, presentAttachment;

        std::for_each(std::begin(m_targets), std::end(m_targets), [&, i = 0](const auto& target) mutable {
            if ((target->getType() == RenderTargetType::Depth && depthAttachment.has_value()) || (target->getType() == RenderTargetType::Present && presentAttachment.has_value()))
                throw std::runtime_error(fmt::format("Invalid render target {0}: only one target attachment of type {1} is allowed.", i, target->getType()));
            else
            {
                VkAttachmentDescription attachment{};
                attachment.format = getFormat(target->getFormat());
                attachment.samples = getSamples(target->getSamples());
                attachment.loadOp = target->getClearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilLoadOp = target->getClearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.storeOp = target->getVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                if (target->getClearBuffer() || target->getClearStencil())
                    m_clearValues.push_back(VkClearValue{ target->getClearValues().x(),target->getClearValues().y(),target->getClearValues().z(),target->getClearValues().w() });

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

                    presentAttachment = VkAttachmentReference { static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
                    colorAttachments.push_back(presentAttachment.value());
                    m_present = true;
                    break;
                }

                attachments.push_back(attachment);
            }
        });

        // Get the color and depths attachments of the dependency.
        Array<VkAttachmentReference> inputAttachments;

        // TODO: Map input attachments
        //if (m_dependency != nullptr)
        //{
        //    inputAttachments.resize(m_dependency->getTargets().size());
        //    std::generate(std::begin(inputAttachments), std::end(inputAttachments), [i = 0]() mutable { return VkAttachmentReference{ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }; });
        //}

        // Setup the sub-pass.
        // NOTE: No need to set up a VK_SUBPASS_EXTERNAL dependency here, since it is done implicitly between individual VkRenderPass instances.
        VkSubpassDescription subPass{};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = static_cast<UInt32>(colorAttachments.size());
        subPass.pColorAttachments = colorAttachments.data();
        subPass.pDepthStencilAttachment = depthAttachment.has_value() ? &depthAttachment.value() : nullptr;
        subPass.inputAttachmentCount = static_cast<UInt32>(inputAttachments.size());
        subPass.pInputAttachments = inputAttachments.data();

        // Setup render pass state.
        VkRenderPassCreateInfo renderPassState{};
        renderPassState.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassState.attachmentCount = static_cast<UInt32>(attachments.size());
        renderPassState.pAttachments = attachments.data();
        renderPassState.subpassCount = 1;
        renderPassState.pSubpasses = &subPass;

        // Create the render pass.
        VkRenderPass renderPass;

        if (::vkCreateRenderPass(m_parent->getDevice()->handle(), &renderPassState, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass.");

        // Initialize frame buffers.
        auto frames = m_swapChain->getFrames();
        Array<VkFramebuffer> frameBuffers(frames.size());

        LITEFX_TRACE(VULKAN_LOG, "Initializing {0} frame buffers...", frames.size());
        
        std::generate(std::begin(frameBuffers), std::end(frameBuffers), [&, i = 0]() mutable {
            Array<VkImageView> attachmentViews;

            std::for_each(std::begin(m_targets), std::end(m_targets), [&, a = 0](const auto& target) mutable {
                if (presentAttachment.has_value() && a++ == presentAttachment->attachment)
                {
                    // Acquire an image view from the swap chain.
                    auto swapChainImage = dynamic_cast<const IVulkanImage*>(frames[i++]);

                    if (swapChainImage == nullptr)
                        throw std::invalid_argument("A frame of the provided swap chain is not a valid Vulkan texture.");

                    // TODO: Create an image view for each attachment.
                    attachmentViews.push_back(swapChainImage->getImageView());
                }
                else
                {
                    // Create an image view for the render target.
                    auto image = this->makeImageView(target.get());
                    attachmentViews.push_back(dynamic_cast<const IVulkanImage*>(image.get())->getImageView());
                    m_attachmentImages.push_back(std::move(image));
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

            return frameBuffer;
        });

        // Store the buffers.
        m_frameBuffers = frameBuffers;

        // Create a command buffer.
        if (m_commandBuffers.empty())
        {
            m_commandBuffers.resize(frames.size());
            std::generate(std::begin(m_commandBuffers), std::end(m_commandBuffers), [&]() mutable { return makeUnique<VulkanCommandBuffer>(dynamic_cast<const VulkanQueue*>(m_parent->getDevice()->getGraphicsQueue())); });
        }

        // Create a semaphore that signals if the render pass has finished.
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        m_semaphores.resize(frames.size());
        std::generate(std::begin(m_semaphores), std::end(m_semaphores), [&]() mutable {
            VkSemaphore semaphore;

            if (::vkCreateSemaphore(m_parent->getDevice()->handle(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
                throw std::runtime_error("Unable to create signal semaphore.");

            return semaphore;
        });

        // Return the render pass.
        return renderPass;
    }

    void begin()
    {
        auto pipeline = dynamic_cast<const IResource<VkPipeline>*>(m_pipeline.get());

        if (pipeline == nullptr)
            throw std::runtime_error("The pipeline of the render pass is not a valid Vulkan pipeline.");

        // Swap out the back buffer, if the render pass has a present target. Otherwise increment the current frame buffer anyways.
        m_backBuffer = m_present ? m_swapChain->swapBackBuffer() : (m_backBuffer + 1) % static_cast<UInt32>(m_frameBuffers.size());

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
        ::vkCmdBindPipeline(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
    }

    void end(const bool present = false)
    {
        auto commandBuffer = this->getCurrentCommandBuffer();
        ::vkCmdEndRenderPass(commandBuffer->handle());
        commandBuffer->end();

        // Submit the command buffer.
        if (!m_present)
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

VulkanRenderPass::VulkanRenderPass(const IGraphicsDevice* device) : 
    VulkanRenderPass(dynamic_cast<const VulkanDevice*>(device)) 
{
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice* device) :
    m_impl(makePimpl<VulkanRenderPassImpl>(this)), VulkanRuntimeObject(device), IResource<VkRenderPass>(nullptr)
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept
{
    m_impl->cleanup();
}

const ICommandBuffer* VulkanRenderPass::getCommandBuffer() const noexcept
{
    return m_impl->getCurrentCommandBuffer();
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

void VulkanRenderPass::setDependency(const IRenderPass* renderPass)
{
    m_impl->m_dependency = renderPass;
}

const IRenderPass* VulkanRenderPass::getDependency() const noexcept
{
    return m_impl->m_dependency;
}

const IRenderPipeline* VulkanRenderPass::getPipeline() const noexcept
{
    return m_impl->m_pipeline.get();
}

IRenderPipeline* VulkanRenderPass::getPipeline() noexcept
{
    return m_impl->m_pipeline.get();
}

void VulkanRenderPass::begin() const
{
    m_impl->begin();
}

void VulkanRenderPass::end(const bool& present)
{
    m_impl->end(present);
}

void VulkanRenderPass::reset()
{
    m_impl->cleanup();
    this->handle() = m_impl->initialize();

    m_impl->m_pipeline->bind(this);
}

void VulkanRenderPass::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
    ::vkCmdDraw(m_impl->getCurrentCommandBuffer()->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanRenderPass::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
    ::vkCmdDrawIndexed(m_impl->getCurrentCommandBuffer()->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderPass::bind(const IVertexBuffer* buffer) const
{
    auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
    auto commandBuffer = m_impl->getCurrentCommandBuffer();

    if (resource == nullptr)
        throw std::invalid_argument("The provided vertex buffer is not a valid Vulkan buffer.");

    // Depending on the type, bind the buffer accordingly.
    constexpr VkDeviceSize offsets[] = { 0 };

    ::vkCmdBindVertexBuffers(commandBuffer->handle(), 0, 1, &resource->handle(), offsets);
}

void VulkanRenderPass::bind(const IIndexBuffer* buffer) const
{
    auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
    auto commandBuffer = m_impl->getCurrentCommandBuffer();

    if (resource == nullptr)
        throw std::invalid_argument("The provided index buffer is not a valid Vulkan buffer.");

    ::vkCmdBindIndexBuffer(commandBuffer->handle(), resource->handle(), 0, buffer->getLayout()->getIndexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderPass::bind(IDescriptorSet* descriptorSet) const
{
    if (descriptorSet == nullptr)
        throw std::invalid_argument("The descriptor set must be initialized.");

    descriptorSet->bind(this);
}

UniquePtr<IVertexBuffer> VulkanRenderPass::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding) const
{
    return this->getDevice()->createVertexBuffer(this->getPipeline()->getLayout()->getInputAssembler()->getVertexBufferLayout(binding), usage, elements);
}

UniquePtr<IIndexBuffer> VulkanRenderPass::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
    return this->getDevice()->createIndexBuffer(this->getPipeline()->getLayout()->getInputAssembler()->getIndexBufferLayout(), usage, elements);
}

UniquePtr<IDescriptorSet> VulkanRenderPass::makeBufferPool(const UInt32& setId) const
{
    auto layouts = this->getPipeline()->getLayout()->getProgram()->getLayouts();
    auto match = std::find_if(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) { return layout->getSetId() == setId; });

    if (match == layouts.end())
        throw std::invalid_argument("The requested buffer set is not defined.");

    return (*match)->createBufferPool();
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

    auto pipeline = instance->getPipeline();
    
    if (pipeline != nullptr)
        pipeline->bind(instance);

    return RenderPassBuilder::go();
}

void VulkanRenderPassBuilder::use(UniquePtr<IRenderTarget>&& target)
{
    this->instance()->addTarget(std::move(target));
}

void VulkanRenderPassBuilder::use(UniquePtr<IRenderPipeline>&& pipeline)
{
    if (pipeline == nullptr)
        throw std::invalid_argument("The pipeline must be initialized.");

    this->instance()->m_impl->m_pipeline = std::move(pipeline);
}

VulkanRenderPipelineBuilder VulkanRenderPassBuilder::setPipeline()
{
    return this->make<VulkanRenderPipeline>();
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachColorTarget(const bool& clear, const Vector4f& clearColor)
{
    auto swapChain = this->instance()->getDevice()->getSwapChain();
    this->attachTarget(RenderTargetType::Color, swapChain->getFormat(), MultiSamplingLevel::x1, clearColor, clear, false, false);

    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachDepthTarget(const bool& clear, const bool& clearStencil, const Vector2f& clearValues, const Format& format)
{
    this->attachTarget(RenderTargetType::Depth, format, MultiSamplingLevel::x1, { clearValues.x(), clearValues.y(), 0.0f, 0.0f }, clear, clearStencil, false);

    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachPresentTarget(const bool& clear, const Vector4f& clearColor, const MultiSamplingLevel& samples)
{
    auto swapChain = this->instance()->getDevice()->getSwapChain();
    this->attachTarget(RenderTargetType::Present, swapChain->getFormat(), samples, clearColor, clear, false, false);
    
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clearColor, bool clearStencil, bool isVolatile)
{
    UniquePtr<IRenderTarget> target = makeUnique<RenderTarget>();
    target->setType(type);
    target->setFormat(format);
    target->setSamples(samples);
    target->setClearBuffer(clearColor);
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