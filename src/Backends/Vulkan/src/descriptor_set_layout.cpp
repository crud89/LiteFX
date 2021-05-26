#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutImpl : public Implement<VulkanDescriptorSetLayout> {
public:
    friend class VulkanDescriptorSetLayoutBuilder;
    friend class VulkanDescriptorSetLayout;

private:
    Array<UniquePtr<VulkanDescriptorLayout>> m_descriptorLayouts;
    Array<VkDescriptorPool> m_descriptorPools;
    Queue<VkDescriptorSet> m_freeDescriptorSets;
    Array<VkDescriptorPoolSize> m_poolSizes {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 0 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0 }
    };
    ShaderStage m_stages;
    UInt32 m_space, m_poolSize;
    mutable std::mutex m_mutex;

public:
    VulkanDescriptorSetLayoutImpl(VulkanDescriptorSetLayout* parent, Array<UniquePtr<VulkanDescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
        base(parent), m_descriptorLayouts(std::move(descriptorLayouts)), m_space(space), m_stages(stages), m_poolSize(poolSize)
    {
    }

public:
    VkDescriptorSetLayout initialize()
    {
        LITEFX_TRACE(VULKAN_LOG, "Defining layout for descriptor set {0} {{ Stages: {1}, Pool Size: {2} }}...", m_space, m_stages, m_poolSize);

        // Parse the shader stage descriptor.
        VkShaderStageFlags shaderStages = {};

        if ((m_stages & ShaderStage::Vertex) == ShaderStage::Vertex)
            shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;
        if ((m_stages & ShaderStage::Geometry) == ShaderStage::Geometry)
            shaderStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if ((m_stages & ShaderStage::Fragment) == ShaderStage::Fragment)
            shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if ((m_stages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
            shaderStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if ((m_stages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
            shaderStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if ((m_stages & ShaderStage::Compute) == ShaderStage::Compute)
            shaderStages |= VK_SHADER_STAGE_COMPUTE_BIT;

        // Parse descriptor set layouts.
        Array<VkDescriptorSetLayoutBinding> bindings;

        std::ranges::for_each(m_descriptorLayouts, [&, i = 0](const UniquePtr<VulkanDescriptorLayout>& layout) mutable {
            auto bindingPoint = layout->binding();
            auto type = layout->descriptorType();

            LITEFX_TRACE(VULKAN_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Offset: {4}, Binding point: {5} }}...", ++i, m_descriptorLayouts.size(), type, layout->elementSize(), 0, bindingPoint);

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = bindingPoint;
            binding.descriptorCount = 1;		// TODO: Implement support for descriptor arrays.
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = shaderStages;

            if (type == DescriptorType::InputAttachment && m_stages != ShaderStage::Fragment)
                throw RuntimeException("Unable to bind input attachment at {0} to a descriptor set that is accessible from other stages, than the fragment shader.", bindingPoint);

            switch (type)
            {
            case DescriptorType::Uniform:         binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;   break;
            case DescriptorType::Storage:         binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;   break;
            case DescriptorType::Image:           binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;    break;
            case DescriptorType::Sampler:         binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;          break;
            case DescriptorType::InputAttachment: binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; break;
            default: LITEFX_WARNING(VULKAN_LOG, "The descriptor type is unsupported. Binding will be skipped."); return;
            }

            m_poolSizes[binding.descriptorType].descriptorCount++;
            bindings.push_back(binding);
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input attachments: {6} }}...", m_space, m_descriptorLayouts.size(), m_poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT].descriptorCount);

        // Create the descriptor set layout.
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = bindings.size();
        descriptorSetLayoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        raiseIfFailed<RuntimeException>(::vkCreateDescriptorSetLayout(m_parent->getDevice()->handle(), &descriptorSetLayoutInfo, nullptr, &layout), "Unable to create descriptor set layout.");

        // Create the initial descriptor pool.
        this->addDescriptorPool();

        return layout;
    }

    void addDescriptorPool()
    {
        LITEFX_TRACE(VULKAN_LOG, "Allocating descriptor pool with {5} sets {{ Uniforms: {0}, Storages: {1}, Images: {2}, Samplers: {3}, Input attachments: {4} }}...", m_poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLER].descriptorCount, m_poolSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT].descriptorCount, m_poolSize);

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = m_poolSizes.size();
        poolInfo.pPoolSizes = m_poolSizes.data();
        poolInfo.maxSets = m_poolSize;

        VkDescriptorPool descriptorPool;
        raiseIfFailed<RuntimeException>(::vkCreateDescriptorPool(m_parent->getDevice()->handle(), &poolInfo, nullptr, &descriptorPool), "Unable to create buffer pool.");
        m_descriptorPools.push_back(descriptorPool);
    }

    VkDescriptorSet tryAllocate() 
    {
        VkDescriptorSetAllocateInfo descriptorSetInfo = {};
        descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetInfo.descriptorPool = m_descriptorPools.back();
        descriptorSetInfo.descriptorSetCount = 1;
        descriptorSetInfo.pSetLayouts = &m_parent->handle();

        // Try to allocate a new descriptor set.
        VkDescriptorSet descriptorSet;
        auto result = ::vkAllocateDescriptorSets(m_parent->getDevice()->handle(), &descriptorSetInfo, &descriptorSet);

        if (result == VK_SUCCESS)
            return descriptorSet;
        else if (result != VK_ERROR_OUT_OF_POOL_MEMORY)
            raiseIfFailed<RuntimeException>(result, "Unable to allocate descriptor set.");

        // The pool is full, so we have to create a new one and retry.
        this->addDescriptorPool();
        return this->tryAllocate();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanRenderPipelineLayout& pipelineLayout, Array<UniquePtr<VulkanDescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
    m_impl(makePimpl<VulkanDescriptorSetLayoutImpl>(this, descriptorLayouts, space, stages, poolSize)), VulkanRuntimeObject<VulkanRenderPipelineLayout>(pipelineLayout, pipelineLayout.getDevice()), IResource<VkDescriptorSetLayout>(nullptr)
{
    this->handle() = m_impl->initialize();
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept
{
    // Release descriptor pools and destroy the descriptor set layouts. Releasing the pools also frees the descriptor sets allocated from it.
    std::ranges::for_each(m_impl->m_descriptorPools, [this](const VkDescriptorPool& pool) { ::vkDestroyDescriptorPool(this->getDevice()->handle(), pool, nullptr); });
    ::vkDestroyDescriptorSetLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

Array<const VulkanDescriptorLayout*> VulkanDescriptorSetLayout::layouts() const noexcept
{
    return m_impl->m_descriptorLayouts |
        std::views::transform([](const UniquePtr<VulkanDescriptorLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const VulkanDescriptorLayout*>>();
}

const VulkanDescriptorLayout& VulkanDescriptorSetLayout::layout(const UInt32& binding) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorLayouts, [&binding](const UniquePtr<VulkanDescriptorLayout>& layout) { return layout->binding() == binding; }); match != m_impl->m_descriptorLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No layout has been provided for the binding {0}.", binding);
}

const UInt32& VulkanDescriptorSetLayout::space() const noexcept
{
    return m_impl->m_space;
}

const ShaderStage& VulkanDescriptorSetLayout::shaderStages() const noexcept
{
    return m_impl->m_stages;
}

UInt32 VulkanDescriptorSetLayout::uniforms() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::storages() const noexcept
{
    return  m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::images() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::samplers() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::inputAttachments() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT].descriptorCount;
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate() const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // If no descriptor sets are free, allocate a new one.
    if (m_impl->m_freeDescriptorSets.empty())
        return makeUnique<VulkanDescriptorSet>(*this, m_impl->tryAllocate());
    
    // Otherwise, pick and remove one from the list.
    auto descriptorSet = makeUnique<VulkanDescriptorSet>(*this, m_impl->m_freeDescriptorSets.front());
    m_impl->m_freeDescriptorSets.pop();
}

void VulkanDescriptorSetLayout::free(const VulkanDescriptorSet& descriptorSet) const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_freeDescriptorSets.push(descriptorSet.handle());
}

const UInt32& VulkanDescriptorSetLayout::poolSize() const noexcept
{
    return m_impl->m_poolSize;
}

size_t VulkanDescriptorSetLayout::pools() const noexcept
{
    return m_impl->m_descriptorPools.size();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilderImpl : public Implement<VulkanDescriptorSetLayoutBuilder> {
public:
    friend class VulkanDescriptorSetLayoutBuilder;

private:
    Array<UniquePtr<VulkanDescriptorLayout>> m_descriptorLayouts;
    UInt32 m_poolSize, m_space;
    ShaderStage m_stages;

public:
    VulkanDescriptorSetLayoutBuilderImpl(VulkanDescriptorSetLayoutBuilder* parent, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
        base(parent), m_poolSize(poolSize), m_space(space), m_stages(stages)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilder(const VulkanRenderPipelineLayoutBuilder& parent, const UInt32& space, const ShaderStage& stages, const UInt32& poolSize) :
    m_impl(makePimpl<VulkanDescriptorSetLayoutBuilderImpl>(this, space, stages, poolSize)), DescriptorSetLayoutBuilder(parent, makeUnique<VulkanDescriptorSetLayout>(*parent.instance()))
{
}

VulkanDescriptorSetLayoutBuilder::~VulkanDescriptorSetLayoutBuilder() noexcept = default;

VulkanRenderPipelineLayoutBuilder& VulkanDescriptorSetLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorLayouts = std::move(m_impl->m_descriptorLayouts);
    instance->m_impl->m_poolSize = std::move(m_impl->m_poolSize);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = std::move(m_impl->m_stages);
    instance->handle() = instance->m_impl->initialize();

    return DescriptorSetLayoutBuilder::go();
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addDescriptor(UniquePtr<VulkanDescriptorLayout>&& layout)
{
    m_impl->m_descriptorLayouts.push_back(std::move(layout));
    return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize)
{
    return this->addDescriptor(makeUnique<VulkanDescriptorLayout>(*(this->instance()), type, binding, descriptorSize));
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::space(const UInt32& space) noexcept
{
    m_impl->m_space = space;
    return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::shaderStages(const ShaderStage& stages) noexcept
{
    m_impl->m_stages = stages;
    return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::poolSize(const UInt32& poolSize) noexcept
{
    m_impl->m_poolSize = poolSize;
    return *this;
}