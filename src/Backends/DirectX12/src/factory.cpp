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
		raiseIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &allocator), "Unable to create D3D12 memory allocator.");
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

UniquePtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const
{
	return this->createBuffer("", type, usage, elementSize, elements, allowWrite);
}

UniquePtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(const String& name, BufferType type, BufferUsage usage, size_t elementSize, UInt32 elements, bool allowWrite) const
{
	// Constant buffers are aligned to 256 byte chunks. All other buffers can be aligned to a multiple of 4 bytes (`sizeof(DWORD)`). The actual amount of memory allocated 
	// is then defined as the smallest multiple of 64kb, that's greater or equal to `resourceDesc.Width` below. For more info, see:
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getresourceallocationinfo#remarks.
	size_t elementAlignment = type == BufferType::Uniform ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : sizeof(DWORD);

	D3D12_RESOURCE_DESC1 resourceDesc { };
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	//resourceDesc.Width = layout.elementSize() * elements;
	resourceDesc.Width = elements * ((elementSize + elementAlignment - 1) & ~(elementAlignment - 1));	// Align elements to 256 bytes.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = allowWrite ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc { };

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("usage", "The buffer usage {0} is not supported.", usage);
	}

	return DirectX12Buffer::allocate(name, m_impl->m_allocator, type, elements, elementSize, elementAlignment, allowWrite, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const DirectX12VertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	return this->createVertexBuffer("", layout, usage, elements);
}

UniquePtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	D3D12_RESOURCE_DESC1 resourceDesc { };
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

	D3D12MA::ALLOCATION_DESC allocationDesc { };

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("usage", "The buffer usage {0} is not supported.", usage);
	}

	return DirectX12VertexBuffer::allocate(name, layout, m_impl->m_allocator, elements, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const DirectX12IndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	return this->createIndexBuffer("", layout, usage, elements);
}

UniquePtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, BufferUsage usage, UInt32 elements) const
{
	D3D12_RESOURCE_DESC1 resourceDesc { };
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
	
	D3D12MA::ALLOCATION_DESC allocationDesc { };

	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case BufferUsage::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case BufferUsage::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("usage", "The buffer usage {0} is not supported.", usage);
	}

	return DirectX12IndexBuffer::allocate(name, layout, m_impl->m_allocator, elements, resourceDesc, allocationDesc);
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createAttachment(Format format, const Size2d& size, MultiSamplingLevel samples) const
{
	return this->createAttachment("", format, size, samples);
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createAttachment(const String& name, Format format, const Size2d& size, MultiSamplingLevel samples) const
{
	auto width = std::max<UInt32>(1, size.width());
	auto height = std::max<UInt32>(1, size.height());

	D3D12_RESOURCE_DESC1 resourceDesc { };
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DX12::getFormat(format);
	resourceDesc.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12MA::ALLOCATION_DESC allocationDesc { .HeapType = D3D12_HEAP_TYPE_DEFAULT };

	if (::hasDepth(format) || ::hasStencil(format))
	{
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		return DirectX12Image::allocate(name, m_impl->m_device, m_impl->m_allocator, { width, height, 1 }, format, ImageDimensions::DIM_2, 1, 1, samples, false, ImageLayout::DepthRead, resourceDesc, allocationDesc);
	}
	else
	{
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		return DirectX12Image::allocate(name, m_impl->m_device, m_impl->m_allocator, { width, height, 1 }, format, ImageDimensions::DIM_2, 1, 1, samples, false, ImageLayout::Common, resourceDesc, allocationDesc);
	}
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, allowWrite);
}

UniquePtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	if (dimension == ImageDimensions::CUBE && layers != 6) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", 6u, 6u, layers, "A cube map must be defined with 6 layers, but {0} are provided.", layers);

	if (dimension == ImageDimensions::DIM_3 && layers != 1) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", 1u, 1u, layers, "A 3D texture can only have one layer, but {0} are provided.", layers);

	auto width = std::max<UInt32>(1, size.width());
	auto height = std::max<UInt32>(1, size.height());
	auto depth = std::max<UInt32>(1, size.depth());

	D3D12_RESOURCE_DESC1 resourceDesc { };
	resourceDesc.Dimension = DX12::getImageType(dimension);
	resourceDesc.Alignment = 0;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = dimension == ImageDimensions::DIM_3 ? depth : layers;
	resourceDesc.MipLevels = levels;
	resourceDesc.Format = DX12::getFormat(format);
	resourceDesc.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = allowWrite ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc { .HeapType = D3D12_HEAP_TYPE_DEFAULT };
	
	return DirectX12Image::allocate(name, m_impl->m_device, m_impl->m_allocator, { width, height, depth }, format, dimension, levels, layers, samples, allowWrite, ImageLayout::Common, resourceDesc, allocationDesc);
}

Enumerable<UniquePtr<IDirectX12Image>> DirectX12GraphicsFactory::createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool allowWrite) const
{
	return [&, this]() -> std::generator<UniquePtr<IDirectX12Image>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield this->createTexture(format, size, dimension, levels, layers, samples, allowWrite);
	}() | std::views::as_rvalue;
}

UniquePtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return makeUnique<DirectX12Sampler>(m_impl->m_device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

UniquePtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return makeUnique<DirectX12Sampler>(m_impl->m_device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
}

Enumerable<UniquePtr<IDirectX12Sampler>> DirectX12GraphicsFactory::createSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [&, this]() -> std::generator<UniquePtr<IDirectX12Sampler>> {
		for (UInt32 i = 0; i < elements; ++i)
			co_yield this->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}() | std::views::as_rvalue;
}