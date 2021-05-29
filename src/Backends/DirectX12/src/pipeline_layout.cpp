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
        Array<Array<D3D12_DESCRIPTOR_RANGE1>> descriptorRanges;
        Array<D3D12_ROOT_PARAMETER1> descriptorParameters;
        Array<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

        std::for_each(std::begin(descriptorLayouts), std::end(descriptorLayouts), [&, space = 0](const auto& layout) mutable {
            auto descriptorSetLayout = dynamic_cast<const DirectX12DescriptorSetLayout*>(layout.get());

            if (descriptorSetLayout == nullptr)
                return;

            // Parse the shader stage descriptor.
            DWORD shaderStages = {};
            auto stages = descriptorSetLayout->getShaderStages();

            if ((stages & ShaderStage::Vertex) == ShaderStage::Vertex)
                shaderStages = shaderStages | D3D12_SHADER_VISIBILITY_VERTEX;
            if ((stages & ShaderStage::Geometry) == ShaderStage::Geometry)
                shaderStages = shaderStages | D3D12_SHADER_VISIBILITY_GEOMETRY;
            if ((stages & ShaderStage::Fragment) == ShaderStage::Fragment)
                shaderStages = shaderStages | D3D12_SHADER_VISIBILITY_PIXEL;
            if ((stages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
                shaderStages = shaderStages | D3D12_SHADER_VISIBILITY_DOMAIN;
            if ((stages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
                shaderStages = shaderStages | D3D12_SHADER_VISIBILITY_HULL;
            if ((stages & ShaderStage::Compute) == ShaderStage::Compute)
                shaderStages = D3D12_SHADER_VISIBILITY_ALL;

            // Define the root parameter ranges.
            auto ranges = descriptorSetLayout->getLayouts();
            Array<D3D12_DESCRIPTOR_RANGE1> rangeSet(ranges.size());

            std::generate(std::begin(rangeSet), std::end(rangeSet), [&, i = 0]() mutable {
                const auto& range = ranges[i++];

                D3D12_DESCRIPTOR_RANGE1 descriptorRange = {};
                descriptorRange.BaseShaderRegister = range->getBinding();
                descriptorRange.NumDescriptors = 1;
                descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                descriptorRange.RegisterSpace = space;

                switch(range->getType())
                { 
                case DescriptorType::Uniform: descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
                case DescriptorType::Storage: descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
                case DescriptorType::Image:   descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
                case DescriptorType::Sampler: descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
                case DescriptorType::InputAttachment:   descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
                }

                return descriptorRange;
            });

            // Define the root parameter.
            D3D12_ROOT_PARAMETER1 rootParameter = {};
            rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(shaderStages);
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameter.DescriptorTable = {};
            rootParameter.DescriptorTable.NumDescriptorRanges = static_cast<UInt32>(rangeSet.size());
            rootParameter.DescriptorTable.pDescriptorRanges = rangeSet.data();

            // Store the range set.
            descriptorParameters.push_back(rootParameter);
            space++;
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
    ComResource<ID3D12RootSignature>(nullptr), DirectX12RuntimeObject(pipeline.getDevice()), m_impl(makePimpl<DirectX12RenderPipelineLayoutImpl>(this, pipeline))
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

DirectX12DescriptorSetLayoutBuilder DirectX12RenderPipelineLayoutBuilder::addDescriptorSet(const UInt32& id, const ShaderStage& stages)
{
    return this->make<DirectX12DescriptorSetLayout>(id, stages);
}