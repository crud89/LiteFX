#pragma once

#include <litefx/backends/dx12.hpp>

namespace LiteFX::Rendering::Backends {

    inline D3D12_FILTER getFilterMode(FilterMode minFilter, FilterMode magFilter, MipMapMode mipFilter, Float anisotropy = 0.f)
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

    inline D3D12_TEXTURE_ADDRESS_MODE getBorderMode(BorderMode mode)
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

    inline void writeBufferDescriptors(const DirectX12Device& device, DescriptorType type, const IDirectX12Buffer& buffer, UInt32 firstElement, UInt32 descriptors, UInt32 elementsPerDescriptor, Format texelFormat, UInt32 structureStride, CD3DX12_CPU_DESCRIPTOR_HANDLE target)
    {
        const UInt64 elements = static_cast<UInt64>(descriptors) * elementsPerDescriptor;

        if (firstElement + elements > buffer.elements()) [[unlikely]]
            throw ArgumentOutOfRangeException("descriptors", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elements, firstElement);

        // Validate the descriptor binding.
        buffer.validateBinding(type, elementsPerDescriptor, structureStride);

        // Check if the input buffer is compatible with the descriptor rules.
        UInt64 alignedSize = buffer.alignedElementSize();
        UInt64 elementSize = buffer.elementSize();
        UInt64 viewSize = (elementsPerDescriptor - 1) * alignedSize + elementSize;
        UInt64 stride = structureStride > 0 ? structureStride : elementSize;
        UInt64 texelSize{ 0ull };

        switch (type)
        {
        case DescriptorType::ConstantBuffer:
        {
            constexpr UInt64 maxConstantBufferSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16u;

            // NOTE: This checks both, if the size as well as the offset between elements follows the alignment rules.
            if (alignedSize % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT != 0) [[unlikely]]
                throw InvalidArgumentException("buffer", "Constant buffer elements must be aligned to {0} bytes, but the buffer uses an aligned element size of {1} bytes. Create the buffer as a uniform buffer.", D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, alignedSize);

            if (elementsPerDescriptor * alignedSize > maxConstantBufferSize) [[unlikely]]
                throw InvalidArgumentException("buffer", "Constant buffer views must not exceed {0} bytes, but the view spans {1} bytes.", maxConstantBufferSize, elementsPerDescriptor * alignedSize);

            break;
        }
        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWStructuredBuffer:
        {
            // Structured views are addressed in units of the stride, so each view's offset must be a multiple of it.
            if (alignedSize % stride != 0) [[unlikely]]
                throw InvalidArgumentException("buffer", "The aligned buffer element size of {0} bytes is not a multiple of the structure stride of {1} bytes. Pad the structure to a multiple of {2} bytes or pack multiple structures into one element.", alignedSize, stride, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);

            break;
        }
        case DescriptorType::ByteAddressBuffer:
        case DescriptorType::RWByteAddressBuffer:
        {
            if (alignedSize % D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT != 0) [[unlikely]]
                throw InvalidArgumentException("buffer", "Byte address buffer views must be aligned to {0} bytes, but the buffer uses an aligned element size of {1} bytes.", D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT, alignedSize);

            if (elementSize % sizeof(DWORD) != 0) [[unlikely]]
                throw InvalidArgumentException("buffer", "Byte address buffer elements must be a multiple of {0} bytes, but the buffer uses an element size of {1} bytes.", sizeof(DWORD), elementSize);

            break;
        }
        case DescriptorType::Buffer:
        case DescriptorType::RWBuffer:
        {
            if (texelFormat == Format::None) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "Texel buffer descriptors require a texel format.");

            if (!device.supportsTexelBufferFormat(texelFormat, type == DescriptorType::RWBuffer)) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "The format {0} is not supported for {1} descriptors on this device.", texelFormat, type);

            texelSize = getSize(texelFormat);

            if (elementSize % texelSize != 0 || alignedSize % texelSize != 0) [[unlikely]]
                throw InvalidArgumentException("texelFormat", "The buffer element size of {0} bytes (aligned: {1} bytes) is not a multiple of the texel size of {2} bytes.", elementSize, alignedSize, texelSize);

            if (viewSize / texelSize > (1ull << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP)) [[unlikely]]
                throw InvalidArgumentException("buffer", "The view spans more texels than supported ({1} > {0}).", 1ull << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP, viewSize / texelSize);

            break;
        }
        default: [[unlikely]]
            throw InvalidArgumentException("type", "Invalid descriptor type. The descriptor type {0} does not describe a buffer.", type);
        }

        auto increment = device.handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        auto* resource = buffer.handle().Get();

        for (UInt32 d{ 0 }; d < descriptors; ++d, target.Offset(1, increment))
        {
            const UInt64 offset = (static_cast<UInt64>(firstElement) + static_cast<UInt64>(d) * elementsPerDescriptor) * alignedSize;

            switch (type)
            {
            case DescriptorType::ConstantBuffer:
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC view{
                    .BufferLocation = resource->GetGPUVirtualAddress() + offset,
                    .SizeInBytes = static_cast<UInt32>(elementsPerDescriptor * alignedSize)   // Must cover full 256-byte blocks.
                };

                device.handle()->CreateConstantBufferView(&view, target);
                break;
            }
            case DescriptorType::StructuredBuffer:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC view{
                    .Format = DXGI_FORMAT_UNKNOWN,
                    .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                    .Buffer = {.FirstElement = offset / stride, .NumElements = static_cast<UInt32>(viewSize / stride), .StructureByteStride = static_cast<UInt32>(stride), .Flags = D3D12_BUFFER_SRV_FLAG_NONE }
                };

                device.handle()->CreateShaderResourceView(resource, &view, target);
                break;
            }
            case DescriptorType::RWStructuredBuffer:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC view{
                    .Format = DXGI_FORMAT_UNKNOWN,
                    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                    .Buffer = {.FirstElement = offset / stride, .NumElements = static_cast<UInt32>(viewSize / stride), .StructureByteStride = static_cast<UInt32>(stride), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE }
                };

                device.handle()->CreateUnorderedAccessView(resource, nullptr, &view, target);
                break;
            }
            case DescriptorType::ByteAddressBuffer:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC view{
                    .Format = DXGI_FORMAT_R32_TYPELESS,
                    .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                    .Buffer = {.FirstElement = offset / sizeof(DWORD), .NumElements = static_cast<UInt32>(viewSize / sizeof(DWORD)), .StructureByteStride = 0, .Flags = D3D12_BUFFER_SRV_FLAG_RAW }
                };

                device.handle()->CreateShaderResourceView(resource, &view, target);
                break;
            }
            case DescriptorType::RWByteAddressBuffer:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC view{
                    .Format = DXGI_FORMAT_R32_TYPELESS,
                    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                    .Buffer = {.FirstElement = offset / sizeof(DWORD), .NumElements = static_cast<UInt32>(viewSize / sizeof(DWORD)), .StructureByteStride = 0, .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_RAW }
                };

                device.handle()->CreateUnorderedAccessView(resource, nullptr, &view, target);
                break;
            }
            case DescriptorType::Buffer:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC view{
                    .Format = DX12::getFormat(texelFormat),
                    .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                    .Buffer = {.FirstElement = offset / texelSize, .NumElements = static_cast<UInt32>(viewSize / texelSize), .StructureByteStride = 0 }
                };

                device.handle()->CreateShaderResourceView(resource, &view, target);
                break;
            }
            case DescriptorType::RWBuffer:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC view{
                    .Format = DX12::getFormat(texelFormat),
                    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                    .Buffer = {.FirstElement = offset / texelSize, .NumElements = static_cast<UInt32>(viewSize / texelSize), .StructureByteStride = 0, .CounterOffsetInBytes = 0 }
                };

                device.handle()->CreateUnorderedAccessView(resource, nullptr, &view, target);
                break;
            }
            default:
                std::unreachable();   // Validated above.
            }
        }
    }

    inline void writeImageDescriptor(const DirectX12Device& device, DescriptorType type, const IDirectX12Image& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers, D3D12_CPU_DESCRIPTOR_HANDLE target)
    {
        // Validate the descriptor binding.
        image.validateBinding(type, firstLevel, levels, firstLayer, layers);
        
        const UInt32 numLevels = levels == 0 ? image.levels() - firstLevel : levels;
        const UInt32 numLayers = layers == 0 ? image.layers() - firstLayer : layers;

        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        if (type == DescriptorType::RWTexture)
        {
            auto format = image.format();

            if (!device.supportsStorageImageFormat(format))
                throw InvalidArgumentException("type", "The device does not support the format {} to be bound onto a writable image descriptor.", format);

            D3D12_UNORDERED_ACCESS_VIEW_DESC textureView = { .Format = DX12::getFormat(format) };

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
                        textureView.Texture2DMS = { };
                    }
                    else
                    {
                        textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
                        textureView.Texture2DMSArray = { .FirstArraySlice = firstLayer, .ArraySize = numLayers };
                    }
                }

                break;
            case ImageDimensions::DIM_3:
                textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                textureView.Texture3D = { .MipSlice = firstLevel, .FirstWSlice = 0, .WSize = UINT_MAX };
                break;
            case ImageDimensions::CUBE:
                textureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                textureView.Texture2DArray = { .MipSlice = firstLevel, .FirstArraySlice = firstLayer, .ArraySize = numLayers, .PlaneSlice = 0 };
                break;
            }

            device.handle()->CreateUnorderedAccessView(image.handle().Get(), nullptr, &textureView, target);
        }
        else
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC textureView = { .Format = DX12::getFormat(image.format()), .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING };

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

            device.handle()->CreateShaderResourceView(image.handle().Get(), &textureView, target);
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    }

    inline void writeSamplerDescriptor(const DirectX12Device& device, const IDirectX12Sampler& sampler, D3D12_CPU_DESCRIPTOR_HANDLE target)
    {
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

        device.handle()->CreateSampler(&samplerInfo, target);
    }

    inline void writeAccelerationStructureDescriptor(const DirectX12Device& device, const IDirectX12AccelerationStructure& accelerationStructure, D3D12_CPU_DESCRIPTOR_HANDLE target)
    {
        auto buffer = accelerationStructure.buffer();

        if (buffer == nullptr) [[unlikely]]
            throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

        D3D12_SHADER_RESOURCE_VIEW_DESC view{
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .RaytracingAccelerationStructure = {.Location = buffer->virtualAddress() }
        };

        device.handle()->CreateShaderResourceView(nullptr, &view, target);
    }

}