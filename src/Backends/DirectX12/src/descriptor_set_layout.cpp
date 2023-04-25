#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>
#include "image.h"
#include <experimental/generator>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayoutImpl : public Implement<DirectX12DescriptorSetLayout> {
public:
    friend class DirectX12DescriptorSetLayoutBuilder;
    friend class DirectX12DescriptorSetLayout;

private:
    Array<UniquePtr<DirectX12DescriptorLayout>> m_layouts;
    UInt32 m_space, m_samplers{ 0 }, m_descriptors{ 0 }, m_rootParameterIndex{ 0 };
    ShaderStage m_stages;
    Queue<ComPtr<ID3D12DescriptorHeap>> m_freeDescriptorSets, m_freeSamplerSets;
    Dictionary<UInt32, UInt32> m_bindingToDescriptor;
    const DirectX12Device& m_device;
    bool m_isRuntimeArray = false;
    mutable std::mutex m_mutex;

public:
    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent, const DirectX12Device& device, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages) :
        base(parent), m_device(device), m_layouts(std::move(descriptorLayouts)), m_space(space), m_stages(stages)
    {
    }

    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }

public:
    void initialize()
    {
        LITEFX_TRACE(DIRECTX12_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

        // Sort the layouts by binding.
        std::sort(std::begin(m_layouts), std::end(m_layouts), [](const UniquePtr<DirectX12DescriptorLayout>& a, const UniquePtr<DirectX12DescriptorLayout>& b) { return a->binding() < b->binding(); });

        // Count the samplers and descriptors.
        std::ranges::for_each(m_layouts, [&, i = 0](const UniquePtr<DirectX12DescriptorLayout>& layout) mutable {
            LITEFX_TRACE(DIRECTX12_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Array size: {6}, Offset: {4}, Binding point: {5} }}...", ++i, m_layouts.size(), layout->descriptorType(), layout->elementSize(), 0, layout->binding(), layout->descriptors());
            
            if (layout->descriptors() == -1)
            {
                if (m_layouts.size() != 1) [[unlikely]]
                    throw InvalidArgumentException("If an unbounded runtime array descriptor is used, it must be the only descriptor in the descriptor set, however the current descriptor set specifies {0} descriptors", m_layouts.size());
                else
                    m_isRuntimeArray = true;
            }
            
            if (layout->descriptorType() == DescriptorType::Sampler)
            {
                // Only count dynamic samplers.
                if (layout->staticSampler() == nullptr)
                {
                    m_bindingToDescriptor[layout->binding()] = m_samplers;
                    m_samplers += layout->descriptors();
                }
            }
            else
            {
                m_bindingToDescriptor[layout->binding()] = m_descriptors;
                m_descriptors += layout->descriptors();
            }
        });
    }

public:
    void tryAllocate(ComPtr<ID3D12DescriptorHeap>& bufferHeap, ComPtr<ID3D12DescriptorHeap>& samplerHeap, const UInt32& descriptorCount)
    {
        // Use descriptor heaps from the queues, if possible.
        if (m_descriptors > 0)
        {
            // If the descriptor set has an unbounded array, use the descriptor count from the parameter to allocate it.
            UInt32 descriptors = m_descriptors == -1 ? descriptorCount : m_descriptors;

            if (!m_freeDescriptorSets.empty())
            {
                bufferHeap = m_freeDescriptorSets.front();
                m_freeDescriptorSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                    .NumDescriptors = descriptors,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
                };

                raiseIfFailed<RuntimeException>(m_device.handle()->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&bufferHeap)), "Unable create constant CPU descriptor heap for constant buffers and images.");
            }
        }

        // Repeat for sampler heaps.
        if (m_samplers > 0)
        {
            // If the descriptor set has an unbounded array, use the descriptor count from the parameter to allocate it.
            UInt32 samplers = m_samplers == -1 ? descriptorCount : m_samplers;

            if (!m_freeSamplerSets.empty())
            {
                bufferHeap = m_freeSamplerSets.front();
                m_freeSamplerSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                    .NumDescriptors = samplers,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
                };

                raiseIfFailed<RuntimeException>(m_device.handle()->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap)), "Unable create constant CPU descriptor heap for samplers.");
            }
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this, device, std::move(descriptorLayouts), space, stages))
{
    m_impl->initialize();
}

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12Device& device) noexcept :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this, device))
{
}

DirectX12DescriptorSetLayout::~DirectX12DescriptorSetLayout() noexcept = default;

const UInt32& DirectX12DescriptorSetLayout::rootParameterIndex() const noexcept
{
    return m_impl->m_rootParameterIndex;
}

UInt32 DirectX12DescriptorSetLayout::descriptorOffsetForBinding(const UInt32& binding) const
{
    if (!m_impl->m_bindingToDescriptor.contains(binding)) [[unlikely]]
        throw ArgumentOutOfRangeException("The descriptor set does not contain a descriptor at binding {0}.", binding);

    return m_impl->m_bindingToDescriptor[binding];
}

UInt32& DirectX12DescriptorSetLayout::rootParameterIndex() noexcept
{
    return m_impl->m_rootParameterIndex;
}

bool DirectX12DescriptorSetLayout::isRuntimeArray() const noexcept
{
    return m_impl->m_isRuntimeArray;
}

const DirectX12Device& DirectX12DescriptorSetLayout::device() const noexcept
{
    return m_impl->m_device;
}

Enumerable<const DirectX12DescriptorLayout*> DirectX12DescriptorSetLayout::descriptors() const noexcept
{
    return m_impl->m_layouts | std::views::transform([](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout.get(); });
}

const DirectX12DescriptorLayout& DirectX12DescriptorSetLayout::descriptor(const UInt32& binding) const
{
    if (auto match = std::ranges::find_if(m_impl->m_layouts, [&binding](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->binding() == binding; }); match != m_impl->m_layouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No layout has been provided for the binding {0}.", binding);
}

const UInt32& DirectX12DescriptorSetLayout::space() const noexcept
{
    return m_impl->m_space;
}

const ShaderStage& DirectX12DescriptorSetLayout::shaderStages() const noexcept
{
    return m_impl->m_stages;
}

UInt32 DirectX12DescriptorSetLayout::uniforms() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::ConstantBuffer; });
}

UInt32 DirectX12DescriptorSetLayout::storages() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::StructuredBuffer || layout->descriptorType() == DescriptorType::RWStructuredBuffer || layout->descriptorType() == DescriptorType::ByteAddressBuffer || layout->descriptorType() == DescriptorType::RWByteAddressBuffer; });
}

UInt32 DirectX12DescriptorSetLayout::buffers() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Buffer || layout->descriptorType() == DescriptorType::RWBuffer; });
}

UInt32 DirectX12DescriptorSetLayout::images() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Texture || layout->descriptorType() == DescriptorType::RWTexture; });
}

UInt32 DirectX12DescriptorSetLayout::samplers() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Sampler && layout->staticSampler() == nullptr; });
}

UInt32 DirectX12DescriptorSetLayout::staticSamplers() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Sampler && layout->staticSampler() != nullptr; });
}

UInt32 DirectX12DescriptorSetLayout::inputAttachments() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::InputAttachment; });
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate(const Enumerable<DescriptorBinding>& bindings) const
{
    return this->allocate(0, bindings);
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate(const UInt32& descriptors, const Enumerable<DescriptorBinding>& bindings) const
{
    // Allocate the descriptor set.
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    ComPtr<ID3D12DescriptorHeap> bufferHeap, samplerHeap;
    m_impl->tryAllocate(bufferHeap, samplerHeap, descriptors);
    auto descriptorSet = makeUnique<DirectX12DescriptorSet>(*this, std::move(bufferHeap), std::move(samplerHeap));

    // Apply the default bindings.
    for (auto& binding : bindings)
        std::visit(type_switch{
            [&descriptorSet, &binding](const ISampler& sampler) { descriptorSet->update(binding.binding, sampler, binding.firstDescriptor); },
            [&descriptorSet, &binding](const IBuffer& buffer) { descriptorSet->update(binding.binding, buffer, binding.firstElement, binding.elements, binding.firstDescriptor); },
            [&descriptorSet, &binding](const IImage& image) { descriptorSet->update(binding.binding, image, binding.firstDescriptor, binding.firstLevel, binding.levels, binding.firstElement, binding.elements); }
        }, binding.resource);

    // Return the descriptor set.
    return descriptorSet;
}

Enumerable<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocateMultiple(const UInt32& descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings) const
{
    return this->allocateMultiple(descriptorSets, 0, bindings);
}

Enumerable<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocateMultiple(const UInt32& descriptorSets, std::function<Enumerable<DescriptorBinding>(const UInt32&)> bindingFactory) const
{
    return this->allocateMultiple(descriptorSets, 0, bindingFactory);
}

Enumerable<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocateMultiple(const UInt32& count, const UInt32& descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings) const
{
    auto generator = [this, descriptors, &bindings]() mutable -> std::experimental::generator<UniquePtr<DirectX12DescriptorSet>> {
        for (auto& binding : bindings)
            co_yield this->allocate(descriptors, binding);

        co_yield this->allocate(descriptors);
    }();

    return generator | std::views::take(count);
}

Enumerable<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocateMultiple(const UInt32& count, const UInt32& descriptors, std::function<Enumerable<DescriptorBinding>(const UInt32&)> bindingFactory) const
{
    auto generator = [this, descriptors, &bindingFactory, i = 0]() mutable -> std::experimental::generator<UniquePtr<DirectX12DescriptorSet>> {
        co_yield this->allocate(descriptors, bindingFactory(i++));
    }();

    return generator | std::views::take(count);
}

void DirectX12DescriptorSetLayout::free(const DirectX12DescriptorSet& descriptorSet) const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);

    // Unbounded array descriptor sets aren't cached.
    if (!m_impl->m_isRuntimeArray)
    {
        m_impl->m_freeDescriptorSets.push(ComPtr<ID3D12DescriptorHeap>(descriptorSet.bufferHeap()));
        m_impl->m_freeSamplerSets.push(ComPtr<ID3D12DescriptorHeap>(descriptorSet.samplerHeap()));
    }
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Descriptor set layout builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSetLayoutBuilder::DirectX12DescriptorSetLayoutBuilderImpl : public Implement<DirectX12DescriptorSetLayoutBuilder> {
public:
    friend class DirectX12DescriptorSetLayoutBuilder;

private:
    Array<UniquePtr<DirectX12DescriptorLayout>> m_layouts;
    UInt32 m_space;
    ShaderStage m_stages;

public:
    DirectX12DescriptorSetLayoutBuilderImpl(DirectX12DescriptorSetLayoutBuilder* parent, const UInt32& space, const ShaderStage& stages) :
        base(parent), m_space(space), m_stages(stages)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Descriptor set layout builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayoutBuilder::DirectX12DescriptorSetLayoutBuilder(DirectX12PipelineLayoutBuilder& parent, const UInt32& space, const ShaderStage& stages, const UInt32& /*poolSize*/, const UInt32& /*maxUnboundedArraySize*/) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutBuilderImpl>(this, space, stages)), DescriptorSetLayoutBuilder(parent, UniquePtr<DirectX12DescriptorSetLayout>(new DirectX12DescriptorSetLayout(parent.device())))
{
}

DirectX12DescriptorSetLayoutBuilder::~DirectX12DescriptorSetLayoutBuilder() noexcept = default;

void DirectX12DescriptorSetLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_layouts = std::move(m_impl->m_layouts);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = std::move(m_impl->m_stages);
    instance->m_impl->initialize();
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::withDescriptor(UniquePtr<DirectX12DescriptorLayout>&& layout)
{
    m_impl->m_layouts.push_back(std::move(layout));
    return *this;
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors)
{
    return this->withDescriptor(makeUnique<DirectX12DescriptorLayout>(type, binding, descriptorSize, descriptors));
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::withStaticSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy)
{
    return this->withDescriptor(makeUnique<DirectX12DescriptorLayout>(makeUnique<DirectX12Sampler>(this->parent().device(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy), binding));
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::space(const UInt32& space) noexcept
{
    m_impl->m_space = space;
    return *this;
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::shaderStages(const ShaderStage& stages) noexcept
{
    m_impl->m_stages = stages;
    return *this;
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::poolSize(const UInt32& /*poolSize*/) noexcept
{
    return *this;
}
#endif // defined(BUILD_DEFINE_BUILDERS)