#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineDescriptorSetLayoutBuilder::VulkanRenderPipelineDescriptorSetLayoutBuilderImpl : public Implement<VulkanRenderPipelineDescriptorSetLayoutBuilder> {
public:
    friend class VulkanRenderPipelineDescriptorSetLayoutBuilder;

private:
    Array<UniquePtr<VulkanDescriptorLayout>> m_descriptorLayouts;
    UInt32 m_poolSize, m_space;
    ShaderStage m_stages;

public:
    VulkanRenderPipelineDescriptorSetLayoutBuilderImpl(VulkanRenderPipelineDescriptorSetLayoutBuilder* parent, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
        base(parent), m_poolSize(poolSize), m_space(space), m_stages(stages)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Render pipeline descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineDescriptorSetLayoutBuilder::VulkanRenderPipelineDescriptorSetLayoutBuilder(VulkanRenderPipelineLayoutBuilder& parent, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
    m_impl(makePimpl<VulkanRenderPipelineDescriptorSetLayoutBuilderImpl>(this, space, stages, poolSize)), DescriptorSetLayoutBuilder(parent, UniquePtr<VulkanDescriptorSetLayout>(new VulkanDescriptorSetLayout(*std::as_const(parent).instance())))
{
}

VulkanRenderPipelineDescriptorSetLayoutBuilder::~VulkanRenderPipelineDescriptorSetLayoutBuilder() noexcept = default;

void VulkanRenderPipelineDescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorLayouts = std::move(m_impl->m_descriptorLayouts);
    instance->m_impl->m_poolSize = std::move(m_impl->m_poolSize);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = std::move(m_impl->m_stages);
    instance->handle() = instance->m_impl->initialize();
}

VulkanRenderPipelineDescriptorSetLayoutBuilder& VulkanRenderPipelineDescriptorSetLayoutBuilder::withDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout)
{
    m_impl->m_descriptorLayouts.push_back(std::move(layout));
    return *this;
}

VulkanRenderPipelineDescriptorSetLayoutBuilder& VulkanRenderPipelineDescriptorSetLayoutBuilder::withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors)
{
    return this->withDescriptor(makeUnique<VulkanDescriptorLayout>(*(this->instance()), type, binding, descriptorSize, descriptors));
}

VulkanRenderPipelineDescriptorSetLayoutBuilder& VulkanRenderPipelineDescriptorSetLayoutBuilder::space(const UInt32& space) noexcept
{
    m_impl->m_space = space;
    return *this;
}

VulkanRenderPipelineDescriptorSetLayoutBuilder& VulkanRenderPipelineDescriptorSetLayoutBuilder::shaderStages(const ShaderStage& stages) noexcept
{
    m_impl->m_stages = stages;
    return *this;
}

VulkanRenderPipelineDescriptorSetLayoutBuilder& VulkanRenderPipelineDescriptorSetLayoutBuilder::poolSize(const UInt32& poolSize) noexcept
{
    m_impl->m_poolSize = poolSize;
    return *this;
}

// ------------------------------------------------------------------------------------------------
// Compute pipeline descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipelineDescriptorSetLayoutBuilder::VulkanComputePipelineDescriptorSetLayoutBuilderImpl : public Implement<VulkanComputePipelineDescriptorSetLayoutBuilder> {
public:
    friend class VulkanComputePipelineDescriptorSetLayoutBuilder;

private:
    Array<UniquePtr<VulkanDescriptorLayout>> m_descriptorLayouts;
    UInt32 m_poolSize, m_space;

public:
    VulkanComputePipelineDescriptorSetLayoutBuilderImpl(VulkanComputePipelineDescriptorSetLayoutBuilder* parent, const UInt32& space, const UInt32& poolSize) :
        base(parent), m_poolSize(poolSize), m_space(space)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Compute pipeline descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipelineDescriptorSetLayoutBuilder::VulkanComputePipelineDescriptorSetLayoutBuilder(VulkanComputePipelineLayoutBuilder& parent, const UInt32& space, const UInt32& poolSize) :
    m_impl(makePimpl<VulkanComputePipelineDescriptorSetLayoutBuilderImpl>(this, space, poolSize)), DescriptorSetLayoutBuilder(parent, UniquePtr<VulkanDescriptorSetLayout>(new VulkanDescriptorSetLayout(*std::as_const(parent).instance())))
{
}

VulkanComputePipelineDescriptorSetLayoutBuilder::~VulkanComputePipelineDescriptorSetLayoutBuilder() noexcept = default;

void VulkanComputePipelineDescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorLayouts = std::move(m_impl->m_descriptorLayouts);
    instance->m_impl->m_poolSize = std::move(m_impl->m_poolSize);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = ShaderStage::Compute;
    instance->handle() = instance->m_impl->initialize();
}

VulkanComputePipelineDescriptorSetLayoutBuilder& VulkanComputePipelineDescriptorSetLayoutBuilder::withDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout)
{
    m_impl->m_descriptorLayouts.push_back(std::move(layout));
    return *this;
}

VulkanComputePipelineDescriptorSetLayoutBuilder& VulkanComputePipelineDescriptorSetLayoutBuilder::withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors)
{
    return this->withDescriptor(makeUnique<VulkanDescriptorLayout>(*(this->instance()), type, binding, descriptorSize, descriptors));
}

VulkanComputePipelineDescriptorSetLayoutBuilder& VulkanComputePipelineDescriptorSetLayoutBuilder::space(const UInt32& space) noexcept
{
    m_impl->m_space = space;
    return *this;
}

VulkanComputePipelineDescriptorSetLayoutBuilder& VulkanComputePipelineDescriptorSetLayoutBuilder::poolSize(const UInt32& poolSize) noexcept
{
    m_impl->m_poolSize = poolSize;
    return *this;
}