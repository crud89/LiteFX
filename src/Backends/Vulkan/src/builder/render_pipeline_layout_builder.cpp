#include <litefx/backends/vulkan_builders.cpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Render pipeline layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilderImpl : public Implement<VulkanRenderPipelineLayoutBuilder> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanRenderPipelineLayoutBuilderImpl(VulkanRenderPipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Render pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayoutBuilder::VulkanRenderPipelineLayoutBuilder(VulkanRenderPipelineBuilder& parent) :
    m_impl(makePimpl<VulkanRenderPipelineLayoutBuilderImpl>(this)), PipelineLayoutBuilder(parent, UniquePtr<VulkanPipelineLayout>(new VulkanPipelineLayout(*std::as_const(parent).instance())))
{
}

VulkanRenderPipelineLayoutBuilder::~VulkanRenderPipelineLayoutBuilder() noexcept = default;

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_shaderProgram = std::move(m_impl->m_shaderProgram);
    instance->m_impl->m_descriptorSetLayouts = std::move(m_impl->m_descriptorSetLayouts);
    instance->m_impl->m_pushConstantsLayout = std::move(m_impl->m_pushConstantsLayout);
    instance->handle() = instance->m_impl->initialize();

    return PipelineLayoutBuilder::go();
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

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<VulkanPushConstantsLayout>&& layout)
{
    m_impl->m_pushConstantsLayout = std::move(layout);
}

VulkanGraphicsShaderProgramBuilder VulkanRenderPipelineLayoutBuilder::shaderProgram()
{
    return VulkanGraphicsShaderProgramBuilder(*this);
}

VulkanRenderPipelineDescriptorSetLayoutBuilder VulkanRenderPipelineLayoutBuilder::addDescriptorSet(const UInt32& space, const ShaderStage& stages, const UInt32& poolSize)
{
    return VulkanRenderPipelineDescriptorSetLayoutBuilder(*this, space, stages, poolSize);
}

VulkanRenderPipelinePushConstantsLayoutBuilder VulkanRenderPipelineLayoutBuilder::addPushConstants(const UInt32& size)
{
    return VulkanRenderPipelinePushConstantsLayoutBuilder(*this, size);
}