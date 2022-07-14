#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

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

UniquePtr<VulkanRenderPass> VulkanRenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->mapRenderTargets(m_impl->m_renderTargets);
    instance->m_impl->mapInputAttachments(m_impl->m_inputAttachments);
    instance->m_impl->m_samples = std::move(m_impl->m_samples);
    instance->handle() = instance->m_impl->initialize();

    // Initialize the frame buffers.
    instance->m_impl->m_frameBuffers.resize(instance->getDevice()->swapChain().buffers());
    std::ranges::generate(instance->m_impl->m_frameBuffers, [this, &instance, i = 0]() mutable { return makeUnique<VulkanFrameBuffer>(*instance, i++, instance->parent().swapChain().renderArea(), m_impl->m_commandBuffers); });

    // Initialize the command buffers.
    instance->m_impl->m_primaryCommandBuffer = instance->getDevice()->graphicsQueue().createCommandBuffer(false);

    return RenderPassBuilder::go();
}

void VulkanRenderPassBuilder::use(RenderTarget&& target)
{
    m_impl->m_renderTargets.push_back(std::move(target));
}

void VulkanRenderPassBuilder::use(VulkanInputAttachmentMapping&& attachment)
{
    m_impl->m_inputAttachments.push_back(std::move(attachment));
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::commandBuffers(const UInt32& count)
{
    m_impl->m_commandBuffers = count;
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

VulkanRenderPassBuilder& VulkanRenderPassBuilder::setMultiSamplingLevel(const MultiSamplingLevel& samples)
{
    m_impl->m_samples = samples;
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