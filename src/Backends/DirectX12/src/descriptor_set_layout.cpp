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
    UInt32 m_space, m_samplers, m_descriptors;
    ShaderStage m_stages;
    Queue<ComPtr<ID3D12DescriptorHeap>> m_freeDescriptorSets;
    mutable std::mutex m_mutex;

public:
    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages) :
        base(parent), m_layouts(std::move(descriptorLayouts)), m_space(space), m_stages(stages)
    {

    }

    DirectX12DescriptorSetLayoutImpl(DirectX12DescriptorSetLayout* parent) :
        base(parent)
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
    ComPtr<ID3D12DescriptorHeap> tryAllocate()
    {
        // TODO: A descriptor set must actually store multiple heaps: One for CBV/SRV/UAV, one for samplers, one for RTV and one for DSV (the last two are depending on the input attachments format).
        //ComPtr<ID3D12DescriptorHeap> constantBufferHeap;
        //D3D12_DESCRIPTOR_HEAP_DESC constantBuffer = {};
        //constantBuffer.NumDescriptors = m_descriptors;
        //constantBuffer.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        //constantBuffer.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        //raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&constantBuffer, IID_PPV_ARGS(&constantBufferHeap)), "Unable create constant CPU descriptor heap for constant buffers and images.");
        throw;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12RenderPipelineLayout& pipelineLayout, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this, std::move(descriptorLayouts), space, stages)), DirectX12RuntimeObject(pipelineLayout, pipelineLayout.getDevice())
{
}

DirectX12DescriptorSetLayout::DirectX12DescriptorSetLayout(const DirectX12RenderPipelineLayout& pipelineLayout) noexcept :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutImpl>(this)), DirectX12RuntimeObject<DirectX12RenderPipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

DirectX12DescriptorSetLayout::~DirectX12DescriptorSetLayout() noexcept = default;

Array<const DirectX12DescriptorLayout*> DirectX12DescriptorSetLayout::layouts() const noexcept
{
    return m_impl->m_layouts |
        std::views::transform([](const UniquePtr<DirectX12DescriptorLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const DirectX12DescriptorLayout*>>();
}

const DirectX12DescriptorLayout& DirectX12DescriptorSetLayout::layout(const UInt32& binding) const
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

    // If no descriptor sets are free, allocate a new one.
    if (m_impl->m_freeDescriptorSets.empty())
        return makeUnique<DirectX12DescriptorSet>(*this, m_impl->tryAllocate());

    // Otherwise, pick and remove one from the list.
    auto heap = m_impl->m_freeDescriptorSets.front();
    m_impl->m_freeDescriptorSets.pop();
    auto descriptorSet = makeUnique<DirectX12DescriptorSet>(*this, std::move(heap));

    return descriptorSet;
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
    m_impl->m_freeDescriptorSets.push(descriptorSet.handle());
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

DirectX12DescriptorSetLayoutBuilder::DirectX12DescriptorSetLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& space, const ShaderStage& stages) :
    m_impl(makePimpl<DirectX12DescriptorSetLayoutBuilderImpl>(this, space, stages)), DescriptorSetLayoutBuilder(parent, UniquePtr<DirectX12DescriptorSetLayout>(new DirectX12DescriptorSetLayout(*std::as_const(parent).instance())))
{
}

DirectX12DescriptorSetLayoutBuilder::~DirectX12DescriptorSetLayoutBuilder() noexcept = default;

DirectX12RenderPipelineLayoutBuilder& DirectX12DescriptorSetLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_layouts = std::move(m_impl->m_layouts);
    instance->m_impl->m_space = std::move(m_impl->m_space);
    instance->m_impl->m_stages = std::move(m_impl->m_stages);

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