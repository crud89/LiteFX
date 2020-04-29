#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPass::VulkanRenderPassImpl {
private:
    const VulkanDevice* m_device;
    Array<UniquePtr<IRenderTarget>> m_targets;

public:
    VulkanRenderPassImpl(const VulkanDevice* device)  noexcept : m_device(device) {}

public:
    VkRenderPass initialize(const VulkanRenderPass& parent)
    {
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

        if (vkCreateRenderPass(m_device->handle(), &renderPassState, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass.");

        return renderPass;
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
    IResource<VkRenderPass>(nullptr)
{
    auto device = dynamic_cast<const VulkanDevice*>(pipeline.getDevice());

    if (device == nullptr)
        throw std::invalid_argument("The pipeline device is not a valid Vulkan device.");

    m_impl = makePimpl<VulkanRenderPassImpl>(device);
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