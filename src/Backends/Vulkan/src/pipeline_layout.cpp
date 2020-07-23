#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl : public Implement<VulkanRenderPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanRenderPipelineLayout;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent) : base(parent) { }

public:
    VkPipelineLayout initialize()
    {
        auto shaderProgram = m_parent->getProgram();

        if (shaderProgram == nullptr)
            throw std::runtime_error("The shader program must be initialized.");

        Array<VkDescriptorSetLayout> descriptorSetLayouts;
        auto layouts = shaderProgram->getLayouts();

        std::for_each(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) {
            auto descriptorSetLayout = dynamic_cast<const VulkanDescriptorSetLayout*>(layout);

            if (descriptorSetLayout != nullptr && descriptorSetLayout->handle() != nullptr)
                descriptorSetLayouts.push_back(descriptorSetLayout->handle());
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0} {{ Sets: {1} }}...", fmt::ptr(m_parent), descriptorSetLayouts.size());

        // Create the pipeline layout.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkPipelineLayout layout;

        if (::vkCreatePipelineLayout(m_parent->getDevice()->handle(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create pipeline layout.");

        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) :
    RenderPipelineLayout(), VulkanRuntimeObject(pipeline.getDevice()), IResource<VkPipelineLayout>(nullptr)
{
    m_impl = makePimpl<VulkanRenderPipelineLayoutImpl>(this);
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept
{
    ::vkDestroyPipelineLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize();

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

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IShaderProgram>&& program)
{
    if (program == nullptr)
        throw std::invalid_argument("The program must be initialized.");

    this->instance()->use(std::move(program));
}

VulkanRasterizerBuilder VulkanRenderPipelineLayoutBuilder::setRasterizer()
{
    return this->make<VulkanRasterizer>();
}

VulkanInputAssemblerBuilder VulkanRenderPipelineLayoutBuilder::setInputAssembler()
{
    return this->make<VulkanInputAssembler>();
}

VulkanShaderProgramBuilder VulkanRenderPipelineLayoutBuilder::setShaderProgram()
{
    return this->make<VulkanShaderProgram>();
}

VulkanViewportBuilder VulkanRenderPipelineLayoutBuilder::addViewport()
{
    return this->make<VulkanViewport>();
}