#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PipelineLayout::DirectX12PipelineLayoutImpl : public Implement<DirectX12PipelineLayout> {
public:
    friend class DirectX12PipelineLayoutBuilder;
    friend class DirectX12PipelineLayout;

private:
    UniquePtr<DirectX12PushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<DirectX12DescriptorSetLayout>> m_descriptorSetLayouts;
    const DirectX12Device& m_device;

public:
    DirectX12PipelineLayoutImpl(DirectX12PipelineLayout* parent, const DirectX12Device& device, Array<UniquePtr<DirectX12DescriptorSetLayout>>&& descriptorLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout) :
        base(parent), m_descriptorSetLayouts(std::move(descriptorLayouts)), m_pushConstantsLayout(std::move(pushConstantsLayout)), m_device(device)
    {
    }

    DirectX12PipelineLayoutImpl(DirectX12PipelineLayout* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }

private:
    D3D12_FILTER getFilterMode(const FilterMode& minFilter, const FilterMode& magFilter, const MipMapMode& mipFilter, const Float& anisotropy = 0.f)
    {
        if (anisotropy > 0.f)
            return D3D12_ENCODE_ANISOTROPIC_FILTER(D3D12_FILTER_REDUCTION_TYPE_STANDARD);
        else
        {
            D3D12_FILTER_TYPE minType = minFilter == FilterMode::Nearest ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
            D3D12_FILTER_TYPE magType = magFilter == FilterMode::Nearest ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
            D3D12_FILTER_TYPE mipType = mipFilter == MipMapMode::Nearest ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;

            return D3D12_ENCODE_BASIC_FILTER(minType, magType, mipType, D3D12_FILTER_REDUCTION_TYPE_STANDARD);
        }
    }

    D3D12_TEXTURE_ADDRESS_MODE getBorderMode(const BorderMode& mode)
    {
        switch (mode)
        {
        case BorderMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case BorderMode::ClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case BorderMode::ClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case BorderMode::RepeatMirrored: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case BorderMode::ClampToEdgeMirrored: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default: throw std::invalid_argument("Invalid border mode.");
        }
    }

public:
    ComPtr<ID3D12RootSignature> initialize()
    {
        // Define the descriptor range from descriptor set layouts.
        Array<D3D12_ROOT_PARAMETER1> descriptorParameters;
        Array<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        Array<Array<D3D12_DESCRIPTOR_RANGE1>> descriptorRanges;
        bool hasInputAttachments = false;
        bool hasInputAttachmentSampler = false;
        UInt32 rootParameterIndex{ 0 };

        LITEFX_TRACE(DIRECTX12_LOG, "Creating render pipeline layout {0} {{ Descriptor Sets: {1}, Push Constant Ranges: {2} }}...", fmt::ptr(m_parent), m_descriptorSetLayouts.size(), m_pushConstantsLayout == nullptr ? 0 : m_pushConstantsLayout->ranges().size());

        if (m_pushConstantsLayout != nullptr)
        {
            std::ranges::for_each(m_pushConstantsLayout->ranges(), [&](DirectX12PushConstantsRange* range) {
                CD3DX12_ROOT_PARAMETER1 rootParameter = {};

                switch (range->stage())
                {
                case ShaderStage::Vertex: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_VERTEX); break;
                case ShaderStage::Geometry: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_GEOMETRY); break;
                case ShaderStage::Fragment: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_PIXEL); break;
                case ShaderStage::TessellationEvaluation: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_DOMAIN); break;
                case ShaderStage::TessellationControl: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_HULL); break;
                default: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_ALL); break;
                }

                // Store the range.
                range->rootParameterIndex() = rootParameterIndex++;
                descriptorParameters.push_back(rootParameter);
            });
        }

        std::ranges::for_each(m_descriptorSetLayouts, [&](const UniquePtr<DirectX12DescriptorSetLayout>& layout) {
            // Parse the shader stage descriptor.
            D3D12_SHADER_VISIBILITY shaderStages = D3D12_SHADER_VISIBILITY_ALL;
            auto stages = layout->shaderStages();
            UInt32 space = layout->space();

            if (stages == ShaderStage::Vertex)
                shaderStages = D3D12_SHADER_VISIBILITY_VERTEX;
            if (stages == ShaderStage::Geometry)
                shaderStages = D3D12_SHADER_VISIBILITY_GEOMETRY;
            if (stages == ShaderStage::Fragment)
                shaderStages = D3D12_SHADER_VISIBILITY_PIXEL;
            if (stages == ShaderStage::TessellationEvaluation)
                shaderStages = D3D12_SHADER_VISIBILITY_DOMAIN;
            if (stages == ShaderStage::TessellationControl)
                shaderStages = D3D12_SHADER_VISIBILITY_HULL;

            // Define the root parameter ranges.
            Array<const DirectX12DescriptorLayout*> layouts = layout->descriptors();
            Array<D3D12_DESCRIPTOR_RANGE1> rangeSet = layouts |
                std::views::filter([](const DirectX12DescriptorLayout* range) { return range->staticSampler() == nullptr; }) |
                std::views::transform([&](const DirectX12DescriptorLayout* range) {
                CD3DX12_DESCRIPTOR_RANGE1 descriptorRange = {};

                switch(range->descriptorType()) 
                { 
                case DescriptorType::ConstantBuffer:    descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                case DescriptorType::InputAttachment:   hasInputAttachments = true; [[fallthrough]];
                case DescriptorType::Buffer:
                case DescriptorType::StructuredBuffer:
                case DescriptorType::ByteAddressBuffer:
                case DescriptorType::Texture:           descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                case DescriptorType::RWBuffer:
                case DescriptorType::RWStructuredBuffer:
                case DescriptorType::RWByteAddressBuffer:
                case DescriptorType::RWTexture:         descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                case DescriptorType::Sampler:
                    if (stages != ShaderStage::Compute && range->binding() == 0 && space == 0)  // NOTE: This is valid for compute shaders and shaders in render passes without input attachments.
                        LITEFX_WARNING(DIRECTX12_LOG, "Sampler bound to register 0 of space 0, which is reserved for input attachment samplers. If your render pass does not have any input attachments, this is fine. To disable this warning, bind the sampler to another register or space, or provide a static sampler state through the root signature instead and use shader reflection to create the pipeline layout.");

                    descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
                    break;
                default: throw InvalidArgumentException("Invalid descriptor type: {0}.", range->descriptorType());
                }

                return descriptorRange;
            }) | ranges::to<Array<D3D12_DESCRIPTOR_RANGE1>>();

            // Define the static samplers.
            std::ranges::for_each(layouts, [&](const DirectX12DescriptorLayout* range) {
                if (range->staticSampler() != nullptr)
                {
                    // Remember, that there's a manually defined input attachment sampler.
                    if (range->binding() == 0 && space == 0)
                        hasInputAttachmentSampler = true;

                    auto sampler = range->staticSampler();

                    D3D12_STATIC_SAMPLER_DESC samplerInfo = {
                        .Filter = getFilterMode(sampler->getMinifyingFilter(), sampler->getMagnifyingFilter(), sampler->getMipMapMode(), sampler->getAnisotropy()),
                        .AddressU = getBorderMode(sampler->getBorderModeU()),
                        .AddressV = getBorderMode(sampler->getBorderModeV()),
                        .AddressW = getBorderMode(sampler->getBorderModeW()),
                        .MipLODBias = sampler->getMipMapBias(),
                        .MaxAnisotropy = static_cast<UInt32>(sampler->getAnisotropy()),
                        .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
                        //.BorderColor = { 0.f, 0.f, 0.f, 0.f },
                        .MinLOD = sampler->getMinLOD(),
                        .MaxLOD = sampler->getMaxLOD(),
                        .ShaderRegister = range->binding(),
                        .RegisterSpace = space,
                        .ShaderVisibility = shaderStages
                    };

                    staticSamplers.push_back(samplerInfo);
                }
            });

            // Define the root parameter.
            CD3DX12_ROOT_PARAMETER1 rootParameter = {};
            rootParameter.InitAsDescriptorTable(rangeSet.size(), rangeSet.data(), static_cast<D3D12_SHADER_VISIBILITY>(shaderStages));
            descriptorRanges.push_back(std::move(rangeSet));

            // Store the range set.
            layout->rootParameterIndex() = rootParameterIndex++;
            descriptorParameters.push_back(rootParameter);
        });

        // Define a static sampler to sample the G-Buffer, if it is not manually defined.
        if (hasInputAttachments && !hasInputAttachmentSampler)
            staticSamplers.push_back(CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR));

        // Create root signature descriptor.
        ComPtr<ID3DBlob> signature, error;
        String errorString = "";
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(static_cast<UInt32>(descriptorParameters.size()), descriptorParameters.data(), static_cast<UInt32>(staticSamplers.size()), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        HRESULT hr = ::D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
        
        if (error != nullptr)
            errorString = String(reinterpret_cast<TCHAR*>(error->GetBufferPointer()), error->GetBufferSize());
        
        raiseIfFailed<RuntimeException>(hr, "Unable to serialize root signature to create pipeline layout: {0}", errorString);

        // Create the root signature.
        ComPtr<ID3D12RootSignature> rootSignature;
        raiseIfFailed<RuntimeException>(m_device.handle()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Unable to create root signature for pipeline layout.");

        return rootSignature;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12PipelineLayout::DirectX12PipelineLayout(const DirectX12Device& device, Array<UniquePtr<DirectX12DescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout) :
    ComResource<ID3D12RootSignature>(nullptr), m_impl(makePimpl<DirectX12PipelineLayoutImpl>(this, device, std::move(descriptorSetLayouts), std::move(pushConstantsLayout)))
{
    this->handle() = m_impl->initialize();
}

DirectX12PipelineLayout::DirectX12PipelineLayout(const DirectX12Device& device) noexcept :
    ComResource<ID3D12RootSignature>(nullptr), m_impl(makePimpl<DirectX12PipelineLayoutImpl>(this, device))
{
}

DirectX12PipelineLayout::~DirectX12PipelineLayout() noexcept = default;

const DirectX12Device& DirectX12PipelineLayout::device() const noexcept
{
    return m_impl->m_device;
}

const DirectX12DescriptorSetLayout& DirectX12PipelineLayout::descriptorSet(const UInt32& space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const UniquePtr<DirectX12DescriptorSetLayout>& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("No descriptor set layout uses the provided space {0}.", space);
}

Array<const DirectX12DescriptorSetLayout*> DirectX12PipelineLayout::descriptorSets() const noexcept
{
    return m_impl->m_descriptorSetLayouts |
        std::views::transform([](const UniquePtr<DirectX12DescriptorSetLayout>& layout) { return layout.get(); }) |
        ranges::to<Array<const DirectX12DescriptorSetLayout*>>();
}

const DirectX12PushConstantsLayout* DirectX12PipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Pipeline layout builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PipelineLayoutBuilder::DirectX12PipelineLayoutBuilderImpl : public Implement<DirectX12PipelineLayoutBuilder> {
public:
    friend class DirectX12PipelineLayoutBuilder;
    friend class DirectX12PipelineLayout;

private:
    UniquePtr<DirectX12PushConstantsLayout> m_pushConstantsLayout;
    Array<UniquePtr<DirectX12DescriptorSetLayout>> m_descriptorSetLayouts;
    const DirectX12Device& m_device;

public:
    DirectX12PipelineLayoutBuilderImpl(DirectX12PipelineLayoutBuilder* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12PipelineLayoutBuilder::DirectX12PipelineLayoutBuilder(const DirectX12Device& parent) :
    m_impl(makePimpl<DirectX12PipelineLayoutBuilderImpl>(this, parent)), PipelineLayoutBuilder(SharedPtr<DirectX12PipelineLayout>(new DirectX12PipelineLayout(parent)))
{
}

DirectX12PipelineLayoutBuilder::~DirectX12PipelineLayoutBuilder() noexcept = default;

void DirectX12PipelineLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_descriptorSetLayouts = std::move(m_impl->m_descriptorSetLayouts);
    instance->m_impl->m_pushConstantsLayout = std::move(m_impl->m_pushConstantsLayout);
    instance->handle() = instance->m_impl->initialize();
}

void DirectX12PipelineLayoutBuilder::use(UniquePtr<DirectX12DescriptorSetLayout>&& layout)
{
    m_impl->m_descriptorSetLayouts.push_back(std::move(layout));
}

void DirectX12PipelineLayoutBuilder::use(UniquePtr<DirectX12PushConstantsLayout>&& layout)
{
    m_impl->m_pushConstantsLayout = std::move(layout);
}

DirectX12DescriptorSetLayoutBuilder DirectX12PipelineLayoutBuilder::descriptorSet(const UInt32& space, const ShaderStage& stages, const UInt32& /*poolSize*/)
{
    return DirectX12DescriptorSetLayoutBuilder(*this, space, stages);
}

DirectX12PushConstantsLayoutBuilder DirectX12PipelineLayoutBuilder::pushConstants(const UInt32& size)
{
    return DirectX12PushConstantsLayoutBuilder(*this, size);
}

const DirectX12Device& DirectX12PipelineLayoutBuilder::device() const noexcept
{
    return m_impl->m_device;
}
#endif // defined(BUILD_DEFINE_BUILDERS)