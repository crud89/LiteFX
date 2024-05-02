#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include "image.h"

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
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0 }
    };
    Dictionary<VkDescriptorType, UInt32> m_poolSizeMapping {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 3 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 6 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 7 }
    };
    ShaderStage m_stages;
    UInt32 m_space;
    mutable std::mutex m_mutex;
    const VulkanDevice& m_device;
    bool m_usesDescriptorIndexing = false;
    Dictionary<const VkDescriptorSet*, const VkDescriptorPool*> m_descriptorSetSources;

public:
    VulkanDescriptorSetLayoutImpl(VulkanDescriptorSetLayout* parent, const VulkanDevice& device, Enumerable<UniquePtr<VulkanDescriptorLayout>>&& descriptorLayouts, UInt32 space, ShaderStage stages) :
        base(parent), m_device(device), m_space(space), m_stages(stages)
    {
        m_descriptorLayouts = descriptorLayouts | std::views::as_rvalue | std::ranges::to<std::vector>();
    }

    VulkanDescriptorSetLayoutImpl(VulkanDescriptorSetLayout* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
    {
    }

public:
    VkDescriptorSetLayout initialize(UInt32 maxUnboundedArraySize)
    {
        LITEFX_TRACE(VULKAN_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

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
        if ((m_stages & ShaderStage::Task) == ShaderStage::Task)
            shaderStages |= VK_SHADER_STAGE_TASK_BIT_EXT;
        if ((m_stages & ShaderStage::Mesh) == ShaderStage::Mesh)
            shaderStages |= VK_SHADER_STAGE_MESH_BIT_EXT;
        if ((m_stages & ShaderStage::RayGeneration) == ShaderStage::RayGeneration)
            shaderStages |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        if ((m_stages & ShaderStage::Miss) == ShaderStage::Miss)
            shaderStages |= VK_SHADER_STAGE_MISS_BIT_KHR;
        if ((m_stages & ShaderStage::Callable) == ShaderStage::Callable)
            shaderStages |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        if ((m_stages & ShaderStage::ClosestHit) == ShaderStage::ClosestHit)
            shaderStages |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        if ((m_stages & ShaderStage::AnyHit) == ShaderStage::AnyHit)
            shaderStages |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        if ((m_stages & ShaderStage::Intersection) == ShaderStage::Intersection)
            shaderStages |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

        // Parse descriptor set layouts.
        Array<VkDescriptorSetLayoutBinding> bindings;
        Array<VkDescriptorBindingFlags> bindingFlags;
        Array<VkDescriptorSetLayoutBindingFlagsCreateInfo> bindingFlagCreateInfo;

        std::ranges::for_each(m_descriptorLayouts, [&, i = 0](const UniquePtr<VulkanDescriptorLayout>& layout) mutable {
            auto bindingPoint = layout->binding();
            auto type = layout->descriptorType();

            LITEFX_TRACE(VULKAN_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Array size: {6}, Offset: {4}, Binding point: {5} }}...", ++i, m_descriptorLayouts.size(), type, layout->elementSize(), 0, bindingPoint, layout->descriptors());

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = bindingPoint;
            binding.descriptorCount = layout->descriptors();
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = shaderStages;

            if (type == DescriptorType::InputAttachment && m_stages != ShaderStage::Fragment)
                throw RuntimeException("Unable to bind input attachment at {0} to a descriptor set that is accessible from other stages, than the fragment shader.", bindingPoint);

            switch (type)
            {
            case DescriptorType::ConstantBuffer:        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;       break;
            case DescriptorType::ByteAddressBuffer:
            case DescriptorType::RWByteAddressBuffer:
            case DescriptorType::StructuredBuffer:
            case DescriptorType::RWStructuredBuffer:    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;       break;
            case DescriptorType::RWTexture:             binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;        break;
            case DescriptorType::Texture:               binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;        break;
            case DescriptorType::RWBuffer:              binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
            case DescriptorType::Buffer:                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
            case DescriptorType::InputAttachment:       binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;     break;
            case DescriptorType::Sampler:               binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;              break;
            case DescriptorType::AccelerationStructure: binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR; break;
            default: LITEFX_WARNING(VULKAN_LOG, "The descriptor type is unsupported. Binding will be skipped.");       return;
            }

            if (type != DescriptorType::Sampler || (type == DescriptorType::Sampler && layout->staticSampler() == nullptr))
                m_poolSizes[m_poolSizeMapping[binding.descriptorType]].descriptorCount++;
            else
                binding.pImmutableSamplers = &layout->staticSampler()->handle();
            
            // If the descriptor is an unbounded runtime array, disable validation warnings about partially bound elements.
            if (binding.descriptorCount != std::numeric_limits<UInt32>::max())
                bindingFlags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);
            else
            {
                // Unbounded arrays must be the only descriptor within a descriptor set.
                if (m_descriptorLayouts.size() != 1) [[unlikely]]
                    throw InvalidArgumentException("descriptorLayouts", "If an unbounded runtime array descriptor is used, it must be the only descriptor in the descriptor set, however the current descriptor set specifies {0} descriptors", m_descriptorLayouts.size());

                bindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);
                m_usesDescriptorIndexing = true;
                binding.descriptorCount = maxUnboundedArraySize;
            }

            bindings.push_back(binding);
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input Attachments: {6}, Writable Images: {7}, Texel Buffers: {8} }}...", m_space, m_descriptorLayouts.size(), m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_SAMPLER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER]].descriptorCount);

        // Create the descriptor set layout.
        VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<UInt32>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data()
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &extendedInfo,
            .bindingCount = static_cast<UInt32>(bindings.size()),
            .pBindings = bindings.data()
        };

        // Allow for descriptors to update after they have been bound. This also means, we have to manually take care of not to update a descriptor before it got used.
        descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VkDescriptorSetLayout layout;
        raiseIfFailed(::vkCreateDescriptorSetLayout(m_device.handle(), &descriptorSetLayoutInfo, nullptr, &layout), "Unable to create descriptor set layout.");

        return layout;
    }

    void reserve(UInt32 descriptorSets)
    {
        LITEFX_TRACE(VULKAN_LOG, "Allocating descriptor pool with {5} sets {{ Uniforms: {0}, Storages: {1}, Images: {2}, Samplers: {3}, Input attachments: {4} }}...", m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_SAMPLER]].descriptorCount, m_poolSizes[m_poolSizeMapping[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT]].descriptorCount, descriptorSets);

        // Filter pool sizes, since descriptorCount must be greater than 0, according to the specs.
        auto poolSizes = m_poolSizes |
            std::views::filter([](const VkDescriptorPoolSize& poolSize) { return poolSize.descriptorCount > 0; }) |
            std::ranges::to<std::vector>();

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = descriptorSets;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        VkDescriptorPool descriptorPool;
        raiseIfFailed(::vkCreateDescriptorPool(m_device.handle(), &poolInfo, nullptr, &descriptorPool), "Unable to create buffer pool.");
        m_descriptorPools.push_back(descriptorPool);
    }

    VkDescriptorSet tryAllocate(UInt32 descriptors)
    {
        return this->tryAllocate(1u, descriptors).front();
    }

    Array<VkDescriptorSet> tryAllocate(UInt32 descriptorSets, UInt32 descriptorsPerSet)
    {
        // NOTE: We're thread safe here, as the calls from the interface use a mutex to synchronize allocation.
        
        // If the descriptor set layout is empty, no descriptor set can be allocated.
        if (m_descriptorLayouts.empty()) [[unlikely]]
            throw RuntimeException("Cannot allocate descriptor set from empty layout.");

        // Start by reserving enough space for all descriptor sets.
        this->reserve(descriptorSets);

        // Allocate the descriptor sets.
        Array<VkDescriptorSetLayout> layouts(descriptorSets, m_parent->handle());

        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo;
        VkDescriptorSetAllocateInfo descriptorSetInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorPools.back(),
            .descriptorSetCount = descriptorSets,
            .pSetLayouts = layouts.data()
        };

        // Define variable descriptor count in pNext-chain, if descriptor set contains an unbounded array.
        Array<UInt32> descriptorCounts(descriptorSets, descriptorsPerSet);

        if (m_usesDescriptorIndexing)
        {
            variableCountInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                .descriptorSetCount = descriptorSets,
                .pDescriptorCounts = descriptorCounts.data()
            };

            descriptorSetInfo.pNext = &variableCountInfo;
        }

        // Try to allocate a new descriptor sets.
        Array<VkDescriptorSet> descriptorSetHandles(descriptorSets, VK_NULL_HANDLE);
        raiseIfFailed(::vkAllocateDescriptorSets(m_device.handle(), &descriptorSetInfo, descriptorSetHandles.data()), "Unable to allocate descriptor set.");

        for (auto handle : descriptorSetHandles)
            m_descriptorSetSources.insert(std::make_pair(&handle, &m_descriptorPools.back()));

        return descriptorSetHandles;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device, Enumerable<UniquePtr<VulkanDescriptorLayout>>&& descriptorLayouts, UInt32 space, ShaderStage stages, UInt32 maxUnboundedArraySize) :
    m_impl(makePimpl<VulkanDescriptorSetLayoutImpl>(this, device, std::move(descriptorLayouts), space, stages)), Resource<VkDescriptorSetLayout>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize(maxUnboundedArraySize);
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanDescriptorSetLayoutImpl>(this, device)), Resource<VkDescriptorSetLayout>(VK_NULL_HANDLE)
{
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept
{
    // Release descriptor pools and destroy the descriptor set layouts. Releasing the pools also frees the descriptor sets allocated from it.
    std::ranges::for_each(m_impl->m_descriptorPools, [this](const VkDescriptorPool& pool) { ::vkDestroyDescriptorPool(m_impl->m_device.handle(), pool, nullptr); });
    ::vkDestroyDescriptorSetLayout(m_impl->m_device.handle(), this->handle(), nullptr);
}

const VulkanDevice& VulkanDescriptorSetLayout::device() const noexcept
{
    return m_impl->m_device;
}

Enumerable<const VulkanDescriptorLayout*> VulkanDescriptorSetLayout::descriptors() const noexcept
{
    return m_impl->m_descriptorLayouts | std::views::transform([](const UniquePtr<VulkanDescriptorLayout>& layout) { return layout.get(); });
}

const VulkanDescriptorLayout& VulkanDescriptorSetLayout::descriptor(UInt32 binding) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorLayouts, [&binding](const UniquePtr<VulkanDescriptorLayout>& layout) { return layout->binding() == binding; }); match != m_impl->m_descriptorLayouts.end()) [[likely]]
        return *match->get();

    throw InvalidArgumentException("binding", "No layout has been provided for the binding {0}.", binding);
}

UInt32 VulkanDescriptorSetLayout::space() const noexcept
{
    return m_impl->m_space;
}

ShaderStage VulkanDescriptorSetLayout::shaderStages() const noexcept
{
    return m_impl->m_stages;
}

UInt32 VulkanDescriptorSetLayout::uniforms() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::storages() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::buffers() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER].descriptorCount + m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::images() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE].descriptorCount + m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::samplers() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_SAMPLER].descriptorCount;
}

UInt32 VulkanDescriptorSetLayout::staticSamplers() const noexcept
{
    return std::ranges::count_if(m_impl->m_descriptorLayouts, [](const UniquePtr<VulkanDescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Sampler && layout->staticSampler() != nullptr; });
}

UInt32 VulkanDescriptorSetLayout::inputAttachments() const noexcept
{
    return m_impl->m_poolSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT].descriptorCount;
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate(const Enumerable<DescriptorBinding>& bindings) const
{
    return this->allocate(0, bindings);
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // If no descriptor sets are free, or the descriptor set contains an unbounded descriptor array, allocate a new descriptor set.
    UniquePtr<VulkanDescriptorSet> descriptorSet;

    if (m_impl->m_usesDescriptorIndexing || m_impl->m_freeDescriptorSets.empty())
        descriptorSet = makeUnique<VulkanDescriptorSet>(*this, m_impl->tryAllocate(descriptors));
    else
    {
        // Otherwise, pick and remove one from the list.
        descriptorSet = makeUnique<VulkanDescriptorSet>(*this, m_impl->m_freeDescriptorSets.front());
        m_impl->m_freeDescriptorSets.pop();
    }

    // Apply the default bindings.
    for (UInt32 i{ 0 }; auto& binding : bindings)
    {
        std::visit(type_switch{
            [](const std::monostate&) { }, // Default: don't bind anything.
            [&descriptorSet, &binding, i](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
            [&descriptorSet, &binding, i](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
            [&descriptorSet, &binding, i](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
            [&descriptorSet, &binding, i](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
        }, binding.resource);

        ++i;
    }

    // Return the descriptor set.
    return descriptorSet;
}

Enumerable<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocateMultiple(UInt32 descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings) const
{
    return this->allocateMultiple(descriptorSets, 0, bindings);
}

Enumerable<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocateMultiple(UInt32 descriptorSets, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const
{
    return this->allocateMultiple(descriptorSets, 0, bindingFactory);
}

Enumerable<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocateMultiple(UInt32 count, UInt32 unboundedDescriptorsCount, const Enumerable<Enumerable<DescriptorBinding>>& bindingsPerSet) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // If the set contains an unbounded array, or there are no free sets left, we need to allocate anyway.
    Enumerable<UniquePtr<VulkanDescriptorSet>> descriptorSets = (m_impl->m_usesDescriptorIndexing || m_impl->m_freeDescriptorSets.empty() ?
        [this, unboundedDescriptorsCount, &count]() -> std::generator<VkDescriptorSet> { co_yield std::ranges::elements_of(m_impl->tryAllocate(count, unboundedDescriptorsCount) | std::views::as_rvalue); }() :
        [this, unboundedDescriptorsCount, &count]() -> std::generator<VkDescriptorSet> {
            UInt32 remainingAllocations = count;

            // If there are free descriptor sets, use them first.
            while (!m_impl->m_freeDescriptorSets.empty() && remainingAllocations --> 0) // Finally a good use for the "-->" operator!!!
            {
                auto descriptorSet = m_impl->m_freeDescriptorSets.front();
                m_impl->m_freeDescriptorSets.pop();
                co_yield descriptorSet;
            }

            // Allocate the rest from a new descriptor pool and return them.
            co_yield std::ranges::elements_of(m_impl->tryAllocate(remainingAllocations, unboundedDescriptorsCount) | std::views::as_rvalue);
        }()) | std::views::transform([this](auto handle) { return makeUnique<VulkanDescriptorSet>(*this, handle); }) | std::views::as_rvalue;

    // Apply the default bindings.
    for (auto [descriptorSet, bindingsPerDescriptor] : std::views::zip(descriptorSets | std::views::transform([](auto& set) { return set.get(); }), bindingsPerSet))
    {
        for (UInt32 i{ 0 }; auto& binding : bindingsPerDescriptor)
        {
            std::visit(type_switch {
                [](const std::monostate&) {}, // Default: don't bind anything.
                [descriptorSet, &binding, i](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
                [descriptorSet, &binding, i](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
                [descriptorSet, &binding, i](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
                [descriptorSet, &binding, i](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
            }, binding.resource);

            ++i;
        }
    }

    return descriptorSets;
}

Enumerable<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocateMultiple(UInt32 count, UInt32 unboundedDescriptorsCount, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // If the set contains an unbounded array, or there are no free sets left, we need to allocate anyway.
    Enumerable<UniquePtr<VulkanDescriptorSet>> descriptorSets = (m_impl->m_usesDescriptorIndexing || m_impl->m_freeDescriptorSets.empty() ?
        [this, unboundedDescriptorsCount, &count]() -> std::generator<VkDescriptorSet> { co_yield std::ranges::elements_of(m_impl->tryAllocate(count, unboundedDescriptorsCount) | std::views::as_rvalue); }() :
        [this, unboundedDescriptorsCount, &count]() -> std::generator<VkDescriptorSet> {
            UInt32 remainingAllocations = count;

            // If there are free descriptor sets, use them first.
            while (!m_impl->m_freeDescriptorSets.empty() && remainingAllocations --> 0) // Finally a good use for the "-->" operator!!!
            {
                auto descriptorSet = m_impl->m_freeDescriptorSets.front();
                m_impl->m_freeDescriptorSets.pop();
                co_yield descriptorSet;
            }

            // Allocate the rest from a new descriptor pool and return them.
            co_yield std::ranges::elements_of(m_impl->tryAllocate(remainingAllocations, unboundedDescriptorsCount) | std::views::as_rvalue);
        }()) | std::views::transform([this](auto handle) { return makeUnique<VulkanDescriptorSet>(*this, handle); }) | std::views::as_rvalue;

    // Apply the default bindings.
    for (UInt32 set{ 0 }; auto& descriptorSet : descriptorSets)
    {
        for (UInt32 i{ 0 }; auto& binding : bindingFactory(set++))
        {
            std::visit(type_switch {
                [](const std::monostate&) {}, // Default: don't bind anything.
                [&descriptorSet, &binding, i](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
                [&descriptorSet, &binding, i](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
                [&descriptorSet, &binding, i](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
                [&descriptorSet, &binding, i](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
            }, binding.resource);

            ++i;
        }
    }

    return descriptorSets;
}

void VulkanDescriptorSetLayout::free(const VulkanDescriptorSet& descriptorSet) const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    if (!m_impl->m_usesDescriptorIndexing)
    {
        // Keep the descriptor set around (get's automatically released when the pool gets destroyed).
        m_impl->m_freeDescriptorSets.push(descriptorSet.handle());
    }
    else
    {
        // Unbounded descriptor sets must be destroyed, because every set may have different descriptor counts.
        auto handle = &descriptorSet.handle();

        if (m_impl->m_descriptorSetSources.contains(handle))
        {
            auto pool = m_impl->m_descriptorSetSources[handle];
            auto result = ::vkFreeDescriptorSets(m_impl->m_device.handle(), *pool, 1, handle);
            
            if (result != VK_SUCCESS) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Unable to properly release descriptor set: {0}.", result);

            m_impl->m_descriptorSetSources.erase(handle);

            // We can even release the pool, if it isn't the current active one and no descriptor sets are in use anymore.
            if (pool != &m_impl->m_descriptorPools.back() && std::ranges::count_if(m_impl->m_descriptorSetSources, [&](const auto& sourceMapping) { return sourceMapping.second == pool; }) == 0)
            {
                ::vkDestroyDescriptorPool(m_impl->m_device.handle(), *pool, nullptr);

                if (auto match = std::ranges::find(m_impl->m_descriptorPools, *pool); match != m_impl->m_descriptorPools.end()) [[likely]]
                    m_impl->m_descriptorPools.erase(match);
            }
        }
    }
}

size_t VulkanDescriptorSetLayout::pools() const noexcept
{
    return m_impl->m_descriptorPools.size();
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 space, ShaderStage stages, UInt32 maxUnboundedArraySize) :
    DescriptorSetLayoutBuilder(parent, UniquePtr<VulkanDescriptorSetLayout>(new VulkanDescriptorSetLayout(parent.device())))
{
    m_state.maxUnboundedArraySize = maxUnboundedArraySize;
}

constexpr VulkanDescriptorSetLayoutBuilder::~VulkanDescriptorSetLayoutBuilder() noexcept = default;

void VulkanDescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorLayouts = std::move(m_state.descriptorLayouts);
    instance->m_impl->m_space = std::move(m_state.space);
    instance->m_impl->m_stages = std::move(m_state.stages);
    instance->m_impl->initialize(m_state.maxUnboundedArraySize);
}

constexpr UniquePtr<VulkanDescriptorLayout> VulkanDescriptorSetLayoutBuilder::makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors)
{
    return makeUnique<VulkanDescriptorLayout>(type, binding, descriptorSize, descriptors);
}

constexpr UniquePtr<VulkanDescriptorLayout> VulkanDescriptorSetLayoutBuilder::makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy)
{
    return makeUnique<VulkanDescriptorLayout>(makeUnique<VulkanSampler>(this->parent().device(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy), binding);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)