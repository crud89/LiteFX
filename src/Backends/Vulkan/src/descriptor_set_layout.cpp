#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffset;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutImpl {
public:
    friend class VulkanDescriptorSetLayoutBuilder;
    friend class VulkanDescriptorSetLayout;

private:
    Array<VulkanDescriptorLayout> m_descriptorLayouts;
    Queue<Array<Byte>> m_freeDescriptorSets;
    ShaderStage m_stages{};
    UInt32 m_space{}, m_maxUnboundedArraySize{};
    mutable std::mutex m_mutex;
    SharedPtr<const VulkanDevice> m_device;
    Optional<VkDescriptorType> m_unboundedDescriptorType{ std::nullopt };

public:
    VulkanDescriptorSetLayoutImpl(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) :
        m_stages(stages), m_space(space), m_device(device.shared_from_this())
    {
        m_descriptorLayouts = descriptorLayouts | std::ranges::to<Array<VulkanDescriptorLayout>>();
    }

    VulkanDescriptorSetLayoutImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this())
    {
    }

private:
    inline bool usesDescriptorIndexing() const noexcept {
        return m_unboundedDescriptorType.has_value();
    }

public:
    VkDescriptorSetLayout initialize()
    {
        LITEFX_TRACE(VULKAN_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

        // Sort the layouts by binding.
        std::sort(std::begin(m_descriptorLayouts), std::end(m_descriptorLayouts), [](const auto& a, const auto& b) { return a.binding() < b.binding(); });

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
        
        // Track maximum number of descriptors in unbounded arrays.
        // NOTE: We do this to make an educated guess on the maximum amount of descriptors that can be bound. However, we rely on the driver to not allocate memory for all the 
        //       descriptors. In fact, when using descriptor buffers, we are free to allocate less memory for descriptors during descriptor set allocation. However, this still
        //       may result in loads of space that is not actually used, so the general rule of thumb is to use only few descriptor sets with unbounded arrays.
        auto maxUniformBuffers = m_device->adapter().limits().maxDescriptorSetUniformBuffers;
        auto maxStorageBuffers = m_device->adapter().limits().maxDescriptorSetStorageBuffers;
        auto maxStorageImages  = m_device->adapter().limits().maxDescriptorSetStorageImages;
        auto maxSampledImages  = m_device->adapter().limits().maxDescriptorSetSampledImages;
        auto maxSamplers       = m_device->adapter().limits().maxDescriptorSetSamplers;
        auto maxAttachments    = m_device->adapter().limits().maxDescriptorSetInputAttachments;

        // Parse descriptor set layouts.
        Array<VkDescriptorSetLayoutBinding> bindings;
        Array<VkDescriptorBindingFlags> bindingFlags;
        Array<VkDescriptorSetLayoutBindingFlagsCreateInfo> bindingFlagCreateInfo;
        Array<VkMutableDescriptorTypeListEXT> mutableDescriptorTypeLists;
        bool hasStaticSampler{ false }, usesDescriptorHeap{ false };
        UInt32 unboundedDescriptorIndex{ }, unboundedDescriptorCount{ };

        static constexpr auto SUPPORTED_MUTABLE_DESCRIPTOR_TYPES = std::array {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        };

        std::ranges::for_each(m_descriptorLayouts, [&, i = 0](const auto& layout) mutable {
            auto bindingPoint = layout.binding();
            auto type = layout.descriptorType();

#ifdef NDEBUG
            (void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
            LITEFX_TRACE(VULKAN_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Array size: {6}, Offset: {4}, Binding point: {5} }}...",
                ++i, m_descriptorLayouts.size(), type, layout.elementSize(), 0, bindingPoint, layout.descriptors());
#endif

            // Unbounded arrays are only allowed for the last descriptor in the descriptor set (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html#_description).
            if (this->usesDescriptorIndexing()) [[unlikely]]
                throw InvalidArgumentException("descriptorLayouts", "If an unbounded runtime array descriptor is used, it must be the last descriptor in the descriptor set.");

            VkDescriptorSetLayoutBinding binding = {
                .binding = bindingPoint,
                .descriptorCount = layout.descriptors(),
                .stageFlags = shaderStages
            };

            if (type == DescriptorType::InputAttachment && m_stages != ShaderStage::Fragment)
                throw RuntimeException("Unable to bind input attachment at {0} to a descriptor set that is accessible from other stages, than the fragment shader.", bindingPoint);

            // Map descriptor type to Vulkan type set.
            switch (type)
            {
            case DescriptorType::ConstantBuffer:         binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;       break;
            case DescriptorType::ByteAddressBuffer:
            case DescriptorType::RWByteAddressBuffer:
            case DescriptorType::StructuredBuffer:
            case DescriptorType::RWStructuredBuffer:     binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;       break;
            case DescriptorType::RWTexture:              binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;        break;
            case DescriptorType::Texture:                binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;        break;
            case DescriptorType::RWBuffer:               binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
            case DescriptorType::Buffer:                 binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
            case DescriptorType::InputAttachment:        binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;     break;
            case DescriptorType::SamplerDescriptorHeap:
            case DescriptorType::Sampler:                binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;              break;
            case DescriptorType::AccelerationStructure:  binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR; break;
            case DescriptorType::ResourceDescriptorHeap: binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;          break;
            default: LITEFX_WARNING(VULKAN_LOG, "The descriptor type is unsupported. Binding will be skipped.");          return;
            }

            // Remember the use of mutable descriptor types for later.
            if (binding.descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT)
                mutableDescriptorTypeLists.emplace_back(0u, nullptr);
            else
            {
                mutableDescriptorTypeLists.emplace_back(static_cast<UInt32>(SUPPORTED_MUTABLE_DESCRIPTOR_TYPES.size()), SUPPORTED_MUTABLE_DESCRIPTOR_TYPES.data());
                usesDescriptorHeap = true;
            }
            
            // If the descriptor is an unbounded runtime array, disable validation warnings about partially bound elements.
            if (layout.unbounded())
            {
                bindingFlags.emplace_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);
                m_unboundedDescriptorType = binding.descriptorType;
                
                // Store the preferred descriptor count, the binding array index and set the descriptor count to 0. We will query the maximum supported descriptor count right before creating
                // the layout handle and overwrite the descriptor count, if required. Note that this does not necessarily validate all required limits. The effective number of bound descriptors
                // can be further influenced by the per stage resource bindings limit.
                switch (binding.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxStorageBuffers);
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxUniformBuffers);
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxStorageImages);
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxSampledImages);
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxAttachments);
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    unboundedDescriptorCount = std::min(binding.descriptorCount, maxSamplers);
                    break;
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    
                    break;
                default:
                    break;
                }

                unboundedDescriptorIndex = static_cast<UInt32>(bindings.size());

                // Set the descriptor count for the binding to 0 for now and patch it later.
                // For descriptor heaps, we straight up pass the heap size to the descriptor count property. As it is required to provide it anyway, any validation errors from this
                // can easily be resolved by reducing the heap sizes. Note that we shouldn't actually reach here anyway, but this might change in the future.
                if (binding.descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT)
                    binding.descriptorCount = 0;
                else
                    binding.descriptorCount = layout.descriptors();
            }
            else
            {
                // TODO: Do we need to support VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT for static arrays?
                if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT)
                    bindingFlags.emplace_back(VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
                else
                    bindingFlags.emplace_back(VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);

                if (type == DescriptorType::Sampler)
                {
                    if (layout.staticSampler() == nullptr)
                        maxSamplers -= binding.descriptorCount;
                    else // Static samplers don't count towards the limit.
                    {
                        binding.pImmutableSamplers = &layout.staticSampler()->handle();
                        hasStaticSampler = true;
                    }
                }

                // Track remaining descriptors towards limit.
                switch (binding.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    maxStorageBuffers -= binding.descriptorCount;
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    maxUniformBuffers -= binding.descriptorCount;
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    maxStorageImages -= binding.descriptorCount;
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    maxSampledImages -= binding.descriptorCount;
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    maxAttachments -= binding.descriptorCount;
                    break;
                default:
                    break;
                }
            }

            bindings.push_back(binding);
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input Attachments: {6}, Texel Buffers: {7} }}...", 
            m_space, m_descriptorLayouts.size(), this->uniforms(), this->storages(), this->images(), this->samplers(), this->inputAttachments(), this->buffers());

        // Create the descriptor set layout.
        VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorTypeInfo = {
            .sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
            .mutableDescriptorTypeListCount = static_cast<UInt32>(mutableDescriptorTypeLists.size()),
            .pMutableDescriptorTypeLists = mutableDescriptorTypeLists.data()
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = usesDescriptorHeap ? &mutableDescriptorTypeInfo : nullptr,
            .bindingCount = static_cast<UInt32>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data()
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &extendedInfo,
            .bindingCount = static_cast<UInt32>(bindings.size()),
            .pBindings = bindings.data()
        };

        // Allow for descriptors to update after they have been bound. This also means, we have to manually take care of not to update a descriptor before it got used.
        descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

        if (hasStaticSampler)
            descriptorSetLayoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;

        // Query support before creating the descriptor set to store the maximum supported unbounded array size for this descriptor set.
        if (this->usesDescriptorIndexing())
        {
            VkDescriptorSetVariableDescriptorCountLayoutSupport descriptorCountSupportInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT };
            VkDescriptorSetLayoutSupport descriptorSetLayoutSupportInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT, .pNext = &descriptorCountSupportInfo };
            ::vkGetDescriptorSetLayoutSupport(m_device->handle(), &descriptorSetLayoutInfo, &descriptorSetLayoutSupportInfo);
            m_maxUnboundedArraySize = descriptorCountSupportInfo.maxVariableDescriptorCount;

            // Reset the unbounded array descriptor count.
            bindings[unboundedDescriptorIndex].descriptorCount = std::min(m_maxUnboundedArraySize, unboundedDescriptorCount);
        }

        VkDescriptorSetLayout layout{};
        raiseIfFailed(::vkCreateDescriptorSetLayout(m_device->handle(), &descriptorSetLayoutInfo, nullptr, &layout), "Unable to create descriptor set layout.");

        return layout;
    }

public:
    template <typename TDescriptorBindings>
    inline auto allocate(SharedPtr<const VulkanDescriptorSetLayout> layout, UInt32 descriptors, TDescriptorBindings bindings) // NOLINT(performance-unnecessary-value-param)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // If no descriptor sets are free, or the descriptor set contains an unbounded descriptor array, allocate a new descriptor set.
        UniquePtr<VulkanDescriptorSet> descriptorSet;

        if (this->usesDescriptorIndexing() || m_freeDescriptorSets.empty())
            descriptorSet = makeUnique<VulkanDescriptorSet>(*layout, descriptors);
        else
        {
            // Otherwise, pick and remove one from the list.
            descriptorSet = UniquePtr<VulkanDescriptorSet>(new VulkanDescriptorSet(*layout, std::move(m_freeDescriptorSets.front())));
            m_freeDescriptorSets.pop();
        }

        // Apply the default bindings.
        for (UInt32 i{ 0 }; auto binding : bindings)
        {
            std::visit(type_switch{
                [](const std::monostate&) {}, // Default: don't bind anything.
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

    inline Generator<UniquePtr<VulkanDescriptorSet>> allocate(SharedPtr<const VulkanDescriptorSetLayout> layout, UInt32 descriptorSets, UInt32 unboundedDescriptorArraySize)
    {
        Array<UniquePtr<VulkanDescriptorSet>> handles;
        handles.reserve(descriptorSets);
        auto& impl = layout->m_impl;

        {
            std::lock_guard<std::mutex> lock(impl->m_mutex);

            // Descriptor sets that use unbounded runtime arrays aren't cached.
            if (this->usesDescriptorIndexing() || impl->m_freeDescriptorSets.empty())
            {
                handles.resize(descriptorSets);
                std::ranges::generate(handles, [&]() { return makeUnique<VulkanDescriptorSet>(*layout, unboundedDescriptorArraySize); });
            }
            else
            {
                // Pop cached descriptor sets.
                while (!impl->m_freeDescriptorSets.empty() && descriptorSets --> 0) // Finally a good use for the "-->" operator!!!
                {
                    handles.emplace_back(UniquePtr<VulkanDescriptorSet>(new VulkanDescriptorSet(*layout, std::move(impl->m_freeDescriptorSets.front()))));
                    impl->m_freeDescriptorSets.pop();
                }

                // Allocate the rest from a new descriptor pool and return them.
                for (UInt32 i{ 0 }; i < descriptorSets; ++i)
                    handles.emplace_back(makeUnique<VulkanDescriptorSet>(*layout, unboundedDescriptorArraySize));
            }
        }

        co_yield std::ranges::elements_of(handles | std::views::as_rvalue);
    }

    inline UInt32 uniforms() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ConstantBuffer; }));
    }

    inline UInt32 storages() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::StructuredBuffer || layout.descriptorType() == DescriptorType::RWStructuredBuffer || layout.descriptorType() == DescriptorType::ByteAddressBuffer || layout.descriptorType() == DescriptorType::RWByteAddressBuffer; }));
    }

    inline UInt32 buffers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Buffer || layout.descriptorType() == DescriptorType::RWBuffer; }));
    }

    inline UInt32 images() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Texture || layout.descriptorType() == DescriptorType::RWTexture; }));
    }

    inline UInt32 samplers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Sampler && layout.staticSampler() == nullptr; }));
    }

    inline UInt32 staticSamplers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Sampler && layout.staticSampler() != nullptr; }));
    }

    inline UInt32 inputAttachments() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::InputAttachment; }));
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) :
    Resource<VkDescriptorSetLayout>(VK_NULL_HANDLE), m_impl(device, descriptorLayouts, space, stages)
{
    this->handle() = m_impl->initialize();
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device) :
    Resource<VkDescriptorSetLayout>(VK_NULL_HANDLE), m_impl(device)
{
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) :
    DescriptorSetLayout(other), Resource<VkDescriptorSetLayout>(VK_NULL_HANDLE), m_impl(other.device())
{
    m_impl->m_descriptorLayouts = other.m_impl->m_descriptorLayouts;
    m_impl->m_space = other.space();
    m_impl->m_stages = other.shaderStages();
    this->handle() = m_impl->initialize();
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept
{
    ::vkDestroyDescriptorSetLayout(m_impl->m_device->handle(), this->handle(), nullptr);
}

const VulkanDevice& VulkanDescriptorSetLayout::device() const noexcept
{
    return *m_impl->m_device;
}

UInt32 VulkanDescriptorSetLayout::maxUnboundedArraySize() const noexcept
{
    return m_impl->m_maxUnboundedArraySize;
}

const Array<VulkanDescriptorLayout>& VulkanDescriptorSetLayout::descriptors() const noexcept
{
    return m_impl->m_descriptorLayouts;
}

const VulkanDescriptorLayout& VulkanDescriptorSetLayout::descriptor(UInt32 binding) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorLayouts, [&binding](const auto& layout) { return layout.binding() == binding; }); match != m_impl->m_descriptorLayouts.end()) [[likely]]
        return *match;

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
    return m_impl->uniforms();
}

UInt32 VulkanDescriptorSetLayout::storages() const noexcept
{
    return m_impl->storages();
}

UInt32 VulkanDescriptorSetLayout::buffers() const noexcept
{
    return m_impl->buffers();
}

UInt32 VulkanDescriptorSetLayout::images() const noexcept
{
    return m_impl->images();
}

UInt32 VulkanDescriptorSetLayout::samplers() const noexcept
{
    return m_impl->samplers();
}

UInt32 VulkanDescriptorSetLayout::staticSamplers() const noexcept
{
    return m_impl->staticSamplers();
}

UInt32 VulkanDescriptorSetLayout::inputAttachments() const noexcept
{
    return m_impl->inputAttachments();
}

bool VulkanDescriptorSetLayout::containsUnboundedArray() const noexcept
{
    return m_impl->usesDescriptorIndexing();
}

UInt32 VulkanDescriptorSetLayout::getDescriptorOffset(UInt32 binding, UInt32 element) const
{
    if (auto descriptorLayout = std::ranges::find_if(m_impl->m_descriptorLayouts, [&binding](auto& layout) { return layout.binding() == binding; }); descriptorLayout != m_impl->m_descriptorLayouts.end())
    {
        VkDeviceSize descriptorOffset{};
        vkGetDescriptorSetLayoutBindingOffset(this->device().handle(), this->handle(), binding, &descriptorOffset);
        descriptorOffset += static_cast<VkDeviceSize>(this->device().descriptorSize(descriptorLayout->descriptorType()) * element);

        return static_cast<UInt32>(descriptorOffset);
    }
    else [[unlikely]]
    {
        throw ArgumentOutOfRangeException("binding", "The descriptor layout does not contain a descriptor bound at {0}.", binding);
    }
}

bool VulkanDescriptorSetLayout::bindsResources() const noexcept
{
    return std::ranges::any_of(m_impl->m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() != DescriptorType::Sampler; });
}

bool VulkanDescriptorSetLayout::bindsSamplers() const noexcept
{
    return std::ranges::any_of(m_impl->m_descriptorLayouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Sampler && layout.staticSampler() == nullptr; });
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const
{
    return m_impl->allocate(this->shared_from_this(), descriptors, bindings);
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const
{
    return m_impl->allocate(this->shared_from_this(), descriptors, bindings);
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const
{
    return m_impl->allocate(this->shared_from_this(), descriptors, std::move(bindings));
}

Generator<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindingsPerSet) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // Create the descriptor set handles and assign each of them their default bindings.
    auto handles = m_impl->allocate(self, descriptorSets, descriptors);
    auto bindings = bindingsPerSet.begin();

    // Iterate the handles and apply the bindings.
    for (auto descriptorSet : handles)
    {
        // Only start binding, if there are any more bindings provided.
        if (bindings != bindingsPerSet.end())
        {
            for (UInt32 i{ 0 }; auto & binding : *bindings)
            {
                std::visit(type_switch{
                    [](const std::monostate&) {}, // Default: don't bind anything.
                    [&](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
                    [&](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
                    [&](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
                    [&](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
                }, binding.resource);

                ++i;
            }

            // Advance to next provided binding set.
            bindings++;
        }

        co_yield std::move(descriptorSet);
    }
}

#ifdef __cpp_lib_mdspan
Generator<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // Make a generator that returns the descriptor set handles.
    auto handles = m_impl->allocate(self, descriptorSets, descriptors);

    // Iterate the handles and bind them.
    for (UInt32 offset{ 0 }; auto descriptorSet : handles)
    {
        // TODO: With C++26 we can use submdspan here. The workaround works, as `layout_right` of the mdspan.
        for (UInt32 i{ 0 }; auto& binding : Span<DescriptorBinding>{ bindings.data_handle() + offset++ * sizeof(DescriptorBinding), bindings.extent(1) * sizeof(DescriptorBinding) }) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        {
            std::visit(type_switch{
                [](const std::monostate&) {}, // Default: don't bind anything.
                [&](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
                [&](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
                [&](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
                [&](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
            }, binding.resource);

            ++i;
        }

        co_yield std::move(descriptorSet);
    }
}
#endif

Generator<UniquePtr<VulkanDescriptorSet>> VulkanDescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // Make a generator that returns the descriptor set handles.
    auto handles = m_impl->allocate(self, descriptorSets, descriptors);

    // Iterate the handles and bind them.
    for (UInt32 setId{ 0 }; auto descriptorSet : handles)
    {
        auto bindingsGenerator = bindingFactory(setId++);

        for (UInt32 i{ 0 }; auto binding : bindingsGenerator)
        {
            std::visit(type_switch{
                [](const std::monostate&) {}, // Default: don't bind anything.
                [&](const ISampler& sampler) { descriptorSet->update(binding.binding.value_or(i), sampler, binding.firstDescriptor); },
                [&](const IBuffer& buffer) { descriptorSet->update(binding.binding.value_or(i), buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
                [&](const IImage& image) { descriptorSet->update(binding.binding.value_or(i), image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
                [&](const IAccelerationStructure& accelerationStructure) { descriptorSet->update(binding.binding.value_or(i), accelerationStructure, binding.firstDescriptor); }
            }, binding.resource);

            ++i;
        }

        co_yield std::move(descriptorSet);
    }
}

void VulkanDescriptorSetLayout::free(const VulkanDescriptorSet& descriptorSet) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // Cache the descriptor set backing buffer for later used (except if the set uses runtime arrays, which we don't cache).
    if (!m_impl->usesDescriptorIndexing())
        m_impl->m_freeDescriptorSets.push(std::move(descriptorSet.releaseBuffer()));
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilder(VulkanPipelineLayoutBuilder& parent, UInt32 space, ShaderStage stages) :
    DescriptorSetLayoutBuilder(parent, SharedPtr<VulkanDescriptorSetLayout>(new VulkanDescriptorSetLayout(parent.instance()->device())))
{
    this->state().space = space;
    this->state().stages = stages;
}

VulkanDescriptorSetLayoutBuilder::~VulkanDescriptorSetLayoutBuilder() noexcept = default;

void VulkanDescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorLayouts = std::move(this->state().descriptorLayouts);
    instance->m_impl->m_space = this->state().space;
    instance->m_impl->m_stages = this->state().stages;
    instance->handle() = instance->m_impl->initialize();
}

VulkanDescriptorLayout VulkanDescriptorSetLayoutBuilder::makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors, bool unbounded)
{
    return { type, binding, descriptorSize, descriptors, unbounded };
}

VulkanDescriptorLayout VulkanDescriptorSetLayoutBuilder::makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy)
{
    // TODO: This could be made more efficient if we provide a constructor that takes an rvalue shared-pointer sampler instead.
    auto sampler = VulkanSampler::allocate(this->parent().instance()->device(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
    return { *sampler, binding };
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)