#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl : public Implement<VulkanRenderPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanRenderPipelineLayout;

private:
    UniquePtr<IShaderProgram> m_shaderProgram;
    Array<UniquePtr<IDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent) : 
        base(parent) 
    { 
    }

public:
    VkPipelineLayout initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts)
    {
        if (shaderProgram == nullptr)
            throw std::runtime_error("The shader program must be initialized.");

        Array<VkDescriptorSetLayout> descriptorSetLayouts;

        std::for_each(std::begin(descriptorLayouts), std::end(descriptorLayouts), [&](const IDescriptorSetLayout* layout) {
            // TODO: Validate layout (if a layout is not initialized, it may raise an exception.
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

        m_shaderProgram = std::move(shaderProgram);
        m_descriptorSetLayouts = std::move(descriptorLayouts);

        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) :
    VulkanRuntimeObject(pipeline.getDevice()), IResource<VkPipelineLayout>(nullptr), m_impl(makePimpl<VulkanRenderPipelineLayoutImpl>(this))
{
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept
{
    if (this->handle() != nullptr)
        ::vkDestroyPipelineLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

bool VulkanRenderPipelineLayout::isInitialized() const noexcept
{
    return this->handle() != nullptr;
}

void VulkanRenderPipelineLayout::initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts)
{
    this->handle() = m_impl->initialize(std::move(shaderProgram), std::move(descriptorLayouts));
}

const IShaderProgram* VulkanRenderPipelineLayout::getProgram() const noexcept
{
    return m_impl->m_shaderProgram.get();
}

Array<const IDescriptorSetLayout*> VulkanRenderPipelineLayout::getDescriptorSetLayouts() const noexcept
{
    Array<const IDescriptorSetLayout*> layouts(m_impl->m_descriptorSetLayouts.size());
    std::for_each(std::begin(m_impl->m_descriptorSetLayouts), std::end(m_impl->m_descriptorSetLayouts), [i = 0, &layouts](const auto& layout) { layouts[i++] = layout.get(); });

    return layouts;
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilderImpl : public Implement<VulkanRenderPipelineLayoutBuilder> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanRenderPipelineLayout;

private:
    UniquePtr<IShaderProgram> m_shaderProgram;
    Array<UniquePtr<IDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanRenderPipelineLayoutBuilderImpl(VulkanRenderPipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineBuilder& parent, UniquePtr<VulkanRenderPipelineLayout>&& instance) :
    RenderPipelineLayoutBuilder(parent, std::move(instance)), m_impl(makePimpl<VulkanRenderPipelineLayoutBuilderImpl>(this))
{
}

VulkanRenderPipelineLayoutBuilder::~VulkanRenderPipelineLayoutBuilder() noexcept = default;

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    this->instance()->initialize(std::move(m_impl->m_shaderProgram), std::move(m_impl->m_descriptorSetLayouts));
    
    return RenderPipelineLayoutBuilder::go();
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IShaderProgram>&& program)
{
#ifndef NDEBUG
    LITEFX_WARNING(VULKAN_LOG, "Another shader program has already been initialized and will be replaced. A pipeline layout can only have one shader program.");
#endif

    m_impl->m_shaderProgram = std::move(program);
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IDescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

VulkanShaderProgramBuilder VulkanRenderPipelineLayoutBuilder::setShaderProgram()
{
    return this->make<VulkanShaderProgram>();
}

VulkanDescriptorSetLayoutBuilder VulkanRenderPipelineLayoutBuilder::addDescriptorSet(const UInt32& id, const ShaderStage& stages)
{
    return this->make<VulkanDescriptorSetLayout>(id, stages);
}