#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanPipelineLayout::VulkanPipelineLayoutImpl : public Implement<VulkanPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayoutBuilder;
    friend class VulkanComputePipelineLayoutBuilder;
    friend class VulkanPipelineLayout;

private:
    UniquePtr<VulkanShaderProgram> m_shaderProgram;
    UniquePtr<VulkanPushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<VulkanDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    VulkanPipelineLayoutImpl(VulkanPipelineLayout* parent, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
        base(parent), m_shaderProgram(std::move(shaderProgram)), m_descriptorSetLayouts(std::move(descriptorLayouts)), m_pushConstantsLayout(std::move(pushConstantsLayout))
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

        // Query for push constant ranges.
        Array<const VulkanPushConstantsRange*> ranges = m_pushConstantsLayout == nullptr ? Array<const VulkanPushConstantsRange*>{} : m_pushConstantsLayout->ranges();
        auto rangeHandles = ranges |
            std::views::transform([](const VulkanPushConstantsRange* range) { return VkPushConstantRange{ .stageFlags = static_cast<VkShaderStageFlags>(Vk::getShaderStage(range->stage())), .offset = range->offset(), .size = range->size() }; }) |
            ranges::to<Array<VkPushConstantRange>>();

        // Create the pipeline layout.
        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0} {{ Descriptor Sets: {1}, Push Constant Ranges: {2} }}...", fmt::ptr(m_parent), descriptorSetLayouts.size(), rangeHandles.size());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<UInt32>(rangeHandles.size());
        pipelineLayoutInfo.pPushConstantRanges = rangeHandles.data();

        VkPipelineLayout layout;
        raiseIfFailed<RuntimeException>(::vkCreatePipelineLayout(m_parent->getDevice()->handle(), &pipelineLayoutInfo, nullptr, &layout), "Unable to create pipeline layout.");
        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanRenderPipeline& pipeline, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this, std::move(shaderProgram), std::move(descriptorSetLayouts), std::move(pushConstantsLayout))), VulkanRuntimeObject<VulkanPipelineState>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanComputePipeline& pipeline, UniquePtr<VulkanShaderProgram>&& shaderProgram, Array<UniquePtr<VulkanDescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this, std::move(shaderProgram), std::move(descriptorSetLayouts), std::move(pushConstantsLayout))), VulkanRuntimeObject<VulkanPipelineState>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanRenderPipeline& pipeline) noexcept :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this)), VulkanRuntimeObject<VulkanPipelineState>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
{
}

VulkanPipelineLayout::VulkanPipelineLayout(const VulkanComputePipeline& pipeline) noexcept :
    m_impl(makePimpl<VulkanPipelineLayoutImpl>(this)), VulkanRuntimeObject<VulkanPipelineState>(pipeline, pipeline.getDevice()), Resource<VkPipelineLayout>(VK_NULL_HANDLE)
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

const VulkanPushConstantsLayout* VulkanPipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}

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