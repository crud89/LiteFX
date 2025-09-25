#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayoutImpl {
public:
    friend class DirectX12DescriptorSetLayoutBuilder;
    friend class DirectX12DescriptorSetLayout;

private:
    Array<DirectX12DescriptorLayout> m_layouts{};
    UInt32 m_space{ 0 }, m_samplers{ 0 }, m_descriptors{ 0 };
    ShaderStage m_stages{ ShaderStage::Other };
    Queue<ComPtr<ID3D12DescriptorHeap>> m_freeDescriptorSets{}, m_freeSamplerSets{};
    Dictionary<UInt32, UInt32> m_bindingToDescriptor{};
    WeakPtr<const DirectX12Device> m_device;
    bool m_isRuntimeArray = false;
    mutable std::mutex m_mutex;

public:
    DirectX12DescriptorSetLayoutImpl(const DirectX12Device& device, const Enumerable<DirectX12DescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) :
        m_space(space), m_stages(stages), m_device(device.weak_from_this())
    {
        m_layouts = descriptorLayouts | std::ranges::to<Array<DirectX12DescriptorLayout>>();
    }

    DirectX12DescriptorSetLayoutImpl(const DirectX12Device& device) :
        m_device(device.weak_from_this())
    {
    }

public:
    void initialize()
    {
        LITEFX_TRACE(DIRECTX12_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

        // Sort the layouts by binding.
        std::sort(std::begin(m_layouts), std::end(m_layouts), [](const auto& a, const auto& b) { return a.binding() < b.binding(); });

        // Count the samplers and descriptors.
        bool hasSamplers{ false }, hasResources{ false };
        std::ranges::for_each(m_layouts, [&, i = 0](const auto& layout) mutable {
#ifdef NDEBUG
            (void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
            LITEFX_TRACE(DIRECTX12_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Array size: {6}, Offset: {4}, Binding point: {5} }}...", ++i, m_layouts.size(), layout.descriptorType(), layout.elementSize(), 0, layout.binding(), layout.descriptors());
#endif
            // Check for invalid mixing of samplers and resources.
            if ((layout.descriptorType() == DescriptorType::Sampler && hasResources) || (layout.descriptorType() != DescriptorType::Sampler && hasSamplers)) [[unlikely]]
                throw InvalidArgumentException("descriptorLayouts", "Mixing samplers and resources in a single descriptor set is not supported. Samplers must be defined in a separate set.");

            if (layout.descriptorType() == DescriptorType::Sampler)
                hasSamplers = true;
            else
                hasResources = true;
            
            if (layout.unbounded())
            {
                if (m_layouts.size() != 1) [[unlikely]]
                    throw InvalidArgumentException("descriptorLayouts", "If an unbounded runtime array descriptor is used, it must be the only descriptor in the descriptor set, however the current descriptor set specifies {0} descriptors", m_layouts.size());
                else
                    m_isRuntimeArray = true;
            }
            
            if (layout.descriptorType() == DescriptorType::Sampler)
            {
                // Only count dynamic samplers.
                if (layout.staticSampler() == nullptr)
                {
                    m_bindingToDescriptor[layout.binding()] = m_samplers;
                    m_samplers += layout.descriptors();
                }
            }
            else
            {
                m_bindingToDescriptor[layout.binding()] = m_descriptors;
                m_descriptors += layout.descriptors();
            }
        });

        LITEFX_TRACE(DIRECTX12_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input Attachments: {6}, Texel Buffers: {7} }}...",
            m_space, m_layouts.size(), this->uniforms(), this->storages(), this->images(), this->samplers(), this->inputAttachments(), this->buffers());
    }

public:
    void tryAllocate(ComPtr<ID3D12DescriptorHeap>& localHeap, UInt32 descriptorCount)
    {
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            throw RuntimeException("Cannot allocate descriptor sets from a released device instance.");

        // Use descriptor heaps from the queues, if possible.
        if (m_descriptors > 0)
        {
            if (!m_freeDescriptorSets.empty() && !m_isRuntimeArray)
            {
                localHeap = m_freeDescriptorSets.front();
                m_freeDescriptorSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                    .NumDescriptors = m_isRuntimeArray ? descriptorCount : m_descriptors,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
                };

                raiseIfFailed(device->handle()->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&localHeap)), "Unable create constant CPU descriptor heap for constant buffers and images.");
            }
        }
        else if (m_samplers > 0)
        {
            if (!m_freeSamplerSets.empty() && !m_isRuntimeArray)
            {
                localHeap = m_freeSamplerSets.front();
                m_freeSamplerSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                    .NumDescriptors = m_isRuntimeArray ? descriptorCount : m_samplers,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
                };

                raiseIfFailed(device->handle()->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&localHeap)), "Unable create constant CPU descriptor heap for samplers.");
            }
        }
    }
    
    inline auto allocate(const DirectX12DescriptorSetLayout& layout, UInt32 descriptors)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // If no descriptor sets are free, or the descriptor set contains an unbounded descriptor array, allocate a new descriptor set.
        ComPtr<ID3D12DescriptorHeap> localHeap;
        this->tryAllocate(localHeap, descriptors);
        return makeUnique<DirectX12DescriptorSet>(layout, std::move(localHeap));
    }

    template <typename TDescriptorBindings>
    inline auto allocate(const DirectX12DescriptorSetLayout& layout, UInt32 descriptors, TDescriptorBindings bindings)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // If no descriptor sets are free, or the descriptor set contains an unbounded descriptor array, allocate a new descriptor set.
        ComPtr<ID3D12DescriptorHeap> localHeap;
        this->tryAllocate(localHeap, descriptors);
        auto descriptorSet = makeUnique<DirectX12DescriptorSet>(layout, std::move(localHeap));

        // Apply the default bindings.
        for (UInt32 i{ 0 }; auto binding : bindings)
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

        // Return the descriptor set.
        return descriptorSet;
    }

    inline UInt32 uniforms() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ConstantBuffer; }));
    }

    inline UInt32 storages() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::StructuredBuffer || layout.descriptorType() == DescriptorType::RWStructuredBuffer || layout.descriptorType() == DescriptorType::ByteAddressBuffer || layout.descriptorType() == DescriptorType::RWByteAddressBuffer; }));
    }

    inline UInt32 buffers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Buffer || layout.descriptorType() == DescriptorType::RWBuffer; }));
    }

    inline UInt32 images() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Texture || layout.descriptorType() == DescriptorType::RWTexture; }));
    }

    inline UInt32 samplers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Sampler && layout.staticSampler() == nullptr; }));
    }

    inline UInt32 staticSamplers() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::Sampler && layout.staticSampler() != nullptr; }));
    }

    inline UInt32 inputAttachments() const noexcept
    {
        return static_cast<UInt32>(std::ranges::count_if(m_layouts, [](const auto& layout) { return layout.descriptorType() == DescriptorType::InputAttachment; }));
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12Device& device, const Enumerable<DirectX12DescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) :
    m_impl(device, descriptorLayouts, space, stages)
{
    m_impl->initialize();
}

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12Device& device) :
    m_impl(device)
{
}

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12DescriptorSetLayout& other) :
    DescriptorSetLayout(other), m_impl(*other.device())
{
    m_impl->m_layouts = other.m_impl->m_layouts;
    m_impl->m_space = other.space();
    m_impl->m_stages = other.shaderStages();
    m_impl->initialize();
}

DirectX12DescriptorSetLayout::~DirectX12DescriptorSetLayout() noexcept = default;

SharedPtr<const DirectX12Device> DirectX12DescriptorSetLayout::device() const noexcept
{
    return m_impl->m_device.lock();
}

const Array<DirectX12DescriptorLayout>& DirectX12DescriptorSetLayout::descriptors() const noexcept
{
    return m_impl->m_layouts;
}

const DirectX12DescriptorLayout& DirectX12DescriptorSetLayout::descriptor(UInt32 binding) const
{
    if (auto match = std::ranges::find_if(m_impl->m_layouts, [&binding](const auto& layout) { return layout.binding() == binding; }); match != m_impl->m_layouts.end()) [[likely]]
        return *match;

    throw InvalidArgumentException("binding", "No layout has been provided for the binding {0}.", binding);
}

UInt32 DirectX12DescriptorSetLayout::space() const noexcept
{
    return m_impl->m_space;
}

ShaderStage DirectX12DescriptorSetLayout::shaderStages() const noexcept
{
    return m_impl->m_stages;
}

UInt32 DirectX12DescriptorSetLayout::uniforms() const noexcept
{
    return m_impl->uniforms();
}

UInt32 DirectX12DescriptorSetLayout::storages() const noexcept
{
    return m_impl->storages();
}

UInt32 DirectX12DescriptorSetLayout::buffers() const noexcept
{
    return m_impl->buffers();
}

UInt32 DirectX12DescriptorSetLayout::images() const noexcept
{
    return m_impl->images();
}

UInt32 DirectX12DescriptorSetLayout::samplers() const noexcept
{
    return m_impl->samplers();
}

UInt32 DirectX12DescriptorSetLayout::staticSamplers() const noexcept
{
    return m_impl->staticSamplers();
}

UInt32 DirectX12DescriptorSetLayout::inputAttachments() const noexcept
{
    return m_impl->inputAttachments();
}

bool DirectX12DescriptorSetLayout::containsUnboundedArray() const noexcept
{
    return m_impl->m_isRuntimeArray;
}

UInt32 DirectX12DescriptorSetLayout::getDescriptorOffset(UInt32 binding, UInt32 element) const
{
    if (!m_impl->m_bindingToDescriptor.contains(binding)) [[unlikely]]
        throw InvalidArgumentException("binding", "The descriptor set does not contain a descriptor at binding {0}.", binding);

    return m_impl->m_bindingToDescriptor[binding] + element;
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const
{
    return m_impl->allocate(*this, descriptors, bindings);
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const
{
    return m_impl->allocate(*this, descriptors, bindings);
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const
{
    return m_impl->allocate(*this, descriptors, std::move(bindings));
}

Generator<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindingsPerSet) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // First, allocate each descriptor set that a binding is provided for.
    for (auto& bindings : bindingsPerSet | std::views::take(descriptorSets))
        co_yield m_impl->allocate(*self, descriptors, bindings);

    // If there are more descriptor sets requested than bindings are provided, continue with default bindings.
    for (auto i = bindingsPerSet.size(); i < descriptorSets; ++i)
        co_yield m_impl->allocate(*self, descriptors);
}

#ifdef __cpp_lib_mdspan
Generator<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // Depending on the set index, allocate with default bindings, if they are provided.
    // TODO: With C++26 we can use submdspan here. The workaround works, as `layout_right` of the mdspan.
    for (size_t i{ 0 }; i < static_cast<size_t>(descriptorSets); ++i)
        co_yield i < bindings.extent(0) ?
            m_impl->allocate(*self, descriptors, Span<DescriptorBinding>{ bindings.data_handle() + i * sizeof(DescriptorBinding), bindings.extent(1) * sizeof(DescriptorBinding) }) : // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic));
            m_impl->allocate(*self, descriptors);
}
#endif

Generator<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const
{
    // Get a shared pointer to the current instance to keep it alive as long as the coroutine lives.
    auto self = this->shared_from_this();

    // Straight up allocate a descriptor set with the bindings provided from the factory.
    // TODO: With C++26 we can use submdspan here. The workaround works, as `layout_right` of the mdspan.
    for (UInt32 i{ 0 }; i < descriptorSets; ++i)
        co_yield m_impl->allocate(*self, descriptors, bindingFactory(i)); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic));
}

void DirectX12DescriptorSetLayout::free(const DirectX12DescriptorSet& descriptorSet) const
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // Unbounded array descriptor sets aren't cached.
    if (!m_impl->m_isRuntimeArray)
    {
        if (m_impl->m_descriptors > 0)
            m_impl->m_freeDescriptorSets.emplace(descriptorSet.localHeap());
        else if (m_impl->m_samplers > 0)
            m_impl->m_freeSamplerSets.emplace(descriptorSet.localHeap());
    }
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayoutBuilder::DirectX12DescriptorSetLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, UInt32 space, ShaderStage stages) :
    DescriptorSetLayoutBuilder(parent, SharedPtr<DirectX12DescriptorSetLayout>(new DirectX12DescriptorSetLayout(*parent.device())))
{
    this->state().space = space;
    this->state().stages = stages;
}

DirectX12DescriptorSetLayoutBuilder::~DirectX12DescriptorSetLayoutBuilder() noexcept = default;

void DirectX12DescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_layouts = std::move(this->state().descriptorLayouts);
    instance->m_impl->m_space = this->state().space;
    instance->m_impl->m_stages = this->state().stages;
    instance->m_impl->initialize();
}

DirectX12DescriptorLayout DirectX12DescriptorSetLayoutBuilder::makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors, bool unbounded)
{
    return { type, binding, descriptorSize, descriptors, unbounded };
}

DirectX12DescriptorLayout DirectX12DescriptorSetLayoutBuilder::makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy)
{
    // TODO: This could be made more efficient if we provide a constructor that takes an rvalue shared-pointer sampler instead.
    auto sampler = DirectX12Sampler::allocate(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
    return { *sampler, binding };
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)