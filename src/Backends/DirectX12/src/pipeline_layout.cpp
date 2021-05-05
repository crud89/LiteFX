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

public:
    DirectX12RenderPipelineLayoutImpl(DirectX12RenderPipelineLayout* parent, const DirectX12RenderPipeline& pipeline) :
        base(parent), m_pipeline(pipeline) { }

public:
    ComPtr<ID3D12RootSignature> initialize()
    {
        // Get the device.
        ComPtr<ID3D12Device> device;
        raiseIfFailed<RuntimeException>(m_pipeline.handle()->GetDevice(IID_PPV_ARGS(&device)), "Unable to query device for creating a pipeline layout.");

        // Get the shader program.
        auto shaderProgram = m_parent->getProgram();

        if (shaderProgram == nullptr)
            throw RuntimeException("The shader program must be initialized before creating a pipeline layout.");

        // Define the descriptor range from descriptor set layouts.
        Array<D3D12_DESCRIPTOR_RANGE1> descriptorRanges;
        Array<CD3DX12_ROOT_PARAMETER1> descriptorParameters;
        Array<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
        auto layouts = shaderProgram->getLayouts();

        std::for_each(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) {
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

        return rootSignature;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineLayout::DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline) :
    RenderPipelineLayout(), IComResource<ID3D12RootSignature>(nullptr), m_impl(makePimpl<DirectX12RenderPipelineLayoutImpl>(this, pipeline))
{
}

DirectX12RenderPipelineLayout::~DirectX12RenderPipelineLayout() noexcept
{
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize();

    return RenderPipelineLayoutBuilder::go();
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IRasterizer>&& rasterizer)
{
    if (rasterizer == nullptr)
        throw std::invalid_argument("The rasterizer must be initialized.");

    this->instance()->use(std::move(rasterizer));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IInputAssembler>&& inputAssembler)
{
    if (inputAssembler == nullptr)
        throw std::invalid_argument("The input assembler must be initialized.");

    this->instance()->use(std::move(inputAssembler));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IViewport>&& viewport)
{
    if (viewport == nullptr)
        throw std::invalid_argument("The viewport must be initialized.");

    this->instance()->use(std::move(viewport));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IShaderProgram>&& program)
{
    if (program == nullptr)
        throw std::invalid_argument("The program must be initialized.");

    this->instance()->use(std::move(program));
}

DirectX12RenderPipelineLayoutBuilder& DirectX12RenderPipelineLayoutBuilder::enableDepthTest(const bool& enable)
{
    this->instance()->setDepthTest(enable);
    return *this;
}

DirectX12RenderPipelineLayoutBuilder& DirectX12RenderPipelineLayoutBuilder::enableStencilTest(const bool& enable)
{
    this->instance()->setStencilTest(enable);
    return *this;
}

DirectX12RasterizerBuilder DirectX12RenderPipelineLayoutBuilder::setRasterizer()
{
    return this->make<DirectX12Rasterizer>();
}

DirectX12InputAssemblerBuilder DirectX12RenderPipelineLayoutBuilder::setInputAssembler()
{
    return this->make<DirectX12InputAssembler>();
}

//DirectX12ShaderProgramBuilder DirectX12RenderPipelineLayoutBuilder::shaderProgram()
//{
//    return this->make<DirectX12ShaderProgram>();
//}

DirectX12ViewportBuilder DirectX12RenderPipelineLayoutBuilder::addViewport()
{
    return this->make<DirectX12Viewport>();
}