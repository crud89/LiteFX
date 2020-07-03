#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl : public Implement<VulkanRenderPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayout;

private:
    const VulkanDevice* m_device;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent, const VulkanDevice* device) :
        base(parent), m_device(device) { }

public:
    VkPipelineLayout initialize()
    {
        LITEFX_DEBUG(VULKAN_LOG, "Creating render pipeline layout {0}...", fmt::ptr(m_parent));

        // TODO: Implement me!
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkPipelineLayout layout;

        if (vkCreatePipelineLayout(m_device->handle(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create pipeline layout.");

        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) :
    RenderPipelineLayout(), IResource<VkPipelineLayout>(nullptr)
{
    auto device = dynamic_cast<const VulkanDevice*>(pipeline.getDevice());

    if (device == nullptr)
        throw std::invalid_argument("The pipeline is not bound to a valid Vulkan device.");

    m_impl = makePimpl<VulkanRenderPipelineLayoutImpl>(this, device);
}

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline, const VulkanBufferLayout& bufferLayout) :
    VulkanRenderPipelineLayout(pipeline)
{
    this->create();
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept = default;

void VulkanRenderPipelineLayout::create() 
{
    this->handle() = m_impl->initialize();
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    this->instance()->create();
    return RenderPipelineLayoutBuilder::go();
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IRasterizer>&& rasterizer)
{
    if (rasterizer == nullptr)
        throw std::invalid_argument("The rasterizer must be initialized.");

    this->instance()->use(std::move(rasterizer));
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IInputAssembler>&& inputAssembler)
{
    if (inputAssembler == nullptr)
        throw std::invalid_argument("The input assembler must be initialized.");

    this->instance()->use(std::move(inputAssembler));
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IViewport>&& viewport)
{
    if (viewport == nullptr)
        throw std::invalid_argument("The viewport must be initialized.");

    this->instance()->use(std::move(viewport));
}