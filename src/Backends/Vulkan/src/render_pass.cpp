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
    const VulkanRenderPipeline& m_pipeline;
    const VulkanDevice* m_device{ nullptr };
    const VulkanSwapChain* m_swapChain{ nullptr };
    const VulkanQueue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    Array<VkFramebuffer> m_frameBuffers;
    UniquePtr<const VulkanCommandBuffer> m_commandBuffer;
    UInt32 m_currentFrameBuffer{ 0 };
    VkSemaphore m_semaphore;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanRenderPipeline& pipeline) : base(parent), m_pipeline(pipeline) { }

    ~VulkanRenderPassImpl()
    {
        ::vkDestroySemaphore(m_device->handle(), m_semaphore, nullptr);
    }

public:
    VkRenderPass initialize()
    {
        // Get the device and swap chain.
        m_device = dynamic_cast<const VulkanDevice*>(m_pipeline.getDevice());

        if (m_device == nullptr)
            throw std::invalid_argument("The provided pipelines' device is not a valid Vulkan device.");

        m_swapChain = dynamic_cast<const VulkanSwapChain*>(m_device->getSwapChain());

        if (m_swapChain == nullptr)
            throw std::invalid_argument("The device swap chain is not a valid Vulkan swap chain.");
        
        m_queue = dynamic_cast<const VulkanQueue*>(m_device->getGraphicsQueue());

        if (m_queue == nullptr)
            throw std::invalid_argument("The device queue is not a valid Vulkan command queue.");

        // Setup the attachments.
        Array<VkAttachmentDescription> attachments(m_targets.size());

        std::generate(std::begin(attachments), std::end(attachments), [&, i = 0]() mutable {
            auto& target = m_targets[i++];

            VkAttachmentDescription attachment{};
            attachment.format = getFormat(target->getFormat());
            attachment.samples = getSamples(target->getSamples());
            attachment.loadOp = target->getClearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilLoadOp = target->getClearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.storeOp = target->getVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            switch (target->getType())
            {
            case RenderTargetType::Color:
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            case RenderTargetType::Depth:
                attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case RenderTargetType::Present:
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                //attachment.initialLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                break;
            case RenderTargetType::Transfer:
                attachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                break;
            }

            return attachment;
        });

        // Setup attachment references.
        // NOTE: Since we are currently only using one sub-pass, the references are a sequential array to each attachment.
        Array<VkAttachmentReference> references(m_targets.size());

        std::generate(std::begin(references), std::end(references), [&, i = 0]() mutable {
            VkAttachmentReference reference{};
            reference.attachment = i;
            reference.layout = attachments[i++].finalLayout;

            return reference;
        });

        // Setup the sub-pass.
        // NOTE: This has room for optimization. Vulkan supports sub-passes, whereas DX12 only supports individual render passes. Hence we are currently only using one sub-pass 
        //       per render pass.
        VkSubpassDescription subPass{};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = static_cast<UInt32>(references.size());
        subPass.pColorAttachments = references.data();

        // Setup render pass state.
        VkRenderPassCreateInfo renderPassState{};
        renderPassState.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassState.attachmentCount = static_cast<UInt32>(attachments.size());
        renderPassState.pAttachments = attachments.data();
        renderPassState.subpassCount = 1;
        renderPassState.pSubpasses = &subPass;

        // Create the render pass.
        VkRenderPass renderPass;

        if (::vkCreateRenderPass(m_device->handle(), &renderPassState, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass.");

        // Initialize frame buffers.
        auto frames = m_swapChain->getFrames();
        Array<VkFramebuffer> frameBuffers(frames.size());

        LITEFX_TRACE(VULKAN_LOG, "Initializing {0} frame buffers...", frames.size());
        
        std::generate(std::begin(frameBuffers), std::end(frameBuffers), [&, i = 0]() mutable {
            auto frame = dynamic_cast<const VulkanTexture*>(frames[i++]);

            if (frame == nullptr)
                throw std::invalid_argument("A frame of the provided swap chain is not a valid Vulkan texture.");
            
            VkImageView attachments[]{ frame->getView() };

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = renderPass;
            frameBufferInfo.attachmentCount = 1;
            frameBufferInfo.pAttachments = attachments;
            frameBufferInfo.width = frame->getSize().width();
            frameBufferInfo.height = frame->getSize().height();
            frameBufferInfo.layers = 1;

            VkFramebuffer frameBuffer;

            if (::vkCreateFramebuffer(m_device->handle(), &frameBufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
                throw std::runtime_error("Unable to create frame buffer from swap chain frame.");

            return frameBuffer;
        });

        // Store the buffers.
        m_frameBuffers = frameBuffers;

        // Create a command buffer.
        m_commandBuffer = makeUnique<const VulkanCommandBuffer>(dynamic_cast<const VulkanQueue*>(m_device->getGraphicsQueue()));

        // Create a semaphore that signals if the render pass has finished.
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (::vkCreateSemaphore(m_device->handle(), &semaphoreInfo, nullptr, &m_semaphore) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass semaphore.");

        // Return the render pass.
        return renderPass;
    }

    void begin()
    {
        m_commandBuffer->begin();

        // Swap out the back buffer.
        m_currentFrameBuffer = m_swapChain->swapBackBuffer();

        // Begin the render pass.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_parent->handle();
        renderPassInfo.framebuffer = m_frameBuffers[m_currentFrameBuffer];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = static_cast<UInt32>(m_device->getBufferWidth());
        renderPassInfo.renderArea.extent.height = static_cast<UInt32>(m_device->getBufferHeight());

        // VkClearValue backColor = m_device->getBackColor();
        VkClearValue backColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &backColor;

        ::vkCmdBeginRenderPass(m_commandBuffer->handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        ::vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());
    }

    void end(const bool present = false)
    {
        ::vkCmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkSemaphore waitForSemaphores[] = { m_swapChain->getSemaphore() };
        VkPipelineStageFlags waitForStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        VkSemaphore signalSemaphores[] = { m_semaphore };

        // Submit the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitSemaphores = waitForSemaphores;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = waitForStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer->handle();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        auto result = ::vkQueueSubmit(m_queue->handle(), 1, &submitInfo, VK_NULL_HANDLE);

        if (result != VK_SUCCESS)
        {
            LITEFX_ERROR(VULKAN_LOG, "Error ({0}) submitting render pass to graphics queue.", result);
            throw std::runtime_error("Failed to submit render pass to device queue!");
        }

        // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        if (present)
        {
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pImageIndices = &m_currentFrameBuffer;
            presentInfo.pResults = nullptr;

            VkSwapchainKHR swapChains[] = { m_swapChain->handle() };
            presentInfo.pSwapchains = swapChains;
            presentInfo.swapchainCount = 1;

            if (::vkQueuePresentKHR(m_queue->handle(), &presentInfo) != VK_SUCCESS)
                throw std::runtime_error("Unable to present swap chain.");
        }
    }

public:
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

VulkanRenderPass::VulkanRenderPass(const VulkanRenderPipeline& pipeline) :
    IResource<VkRenderPass>(nullptr), RuntimeObject(pipeline.getDevice()), m_impl(makePimpl<VulkanRenderPassImpl>(this, pipeline))
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept = default;

const ICommandBuffer* VulkanRenderPass::getCommandBuffer() const noexcept
{
    return m_impl->m_commandBuffer.get();
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

void VulkanRenderPass::create()
{
    this->handle() = m_impl->initialize();
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
    ::vkCmdDraw(m_impl->m_commandBuffer->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanRenderPass::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
    ::vkCmdDrawIndexed(m_impl->m_commandBuffer->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder& VulkanRenderPassBuilder::go()
{
    this->instance()->create();
    return RenderPassBuilder::go();
}

void VulkanRenderPassBuilder::use(UniquePtr<IRenderTarget>&& target)
{
    this->instance()->addTarget(std::move(target));
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::withColorTarget(const MultiSamplingLevel& samples)
{
    auto swapChain = this->instance()->m_impl->m_pipeline.getDevice()->getSwapChain();
    this->addTarget(RenderTargetType::Color, swapChain->getFormat(), samples, true, true, false);
    
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::addTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, bool clearColor, bool clearStencil, bool isVolatile)
{
    UniquePtr<IRenderTarget> target = makeUnique<RenderTarget>();
    target->setType(type);
    target->setFormat(format);
    target->setSamples(samples);
    target->setClearBuffer(clearColor);
    target->setClearStencil(clearStencil);
    target->setVolatile(isVolatile);

    this->use(std::move(target));

    return *this;
}