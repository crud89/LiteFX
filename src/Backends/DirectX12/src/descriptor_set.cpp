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

public:
    DirectX12DescriptorSetImpl(DirectX12DescriptorSet* parent, ComPtr<ID3D12DescriptorHeap>&& bufferHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
        base(parent), m_bufferHeap(std::move(bufferHeap)), m_samplerHeap(std::move(samplerHeap))
    {
        auto buffers = parent->parent().uniforms() + parent->parent().images() + parent->parent().storages() + parent->parent().buffers();

        if (buffers > 0 && m_bufferHeap == nullptr)
            throw ArgumentNotInitializedException("The buffer descriptor heap handle must be initialized, if the descriptor set layout contains uniform buffers, storage buffers or images.");

        if (parent->parent().samplers() > 0 && m_samplerHeap == nullptr)
            throw ArgumentNotInitializedException("The sampler descriptor heap handle must be initialized, if the descriptor set layout contains samplers.");
    }

public:
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
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& bufferHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
    m_impl(makePimpl<DirectX12DescriptorSetImpl>(this, std::move(bufferHeap), std::move(samplerHeap))), DirectX12RuntimeObject(layout, layout.getDevice())
{
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept
{
    this->parent().free(*this);
}

void DirectX12DescriptorSet::update(const UInt32& binding, const IDirectX12Buffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const
{
    if (bufferElement + elements > buffer.elements()) [[unlikely]]
        LITEFX_WARNING(DIRECTX12_LOG, "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elements, bufferElement);

    const auto& descriptorLayout = this->parent().descriptor(binding);
    auto offset = this->parent().descriptorOffsetForBinding(binding) + firstDescriptor;
    auto descriptorSize = this->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptorSize);

    if (descriptorLayout.descriptorType() == DescriptorType::Uniform)
    {
        for (UInt32 i(0); i < elements; ++i)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {
                .BufferLocation = buffer.handle()->GetGPUVirtualAddress() + static_cast<size_t>(bufferElement + i) * buffer.alignedElementSize(),
                .SizeInBytes = static_cast<UInt32>(buffer.alignedElementSize())
            };

            this->getDevice()->handle()->CreateConstantBufferView(&constantBufferView, descriptorHandle);
            descriptorHandle = descriptorHandle.Offset(descriptorSize);
        }
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::Storage || descriptorLayout.descriptorType() == DescriptorType::Buffer)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = { .FirstElement = bufferElement, .NumElements = elements, .StructureByteStride = static_cast<UInt32>(descriptorLayout.elementSize()), .Flags = D3D12_BUFFER_SRV_FLAG_NONE }
        };

        this->getDevice()->handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::WritableStorage || descriptorLayout.descriptorType() == DescriptorType::WritableBuffer)
    {
        if (!buffer.writable())
            throw InvalidArgumentException("The provided buffer is not writable and cannot be bound to a read/write descriptor.");

        // TODO: Support allocating counter resources?
        D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
            .Buffer = { .FirstElement = bufferElement, .NumElements = elements, .StructureByteStride = static_cast<UInt32>(descriptorLayout.elementSize()), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE }
        };

        this->getDevice()->handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
    }
    else [[unlikely]]
    {
        throw InvalidArgumentException("The descriptor at binding point {0} does not reference a buffer, uniform or storage resource.", binding);
    }
}

void DirectX12DescriptorSet::update(const UInt32& binding, const IDirectX12Image& texture, const UInt32& descriptor, const UInt32& firstLevel, const UInt32& levels, const UInt32& firstLayer, const UInt32& layers) const
{
    // TODO: Add LOD lower bound (for clamping) as parameter?
    // Acquire a descriptor handle.
    const auto& descriptorLayout = this->parent().descriptor(binding);
    auto offset = this->parent().descriptorOffsetForBinding(binding);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset + descriptor, this->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

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

        this->getDevice()->handle()->CreateShaderResourceView(texture.handle().Get(), &textureView, descriptorHandle);
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::WritableTexture)
    {
        if (!texture.writable())
            throw InvalidArgumentException("The provided texture is not writable and cannot be bound to a read/write descriptor.");

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

        this->getDevice()->handle()->CreateUnorderedAccessView(texture.handle().Get(), nullptr, &textureView, descriptorHandle);
    }
    else
    {
        throw InvalidArgumentException("The provided texture is bound to a descriptor that is does neither describe a `Texture`, nor a `WritableTexture`.");
    }
}

void DirectX12DescriptorSet::update(const UInt32& binding, const IDirectX12Sampler& sampler, const UInt32& descriptor) const
{
    auto offset = this->parent().descriptorOffsetForBinding(binding);

    D3D12_SAMPLER_DESC samplerInfo = {
        .Filter = m_impl->getFilterMode(sampler.getMinifyingFilter(), sampler.getMagnifyingFilter(), sampler.getMipMapMode(), sampler.getAnisotropy()),
        .AddressU = m_impl->getBorderMode(sampler.getBorderModeU()),
        .AddressV = m_impl->getBorderMode(sampler.getBorderModeV()),
        .AddressW = m_impl->getBorderMode(sampler.getBorderModeW()),
        .MipLODBias = sampler.getMipMapBias(),
        .MaxAnisotropy = static_cast<UInt32>(sampler.getAnisotropy()),
        .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
        .BorderColor = { 0.f, 0.f, 0.f, 0.f },
        .MinLOD = sampler.getMinLOD(),
        .MaxLOD = sampler.getMaxLOD()
    };

    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_samplerHeap->GetCPUDescriptorHandleForHeapStart(), offset + descriptor, this->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
    this->getDevice()->handle()->CreateSampler(&samplerInfo, descriptorHandle);
}

void DirectX12DescriptorSet::attach(const UInt32& binding, const IDirectX12Image& image) const
{
    auto offset = this->parent().descriptorOffsetForBinding(binding);

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

    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), offset, this->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    this->getDevice()->handle()->CreateShaderResourceView(image.handle().Get(), &inputAttachmentView, descriptorHandle);
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::bufferHeap() const noexcept 
{
    return m_impl->m_bufferHeap;
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::samplerHeap() const noexcept
{
    return m_impl->m_samplerHeap;
}