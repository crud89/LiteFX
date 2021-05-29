#include <litefx/backends/dx12.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSet::DirectX12DescriptorSetImpl : public Implement<DirectX12DescriptorSet> {
public:
    friend class DirectX12DescriptorSet;

private:
    const DirectX12DescriptorSetLayout& m_layout;

public:
    DirectX12DescriptorSetImpl(DirectX12DescriptorSet* parent, const DirectX12DescriptorSetLayout& descriptorSetLayout) :
        base(parent), m_layout(descriptorSetLayout)
    { 
    }

public:
    ComPtr<ID3D12DescriptorHeap> initialize()
    {
        const bool hasSamplers = m_layout.samplers() > 0;
        auto descriptors = m_layout.uniforms() + m_layout.storages() + m_layout.images() + m_layout.inputAttachments();

        if (hasSamplers && descriptors > 0)
            throw RuntimeException("Samplers are only allowed in sampler-only descriptor sets.");

        // Create a CPU-visible descriptor heap for the descriptor set. The GPU-visible descriptor heap is part of the render pass.
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
        descriptorHeapDesc.NumDescriptors = hasSamplers ? m_layout.samplers() : descriptors;
        descriptorHeapDesc.Type = hasSamplers ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ComPtr<ID3D12DescriptorHeap> descriptorHeap;
        raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)), "Unable to create host descriptor heap.");

        return descriptorHeap;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& bufferSet) :
    ComResource<ID3D12DescriptorHeap>(nullptr), DirectX12RuntimeObject(bufferSet.getDevice()), m_impl(makePimpl<DirectX12DescriptorSetImpl>(this, bufferSet))
{
    this->handle() = m_impl->initialize();
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept = default;

const IDescriptorSetLayout* DirectX12DescriptorSet::getDescriptorSetLayout() const noexcept
{
    return &m_impl->m_layout;
}

UniquePtr<IConstantBuffer> DirectX12DescriptorSet::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const noexcept
{
    //auto layout = this->getDescriptorSetLayout()->getLayout(binding);
    //return this->getDevice()->createConstantBuffer(layout, usage, elements);
    throw;
}

UniquePtr<ITexture> DirectX12DescriptorSet::makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const noexcept
{
    //auto layout = this->getDescriptorSetLayout()->getLayout(binding);
    //return this->getDevice()->createTexture(layout, format, size, levels, samples);
    throw;
}

UniquePtr<ISampler> DirectX12DescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const noexcept
{
    //auto layout = this->getDescriptorSetLayout()->getLayout(binding);
    //return this->getDevice()->createSampler(layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
    throw;
}

void DirectX12DescriptorSet::update(const IConstantBuffer* buffer) const
{
    throw;
}

void DirectX12DescriptorSet::update(const ITexture* texture) const
{
    throw;
}

void DirectX12DescriptorSet::update(const ISampler* sampler) const
{
    throw;
}

void DirectX12DescriptorSet::updateAll(const IConstantBuffer* buffer) const
{
    throw;
}

void DirectX12DescriptorSet::updateAll(const ITexture* texture) const
{
    throw;
}

void DirectX12DescriptorSet::updateAll(const ISampler* sampler) const
{
    throw;
}

void DirectX12DescriptorSet::attach(const UInt32& binding, const IRenderPass* renderPass, const UInt32& attachmentId) const
{
    throw;
}

void DirectX12DescriptorSet::attach(const UInt32& binding, const IImage* image) const
{
    throw;
}