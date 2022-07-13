#include <litefx/backends/vulkan_builders.cpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelinePushConstantsLayoutBuilder::VulkanRenderPipelinePushConstantsLayoutBuilderImpl : public Implement<VulkanRenderPipelinePushConstantsLayoutBuilder> {
public:
    friend class VulkanRenderPipelinePushConstantsLayoutBuilder;

private:
    Array<UniquePtr<VulkanPushConstantsRange>> m_ranges;
    UInt32 m_size;

public:
    VulkanRenderPipelinePushConstantsLayoutBuilderImpl(VulkanRenderPipelinePushConstantsLayoutBuilder* parent, const UInt32& size) :
        base(parent), m_size(size)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelinePushConstantsLayoutBuilder::VulkanRenderPipelinePushConstantsLayoutBuilder(VulkanRenderPipelineLayoutBuilder& parent, const UInt32& size) :
    m_impl(makePimpl<VulkanRenderPipelinePushConstantsLayoutBuilderImpl>(this, size)), PushConstantsLayoutBuilder(parent, UniquePtr<VulkanPushConstantsLayout>(new VulkanPushConstantsLayout(*std::as_const(parent).instance(), size)))
{
}

VulkanRenderPipelinePushConstantsLayoutBuilder::~VulkanRenderPipelinePushConstantsLayoutBuilder() noexcept = default;

VulkanRenderPipelineLayoutBuilder& VulkanRenderPipelinePushConstantsLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->setRanges(std::move(m_impl->m_ranges));

    return PushConstantsLayoutBuilder::go();
}

VulkanRenderPipelinePushConstantsLayoutBuilder& VulkanRenderPipelinePushConstantsLayoutBuilder::addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding)
{
    m_impl->m_ranges.push_back(makeUnique<VulkanPushConstantsRange>(shaderStages, offset, size, space, binding));
    return *this;
}

// ------------------------------------------------------------------------------------------------
// Compute pipeline descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipelinePushConstantsLayoutBuilder::VulkanComputePipelinePushConstantsLayoutBuilderImpl : public Implement<VulkanComputePipelinePushConstantsLayoutBuilder> {
public:
    friend class VulkanComputePipelinePushConstantsLayoutBuilder;

private:
    Array<UniquePtr<VulkanPushConstantsRange>> m_ranges;
    UInt32 m_size;

public:
    VulkanComputePipelinePushConstantsLayoutBuilderImpl(VulkanComputePipelinePushConstantsLayoutBuilder* parent, const UInt32& size) :
        base(parent), m_size(size)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Compute pipeline descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipelinePushConstantsLayoutBuilder::VulkanComputePipelinePushConstantsLayoutBuilder(VulkanComputePipelineLayoutBuilder& parent, const UInt32& size) :
    m_impl(makePimpl<VulkanComputePipelinePushConstantsLayoutBuilderImpl>(this, size)), PushConstantsLayoutBuilder(parent, UniquePtr<VulkanPushConstantsLayout>(new VulkanPushConstantsLayout(*std::as_const(parent).instance(), size)))
{
}

VulkanComputePipelinePushConstantsLayoutBuilder::~VulkanComputePipelinePushConstantsLayoutBuilder() noexcept = default;

VulkanComputePipelineLayoutBuilder& VulkanComputePipelinePushConstantsLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->setRanges(std::move(m_impl->m_ranges));

    return PushConstantsLayoutBuilder::go();
}

VulkanComputePipelinePushConstantsLayoutBuilder& VulkanComputePipelinePushConstantsLayoutBuilder::addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding)
{
    m_impl->m_ranges.push_back(makeUnique<VulkanPushConstantsRange>(shaderStages, offset, size, space, binding));
    return *this;
}