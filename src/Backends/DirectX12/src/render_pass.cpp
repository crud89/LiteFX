#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPass::DirectX12RenderPassImpl : public Implement<DirectX12RenderPass> {
public:
    friend class DirectX12RenderPassBuilder;
    friend class DirectX12RenderPass;

private:
    const DirectX12Device* m_device;
    //UniquePtr<IRenderPipeline> m_pipeline;
    //const DirectX12SwapChain* m_swapChain{ nullptr };
    //const DirectX12Queue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    //Array<VkClearValue> m_clearValues;
    //Array<VkFramebuffer> m_frameBuffers;
    //Array<UniquePtr<DirectX12CommandBuffer>> m_commandBuffers;
    //UInt32 m_backBuffer{ 0 };
    //Array<VkSemaphore> m_semaphores;
    //const DirectX12RenderPass* m_dependency{ nullptr };
    //bool m_present{ false };

    ///// <summary>
    ///// Stores the images for all attachments (except the present attachment, which is a swap-chain image) and maps them to the frame buffer index.
    ///// </summary>
    //Dictionary<UInt32, Array<UniquePtr<IImage>>> m_attachmentImages;

public:
    DirectX12RenderPassImpl(DirectX12RenderPass* parent, const DirectX12Device* device) : 
        base(parent), m_device(device)
    {
        if (device == nullptr)
            throw ArgumentNotInitializedException("The device is not a valid DirectX 12 device.");
    }

private:
    void cleanup()
    {
    }

public:
    void initialize()
    {
    //    // Store swap chain and graphics queue.
    //    m_swapChain = dynamic_cast<const DirectX12SwapChain*>(m_parent->getDevice()->getSwapChain());

    //    if (m_swapChain == nullptr)
    //        throw std::invalid_argument("The device swap chain is not a valid DirectX12 swap chain.");

    //    m_queue = dynamic_cast<const DirectX12Queue*>(m_parent->getDevice()->graphicsQueue());

    //    if (m_queue == nullptr)
    //        throw std::invalid_argument("The device queue is not a valid DirectX12 command queue.");

    //    // Get the render targets of the render pass dependency, if there is any.
    //    Array<const IRenderTarget*> dependencyTargets;

    //    if (m_dependency != nullptr)
    //        dependencyTargets = m_dependency->getTargets();

    //    // Setup the attachments.
    //    Array<VkAttachmentDescription> attachments;
    //    Array<VkAttachmentReference> colorAttachments;
    //    Array<VkAttachmentReference> inputAttachments;
    //    Optional<VkAttachmentReference> depthAttachment, presentAttachment;

    //    // Map input attachments.
    //    // TODO: We need to check if there's some better way to map dependency outputs to input attachments, since simply assuming the right order
    //    //       might actually be tricky in situations where there are more than two render passes or where one render pass is executed multiple
    //    //       times.
    //    std::for_each(std::begin(dependencyTargets), std::end(dependencyTargets), [&, i = 0](const auto& target) mutable {
    //        VkAttachmentDescription attachment{};
    //        attachment.format = getFormat(target->getFormat());
    //        attachment.samples = getSamples(target->getSamples());
    //        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    //        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    //        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //        // Add a clear value, so that the indexing stays valid.
    //        m_clearValues.push_back(VkClearValue{ });

    //        switch (target->getType())
    //        {
    //        case RenderTargetType::Color:
    //            attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //            inputAttachments.push_back({ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    //            attachments.push_back(attachment);
    //            break;
    //        case RenderTargetType::Depth:
    //            attachment.initialLayout = attachment.finalLayout = ::hasStencil(target->getFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    //            inputAttachments.push_back({ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    //            attachments.push_back(attachment);
    //            break;
    //        case RenderTargetType::Present:
    //            LITEFX_WARNING(VULKAN_LOG, "The render pass dependency defines a present render target, which can not be used as input attachment.");
    //            break;
    //        }
    //    });

    //    // Create attachments for each render target.
    //    std::for_each(std::begin(m_targets), std::end(m_targets), [&, i = inputAttachments.size()](const auto& target) mutable {
    //        if ((target->getType() == RenderTargetType::Depth && depthAttachment.has_value()) || (target->getType() == RenderTargetType::Present && presentAttachment.has_value()))
    //            throw std::runtime_error(fmt::format("Invalid render target {0}: only one target attachment of type {1} is allowed.", i, target->getType()));
    //        else
    //        {
    //            VkAttachmentDescription attachment{};
    //            attachment.format = getFormat(target->getFormat());
    //            attachment.samples = getSamples(target->getSamples());
    //            attachment.loadOp = target->getClearBuffer() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //            attachment.stencilLoadOp = target->getClearStencil() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //            attachment.storeOp = target->getVolatile() ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
    //            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //            // Add a clear values (even if it's unused).
    //            if (target->getClearBuffer() || target->getClearStencil())
    //                m_clearValues.push_back(VkClearValue{ target->getClearValues().x(), target->getClearValues().y(), target->getClearValues().z(), target->getClearValues().w() });
    //            else
    //                m_clearValues.push_back(VkClearValue{ });

    //            switch (target->getType())
    //            {
    //            case RenderTargetType::Color:
    //                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //                colorAttachments.push_back({ static_cast<UInt32>(i++), attachment.finalLayout });
    //                break;
    //            case RenderTargetType::Depth:
    //                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //                attachment.finalLayout = ::hasStencil(target->getFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    //                depthAttachment = VkAttachmentReference{ static_cast<UInt32>(i++), attachment.finalLayout };
    //                break;
    //            case RenderTargetType::Present:
    //                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //                presentAttachment = VkAttachmentReference{ static_cast<UInt32>(i++), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    //                colorAttachments.push_back(presentAttachment.value());
    //                m_present = true;
    //                break;
    //            }

    //            attachments.push_back(attachment);
    //        }
    //    });

    //    // Setup the sub-pass.
    //    VkSubpassDescription subPass{};
    //    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //    subPass.colorAttachmentCount = static_cast<UInt32>(colorAttachments.size());
    //    subPass.pColorAttachments = colorAttachments.data();
    //    subPass.pDepthStencilAttachment = depthAttachment.has_value() ? &depthAttachment.value() : nullptr;
    //    subPass.inputAttachmentCount = static_cast<UInt32>(inputAttachments.size());
    //    subPass.pInputAttachments = inputAttachments.data();

    //    // Define an external sub-pass dependency.
    //    Array<VkSubpassDependency> dependencies;

    //    if (m_dependency != nullptr)
    //    {
    //        VkSubpassDependency dependency{};
    //        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    //        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //        dependency.dstSubpass = 0;
    //        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    //        dependencies.push_back(dependency);
    //    }

    //    // Setup render pass state.
    //    VkRenderPassCreateInfo renderPassState{};
    //    renderPassState.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //    renderPassState.attachmentCount = static_cast<UInt32>(attachments.size());
    //    renderPassState.pAttachments = attachments.data();
    //    renderPassState.subpassCount = 1;
    //    renderPassState.pSubpasses = &subPass;
    //    renderPassState.dependencyCount = static_cast<UInt32>(dependencies.size());
    //    renderPassState.pDependencies = dependencies.data();

    //    // Create the render pass.
    //    VkRenderPass renderPass;

    //    if (::vkCreateRenderPass(m_parent->getDevice()->handle(), &renderPassState, nullptr, &renderPass) != VK_SUCCESS)
    //        throw std::runtime_error("Unable to create render pass.");

    //    // Initialize frame buffers.
    //    auto frames = m_swapChain->getFrames();
    //    Array<VkFramebuffer> frameBuffers(frames.size());

    //    LITEFX_TRACE(VULKAN_LOG, "Initializing {0} frame buffers...", frames.size());

    //    std::generate(std::begin(frameBuffers), std::end(frameBuffers), [&, i = 0]() mutable {
    //        Array<VkImageView> attachmentViews;
    //        Array<UniquePtr<IImage>> attachmentImages;

    //        std::for_each(std::begin(dependencyTargets), std::end(dependencyTargets), [&, a = 0](const auto& target) mutable {
    //            auto inputAttachmentImage = dynamic_cast<const IDirectX12Image*>(m_dependency->m_impl->m_attachmentImages[i][a++].get());

    //            if (inputAttachmentImage == nullptr)
    //                throw std::invalid_argument("An input attachment of the render pass dependency is not a valid DirectX12 image.");

    //            attachmentViews.push_back(inputAttachmentImage->getImageView());
    //        });

    //        std::for_each(std::begin(m_targets), std::end(m_targets), [&, a = attachmentViews.size()](const auto& target) mutable {
    //            if (presentAttachment.has_value() && a++ == presentAttachment->attachment)
    //            {
    //                // Acquire an image view from the swap chain.
    //                auto swapChainImage = dynamic_cast<const IDirectX12Image*>(frames[i]);

    //                if (swapChainImage == nullptr)
    //                    throw std::invalid_argument("A frame of the provided swap chain is not a valid DirectX12 texture.");

    //                attachmentViews.push_back(swapChainImage->getImageView());
    //            }
    //            else
    //            {
    //                // Create an image view for the render target.
    //                auto image = this->makeImageView(target.get());
    //                attachmentViews.push_back(dynamic_cast<const IDirectX12Image*>(image.get())->getImageView());
    //                attachmentImages.push_back(std::move(image));
    //            }
    //        });

    //        VkFramebufferCreateInfo frameBufferInfo{};
    //        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //        frameBufferInfo.renderPass = renderPass;
    //        frameBufferInfo.attachmentCount = static_cast<UInt32>(attachmentViews.size());
    //        frameBufferInfo.pAttachments = attachmentViews.data();
    //        frameBufferInfo.width = m_parent->getDevice()->getSwapChain()->getBufferSize().width();
    //        frameBufferInfo.height = m_parent->getDevice()->getSwapChain()->getBufferSize().height();
    //        frameBufferInfo.layers = 1;

    //        VkFramebuffer frameBuffer;

    //        if (::vkCreateFramebuffer(m_parent->getDevice()->handle(), &frameBufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
    //            throw std::runtime_error("Unable to create frame buffer from swap chain frame.");

    //        m_attachmentImages[i++] = std::move(attachmentImages);

    //        return frameBuffer;
    //    });

    //    // Store the buffers.
    //    m_frameBuffers = frameBuffers;

    //    // Create a command buffer.
    //    if (m_commandBuffers.empty())
    //    {
    //        m_commandBuffers.resize(frames.size());
    //        std::generate(std::begin(m_commandBuffers), std::end(m_commandBuffers), [&]() mutable { return makeUnique<DirectX12CommandBuffer>(dynamic_cast<const DirectX12Queue*>(m_parent->getDevice()->graphicsQueue())); });
    //    }

    //    // Create a semaphore that signals if the render pass has finished.
    //    VkSemaphoreCreateInfo semaphoreInfo{};
    //    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //    m_semaphores.resize(frames.size());
    //    std::generate(std::begin(m_semaphores), std::end(m_semaphores), [&]() mutable {
    //        VkSemaphore semaphore;

    //        if (::vkCreateSemaphore(m_parent->getDevice()->handle(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
    //            throw std::runtime_error("Unable to create signal semaphore.");

    //        return semaphore;
    //    });

    //    // Return the render pass.
    //    return renderPass;
    }

    void begin()
    {
        //auto pipeline = dynamic_cast<const IResource<VkPipeline>*>(m_pipeline.get());

        //if (pipeline == nullptr)
        //    throw std::runtime_error("The pipeline of the render pass is not a valid DirectX12 pipeline.");

        //// Swap out the back buffer, if the render pass has a present target. Otherwise increment the current frame buffer anyways.
        //// NOTE: Maybe this can be refactored to a boolean parameter `swapBackBuffer` which defaults to `false`?
        //m_backBuffer = m_dependency == nullptr ? m_swapChain->swapBackBuffer() : m_dependency->m_impl->m_backBuffer;

        //// Get current command buffer.
        //auto commandBuffer = this->getCurrentCommandBuffer();
        //commandBuffer->begin();

        //// Begin the render pass.
        //VkRenderPassBeginInfo renderPassInfo{};
        //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //renderPassInfo.renderPass = m_parent->handle();
        //renderPassInfo.framebuffer = m_frameBuffers[m_backBuffer];
        //renderPassInfo.renderArea.offset = { 0, 0 };
        //renderPassInfo.renderArea.extent.width = static_cast<UInt32>(m_parent->getDevice()->getBufferWidth());
        //renderPassInfo.renderArea.extent.height = static_cast<UInt32>(m_parent->getDevice()->getBufferHeight());
        //renderPassInfo.clearValueCount = m_clearValues.size();
        //renderPassInfo.pClearValues = m_clearValues.data();

        //::vkCmdBeginRenderPass(commandBuffer->handle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        //::vkCmdBindPipeline(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
    }

    void end(const bool present = false)
    {
        //auto commandBuffer = this->getCurrentCommandBuffer();
        //::vkCmdEndRenderPass(commandBuffer->handle());
        //commandBuffer->end();

        //// Submit the command buffer.

        //if (!m_present)
        //    commandBuffer->submit({}, {});
        //else
        //{
        //    Array<VkSemaphore> waitForSemaphores = { m_swapChain->getCurrentSemaphore() };
        //    Array<VkPipelineStageFlags> waitForStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        //    Array<VkSemaphore> signalSemaphores = { this->getCurrentSemaphore() };

        //    commandBuffer->submit(waitForSemaphores, waitForStages, signalSemaphores, false);

        //    // Draw the frame, if the result of the render pass it should be presented to the swap chain.
        //    VkPresentInfoKHR presentInfo{};
        //    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //    presentInfo.waitSemaphoreCount = signalSemaphores.size();
        //    presentInfo.pWaitSemaphores = signalSemaphores.data();
        //    presentInfo.pImageIndices = &m_backBuffer;
        //    presentInfo.pResults = nullptr;

        //    VkSwapchainKHR swapChains[] = { m_swapChain->handle() };
        //    presentInfo.pSwapchains = swapChains;
        //    presentInfo.swapchainCount = 1;

        //    if (::vkQueuePresentKHR(m_queue->handle(), &presentInfo) != VK_SUCCESS)
        //        throw std::runtime_error("Unable to present swap chain.");
        //}
    }

public:
    //const DirectX12CommandBuffer* getCurrentCommandBuffer() const noexcept
    //{
    //    return m_commandBuffers[m_backBuffer].get();
    //}

    //const VkSemaphore& getCurrentSemaphore() const noexcept
    //{
    //    return m_semaphores[m_backBuffer];
    //}

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

DirectX12RenderPass::DirectX12RenderPass(const IGraphicsDevice* device) :
    DirectX12RenderPass(dynamic_cast<const DirectX12Device*>(device))
{
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device* device) :
    m_impl(makePimpl<DirectX12RenderPassImpl>(this, device))
{
}

DirectX12RenderPass::~DirectX12RenderPass() noexcept
{
    m_impl->cleanup();
}

const ICommandBuffer* DirectX12RenderPass::getCommandBuffer() const noexcept
{
    //return m_impl->getCurrentCommandBuffer();
    throw;
}

void DirectX12RenderPass::addTarget(UniquePtr<IRenderTarget>&& target)
{
    m_impl->addTarget(std::move(target));
}

const Array<const IRenderTarget*> DirectX12RenderPass::getTargets() const noexcept
{
    return m_impl->getTargets();
}

UniquePtr<IRenderTarget> DirectX12RenderPass::removeTarget(const IRenderTarget* target)
{
    return m_impl->removeTarget(target);
}

void DirectX12RenderPass::setDependency(const IRenderPass* renderPass)
{
    //auto dependency = dynamic_cast<const DirectX12RenderPass*>(renderPass);

    //if (dependency == nullptr)
    //    throw std::invalid_argument("The render pass dependency must be a valid DirectX12 render pass.");

    //m_impl->m_dependency = dependency;
    throw;
}

const IRenderPass* DirectX12RenderPass::getDependency() const noexcept
{
    //return m_impl->m_dependency;
    throw;
}

const IRenderPipeline* DirectX12RenderPass::getPipeline() const noexcept
{
    //return m_impl->m_pipeline.get();
    throw;
}

IRenderPipeline* DirectX12RenderPass::getPipeline() noexcept
{
    //return m_impl->m_pipeline.get();
    throw;
}

void DirectX12RenderPass::begin() const
{
    m_impl->begin();
}

void DirectX12RenderPass::end(const bool& present)
{
    m_impl->end(present);
}

void DirectX12RenderPass::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
    //::vkCmdDraw(m_impl->getCurrentCommandBuffer()->handle(), vertices, instances, firstVertex, firstInstance);
    throw;
}

void DirectX12RenderPass::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
    //::vkCmdDrawIndexed(m_impl->getCurrentCommandBuffer()->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
    throw;
}

const IImage* DirectX12RenderPass::getAttachment(const UInt32& attachmentId) const
{
    //if (m_impl->m_attachmentImages[m_impl->m_backBuffer].size() <= attachmentId)
    //    throw std::invalid_argument(fmt::format("Invalid attachment index ({0}, but expected {1} or less).", attachmentId, m_impl->m_attachmentImages[m_impl->m_backBuffer].size() - 1));

    //return m_impl->m_attachmentImages[m_impl->m_backBuffer][attachmentId].get();
    throw;
}

void DirectX12RenderPass::bind(const IVertexBuffer* buffer) const
{
    //auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
    //auto commandBuffer = m_impl->getCurrentCommandBuffer();

    //if (resource == nullptr)
    //    throw std::invalid_argument("The provided vertex buffer is not a valid DirectX12 buffer.");

    //// Depending on the type, bind the buffer accordingly.
    //constexpr VkDeviceSize offsets[] = { 0 };

    //::vkCmdBindVertexBuffers(commandBuffer->handle(), 0, 1, &resource->handle(), offsets);
    throw;
}

void DirectX12RenderPass::bind(const IIndexBuffer* buffer) const
{
    //auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
    //auto commandBuffer = m_impl->getCurrentCommandBuffer();

    //if (resource == nullptr)
    //    throw std::invalid_argument("The provided index buffer is not a valid DirectX12 buffer.");

    //::vkCmdBindIndexBuffer(commandBuffer->handle(), resource->handle(), 0, buffer->getLayout()->getIndexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    throw;
}

void DirectX12RenderPass::bind(IDescriptorSet* descriptorSet) const
{
    //if (descriptorSet == nullptr)
    //    throw std::invalid_argument("The descriptor set must be initialized.");

    //descriptorSet->bind(this);
    throw;
}

UniquePtr<IVertexBuffer> DirectX12RenderPass::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding) const
{
    //return this->getDevice()->createVertexBuffer(this->getPipeline()->getLayout()->getInputAssembler()->getVertexBufferLayout(binding), usage, elements);
    throw;
}

UniquePtr<IIndexBuffer> DirectX12RenderPass::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
    //return this->getDevice()->createIndexBuffer(this->getPipeline()->getLayout()->getInputAssembler()->getIndexBufferLayout(), usage, elements);
    throw;
}

UniquePtr<IDescriptorSet> DirectX12RenderPass::makeBufferPool(const UInt32& setId) const
{
    //auto layouts = this->getPipeline()->getLayout()->getProgram()->getLayouts();
    //auto match = std::find_if(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) { return layout->getSetId() == setId; });

    //if (match == layouts.end())
    //    throw std::invalid_argument("The requested buffer set is not defined.");

    //return (*match)->createBufferPool();
    throw;
}

const DirectX12Device* DirectX12RenderPass::getDevice() const noexcept
{
    return m_impl->m_device;
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(UniquePtr<DirectX12RenderPass>&& instance) :
    RenderPassBuilder(std::move(instance))
{
}

DirectX12RenderPassBuilder::~DirectX12RenderPassBuilder() noexcept = default;

UniquePtr<DirectX12RenderPass> DirectX12RenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->initialize();

    auto pipeline = instance->getPipeline();

    if (pipeline != nullptr)
        pipeline->bind(instance);

    return RenderPassBuilder::go();
}

void DirectX12RenderPassBuilder::use(UniquePtr<IRenderTarget>&& target)
{
    this->instance()->addTarget(std::move(target));
}

void DirectX12RenderPassBuilder::use(UniquePtr<IRenderPipeline>&& pipeline)
{
    //if (pipeline == nullptr)
    //    throw std::invalid_argument("The pipeline must be initialized.");

    //this->instance()->m_impl->m_pipeline = std::move(pipeline);
}

DirectX12RenderPipelineBuilder DirectX12RenderPassBuilder::addPipeline()
{
    return this->make<DirectX12RenderPipeline>();
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clearColor, bool clearStencil, bool isVolatile)
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

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::dependsOn(const IRenderPass* renderPass)
{
    //if (renderPass == nullptr)
    //    throw std::invalid_argument("The render pass must be initialized.");

    //this->instance()->setDependency(renderPass);

    //return *this;
    throw;
}