#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipelineLayout::DirectX12RenderPipelineLayoutImpl : public Implement<DirectX12RenderPipelineLayout> {
public:
    friend class DirectX12RenderPipelineLayoutBuilder;
    friend class DirectX12RenderPipelineLayout;

private:
    const DirectX12RenderPipeline& m_pipeline;
    UniquePtr<IShaderProgram> m_shaderProgram;
    Array<UniquePtr<IDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    DirectX12RenderPipelineLayoutImpl(DirectX12RenderPipelineLayout* parent, const DirectX12RenderPipeline& pipeline) :
        base(parent), m_pipeline(pipeline) { }

public:
    ComPtr<ID3D12RootSignature> initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts)
    {
        // Get the device.
        ComPtr<ID3D12Device> device;
        raiseIfFailed<RuntimeException>(m_pipeline.handle()->GetDevice(IID_PPV_ARGS(&device)), "Unable to query device for creating a pipeline layout.");

        if (shaderProgram == nullptr)
            throw RuntimeException("The shader program must be initialized before creating a pipeline layout.");

        // Define the descriptor range from descriptor set layouts.
        Array<D3D12_DESCRIPTOR_RANGE1> descriptorRanges;
        Array<CD3DX12_ROOT_PARAMETER1> descriptorParameters;
        Array<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;

        std::for_each(std::begin(descriptorLayouts), std::end(descriptorLayouts), [&](const auto& layout) {
            //auto descriptorSetLayout = dynamic_cast<const DirectX12DescriptorSetLayout*>(layout);

            //if (descriptorSetLayout == nullptr)
            //    return;

            // TODO: 
            // - create descriptor range
            // - descriptorRanges.push_back();
            // 
            //DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_SRV, 6, 2); // t2-t7
            //DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_UAV, 4, 0); // u0-u3
            //DescRange[2].Init(D3D12_DESCRIPTOR_RANGE_SAMPLER, 2, 0); // s0-s1
            //DescRange[3].Init(D3D12_DESCRIPTOR_RANGE_SRV, -1, 8, 0,
            //    D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // t8-unbounded
            //DescRange[4].Init(D3D12_DESCRIPTOR_RANGE_SRV, -1, 0, 1,
            //    D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
            //// (t0,space1)-unbounded
            //DescRange[5].Init(D3D12_DESCRIPTOR_RANGE_CBV, 1, 1,
            //    D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // b1

            // TODO: Init parameters.
            //RP[0].InitAsConstants(3, 2); // 3 constants at b2
            //RP[1].InitAsDescriptorTable(2, &DescRange[0]); // 2 ranges t2-t7 and u0-u3
            //RP[2].InitAsConstantBufferView(0, 0,
            //    D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC); // b0
            //RP[3].InitAsDescriptorTable(1, &DescRange[2]); // s0-s1
            //RP[4].InitAsDescriptorTable(1, &DescRange[3]); // t8-unbounded
            //RP[5].InitAsDescriptorTable(1, &DescRange[4]); // (t0,space1)-unbounded
            //RP[6].InitAsDescriptorTable(1, &DescRange[5]); // b1

            // TODO: init samplers
            //StaticSamplers[0].Init(3, D3D12_FILTER_ANISOTROPIC); // s3

            throw;
        });

        // Create root signature descriptor.
        // TODO: Read error blob.
        ComPtr<ID3DBlob> signature, error;
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(static_cast<UInt32>(descriptorParameters.size()), descriptorParameters.data(), static_cast<UInt32>(staticSamplers.size()), staticSamplers.data());
        raiseIfFailed<RuntimeException>(::D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error), "Unable to serialize root signature to create pipeline layout.");

        // Create the root signature.
        ComPtr<ID3D12RootSignature> rootSignature;
        raiseIfFailed<RuntimeException>(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Unable to create root signature for pipeline layout.");

        // Store the shader program and descriptor layouts.
        m_shaderProgram = std::move(shaderProgram);
        m_descriptorSetLayouts = std::move(descriptorLayouts);

        return rootSignature;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineLayout::DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline) :
    IComResource<ID3D12RootSignature>(nullptr), m_impl(makePimpl<DirectX12RenderPipelineLayoutImpl>(this, pipeline))
{
}

DirectX12RenderPipelineLayout::~DirectX12RenderPipelineLayout() noexcept = default;

bool DirectX12RenderPipelineLayout::isInitialized() const noexcept
{
    return this->handle() != nullptr;
}

void DirectX12RenderPipelineLayout::initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts)
{
    if (this->isInitialized())
        throw RuntimeException("The render pipeline layout already has been initialized.");

    this->handle() = m_impl->initialize(std::move(shaderProgram), std::move(descriptorLayouts));
}

const IShaderProgram* DirectX12RenderPipelineLayout::getProgram() const noexcept
{
    return m_impl->m_shaderProgram.get();
}

Array<const IDescriptorSetLayout*> DirectX12RenderPipelineLayout::getDescriptorSetLayouts() const noexcept
{
    Array<const IDescriptorSetLayout*> layouts(m_impl->m_descriptorSetLayouts.size());
    std::generate(std::begin(layouts), std::end(layouts), [&, i = 0]() mutable { return m_impl->m_descriptorSetLayouts[i++].get(); });

    return layouts;
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipelineLayoutBuilder::DirectX12RenderPipelineLayoutBuilderImpl : public Implement<DirectX12RenderPipelineLayoutBuilder> {
public:
    friend class DirectX12RenderPipelineLayoutBuilder;
    friend class DirectX12RenderPipelineLayout;

private:
    UniquePtr<IShaderProgram> m_shaderProgram;
    Array<UniquePtr<IDescriptorSetLayout>> m_descriptorSetLayouts;

public:
    DirectX12RenderPipelineLayoutBuilderImpl(DirectX12RenderPipelineLayoutBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineLayoutBuilder::DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineBuilder& parent, UniquePtr<DirectX12RenderPipelineLayout>&& instance) :
    RenderPipelineLayoutBuilder(parent, std::move(instance)), m_impl(makePimpl<DirectX12RenderPipelineLayoutBuilderImpl>(this))
{
}

DirectX12RenderPipelineLayoutBuilder::~DirectX12RenderPipelineLayoutBuilder() noexcept = default;

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineLayoutBuilder::go()
{
    this->instance()->initialize(std::move(m_impl->m_shaderProgram), std::move(m_impl->m_descriptorSetLayouts));

    return RenderPipelineLayoutBuilder::go();
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IShaderProgram>&& program)
{
#ifndef NDEBUG
    if (m_impl->m_shaderProgram != nullptr)
        LITEFX_WARNING(DIRECTX12_LOG, "Another shader program has already been initialized and will be replaced. A pipeline layout can only have one shader program.");
#endif

    m_impl->m_shaderProgram = std::move(program);
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IDescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

DirectX12ShaderProgramBuilder DirectX12RenderPipelineLayoutBuilder::shaderProgram()
{
    return this->make<DirectX12ShaderProgram>();
}

//DirectX12DescriptorSetLayoutBuilder DirectX12RenderPipelineLayoutBuilder::addDescriptorSet(const UInt32& id, const ShaderStage& stages)
//{
//    return this->make<DirectX12DescriptorSetLayout>(id, stages);
//}