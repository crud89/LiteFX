#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSet::DirectX12DescriptorSetImpl : public Implement<DirectX12DescriptorSet> {
public:
    friend class DirectX12DescriptorSet;

private:
    ComPtr<ID3D12DescriptorHeap> m_bufferHeap, m_samplerHeap;
    UInt32 m_bufferOffset{ 0 }, m_samplerOffset{ 0 };
    const DirectX12DescriptorSetLayout& m_layout;

public:
    DirectX12DescriptorSetImpl(DirectX12DescriptorSet* parent, const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& bufferHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
        base(parent), m_layout(layout), m_bufferHeap(std::move(bufferHeap)), m_samplerHeap(std::move(samplerHeap))
    {
        auto buffers = m_layout.uniforms() + m_layout.images() + m_layout.storages() + m_layout.buffers();

        if (buffers > 0 && m_bufferHeap == nullptr)
            throw ArgumentNotInitializedException("bufferHeap", "The buffer descriptor heap handle must be initialized, if the descriptor set layout contains uniform buffers, storage buffers or images.");

        if (m_layout.samplers() > 0 && m_samplerHeap == nullptr)
            throw ArgumentNotInitializedException("samplerHeap", "The sampler descriptor heap handle must be initialized, if the descriptor set layout contains samplers.");
    }

public:
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
        default: [[unlikely]] throw InvalidArgumentException("mode", "Invalid border mode.");
        }
    }

    void updateGlobalBuffers(UInt32 offset, UInt32 descriptors)
    {
        m_layout.device().updateBufferDescriptors(*this->m_parent, offset, descriptors);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& bufferHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
    m_impl(makePimpl<DirectX12DescriptorSetImpl>(this, layout, std::move(bufferHeap), std::move(samplerHeap)))
{
    layout.device().allocateGlobalDescriptors(*this, m_impl->m_bufferOffset, m_impl->m_samplerOffset);
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept
{
    m_impl->m_layout.device().releaseGlobalDescriptors(*this);
    m_impl->m_layout.free(*this);
}

const DirectX12DescriptorSetLayout& DirectX12DescriptorSet::layout() const noexcept
{
    return m_impl->m_layout;
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const
{
    UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

    if (bufferElement + elementCount > buffer.elements()) [[unlikely]]
        LITEFX_WARNING(DIRECTX12_LOG, "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elementCount, bufferElement);

    const auto& descriptorLayout = m_impl->m_layout.descriptor(binding);
    auto offset = m_impl->m_layout.descriptorOffsetForBinding(binding) + firstDescriptor;
    auto descriptorSize = m_impl->m_layout.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptorSize);
    
    switch (descriptorLayout.descriptorType())
    {
    case DescriptorType::ConstantBuffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {
                .BufferLocation = buffer.handle()->GetGPUVirtualAddress() + static_cast<size_t>(bufferElement + i) * buffer.alignedElementSize(),
                .SizeInBytes = static_cast<UInt32>(buffer.alignedElementSize())
            };

            m_impl->m_layout.device().handle()->CreateConstantBufferView(&constantBufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::StructuredBuffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_UNKNOWN,
                .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer = { .FirstElement = bufferElement + i, .NumElements = 1, .StructureByteStride = static_cast<UInt32>(buffer.alignedElementSize()), .Flags = D3D12_BUFFER_SRV_FLAG_NONE }
            };

            m_impl->m_layout.device().handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::RWStructuredBuffer:
    {
        // TODO: Support counter in AppendStructuredBuffer.
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_UNKNOWN,
                .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                .Buffer = { .FirstElement = bufferElement + i, .NumElements = 1, .StructureByteStride = static_cast<UInt32>(buffer.alignedElementSize()), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE }
            };

            m_impl->m_layout.device().handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::ByteAddressBuffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_R32_TYPELESS,
                .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer = { .FirstElement = ((bufferElement + i) * buffer.alignedElementSize()) / 4, .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / 4), .StructureByteStride = 0, .Flags = D3D12_BUFFER_SRV_FLAG_RAW }
            };

            m_impl->m_layout.device().handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::RWByteAddressBuffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_R32_TYPELESS,
                .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                .Buffer = { .FirstElement = ((bufferElement + i) * buffer.alignedElementSize()) / 4, .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / 4), .StructureByteStride = 0, .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_RAW }
            };

            m_impl->m_layout.device().handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::Buffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_R32_TYPELESS, // TODO: Actually set the proper texel format.
                .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer = { .FirstElement = ((bufferElement + i) * buffer.alignedElementSize()) / 4, .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / 4), .StructureByteStride = 0, .Flags = D3D12_BUFFER_SRV_FLAG_RAW }
            };

            m_impl->m_layout.device().handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    case DescriptorType::RWBuffer:
    {
        for (UInt32 i(0); i < elementCount; ++i)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
                .Format = DXGI_FORMAT_R32_TYPELESS, // TODO: Actually set the proper texel format.
                .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                .Buffer = { .FirstElement = ((bufferElement + i) * buffer.alignedElementSize()) / 4, .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / 4), .StructureByteStride = 0, .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_RAW }
            };

            m_impl->m_layout.device().handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }

        break;
    }
    default: [[unlikely]]
        throw InvalidArgumentException("binding", "The descriptor at binding point {0} does not reference a buffer, uniform or storage resource.", binding);
    }

    m_impl->updateGlobalBuffers(offset, elementCount);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Image& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    // TODO: Add LOD lower bound (for clamping) as parameter?
    // Acquire a descriptor handle.
    const auto& descriptorLayout = m_impl->m_layout.descriptor(binding);
    auto offset = m_impl->m_layout.descriptorOffsetForBinding(binding);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset + descriptor, m_impl->m_layout.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    // Get the number of levels and layers.
    const UInt32 numLevels = levels == 0 ? texture.levels() - firstLevel : levels;
    const UInt32 numLayers = layers == 0 ? texture.layers() - firstLayer : layers;

    if (descriptorLayout.descriptorType() == DescriptorType::Texture)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC textureView = {
            .Format = DX12::getFormat(texture.format()),
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };

        switch (texture.dimensions())
        {
        case ImageDimensions::DIM_1:
            if (texture.layers() == 1)
            {
                textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                textureView.Texture1D = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .ResourceMinLODClamp = 0 };
            }
            else
            {
                textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                textureView.Texture1DArray = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .ResourceMinLODClamp = 0 };
            }

            break;
        case ImageDimensions::DIM_2:
            if (texture.samples() == MultiSamplingLevel::x1)
            {
                if (texture.layers() == 1)
                {
                    textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    textureView.Texture2D = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0 };
                }
                else
                {
                    textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    textureView.Texture2DArray = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0, .ResourceMinLODClamp = 0 };
                }
            }
            else
            {
                if (texture.layers() == 1)
                {
                    textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    textureView.Texture2DMS = { };
                }
                else
                {
                    textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    textureView.Texture2DMSArray = { .FirstArraySlice = firstLayer, .ArraySize = numLayers };
                }
            }

            break;
        case ImageDimensions::DIM_3:
            textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            textureView.Texture3D = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .ResourceMinLODClamp = 0 };
            break;
        case ImageDimensions::CUBE:
            textureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            textureView.TextureCube = { .MostDetailedMip = firstLevel, .MipLevels = numLevels, .ResourceMinLODClamp = 0 };
            break;
        }

        m_impl->m_layout.device().handle()->CreateShaderResourceView(texture.handle().Get(), &textureView, descriptorHandle);
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::RWTexture)
    {
        if (!texture.writable())
            throw InvalidArgumentException("binding", "The provided texture is not writable and cannot be bound to a read/write descriptor.");

        D3D12_UNORDERED_ACCESS_VIEW_DESC textureView = {
            .Format = DX12::getFormat(texture.format())
        };

        switch (texture.dimensions())
        {
        case ImageDimensions::DIM_1:
            if (texture.layers() == 1)
            {
                textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                textureView.Texture1D = { .MipSlice = firstLevel };
            }
            else
            {
                textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                textureView.Texture1DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers };
            }

            break;
        case ImageDimensions::DIM_2:
            if (texture.samples() == MultiSamplingLevel::x1)
            {
                if (texture.layers() == 1)
                {
                    textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    textureView.Texture2D = { .MipSlice = firstLevel, .PlaneSlice = 0 };
                }
                else
                {
                    textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    textureView.Texture2DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0 };
                }
            }
            else
            {
                if (texture.layers() == 1)
                {
                    textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
                    textureView.Texture2D = { .MipSlice = firstLevel, .PlaneSlice = 0 };
                }
                else
                {
                    textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
                    textureView.Texture2DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0 };
                }
            }

            break;
        case ImageDimensions::DIM_3:
            textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            textureView.Texture3D = { .MipSlice = firstLevel, .FirstWSlice = firstLayer, .WSize = layers == 0 ? static_cast<UInt32>(texture.extent().depth()) : layers };
            break;
        case ImageDimensions::CUBE:
            textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            textureView.Texture2DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0 };
            break;
        }

        m_impl->m_layout.device().handle()->CreateUnorderedAccessView(texture.handle().Get(), nullptr, &textureView, descriptorHandle);
    }
    else [[unlikely]]
    {
        throw InvalidArgumentException("binding", "The provided texture is bound to a descriptor that is does neither describe a `Texture`, nor a `WritableTexture`.");
    }

    m_impl->updateGlobalBuffers(offset, 1);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Sampler& sampler, UInt32 descriptor) const
{
    auto offset = m_impl->m_layout.descriptorOffsetForBinding(binding);

    D3D12_SAMPLER_DESC samplerInfo = {
        .Filter = m_impl->getFilterMode(sampler.getMinifyingFilter(), sampler.getMagnifyingFilter(), sampler.getMipMapMode(), sampler.getAnisotropy()),
        .AddressU = m_impl->getBorderMode(sampler.getBorderModeU()),
        .AddressV = m_impl->getBorderMode(sampler.getBorderModeV()),
        .AddressW = m_impl->getBorderMode(sampler.getBorderModeW()),
        .MipLODBias = sampler.getMipMapBias(),
        .MaxAnisotropy = static_cast<UInt32>(sampler.getAnisotropy()),
        .ComparisonFunc = D3D12_COMPARISON_FUNC_NONE,
        .BorderColor = { 0.f, 0.f, 0.f, 0.f },
        .MinLOD = sampler.getMinLOD(),
        .MaxLOD = sampler.getMaxLOD()
    };

    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_samplerHeap->GetCPUDescriptorHandleForHeapStart(), offset + descriptor, m_impl->m_layout.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
    m_impl->m_layout.device().handle()->CreateSampler(&samplerInfo, descriptorHandle);
    m_impl->m_layout.device().updateSamplerDescriptors(*this, offset, 1);
}

void DirectX12DescriptorSet::attach(UInt32 binding, const IDirectX12Image& image) const
{
    auto offset = m_impl->m_layout.descriptorOffsetForBinding(binding);

    D3D12_SHADER_RESOURCE_VIEW_DESC inputAttachmentView = {
        .Format = DX12::getFormat(image.format()),
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Texture2D = { .MostDetailedMip = 0, .MipLevels = 1, .PlaneSlice = 0, .ResourceMinLODClamp = 0 },
    };

    // DSV needs special care.
    // TODO: Support stencil targets, which require a separate view.
    if (::hasDepth(image.format()))
    {
        switch (image.format())
        {
        case Format::D16_UNORM:             inputAttachmentView.Format = DXGI_FORMAT_R16_UNORM; break;
        case Format::D32_SFLOAT:            inputAttachmentView.Format = DXGI_FORMAT_R32_FLOAT; break;
        case Format::D24_UNORM_S8_UINT:     inputAttachmentView.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break;
        case Format::D32_SFLOAT_S8_UINT:    inputAttachmentView.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; break;
        //case Format::D16_UNORM_S8_UINT: ??
        //case Format::X8_D24_UNORM: ??
        }
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset, m_impl->m_layout.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    m_impl->m_layout.device().handle()->CreateShaderResourceView(image.handle().Get(), &inputAttachmentView, descriptorHandle);
    m_impl->updateGlobalBuffers(offset, 1);
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::bufferHeap() const noexcept 
{
    return m_impl->m_bufferHeap;
}

UInt32 DirectX12DescriptorSet::bufferOffset() const noexcept
{
    return m_impl->m_bufferOffset;
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::samplerHeap() const noexcept
{
    return m_impl->m_samplerHeap;
}

UInt32 DirectX12DescriptorSet::samplerOffset() const noexcept
{
    return m_impl->m_samplerOffset;
}