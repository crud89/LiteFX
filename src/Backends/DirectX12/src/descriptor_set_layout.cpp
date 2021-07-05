#include <litefx/backends/dx12.hpp>

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
    mutable std::mutex m_mutex;

public:
    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages, const UInt32& rootParameterIndex) :
        base(parent), m_layouts(std::move(descriptorLayouts)), m_space(space), m_stages(stages), m_rootParameterIndex(rootParameterIndex)
    {
    }

    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent, const UInt32& rootParameterIndex) :
        base(parent), m_rootParameterIndex(rootParameterIndex)
    {
    }

public:
    void initialize()
    {
        LITEFX_TRACE(DIRECTX12_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_space, m_stages);

        std::ranges::for_each(m_layouts, [&, i = 0](const UniquePtr<DirectX12DescriptorLayout>& layout) mutable {
            LITEFX_TRACE(DIRECTX12_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Offset: {4}, Binding point: {5} }}...", ++i, m_layouts.size(), layout->descriptorType(), layout->elementSize(), 0, layout->binding());
            
            if (layout->descriptorType() == DescriptorType::Sampler)
                m_samplers++;
            else
                m_descriptors++;
        });
    }

public:
    void tryAllocate(ComPtr<ID3D12DescriptorHeap>& bufferHeap, ComPtr<ID3D12DescriptorHeap>& samplerHeap)
    {
        // Use descriptor heaps from the queues, if possible.
        if (m_descriptors > 0)
        {
            if (!m_freeDescriptorSets.empty())
            {
                bufferHeap = m_freeDescriptorSets.front();
                m_freeDescriptorSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {};
                bufferHeapDesc.NumDescriptors = m_descriptors;
                bufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                bufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&bufferHeap)), "Unable create constant CPU descriptor heap for constant buffers and images.");
            }
        }

        // Repeat for sampler heaps.
        if (m_samplers > 0)
        {
            if (!m_freeSamplerSets.empty())
            {
                bufferHeap = m_freeSamplerSets.front();
                m_freeSamplerSets.pop();
            }
            else
            {
                D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
                samplerHeapDesc.NumDescriptors = m_samplers;
                samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap)), "Unable create constant CPU descriptor heap for samplers.");
            }
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12PipelineLayout& pipelineLayout, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages, const UInt32& rootParameterIndex) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this, std::move(descriptorLayouts), space, stages, rootParameterIndex)), DirectX12RuntimeObject(pipelineLayout, pipelineLayout.getDevice())
{
    m_impl->initialize();
}

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12PipelineLayout& pipelineLayout, const UInt32& rootParameterIndex) noexcept :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this, rootParameterIndex)), DirectX12RuntimeObject<DirectX12PipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

DirectX12DescriptorSetLayout::~DirectX12DescriptorSetLayout() noexcept = default;

const UInt32& DirectX12DescriptorSetLayout::rootParameterIndex() const noexcept
{
    return m_impl->m_rootParameterIndex;
}

Array<const DirectX12DescriptorLayout*> DirectX12DescriptorSetLayout::descriptors() const noexcept
{
    return m_impl->m_layouts |
        std::views::transform([](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const DirectX12DescriptorLayout*>>();
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
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Uniform; });
}

UInt32 DirectX12DescriptorSetLayout::storages() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Storage; });
}

UInt32 DirectX12DescriptorSetLayout::images() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Image; });
}

UInt32 DirectX12DescriptorSetLayout::samplers() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::Sampler; });
}

UInt32 DirectX12DescriptorSetLayout::inputAttachments() const noexcept
{
    return std::ranges::count_if(m_impl->m_layouts, [](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout->descriptorType() == DescriptorType::InputAttachment; });
}

UniquePtr<DirectX12DescriptorSet> DirectX12DescriptorSetLayout::allocate() const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    ComPtr<ID3D12DescriptorHeap> bufferHeap, samplerHeap;
    m_impl->tryAllocate(bufferHeap, samplerHeap);

    return makeUnique<DirectX12DescriptorSet>(*this, std::move(bufferHeap), std::move(samplerHeap));
}

Array<UniquePtr<DirectX12DescriptorSet>> DirectX12DescriptorSetLayout::allocate(const UInt32& count) const noexcept
{
    Array<UniquePtr<DirectX12DescriptorSet>> descriptorSets(count);
    std::ranges::generate(descriptorSets, [this]() { return this->allocate(); });
    return descriptorSets;
}

void DirectX12DescriptorSetLayout::free(const DirectX12DescriptorSet& descriptorSet) const noexcept
{
    std::lock_guard<std::mutex> lock(m_impl->m_mutex);
    m_impl->m_freeDescriptorSets.push(ComPtr<ID3D12DescriptorHeap>(descriptorSet.bufferHeap()));
    m_impl->m_freeSamplerSets.push(ComPtr<ID3D12DescriptorHeap>(descriptorSet.samplerHeap()));
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
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
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayoutBuilder::DirectX12DescriptorSetLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& rootParameterIndex, const UInt32& space, const ShaderStage& stages) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutBuilderImpl>(this, space, stages)), DescriptorSetLayoutBuilder(parent, UniquePtr<DirectX12DescriptorSetLayout>(new DirectX12DescriptorSetLayout(*std::as_const(parent).instance(), rootParameterIndex)))
{
}

DirectX12DescriptorSetLayoutBuilder::~DirectX12DescriptorSetLayoutBuilder() noexcept = default;

DirectX12RenderPipelineLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_layouts = std::move(m_impl->m_layouts);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = std::move(m_impl->m_stages);
    instance->m_impl->initialize();

    return DescriptorSetLayoutBuilder::go();
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::addDescriptor(UniquePtr<DirectX12DescriptorLayout>&& layout)
{
    m_impl->m_layouts.push_back(std::move(layout));
    return *this;
}

DirectX12DescriptorSetLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize)
{
    return this->addDescriptor(makeUnique<DirectX12DescriptorLayout>(*(this->instance()), type, binding, descriptorSize));
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