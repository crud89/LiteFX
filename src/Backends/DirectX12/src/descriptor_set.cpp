#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSet::DirectX12DescriptorSetImpl {
public:
    friend class DirectX12DescriptorSet;

private:
    struct LocalHeap {
        ComPtr<ID3D12DescriptorHeap> Heap{};
        UInt32 Offset{ 0 }, Size{ 0 };
    } m_resourceHeap{}, m_samplerHeap{};

    SharedPtr<const DirectX12DescriptorSetLayout> m_layout;

public:
    DirectX12DescriptorSetImpl(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& resourceHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
        m_resourceHeap{ .Heap = std::move(resourceHeap) }, m_samplerHeap{ .Heap = std::move(samplerHeap) }, m_layout(layout.shared_from_this())
    {
        if (m_layout->bindsResources() && m_resourceHeap.Heap == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("resourceHeap", "The local resource heap must be initialized, if the descriptor set binds resources.");

        if (m_layout->bindsSamplers() && m_samplerHeap.Heap == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("samplerHeap", "The local sampler heap must be initialized, if the descriptor set binds samplers.");
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

    inline UInt32 globalHeapOffset(DescriptorHeapType heapType) const noexcept
    {
        switch (heapType)
        {
        case DescriptorHeapType::Resource:
            return m_resourceHeap.Offset;
        case DescriptorHeapType::Sampler:
            return m_samplerHeap.Offset;
        default:
            return std::numeric_limits<UInt32>::max();
        }
    }

    inline UInt32 globalHeapAddressRange(DescriptorHeapType heapType) const noexcept
    {
        switch (heapType)
        {
        case DescriptorHeapType::Resource:
            return m_resourceHeap.Size;
        case DescriptorHeapType::Sampler:
            return m_samplerHeap.Size;
        default:
            return 0u;
        }
    }

    UInt32 updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 firstDescriptor, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements)
    {
        // Validate the buffer element bounds.
        UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

        if (bufferElement + elementCount > buffer.elements()) [[unlikely]]
            throw InvalidArgumentException("bufferElement", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elementCount, bufferElement);

        // Validate the descriptor index.
        if (firstDescriptor + elementCount > descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("firstDescriptor", "The descriptor array only has {0} elements, however there are {1} elements starting at descriptor {2} specified.", descriptorLayout.descriptors(), elementCount, firstDescriptor);

        // Get the descriptor handle for binding.
        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(descriptorLayout.binding(), firstDescriptor);
        auto descriptorSize = device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_resourceHeap.Heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), descriptorSize);
    
        // Create a resource view based on the provided binding type.
        switch (bindingType)
        {
        case DescriptorType::ConstantBuffer:
        {
            for (UInt32 i(0); i < elementCount; ++i)
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {
                    .BufferLocation = buffer.handle()->GetGPUVirtualAddress() + static_cast<size_t>(bufferElement + i) * buffer.alignedElementSize(),
                    .SizeInBytes = static_cast<UInt32>(buffer.alignedElementSize())
                };

                device->handle()->CreateConstantBufferView(&constantBufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
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

                device->handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
            }

            break;
        }
        case DescriptorType::RWStructuredBuffer:
        {
            for (UInt32 i(0); i < elementCount; ++i)
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
                    .Format = DXGI_FORMAT_UNKNOWN,
                    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                    .Buffer = { .FirstElement = bufferElement + i, .NumElements = 1, .StructureByteStride = static_cast<UInt32>(buffer.alignedElementSize()), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE }
                };

                device->handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
            }

            break;
        }
        case DescriptorType::ByteAddressBuffer:
        {
            for (UInt32 i(0); i < elementCount; ++i)
            {
                // NOTE: One takes 4 byte size (sizeof(DWORD)) in DXGI_FORMAT_R32_TYPELESS format, which is required for raw buffers.
                D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
                    .Format = DXGI_FORMAT_R32_TYPELESS,
                    .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                    .Buffer = { .FirstElement = (bufferElement + i) * sizeof(DWORD), .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / sizeof(DWORD)), .StructureByteStride = 0, .Flags = D3D12_BUFFER_SRV_FLAG_RAW }
                };

                device->handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
            }

            break;
        }
        case DescriptorType::RWByteAddressBuffer:
        {
            for (UInt32 i(0); i < elementCount; ++i)
            {
                // NOTE: Individual fields in a buffer are always required to be 4 bytes wide, while alignment between elements is 16 bytes (D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT).
                D3D12_UNORDERED_ACCESS_VIEW_DESC bufferView = {
                    .Format = DXGI_FORMAT_R32_TYPELESS,
                    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                    .Buffer = { .FirstElement = (bufferElement + i) * sizeof(DWORD), .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / sizeof(DWORD)), .StructureByteStride = 0, .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_RAW }
                };

                device->handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
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
                    .Buffer = { .FirstElement = (bufferElement + i) * sizeof(DWORD), .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / sizeof(DWORD)), .StructureByteStride = 0 }
                };

                device->handle()->CreateShaderResourceView(buffer.handle().Get(), &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
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
                    .Buffer = { .FirstElement = (bufferElement + i) * sizeof(DWORD), .NumElements = static_cast<UInt32>(buffer.alignedElementSize() / sizeof(DWORD)), .StructureByteStride = 0, .CounterOffsetInBytes = 0 }
                };

                device->handle()->CreateUnorderedAccessView(buffer.handle().Get(), nullptr, &bufferView, descriptorHandle);
                descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));
            }

            break;
        }
        default: [[unlikely]]
            throw InvalidArgumentException("bindingType", "The provided binding type {0} cannot be bound to a buffer resource.", bindingType);
        }

        // Update the descriptor in the global descriptor heap.
        m_layout->device()->updateGlobalDescriptors(parent, descriptorLayout.binding(), firstDescriptor, elementCount);

        // Return the global descriptor offset in the global descriptor heap.
        return this->globalHeapOffset(DescriptorHeapType::Resource) + offset;
    }

    UInt32 updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 descriptor, const IDirectX12Image& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        // Validate the descriptor index.
        if (descriptor >= descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The resource descriptor heap only contains {1} descriptors.", descriptor, descriptorLayout.descriptors());

        // Get the descriptor handle for binding.
        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(descriptorLayout.binding(), descriptor);
        auto descriptorSize = device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_resourceHeap.Heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), descriptorSize);

        // Get the number of levels and layers.
        const UInt32 numLevels = levels == 0 ? image.levels() - firstLevel : levels;
        const UInt32 numLayers = layers == 0 ? image.layers() - firstLayer : layers;

        // Create a resource view in the global descriptor heap.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        if (bindingType == DescriptorType::Texture || bindingType == DescriptorType::InputAttachment)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC textureView = {
                .Format = DX12::getFormat(image.format()),
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            };

            // Handle depth images.
            if (::hasDepth(image.format()))
            {
                switch (image.format())
                {
                case Format::D16_UNORM:             textureView.Format = DXGI_FORMAT_R16_UNORM; break;
                case Format::D32_SFLOAT:            textureView.Format = DXGI_FORMAT_R32_FLOAT; break;
                case Format::D24_UNORM_S8_UINT:     textureView.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break;
                case Format::D32_SFLOAT_S8_UINT:    textureView.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; break;
                case Format::D16_UNORM_S8_UINT:
                case Format::X8_D24_UNORM:
                    throw InvalidArgumentException("image", "Unsupported format {0}.", image.format());
                default:
                    std::unreachable();
                }
            }

            switch (image.dimensions())
            {
            case ImageDimensions::DIM_1:
                if (image.layers() == 1)
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
                if (image.samples() == MultiSamplingLevel::x1)
                {
                    if (image.layers() == 1)
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
                    if (image.layers() == 1)
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

            device->handle()->CreateShaderResourceView(image.handle().Get(), &textureView, descriptorHandle);
        }
        else if (bindingType == DescriptorType::RWTexture)
        {
            if (!image.writable()) [[unlikely]]
                throw InvalidArgumentException("binding", "The provided texture is not writable and cannot be bound to a read/write descriptor.");

            D3D12_UNORDERED_ACCESS_VIEW_DESC textureView = {
                .Format = DX12::getFormat(image.format())
            };

            switch (image.dimensions())
            {
            case ImageDimensions::DIM_1:
                if (image.layers() == 1)
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
                if (image.samples() == MultiSamplingLevel::x1)
                {
                    if (image.layers() == 1)
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
                    if (image.layers() == 1)
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
                textureView.Texture3D = { .MipSlice = firstLevel, .FirstWSlice = firstLayer, .WSize = layers == 0 ? static_cast<UInt32>(image.extent().depth()) : layers };
                break;
            case ImageDimensions::CUBE:
                textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                textureView.Texture2DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0 };
                break;
            }

            device->handle()->CreateUnorderedAccessView(image.handle().Get(), nullptr, &textureView, descriptorHandle);
        }
        else [[unlikely]]
        {
            throw InvalidArgumentException("binding", "The provided texture is bound to a descriptor that is does neither describe a `Texture`, nor a `WritableTexture`.");
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)

        // Update the descriptor in the global descriptor heap.
        m_layout->device()->updateGlobalDescriptors(parent, descriptorLayout.binding(), descriptor, 1u);

        // Return the global descriptor offset in the global descriptor heap.
        return this->globalHeapOffset(DescriptorHeapType::Resource) + offset;
    }

    UInt32 updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, UInt32 descriptor, const IDirectX12Sampler& sampler)
    {
        // Validate the descriptor index.
        if (descriptor >= descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), descriptorLayout.binding(), descriptorLayout.descriptors(), descriptor);
        
        // Validate the descriptor type.
        if (descriptorLayout.descriptorType() != DescriptorType::Sampler && descriptorLayout.descriptorType() != DescriptorType::SamplerDescriptorHeap) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not bind a sampler or sampler heap, but rather a {1}.", descriptorLayout.binding(), descriptorLayout.descriptorType());

        // Validate the static sampler state.
        if (descriptorLayout.descriptorType() == DescriptorType::Sampler && descriptorLayout.staticSampler() != nullptr)
            throw InvalidArgumentException("descriptorLayout", "The provided binding does bind a sampler, but also defines a static sampler, which makes binding to it invalid.");

        // Get the descriptor handle for binding.
        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(descriptorLayout.binding(), descriptor);
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_samplerHeap.Heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

        // Create a sampler state.
        D3D12_SAMPLER_DESC samplerInfo = {
            .Filter = getFilterMode(sampler.getMinifyingFilter(), sampler.getMagnifyingFilter(), sampler.getMipMapMode(), sampler.getAnisotropy()),
            .AddressU = getBorderMode(sampler.getBorderModeU()),
            .AddressV = getBorderMode(sampler.getBorderModeV()),
            .AddressW = getBorderMode(sampler.getBorderModeW()),
            .MipLODBias = sampler.getMipMapBias(),
            .MaxAnisotropy = static_cast<UInt32>(sampler.getAnisotropy()),
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NONE,
            .BorderColor = { 0.f, 0.f, 0.f, 0.f },
            .MinLOD = sampler.getMinLOD(),
            .MaxLOD = sampler.getMaxLOD()
        };

        device->handle()->CreateSampler(&samplerInfo, descriptorHandle);

        // Update the descriptor in the global descriptor heap.
        device->updateGlobalDescriptors(parent, descriptorLayout.binding(), descriptor, 1u);

        // Return the global descriptor offset in the global descriptor heap.
        return this->globalHeapOffset(DescriptorHeapType::Sampler) + offset;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& resourceHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap) :
    m_impl(layout, std::move(resourceHeap), std::move(samplerHeap))
{
    if (layout.bindsResources())
        layout.device()->allocateGlobalDescriptors(*this, DescriptorHeapType::Resource, m_impl->m_resourceHeap.Offset, m_impl->m_resourceHeap.Size);

    if (layout.bindsSamplers())
        layout.device()->allocateGlobalDescriptors(*this, DescriptorHeapType::Sampler, m_impl->m_samplerHeap.Offset, m_impl->m_samplerHeap.Size);
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept
{
    m_impl->m_layout->device()->releaseGlobalDescriptors(*this);
    m_impl->m_layout->free(*this);
}

const DirectX12DescriptorSetLayout& DirectX12DescriptorSet::layout() const noexcept
{
    return *m_impl->m_layout;
}

UInt32 DirectX12DescriptorSet::globalHeapOffset(DescriptorHeapType heapType) const noexcept
{
    return m_impl->globalHeapOffset(heapType);
}

UInt32 DirectX12DescriptorSet::globalHeapAddressRange(DescriptorHeapType heapType) const noexcept
{
    return m_impl->globalHeapAddressRange(heapType);
}

UInt32 DirectX12DescriptorSet::bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements) const
{
    // Find the resource descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ResourceDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a resource heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), bindingType, descriptor, buffer, bufferElement, elements);
}

UInt32 DirectX12DescriptorSet::bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IDirectX12Image& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    // Find the resource descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ResourceDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a resource heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), bindingType, descriptor, image, firstLevel, levels, firstLayer, layers);
}

UInt32 DirectX12DescriptorSet::bindToHeap(UInt32 descriptor, const IDirectX12Sampler& sampler) const
{
    // Find the sampler descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::SamplerDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a sampler heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), descriptor, sampler);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto match = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (match == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(DIRECTX12_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    if (match->descriptorType() == DescriptorType::ResourceDescriptorHeap || match->descriptorType() == DescriptorType::SamplerDescriptorHeap) [[unlikely]]
        throw InvalidArgumentException("binding", "Resources that are bound to descriptor heaps directly must use `bindToHeap`.");

    // Update the binding.
    m_impl->updateBinding(*this, (*match), match->descriptorType(), firstDescriptor, buffer, bufferElement, elements);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Image& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    // TODO: Add LOD lower bound (for clamping) as parameter?
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto match = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (match == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(DIRECTX12_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    if (match->descriptorType() == DescriptorType::ResourceDescriptorHeap || match->descriptorType() == DescriptorType::SamplerDescriptorHeap) [[unlikely]]
        throw InvalidArgumentException("binding", "Resources that are bound to descriptor heaps directly must use `bindToHeap`.");

    // Update the binding.
    m_impl->updateBinding(*this, (*match), match->descriptorType(),descriptor, texture, firstLevel, levels, firstLayer, layers);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12Sampler& sampler, UInt32 descriptor) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto match = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (match == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(DIRECTX12_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    if (match->descriptorType() == DescriptorType::ResourceDescriptorHeap || match->descriptorType() == DescriptorType::SamplerDescriptorHeap) [[unlikely]]
        throw InvalidArgumentException("binding", "Resources that are bound to descriptor heaps directly must use `bindToHeap`.");

    // Update the binding.
    m_impl->updateBinding(*this, (*match), descriptor, sampler);
}

void DirectX12DescriptorSet::update(UInt32 binding, const IDirectX12AccelerationStructure& accelerationStructure, UInt32 descriptor) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto match = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (match == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(DIRECTX12_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    if (match->descriptorType() == DescriptorType::ResourceDescriptorHeap || match->descriptorType() == DescriptorType::SamplerDescriptorHeap) [[unlikely]]
        //throw InvalidArgumentException("binding", "Resources that are bound to descriptor heaps directly must use `bindToHeap`.");
        throw InvalidArgumentException("binding", "Acceleration structures cannot be bound to descriptor heaps directly.");

    auto& layout = *match;

    if (layout.descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor.", binding);

    auto buffer = accelerationStructure.buffer();

    if (buffer == nullptr) [[unlikely]]
        throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

    auto offset = m_impl->m_layout->getDescriptorOffset(binding, descriptor);
    auto device = m_impl->m_layout->device();
    auto descriptorSize = device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_resourceHeap.Heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(offset), descriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC bufferView = {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .RaytracingAccelerationStructure = {.Location = buffer->virtualAddress() }
    };

    device->handle()->CreateShaderResourceView(nullptr, &bufferView, descriptorHandle);
    descriptorHandle = descriptorHandle.Offset(static_cast<INT>(descriptorSize));

    m_impl->m_layout->device()->updateGlobalDescriptors(*this, binding, descriptor, 1u);
}

const ComPtr<ID3D12DescriptorHeap> DirectX12DescriptorSet::localHeap(DescriptorHeapType heapType) const noexcept
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return m_impl->m_resourceHeap.Heap;
    case DescriptorHeapType::Sampler:
        return m_impl->m_samplerHeap.Heap;
    default:
        return nullptr;
    }
}