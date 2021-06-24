#include <litefx/backends/dx12.hpp>
#include "buffer.h"
#include "image.h"

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
		m_allocator.reset(allocator, D3D12MADeleter{});
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
	resourceDesc.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;		// TODO: Check me!

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	return DirectX12Image::allocate(m_impl->m_device, m_impl->m_allocator, size, format, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_UNORDERED_ACCESS, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = size.width();
	resourceDesc.Height = size.height();
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = ::getFormat(format);
	resourceDesc.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	if (::hasDepth(format) || ::hasStencil(format))
	{
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		return DirectX12Image::allocate(m_impl->m_device, m_impl->m_allocator, size, format, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ, resourceDesc, allocationDesc);
	}
	else
	{
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		return DirectX12Image::allocate(m_impl->m_device, m_impl->m_allocator, size, format, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, resourceDesc, allocationDesc);
	}
}

UniquePtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = elementSize * static_cast<size_t>(elements);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		return DirectX12Buffer::allocate(m_impl->m_device, m_impl->m_allocator, type, elements, elementSize, 0, D3D12_RESOURCE_STATE_GENERIC_READ, resourceDesc, allocationDesc);
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		return DirectX12Buffer::allocate(m_impl->m_device, m_impl->m_allocator, type, elements, elementSize, 0, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		return DirectX12Buffer::allocate(m_impl->m_device, m_impl->m_allocator, type, elements, elementSize, 0, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	default:
		throw InvalidArgumentException("The buffer usage {0} is not supported.", usage);
	}
}

UniquePtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const DirectX12VertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = layout.elementSize() * static_cast<size_t>(elements);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		return DirectX12VertexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_GENERIC_READ, resourceDesc, allocationDesc);
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		return DirectX12VertexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		return DirectX12VertexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	default:
		throw InvalidArgumentException("The buffer usage {0} is not supported.", usage);
	}
}

UniquePtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const DirectX12IndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = layout.elementSize() * static_cast<size_t>(elements);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		return DirectX12IndexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_GENERIC_READ, resourceDesc, allocationDesc);
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		return DirectX12IndexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		return DirectX12IndexBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	default:
		throw InvalidArgumentException("The buffer usage {0} is not supported.", usage);
	}
}

UniquePtr<IDirectX12ConstantBuffer> DirectX12GraphicsFactory::createConstantBuffer(const DirectX12DescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	//resourceDesc.Width = layout.elementSize() * elements;
	resourceDesc.Width = elements * ((layout.elementSize() + 255) & ~(255));	// Align elements to 256 bytes.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		return DirectX12ConstantBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_GENERIC_READ, resourceDesc, allocationDesc);
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		return DirectX12ConstantBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		return DirectX12ConstantBuffer::allocate(m_impl->m_device, layout, m_impl->m_allocator, elements, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
	default:
		throw InvalidArgumentException("The buffer usage {0} is not supported.", usage);
	}
}

UniquePtr<IDirectX12Texture> DirectX12GraphicsFactory::createTexture(const DirectX12DescriptorLayout& layout, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = size.width();
	resourceDesc.Height = size.height();
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = levels;
	resourceDesc.Format = ::getFormat(format);
	resourceDesc.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	
	return DirectX12Texture::allocate(m_impl->m_device, layout, m_impl->m_allocator, size, format, levels, samples, D3D12_RESOURCE_STATE_COPY_DEST, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(const DirectX12DescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
	return makeUnique<DirectX12Sampler>(m_impl->m_device, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}