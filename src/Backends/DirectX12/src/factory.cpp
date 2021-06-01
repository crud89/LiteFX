#include <litefx/backends/dx12.hpp>
#include "buffer.h"
//#include "image.h"
#include "D3D12MemAlloc.h"

struct D3D12MADeleter {
	void operator()(auto* ptr) noexcept {
		ptr->Release();
	}
};

typedef UniquePtr<D3D12MA::Allocator, D3D12MADeleter> AllocatorPtr;
typedef UniquePtr<D3D12MA::Allocation, D3D12MADeleter> AllocationPtr;

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl : public Implement<DirectX12GraphicsFactory> {
public:
	friend class DirectX12GraphicsFactory;

private:
	const DirectX12Device& m_device;
	AllocatorPtr m_allocator;

public:
	DirectX12GraphicsFactoryImpl(DirectX12GraphicsFactory* parent, const DirectX12Device& device) :
		base(parent), m_device(device)
	{
		// Initialize memory allocator.
		D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
		//allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED;
		allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
		allocatorDesc.pAdapter = device.adapter().handle().Get();
		allocatorDesc.pDevice = device.handle().Get();
		allocatorDesc.PreferredBlockSize = 0;	// TODO: Make configurable.

		D3D12MA::Allocator* allocator;
		raiseIfFailed<RuntimeException>(D3D12MA::CreateAllocator(&allocatorDesc, &allocator), "Unable to create D3D12 memory allocator.");
		m_allocator.reset(allocator);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsFactory::DirectX12GraphicsFactory(const DirectX12Device& device) :
	m_impl(makePimpl<DirectX12GraphicsFactoryImpl>(this, device))
{
}

DirectX12GraphicsFactory::~DirectX12GraphicsFactory() noexcept = default;

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createImage(const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;	// NOTE: Can be overwritten for "small textures" (see: https://asawicki.info/news_1726_secrets_of_direct3d_12_resource_alignment)
	resourceDesc.Width = size.width();
	resourceDesc.Height = size.height();
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = levels;
	resourceDesc.Format = ::getFormat(format);
	resourceDesc.SampleDesc.Count = static_cast<UInt32>(samples);
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;		// TODO: Check me!

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocationPtr;
	raiseIfFailed<RuntimeException>(m_impl->m_allocator->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &allocationPtr, IID_PPV_ARGS(&resource)), "Unable to create image resource.");
	AllocationPtr allocation(allocationPtr);

	throw;
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const
{
	throw;
}

UniquePtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements) const
{
	throw;
}

UniquePtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const DirectX12VertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const DirectX12IndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IDirectX12ConstantBuffer> DirectX12GraphicsFactory::createConstantBuffer(const DirectX12DescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	throw;
}

UniquePtr<IDirectX12Texture> DirectX12GraphicsFactory::createTexture(const DirectX12DescriptorLayout& layout, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	throw;
}

UniquePtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(const DirectX12DescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	throw;
}