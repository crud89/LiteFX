#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& descriptorHeap) :
    ComResource<ID3D12DescriptorHeap>(std::move(descriptorHeap)), DirectX12RuntimeObject(layout, layout.getDevice())
{
    if (descriptorHeap == nullptr)
        throw ArgumentNotInitializedException("The descriptor heap handle must be initialized.");
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept
{
    this->parent().free(*this);
}

UniquePtr<IDirectX12ConstantBuffer> DirectX12DescriptorSet::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const
{
    return this->getDevice()->factory().createConstantBuffer(this->parent().layout(binding), usage, elements);
}

UniquePtr<IDirectX12Texture> DirectX12DescriptorSet::makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
    return this->getDevice()->factory().createTexture(this->parent().layout(binding), format, size, levels, samples);
}

UniquePtr<IDirectX12Sampler> DirectX12DescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) const
{
    return this->getDevice()->factory().createSampler(this->parent().layout(binding), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

void DirectX12DescriptorSet::update(const IDirectX12ConstantBuffer& buffer, const UInt32& bufferElement) const
{
    // TODO: copy descriptor to global heap. It may be more efficient to cache all updates (rename `update` to `bind`) and flush them all together (new `update` method).
    throw;
}

void DirectX12DescriptorSet::update(const IDirectX12Texture& texture) const
{
    throw;
}

void DirectX12DescriptorSet::update(const IDirectX12Sampler& sampler) const
{
    throw;
}

void DirectX12DescriptorSet::attach(const UInt32& binding, const IDirectX12Image& image) const
{
    throw;
}