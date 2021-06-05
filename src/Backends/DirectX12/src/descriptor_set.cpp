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
        auto buffers = parent->parent().uniforms() + parent->parent().images() + parent->parent().storages();

        if (buffers > 0 && m_bufferHeap == nullptr)
            throw ArgumentNotInitializedException("The buffer descriptor heap handle must be initialized, if the descriptor set layout contains uniform buffers, storage buffers or images.");

        if (parent->parent().samplers() > 0 && m_samplerHeap == nullptr)
            throw ArgumentNotInitializedException("The sampler descriptor heap handle must be initialized, if the descriptor set layout contains samplers.");
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

void DirectX12DescriptorSet::update(const IDirectX12ConstantBuffer& buffer, const UInt32& bufferElement) const noexcept
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {
        .BufferLocation = buffer.handle()->GetGPUVirtualAddress() + static_cast<size_t>(bufferElement) * buffer.alignedElementSize(),
        .SizeInBytes = static_cast<UInt32>(buffer.alignedElementSize())
    };

    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_impl->m_bufferHeap->GetCPUDescriptorHandleForHeapStart(), buffer.layout().binding(), this->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    this->getDevice()->handle()->CreateConstantBufferView(&constantBufferView, descriptorHandle);
}

void DirectX12DescriptorSet::update(const IDirectX12Texture& texture) const noexcept
{
    throw;
}

void DirectX12DescriptorSet::update(const IDirectX12Sampler& sampler) const noexcept
{
    throw;
}

void DirectX12DescriptorSet::attach(const UInt32& binding, const IDirectX12Image& image) const noexcept
{
    throw;
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::bufferHeap() const noexcept 
{
    return m_impl->m_bufferHeap;
}

const ComPtr<ID3D12DescriptorHeap>& DirectX12DescriptorSet::samplerHeap() const noexcept
{
    return m_impl->m_samplerHeap;
}