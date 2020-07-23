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
    const VulkanRenderPipeline& m_pipeline;
    const VulkanDevice* m_device{ nullptr };
    const VulkanSwapChain* m_swapChain{ nullptr };
    const VulkanQueue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    Array<UniquePtr<IImage>> m_depthViews;
    Array<VkClearValue> m_clearValues;
    Array<VkFramebuffer> m_frameBuffers;
    UniquePtr<const VulkanCommandBuffer> m_commandBuffer;
    UInt32 m_currentFrameBuffer{ 0 };
    VkSemaphore m_semaphore;

public:
    VulkanRenderPassImpl(VulkanRenderPass* parent, const VulkanRenderPipeline& pipeline) : base(parent), m_pipeline(pipeline) { }

private:
    void cleanup()
    {
        m_depthViews.clear();
        m_clearValues.clear();

        ::vkDestroyRenderPass(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);
        ::vkDestroySemaphore(m_device->handle(), m_semaphore, nullptr);

        std::for_each(std::begin(m_frameBuffers), std::end(m_frameBuffers), [&](VkFramebuffer& frameBuffer) {
            ::vkDestroyFramebuffer(m_parent->getDevice()->handle(), frameBuffer, nullptr);
        });
    }

    UniquePtr<IImage> makeDepthView(const IRenderTarget* target)
    {
        return m_parent->getDevice()->createImage(target->getFormat(), m_parent->getDevice()->getSwapChain()->getBufferSize());
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
        Array<VkAttachmentReference> colorAttachments;
        Array<VkAttachmentReference> depthAttachments;

        std::generate(std::begin(attachments), std::end(attachments), [&, i = 0]() mutable {
            auto& target = m_targets[i];

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

                colorAttachments.push_back({ static_cast<UInt32>(i),  attachment.finalLayout });
                break;
            case RenderTargetType::Depth:
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment.finalLayout = ::hasStencil(target->getFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                
                m_depthViews.push_back(this->makeDepthView(target.get()));
                depthAttachments.push_back({ static_cast<UInt32>(i),  attachment.finalLayout });
                break;
            case RenderTargetType::Present:
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                colorAttachments.push_back({ static_cast<UInt32>(i),  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
                break;
            case RenderTargetType::Transfer:
                attachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                break;
            }

            i++;
            return attachment;
        });

        if (depthAttachments.size() > 1)
            LITEFX_WARNING(VULKAN_LOG, "The render pass has been attached with multiple depth targets, however Vulkan only supports one depth/stencil target.");

        // Setup the sub-pass.
        // NOTE: This has room for optimization. Vulkan supports sub-passes, whereas DX12 only supports individual render passes. Hence we are currently only using one sub-pass 
        //       per render pass.
        VkSubpassDescription subPass{};
        subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPass.colorAttachmentCount = static_cast<UInt32>(colorAttachments.size());
        subPass.pColorAttachments = colorAttachments.data();
        subPass.pDepthStencilAttachment = depthAttachments.size() > 0 ? &depthAttachments.front() : nullptr;

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
            Array<VkImageView> attachments;

            auto swapChainImage = dynamic_cast<const IVulkanImage*>(frames[i++]);
            
            if (swapChainImage == nullptr)
                throw std::invalid_argument("A frame of the provided swap chain is not a valid Vulkan texture.");
            
            attachments.push_back(swapChainImage->getImageView());

            std::for_each(std::begin(m_depthViews), std::end(m_depthViews), [&](const auto& view) { 
                attachments.push_back(dynamic_cast<const IVulkanImage*>(view.get())->getImageView());
            });

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = renderPass;
            frameBufferInfo.attachmentCount = static_cast<UInt32>(attachments.size());
            frameBufferInfo.pAttachments = attachments.data();
            frameBufferInfo.width = swapChainImage->getExtent().width();
            frameBufferInfo.height = swapChainImage->getExtent().height();
            frameBufferInfo.layers = 1;

            VkFramebuffer frameBuffer;

            if (::vkCreateFramebuffer(m_device->handle(), &frameBufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
                throw std::runtime_error("Unable to create frame buffer from swap chain frame.");

            return frameBuffer;
        });

        // Store the buffers.
        m_frameBuffers = frameBuffers;

        // Create a command buffer.
        if (m_commandBuffer == nullptr)
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
        renderPassInfo.clearValueCount = m_clearValues.size();
        renderPassInfo.pClearValues = m_clearValues.data();

        ::vkCmdBeginRenderPass(m_commandBuffer->handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        ::vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());
    }

    void end(const bool present = false)
    {
        ::vkCmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        // Submit the command buffer.
        Array<VkSemaphore> waitForSemaphores = { m_swapChain->getSemaphore() };
        Array<VkPipelineStageFlags> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        Array<VkSemaphore> signalSemaphores = { m_semaphore };
        m_commandBuffer->submit(waitForSemaphores, waitForStages, signalSemaphores, false);

        // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        if (present)
        {
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = signalSemaphores.size();
            presentInfo.pWaitSemaphores = signalSemaphores.data();
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
    IResource<VkRenderPass>(nullptr), VulkanRuntimeObject(pipeline.getDevice()), m_impl(makePimpl<VulkanRenderPassImpl>(this, pipeline))
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept
{
    m_impl->cleanup();
}

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
    if (this->handle() != nullptr)
        throw std::runtime_error("The render pass can only created once.");

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

void VulkanRenderPass::reset()
{
    m_impl->cleanup();
    this->handle() = m_impl->initialize();
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

VulkanRenderPassBuilder& VulkanRenderPassBuilder::attachColorTarget(const bool& clear, const Vector4f& clearColor)
{
    auto swapChain = this->instance()->m_impl->m_pipeline.getDevice()->getSwapChain();
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
    auto swapChain = this->instance()->m_impl->m_pipeline.getDevice()->getSwapChain();
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