#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl {
public:
    friend class VulkanRenderPassBuilder;

private:
    const VulkanRenderPipeline& m_pipeline;
    const VulkanDevice* m_device{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    Array<VkFramebuffer> m_frameBuffers;
    UniquePtr<const VulkanCommandBuffer> m_commandBuffer;
    UInt32 m_currentFrameBuffer{ 0 };

public:
    VulkanRenderPassImpl(const VulkanRenderPipeline& pipeline) noexcept : m_pipeline(pipeline) {}

public:
    VkRenderPass initialize(const VulkanRenderPass& parent)
    {
        m_device = dynamic_cast<const VulkanDevice*>(m_pipeline.getDevice());

        if (m_device == nullptr)
            throw std::invalid_argument("The provided pipelines' device is not a valid Vulkan device.");

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
        auto frames = m_device->getSwapChain()->getFrames();
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
        auto commandBuffer = m_device->createCommandBuffer();
        auto vulkanCommandBuffer = dynamic_cast<const VulkanCommandBuffer*>(commandBuffer.get());

        if (vulkanCommandBuffer == nullptr)
            throw std::runtime_error("The device has not returned a valid Vulkan command buffer.");
        else
        {
            m_commandBuffer = UniquePtr<const VulkanCommandBuffer>(vulkanCommandBuffer);
            commandBuffer.release();
        }

        // Return the render pass.
        return renderPass;
    }

    void begin(const VulkanRenderPass& parent) const
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (::vkBeginCommandBuffer(m_commandBuffer->handle(), &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Unable to begin render pass on command buffer.");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = parent.handle();
        renderPassInfo.framebuffer = m_frameBuffers[0];     // TODO: Select current back buffer.
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

    void end() const
    {
        ::vkCmdEndRenderPass(m_commandBuffer->handle());

        if (::vkEndCommandBuffer(m_commandBuffer->handle()) != VK_SUCCESS)
            throw std::runtime_error("Unable to end render pass on command buffer.");

        auto image = m_device->getSwapChain()->swapFrontBuffer();
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
    IResource<VkRenderPass>(nullptr), m_impl(makePimpl<VulkanRenderPassImpl>(pipeline))
{
}

VulkanRenderPass::~VulkanRenderPass() noexcept = default;

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
    this->handle() = m_impl->initialize(*this);
}

void VulkanRenderPass::begin() const
{
    m_impl->begin(*this);
}

void VulkanRenderPass::end() const
{
    m_impl->end();
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