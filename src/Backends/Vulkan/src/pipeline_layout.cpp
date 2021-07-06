#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayout::VulkanPipelineLayoutImpl : public Implement<VulkanPipelineLayout> {
public:
    friend class VulkanPipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanPipelineLayoutImpl(VulkanPipelineLayout* parent, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorLayouts) :
        base(parent), m_shaderProgram(std::move(shaderProgram)), m_descriptorSetLayouts(std::move(descriptorLayouts))
    {
    }

    VulkanPipelineLayoutImpl(VulkanPipelineLayout* parent) :
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

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanRenderPipeline& pipeline, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts) :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this, std::move(shaderProgram), std::move(descriptorSetLayouts))), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanRenderPipeline& pipeline) noexcept :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this)), VulkanRuntimeObject<VulkanRenderPipeline>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout() noexcept
{
    ::vkDestroyPipelineLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

const VulkanShaderProgram& VulkanPipelineLayout::program() const noexcept
{
    return *m_impl->m_shaderProgram.get();
}

const VulkanDescriptorSetLayout& VulkanPipelineLayout::descriptorSet(const UInt32& space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No descriptor set layout uses the provided space {0}.", space);
}

Array<const VulkanDescriptorSetLayout*> VulkanPipelineLayout::descriptorSets() const noexcept
{
    return m_impl->m_descriptorSetLayouts |
        std::views::transform([](const UniquePtr<VulkanDescriptorSetLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const VulkanDescriptorSetLayout*>>();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayoutBuilder::VulkanPipelineLayoutBuilderImpl : public Implement<VulkanPipelineLayoutBuilder> {
public:
    friend class VulkanPipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanPipelineLayoutBuilderImpl(VulkanPipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayoutBuilder::VulkanPipelineLayoutBuilder(VulkanRenderPipelineBuilder& parent) :
    m_impl(makePimpl<VulkanPipelineLayoutBuilderImpl>(this)), PipelineLayoutBuilder(parent, UniquePtr<VulkanPipelineLayout>(new VulkanPipelineLayout(*std::as_const(parent).instance())))
{
}

VulkanPipelineLayoutBuilder::~VulkanPipelineLayoutBuilder() noexcept = default;

VulkanRenderPipelineBuilder& VulkanPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_shaderProgram = std::move(m_impl->m_shaderProgram);
    instance->m_impl->m_descriptorSetLayouts = std::move(m_impl->m_descriptorSetLayouts);
    instance->handle() = instance->m_impl->initialize();
    
    return PipelineLayoutBuilder::go();
}

void VulkanPipelineLayoutBuilder::use(UniquePtr<VulkanShaderProgram>&& program)
{
#ifndef NDEBUG
    if (m_impl->m_shaderProgram != nullptr)
        LITEFX_WARNING(VULKAN_LOG, "Another shader program has already been initialized and will be replaced. A pipeline layout can only have one shader program.");
#endif

    m_impl->m_shaderProgram = std::move(program);
}

void VulkanPipelineLayoutBuilder::use(UniquePtr<VulkanDescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

VulkanShaderProgramBuilder VulkanPipelineLayoutBuilder::shaderProgram()
{
    return VulkanShaderProgramBuilder(*this);
}

VulkanDescriptorSetLayoutBuilder VulkanPipelineLayoutBuilder::addDescriptorSet(const UInt32& space, const ShaderStage& stages, const UInt32& poolSize)
{
    return VulkanDescriptorSetLayoutBuilder(*this, space, stages, poolSize);
}