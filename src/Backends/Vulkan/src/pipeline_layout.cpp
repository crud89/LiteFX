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
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorLayouts) :
        base(parent), m_shaderProgram(std::move(shaderProgram)), m_descriptorSetLayouts(std::move(descriptorLayouts))
    {
    }

    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent) :
        base(parent)
    {
    }

public:
    VkPipelineLayout initialize()
    {
        // Query for the descriptor set layout handles.
        auto descriptorSetLayouts = m_descriptorSetLayouts |
            std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return std::as_const(*layout.get()).handle(); }) |
            ranges::to<Array<VkDescriptorSetLayout>>();

        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0} {{ Descriptor Sets: {1} }}...", fmt::ptr(m_parent), descriptorSetLayouts.size());

        // Create the pipeline layout.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkPipelineLayout layout;
        raiseIfFailed<RuntimeException>(::vkCreatePipelineLayout(m_parent->getDevice()->handle(), &pipelineLayoutInfo, nullptr, &layout), "Unable to create pipeline layout.");
        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorLayout>>&& descriptorSetLayouts) :
    m_impl(makePimpl<VulkanRenderPipelineLayoutImpl>(this, shaderProgram, descriptorSetLayouts)), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice()), IResource<VkPipelineLayout>(nullptr)
{
    this->handle() = m_impl->initialize();
}

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) noexcept :
    m_impl(makePimpl<VulkanRenderPipelineLayoutImpl>(this)), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice()), IResource<VkPipelineLayout>(nullptr)
{
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept
{
    ::vkDestroyPipelineLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

const VulkanShaderProgram& VulkanRenderPipelineLayout::program() const noexcept
{
    return *m_impl->m_shaderProgram.get();
}

const VulkanDescriptorSetLayout& VulkanRenderPipelineLayout::layout(const UInt32& space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No descriptor set layout uses the provided space {0}.", space);
}

Array<const VulkanDescriptorSetLayout*> VulkanRenderPipelineLayout::layouts() const noexcept
{
    return m_impl->m_descriptorSetLayouts |
        std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const VulkanDescriptorSetLayout*>>();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilderImpl : public Implement<VulkanRenderPipelineLayoutBuilder> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanRenderPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanRenderPipelineLayoutBuilderImpl(VulkanRenderPipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineBuilder& parent) :
    RenderPipelineLayoutBuilder(parent, makeUnique<VulkanRenderPipeline>(*std::as_const(parent).instance())), m_impl(makePimpl<VulkanRenderPipelineLayoutBuilderImpl>(this))
{
}

VulkanRenderPipelineLayoutBuilder::~VulkanRenderPipelineLayoutBuilder() noexcept = default;

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_shaderProgram = std::move(m_impl->m_shaderProgram);
    instance->m_impl->m_descriptorSetLayouts = std::move(m_impl->m_descriptorSetLayouts);
    instance->handle() = instance->m_impl->initialize();
    
    return RenderPipelineLayoutBuilder::go();
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<VulkanShaderProgram>&& program)
{
#ifndef NDEBUG
    if (m_impl->m_shaderProgram != nullptr)
        LITEFX_WARNING(VULKAN_LOG, "Another shader program has already been initialized and will be replaced. A pipeline layout can only have one shader program.");
#endif

    m_impl->m_shaderProgram = std::move(program);
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<VulkanDescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

VulkanShaderProgramBuilder VulkanRenderPipelineLayoutBuilder::shaderProgram()
{
    return this->make<VulkanShaderProgram>();
}

VulkanDescriptorSetLayoutBuilder VulkanRenderPipelineLayoutBuilder::addDescriptorSet(const UInt32& space, const ShaderStage& stages, const UInt32& poolSize)
{
    return this->make<VulkanDescriptorSetLayout>(space, stages, poolSize);
}