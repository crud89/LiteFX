#include <litefx/backends/dx12.hpp>
//#include "buffer.h"
//#include "image.h"

// TODO: Include D3D12 Memory Allocator.

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl : public Implement<DirectX12GraphicsFactory> {
public:
	friend class DirectX12GraphicsFactory;

private:
	const DirectX12Device& m_device;

public:
	DirectX12GraphicsFactoryImpl(DirectX12GraphicsFactory* parent, const DirectX12Device& device) :
		base(parent), m_device(device)
	{
		// TODO: Create D3D12 Memory Allocator.
	}

	~DirectX12GraphicsFactoryImpl()
	{
		// TODO: Destroy D3D12 Memory Allocator.
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