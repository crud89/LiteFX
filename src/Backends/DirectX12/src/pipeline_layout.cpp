#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PipelineLayout::DirectX12PipelineLayoutImpl {
public:
    friend class DirectX12PipelineLayoutBuilder;
    friend class DirectX12PipelineLayout;

private:
    UniquePtr<DirectX12PushConstantsLayout> m_pushConstantsLayout{};
    Array<SharedPtr<const DirectX12DescriptorSetLayout>> m_descriptorSetLayouts{};
    SharedPtr<const DirectX12Device> m_device;

    /// <summary>
    /// The flags for a root parameter entry.
    /// </summary>
    enum class RootParameterFlags : UInt32
    {
        /// <summary>
        /// Indicates that the root parameter is a root/push constant. Must not be combined with <see cref="IsResourceTable" /> or <see cref="IsSamplerTable" />.
        /// </summary>
        IsRootConstant = 0x00000001,

        /// <summary>
        /// Indicates that the root parameter is a resource table. Must not be combined with <see cref="IsRootConstant" /> or <see cref="IsSamplerTable" />.
        /// </summary>
        IsResourceTable = 0x00000010,

        /// <summary>
        /// Indicates that the root parameter is a sampler table. Must not be combined with <see cref="IsRootConstant" /> or <see cref="IsResourceTable" />.
        /// </summary>
        IsSamplerTable = 0x00000020,
    };

    /// <summary>
    /// Generates a root parameter identifier from a set of flags and the descriptor space.
    /// </summary>
    /// <param name="flags">The flags that store metadata about the root parameter.</param>
    /// <param name="descriptorSpace">The target space to bind the root descriptor to.</param>
    /// <returns>The root parameter identifier.</returns>
    constexpr static UInt64 makeRootParameterId(RootParameterFlags flags, UInt32 descriptorSpace) noexcept {
        return (static_cast<UInt64>(std::to_underlying(flags)) << 32) | static_cast<UInt64>(descriptorSpace); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }
    
    /// <summary>
    /// Maps the indices of the root parameters for a descriptor set or a push constant range.
    /// </summary>
    Dictionary<UInt64, UInt32> m_rootParameterIndices{};

public:
    DirectX12PipelineLayoutImpl(const DirectX12Device& device) :
        m_device(device.shared_from_this())
    {
    }

private:
    D3D12_FILTER getFilterMode(FilterMode minFilter, FilterMode magFilter, MipMapMode mipFilter, Float anisotropy = 0.f)
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

    D3D12_TEXTURE_ADDRESS_MODE getBorderMode(BorderMode mode)
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
    ComPtr<ID3D12RootSignature> initialize([[maybe_unused]] const DirectX12PipelineLayout& pipelineLayout, Array<SharedPtr<DirectX12DescriptorSetLayout>> descriptorLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout)
    {
        if (std::ranges::any_of(descriptorLayouts, [](const auto& layout) { return layout == nullptr; })) [[unlikely]]
            throw ArgumentNotInitializedException("descriptorLayouts", "At least one of the provided descriptor layouts is not initialized.");

        // Sort and check if there are duplicate space indices.
        std::ranges::sort(descriptorLayouts, [](const auto& a, const auto& b) { return a->space() < b->space(); });

        for (Tuple<UInt32, UInt32> spaces : descriptorLayouts |
            std::views::transform([](const auto& layout) { return layout->space(); }) | 
            std::views::adjacent_transform<2>([](UInt32 a, UInt32 b) { return std::make_tuple(a, b); }))
        {
            auto [a, b] = spaces;

            if (a == b) [[unlikely]]
                throw InvalidArgumentException("descriptorSetLayouts", "Two layouts defined for the same descriptor set {}. Each descriptor set must use it's own space.", a);
        }

        // Define the descriptor range from descriptor set layouts.
        // NOTE: The following arrays keep the description structs alive, so they don't go out of scope.
        Array<D3D12_ROOT_PARAMETER1> descriptorParameters;
        Array<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        Array<Array<D3D12_DESCRIPTOR_RANGE1>> descriptorRanges;
        bool hasInputAttachments = false;
        bool hasInputAttachmentSampler = false;
        UInt32 rootParameterIndex{ 0 };

        LITEFX_TRACE(DIRECTX12_LOG, "Creating render pipeline layout {0} {{ Descriptor Sets: {1}, Push Constant Ranges: {2} }}...", 
            static_cast<const void*>(&pipelineLayout), descriptorLayouts.size(), pushConstantsLayout == nullptr ? 0 : pushConstantsLayout->ranges().size());

        if (pushConstantsLayout != nullptr)
        {
            std::ranges::for_each(pushConstantsLayout->ranges(), [&](const auto& range) {
                CD3DX12_ROOT_PARAMETER1 rootParameter = {};

                switch (range->stage())
                {
                case ShaderStage::Vertex: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_VERTEX); break;
                case ShaderStage::Geometry: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_GEOMETRY); break;
                case ShaderStage::Fragment: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_PIXEL); break;
                case ShaderStage::TessellationEvaluation: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_DOMAIN); break;
                case ShaderStage::TessellationControl: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_HULL); break;
                case ShaderStage::Task: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_AMPLIFICATION); break;
                case ShaderStage::Mesh: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_MESH); break;
                
                // Combinations of shader stages need to be visible everywhere. Note that this includes ray-tracing shaders (https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#note-on-shader-visibility).
                default: rootParameter.InitAsConstants(range->size() / 4, range->binding(), range->space(), D3D12_SHADER_VISIBILITY_ALL); break;
                }

                // Store the range. Note we do not check for duplicates here.
                auto key = makeRootParameterId(RootParameterFlags::IsRootConstant, range->space());
                m_rootParameterIndices[key] = rootParameterIndex++;
                descriptorParameters.push_back(rootParameter);
            });
        }

        std::ranges::for_each(descriptorLayouts, [&](const SharedPtr<DirectX12DescriptorSetLayout>& layout) {
            // Parse the shader stage descriptor.
            D3D12_SHADER_VISIBILITY shaderStages = D3D12_SHADER_VISIBILITY_ALL;
            auto stages = layout->shaderStages();
            UInt32 space = layout->space();

            switch (stages)
            {
            case ShaderStage::Vertex: shaderStages = D3D12_SHADER_VISIBILITY_VERTEX; break;
            case ShaderStage::Geometry: shaderStages = D3D12_SHADER_VISIBILITY_GEOMETRY; break;
            case ShaderStage::Fragment: shaderStages = D3D12_SHADER_VISIBILITY_PIXEL; break;
            case ShaderStage::TessellationEvaluation: shaderStages = D3D12_SHADER_VISIBILITY_DOMAIN; break;
            case ShaderStage::TessellationControl: shaderStages = D3D12_SHADER_VISIBILITY_HULL; break;
            case ShaderStage::Task: shaderStages = D3D12_SHADER_VISIBILITY_AMPLIFICATION; break;
            case ShaderStage::Mesh: shaderStages = D3D12_SHADER_VISIBILITY_MESH; break;
            default: break;
            }

            // Define the root parameter ranges. Those ranges encode the individual binding points, i.e., scalar bindings, static or unbounded arrays. Each range represents a singular binding
            // point. However, we need to keep in mind, that samplers need to be bound at a different heap. In case samplers are mixed with resources in a single descriptor heap, we need to
            // create separate root descriptor tables, as described here: https://learn.microsoft.com/en-us/windows/win32/direct3d12/example-root-signatures#binding-descriptor-tables.
            auto layouts = layout->descriptors();
            Array<D3D12_DESCRIPTOR_RANGE1> resourceSet = layouts 
                | std::views::filter([](auto& range) { return range.descriptorType() != DescriptorType::Sampler && !range.local(); }) 
                | std::views::transform([&](auto& range) {
                    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange = {};

                    switch(range.descriptorType()) 
                    { 
                    case DescriptorType::ConstantBuffer:    descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, range.descriptors(), range.binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                    case DescriptorType::InputAttachment:   hasInputAttachments = true; [[fallthrough]];
                    case DescriptorType::AccelerationStructure:
                    case DescriptorType::Buffer:
                    case DescriptorType::StructuredBuffer:
                    case DescriptorType::ByteAddressBuffer:
                    case DescriptorType::Texture:           descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, range.descriptors(), range.binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                    case DescriptorType::RWBuffer:
                    case DescriptorType::RWStructuredBuffer:
                    case DescriptorType::RWByteAddressBuffer:
                    case DescriptorType::RWTexture:         descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, range.descriptors(), range.binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
                    default: throw InvalidArgumentException("descriptorSetLayouts", "Invalid descriptor type: {0}.", range.descriptorType());
                    }

                    return descriptorRange;
                })
                | std::ranges::to<Array<D3D12_DESCRIPTOR_RANGE1>>();

            Array<D3D12_DESCRIPTOR_RANGE1> samplerSet = layouts 
                | std::views::filter([](auto& range) { return range.descriptorType() == DescriptorType::Sampler && range.staticSampler() == nullptr && !range.local(); }) 
                | std::views::transform([&](auto& range) { 
                    return CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, range.descriptors(), range.binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); }) 
                | std::ranges::to<Array<D3D12_DESCRIPTOR_RANGE1>>();

            // Define the static samplers. Those do not occur within the descriptor table and instead are part of the pipeline state object, so we handle them separately.
            std::ranges::for_each(layouts | std::views::filter([](auto& range) { return range.staticSampler() != nullptr; }), [&](auto& range) {
                // Remember, that there's a manually defined input attachment sampler.
                if (range.binding() == 0 && space == 0)
                    hasInputAttachmentSampler = true;

                auto sampler = range.staticSampler();

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
                    .ShaderRegister = range.binding(),
                    .RegisterSpace = space,
                    .ShaderVisibility = shaderStages
                };

                staticSamplers.push_back(samplerInfo);
            });

            // Define the root parameter(s).
            if (!resourceSet.empty())
            {
                CD3DX12_ROOT_PARAMETER1 rootParameter = {};
                rootParameter.InitAsDescriptorTable(static_cast<UINT>(resourceSet.size()), resourceSet.data(), static_cast<D3D12_SHADER_VISIBILITY>(shaderStages));
                descriptorRanges.push_back(std::move(resourceSet));

                // Store the set. Note we do not check for duplicates here.
                auto key = makeRootParameterId(RootParameterFlags::IsResourceTable, layout->space());
                m_rootParameterIndices[key] = rootParameterIndex++;
                descriptorParameters.push_back(rootParameter);
            }

            if (!samplerSet.empty())
            {
                CD3DX12_ROOT_PARAMETER1 rootParameter = {};
                rootParameter.InitAsDescriptorTable(static_cast<UINT>(samplerSet.size()), samplerSet.data(), static_cast<D3D12_SHADER_VISIBILITY>(shaderStages));
                descriptorRanges.push_back(std::move(samplerSet));

                // Store the set. Note we do not check for duplicates here.
                auto key = makeRootParameterId(RootParameterFlags::IsSamplerTable, layout->space());
                m_rootParameterIndices[key] = rootParameterIndex++;
                descriptorParameters.push_back(rootParameter);
            }
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
            errorString = String(static_cast<TCHAR*>(error->GetBufferPointer()), error->GetBufferSize());
        
        raiseIfFailed(hr, "Unable to serialize root signature to create pipeline layout: {0}", errorString);

        // Create the root signature.
        ComPtr<ID3D12RootSignature> rootSignature;
        raiseIfFailed(m_device->handle()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Unable to create root signature for pipeline layout.");

        // Store the layouts.
        m_pushConstantsLayout = std::move(pushConstantsLayout);
        m_descriptorSetLayouts.append_range(std::move(descriptorLayouts));

        return rootSignature;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12PipelineLayout::DirectX12PipelineLayout(const DirectX12Device& device, const Enumerable<SharedPtr<DirectX12DescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout) :
    ComResource<ID3D12RootSignature>(nullptr), m_impl(device)
{
    this->handle() = m_impl->initialize(*this, descriptorSetLayouts | std::ranges::to<std::vector>(), std::move(pushConstantsLayout));
}

DirectX12PipelineLayout::DirectX12PipelineLayout(const DirectX12Device& device) :
    ComResource<ID3D12RootSignature>(nullptr), m_impl(device)
{
}

DirectX12PipelineLayout::~DirectX12PipelineLayout() noexcept = default;

const DirectX12Device& DirectX12PipelineLayout::device() const noexcept
{
    return *m_impl->m_device;
}

const DirectX12DescriptorSetLayout& DirectX12PipelineLayout::descriptorSet(UInt32 space) const
{
    if (auto match = std::ranges::find_if(m_impl->m_descriptorSetLayouts, [&space](const auto& layout) { return layout->space() == space; }); match != m_impl->m_descriptorSetLayouts.end())
        return *match->get();

    throw ArgumentOutOfRangeException("space", "No descriptor set layout uses the provided space {0}.", space);
}

const Array<SharedPtr<const DirectX12DescriptorSetLayout>>& DirectX12PipelineLayout::descriptorSets() const
{
    return m_impl->m_descriptorSetLayouts;
}

const DirectX12PushConstantsLayout* DirectX12PipelineLayout::pushConstants() const noexcept
{
    return m_impl->m_pushConstantsLayout.get();
}

Optional<UInt32> DirectX12PipelineLayout::rootParameterIndex(const DirectX12DescriptorSetLayout& layout, DescriptorHeapType heapType) const noexcept
{
    if (heapType == DescriptorHeapType::None)
        return std::nullopt;

    auto key = DirectX12PipelineLayoutImpl::makeRootParameterId(heapType == DescriptorHeapType::Resource ? DirectX12PipelineLayoutImpl::RootParameterFlags::IsResourceTable : DirectX12PipelineLayoutImpl::RootParameterFlags::IsSamplerTable, layout.space());

    if (auto match = m_impl->m_rootParameterIndices.find(key); match != m_impl->m_rootParameterIndices.end())
        return match->second;
    else
        return std::nullopt;
}

Optional<UInt32> DirectX12PipelineLayout::rootParameterIndex(const DirectX12PushConstantsRange& range) const noexcept
{
    if (auto match = m_impl->m_rootParameterIndices.find(DirectX12PipelineLayoutImpl::makeRootParameterId(DirectX12PipelineLayoutImpl::RootParameterFlags::IsRootConstant, range.space())); match != m_impl->m_rootParameterIndices.end())
        return match->second;
    else
        return std::nullopt;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Pipeline layout builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12PipelineLayoutBuilder::DirectX12PipelineLayoutBuilderImpl {
public:
    friend class DirectX12PipelineLayoutBuilder;
    friend class DirectX12PipelineLayout;

private:
    SharedPtr<const DirectX12Device> m_device;

public:
    DirectX12PipelineLayoutBuilderImpl(const DirectX12Device& device) :
        m_device(device.shared_from_this())
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Pipeline layout builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12PipelineLayoutBuilder::DirectX12PipelineLayoutBuilder(const DirectX12Device& parent) :
    PipelineLayoutBuilder(DirectX12PipelineLayout::create(parent)), m_impl(parent)
{
}

DirectX12PipelineLayoutBuilder::~DirectX12PipelineLayoutBuilder() noexcept = default;

void DirectX12PipelineLayoutBuilder::build()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize(*instance, std::move(this->state().descriptorSetLayouts), std::move(this->state().pushConstantsLayout));
}

DirectX12DescriptorSetLayoutBuilder DirectX12PipelineLayoutBuilder::descriptorSet(UInt32 space, ShaderStage stages)
{
    return DirectX12DescriptorSetLayoutBuilder(*this, space, stages);
}

DirectX12PushConstantsLayoutBuilder DirectX12PipelineLayoutBuilder::pushConstants(UInt32 size)
{
    return DirectX12PushConstantsLayoutBuilder(*this, size);
}

SharedPtr<const DirectX12Device> DirectX12PipelineLayoutBuilder::device() const noexcept
{
    return m_impl->m_device;
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)