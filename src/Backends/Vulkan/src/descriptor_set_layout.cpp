#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutImpl {
public:
    friend class VulkanDescriptorSetLayoutBuilder;
    friend class VulkanDescriptorSetLayout;

private:
    Array<VulkanDescriptorLayout> m_descriptorLayouts;
    UniquePtr<DescriptorAllocationLayout> m_allocationLayout;
    Queue<Tuple<Array<Byte>, Array<Byte>>> m_freeDescriptorSets; // [ Resource, Sampler ]
    ShaderStage m_stages{ ShaderStage::Other };
    UInt32 m_space{}, m_maxUnboundedArraySize{};
    mutable std::mutex m_mutex;
    SharedPtr<const VulkanDevice> m_device;

public:
    VulkanDescriptorSetLayoutImpl(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) :
        m_stages(stages), m_space(space), m_device(device.shared_from_this())
    {
        m_descriptorLayouts = descriptorLayouts | std::ranges::to<Array<VulkanDescriptorLayout>>();
        m_allocationLayout = makeUnique<DescriptorAllocationLayout>(m_descriptorLayouts);
    }

    VulkanDescriptorSetLayoutImpl(const VulkanDevice& device) :
        m_device(device.shared_from_this())
    {
    }

public:
    void initialize()
    {
        LITEFX_TRACE(VULKAN_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

        // Sort the layouts by binding.
        std::sort(std::begin(m_descriptorLayouts), std::end(m_descriptorLayouts), [](const auto& a, const auto& b) { return a.binding() < b.binding(); });

        std::ranges::for_each(m_descriptorLayouts, [&, i = 0](const auto& layout) mutable {
#ifdef NDEBUG
            (void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
            LITEFX_TRACE(VULKAN_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Array size: {6}, Offset: {4}, Binding point: {5} }}...",
                ++i, m_descriptorLayouts.size(), layout.descriptorType(), layout.elementSize(),
                m_allocationLayout->range(layout.binding()).transform([](const DescriptorAllocationRange& range) { return range.RelativeOffset; }).value_or(0u),
                layout.binding(), layout.descriptors());
#endif

            if (layout.descriptorType() == DescriptorType::InputAttachment && m_stages != ShaderStage::Fragment) [[unlikely]]
                throw RuntimeException("Unable to bind input attachment at {0} to a descriptor set that is accessible from other stages, than the fragment shader.", layout.binding());
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input Attachments: {6}, Texel Buffers: {7} }}...",
            m_space, m_descriptorLayouts.size(), this->uniforms(), this->storages(), this->images(), this->samplers(), this->inputAttachments(), this->buffers());

        // The maximum unbounded array size is limited by the space left in its heap behind the preceding descriptors of this set.
        if (auto unboundedRange = m_allocationLayout->unboundedArrayRange(); unboundedRange.has_value())
        {
            auto capacity = m_device->descriptorHeaps().capacity(unboundedRange->Heap);
            m_maxUnboundedArraySize = capacity > unboundedRange->RelativeOffset ? capacity - unboundedRange->RelativeOffset : 0u;
        }
    }

public:
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
    m_impl(device, descriptorLayouts, space, stages)
{
    m_impl->initialize();
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device) :
    m_impl(device)
{
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) :
    DescriptorSetLayout(other), m_impl(other.device())
{
    m_impl->m_descriptorLayouts = other.m_impl->m_descriptorLayouts;
    m_impl->m_allocationLayout = makeUnique<DescriptorAllocationLayout>(m_impl->m_descriptorLayouts);
    m_impl->m_space = other.space();
    m_impl->m_stages = other.shaderStages();
    m_impl->initialize();
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

const DescriptorAllocationLayout& VulkanDescriptorSetLayout::allocationLayout() const noexcept
{
    return *m_impl->m_allocationLayout;
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
    return m_impl->m_allocationLayout->unboundedArrayRange().has_value();
}

UInt32 VulkanDescriptorSetLayout::getDescriptorOffset(UInt32 binding, UInt32 element) const
{
    auto allocationRange = m_impl->m_allocationLayout->range(binding);

    if (!allocationRange.has_value())
        throw InvalidArgumentException("binding", "The descriptor set does not contain a descriptor at binding {0}.", binding);
    else
       return allocationRange->RelativeOffset + element;
}

bool VulkanDescriptorSetLayout::bindsResources() const noexcept
{
    return m_impl->m_allocationLayout->binds(DescriptorHeapType::Resource);
}

bool VulkanDescriptorSetLayout::bindsSamplers() const noexcept
{
    return m_impl->m_allocationLayout->binds(DescriptorHeapType::Sampler);
}

UniquePtr<VulkanDescriptorSet> VulkanDescriptorSetLayout::allocateDescriptorSet(UInt32 unboundedArraySize) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // Descriptor sets with unbounded arrays aren't cached.
    if (this->containsUnboundedArray() || m_impl->m_freeDescriptorSets.empty())
        return makeUnique<VulkanDescriptorSet>(*this, unboundedArraySize);

    auto& [resourceHeap, samplerHeap] = m_impl->m_freeDescriptorSets.front();
    auto descriptorSet = UniquePtr<VulkanDescriptorSet>(new VulkanDescriptorSet(*this, std::move(resourceHeap), std::move(samplerHeap)));
    m_impl->m_freeDescriptorSets.pop();

    return descriptorSet;
}

void VulkanDescriptorSetLayout::free(const VulkanDescriptorSet& descriptorSet) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // Cache the descriptor set backing buffer for later used (except if the set uses runtime arrays, which we don't cache).
    if (!this->containsUnboundedArray())
        m_impl->m_freeDescriptorSets.emplace(descriptorSet.releaseBuffer(DescriptorHeapType::Resource), descriptorSet.releaseBuffer(DescriptorHeapType::Sampler));
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
    instance->m_impl->m_allocationLayout = makeUnique<DescriptorAllocationLayout>(instance->m_impl->m_descriptorLayouts);
    instance->m_impl->m_space = this->state().space;
    instance->m_impl->m_stages = this->state().stages;
    instance->m_impl->initialize();
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