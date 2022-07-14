#include <litefx/backends/vulkan_builders.cpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Compute pipeline layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipelineLayoutBuilder::VulkanComputePipelineLayoutBuilderImpl : public Implement<VulkanComputePipelineLayoutBuilder> {
public:
    friend class VulkanComputePipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanComputePipelineLayoutBuilderImpl(VulkanComputePipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Compute pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipelineLayoutBuilder::VulkanComputePipelineLayoutBuilder(VulkanComputePipelineBuilder& parent) :
    m_impl(makePimpl<VulkanComputePipelineLayoutBuilderImpl>(this)), PipelineLayoutBuilder(parent, UniquePtr<VulkanPipelineLayout>(new VulkanPipelineLayout(*std::as_const(parent).instance())))
{
}

VulkanComputePipelineLayoutBuilder::~VulkanComputePipelineLayoutBuilder() noexcept = default;

VulkanComputePipelineBuilder& VulkanComputePipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_shaderProgram = std::move(m_impl->m_shaderProgram);
    instance->m_impl->m_descriptorSetLayouts = std::move(m_impl->m_descriptorSetLayouts);
    instance->m_impl->m_pushConstantsLayout = std::move(m_impl->m_pushConstantsLayout);
    instance->handle() = instance->m_impl->initialize();

    return PipelineLayoutBuilder::go();
}

void VulkanComputePipelineLayoutBuilder::use(UniquePtr<VulkanShaderProgram>&& program)
{
#ifndef NDEBUG
    if (m_impl->m_shaderProgram != nullptr)
        LITEFX_WARNING(VULKAN_LOG, "Another shader program has already been initialized and will be replaced. A pipeline layout can only have one shader program.");
#endif

    m_impl->m_shaderProgram = std::move(program);
}

void VulkanComputePipelineLayoutBuilder::use(UniquePtr<VulkanDescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

void VulkanComputePipelineLayoutBuilder::use(UniquePtr<VulkanPushConstantsLayout>&& layout)
{
    m_impl->m_pushConstantsLayout = std::move(layout);
}

VulkanComputeShaderProgramBuilder VulkanComputePipelineLayoutBuilder::shaderProgram()
{
    return VulkanComputeShaderProgramBuilder(*this);
}

VulkanComputePipelineDescriptorSetLayoutBuilder VulkanComputePipelineLayoutBuilder::addDescriptorSet(const UInt32& space, const UInt32& poolSize)
{
    return VulkanComputePipelineDescriptorSetLayoutBuilder(*this, space, poolSize);
}

VulkanComputePipelinePushConstantsLayoutBuilder VulkanComputePipelineLayoutBuilder::addPushConstants(const UInt32& size)
{
    return VulkanComputePipelinePushConstantsLayoutBuilder(*this, size);
}