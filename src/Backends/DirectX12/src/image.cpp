#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Image::DirectX12ImageImpl : public Implement<DirectX12Image> {
public:
	friend class DirectX12Image;

private:
	AllocatorPtr m_allocator;
	AllocationPtr m_allocation;
	Format m_format;
	Size3d m_extent;
	UInt32 m_elements, m_levels, m_layers, m_planes; 
	Array<ImageLayout> m_layouts;
	ImageDimensions m_dimensions;
	bool m_writable;
	MultiSamplingLevel m_samples;
	const DirectX12Device& m_device;

public:
	DirectX12ImageImpl(DirectX12Image* parent, const DirectX12Device& device, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_device(device), m_allocator(allocator), m_allocation(std::move(allocation)), m_extent(extent), m_format(format), m_dimensions(dimension), m_levels(levels), m_layers(layers), m_writable(writable), m_samples(samples)
	{
		m_planes = ::D3D12GetFormatPlaneCount(device.handle().Get(), DX12::getFormat(format));
		m_elements = m_planes * m_layers * m_levels;
		m_layouts.resize(m_elements, initialLayout);
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Image::DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialState, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	m_impl(makePimpl<DirectX12ImageImpl>(this, device, extent, format, dimension, levels, layers, samples, writable, initialState, allocator, std::move(allocation))), ComResource<ID3D12Resource>(nullptr)
{
	this->handle() = std::move(image);

	if (!name.empty())
	{
		this->name() = name;

#ifndef NDEBUG
		this->handle()->SetName(Widen(name).c_str());
#endif
	}
}

DirectX12Image::~DirectX12Image() noexcept = default;

UInt32 DirectX12Image::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t DirectX12Image::size() const noexcept
{
	if (m_impl->m_allocation) [[likely]]
		return m_impl->m_allocation->GetSize();
	else
	{
		auto elementSize = ::getSize(m_impl->m_format) * m_impl->m_extent.width() * m_impl->m_extent.height() * m_impl->m_extent.depth() * m_impl->m_layers;
		auto totalSize = elementSize;

		for (int l(1); l < m_impl->m_levels; ++l)
		{
			elementSize /= 2;
			totalSize += elementSize;
		}

		return totalSize * m_impl->m_planes;
	}
}

size_t DirectX12Image::elementSize() const noexcept
{
	return this->size();
}

size_t DirectX12Image::elementAlignment() const noexcept
{
	// TODO: Support for 64 byte packed "small" resources.
	return 256;
}

size_t DirectX12Image::alignedElementSize() const noexcept
{
	// TODO: Align this by `elementAlignment`.
	return this->elementSize();
}

bool DirectX12Image::writable() const noexcept
{
	return m_impl->m_writable;
}

UInt64 DirectX12Image::virtualAddress() const noexcept
{
	return static_cast<UInt64>(this->handle()->GetGPUVirtualAddress());
}

ImageLayout DirectX12Image::layout(UInt32 subresource) const
{
	if (subresource >= m_impl->m_layouts.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("subresource", 0u, static_cast<UInt32>(m_impl->m_layouts.size()), subresource, "The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_layouts[subresource];
}

ImageLayout& DirectX12Image::layout(UInt32 subresource)
{
	if (subresource >= m_impl->m_layouts.size()) [[unlikely]]
		throw ArgumentOutOfRangeException("subresource", 0u, static_cast<UInt32>(m_impl->m_layouts.size()), subresource, "The sub-resource with the provided index {0} does not exist.", subresource);

	return m_impl->m_layouts[subresource];
}

size_t DirectX12Image::size(UInt32 level) const noexcept
{
	if (level >= m_impl->m_levels)
		return 0;

	auto size = this->extent(level);

	switch (this->dimensions())
	{
	case ImageDimensions::DIM_1: return ::getSize(this->format()) * size.width();
	case ImageDimensions::CUBE:
	case ImageDimensions::DIM_2: return ::getSize(this->format()) * size.width() * size.height();
	default:
	case ImageDimensions::DIM_3: return ::getSize(this->format()) * size.width() * size.height() * size.depth();
	}
}

Size3d DirectX12Image::extent(UInt32 level) const noexcept
{
	if (level >= m_impl->m_levels)
		return Size3d{ 0, 0, 0 };

	Size3d size = m_impl->m_extent;

	for (size_t l(0); l < level; ++l)
		size /= 2;

	size.width() = std::max<size_t>(size.width(), 1);
	size.height() = std::max<size_t>(size.height(), 1);
	size.depth() = std::max<size_t>(size.depth(), 1);

	return size;
}

Format DirectX12Image::format() const noexcept
{
	return m_impl->m_format;
}

ImageDimensions DirectX12Image::dimensions() const noexcept 
{
	return m_impl->m_dimensions;
}

UInt32 DirectX12Image::levels() const noexcept
{
	return m_impl->m_levels;
}

UInt32 DirectX12Image::layers() const noexcept
{
	return m_impl->m_layers;
}

UInt32 DirectX12Image::planes() const noexcept
{
	return m_impl->m_planes;
}

MultiSamplingLevel DirectX12Image::samples() const noexcept
{
	return m_impl->m_samples;
}

AllocatorPtr DirectX12Image::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Image::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

UniquePtr<DirectX12Image> DirectX12Image::allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	return DirectX12Image::allocate("", device, allocator, extent, format, dimension, levels, layers, samples, writable, initialLayout, resourceDesc, allocationDesc);
}

UniquePtr<DirectX12Image> DirectX12Image::allocate(const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, DX12::getImageLayout(initialLayout), nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create image resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Writable: {7} }}", name.empty() ? fmt::to_string(fmt::ptr(resource.Get())) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, writable, samples);
	
	return makeUnique<DirectX12Image>(device, std::move(resource), extent, format, dimension, levels, layers, samples, writable, initialLayout, allocator, AllocationPtr(allocation), name);
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Sampler::DirectX12SamplerImpl : public Implement<DirectX12Sampler> {
public:
	friend class DirectX12Sampler;

private:
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;
	const DirectX12Device& m_device;

public:
	DirectX12SamplerImpl(DirectX12Sampler* parent, const DirectX12Device& device, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) :
		base(parent), m_device(device), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Sampler::DirectX12Sampler(const DirectX12Device& device, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy, const String& name) :
	m_impl(makePimpl<DirectX12SamplerImpl>(this, device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
{
	if (!name.empty())
		this->name() = name;
}

DirectX12Sampler::~DirectX12Sampler() noexcept = default;

FilterMode DirectX12Sampler::getMinifyingFilter() const noexcept
{
	return m_impl->m_minFilter;
}

FilterMode DirectX12Sampler::getMagnifyingFilter() const noexcept
{
	return m_impl->m_magFilter;
}

BorderMode DirectX12Sampler::getBorderModeU() const noexcept
{
	return m_impl->m_borderU;
}

BorderMode DirectX12Sampler::getBorderModeV() const noexcept
{
	return m_impl->m_borderV;
}

BorderMode DirectX12Sampler::getBorderModeW() const noexcept
{
	return m_impl->m_borderW;
}

Float DirectX12Sampler::getAnisotropy() const noexcept
{
	return m_impl->m_anisotropy;
}

MipMapMode DirectX12Sampler::getMipMapMode() const noexcept
{
	return m_impl->m_mipMapMode;
}

Float DirectX12Sampler::getMipMapBias() const noexcept
{
	return m_impl->m_mipMapBias;
}

Float DirectX12Sampler::getMaxLOD() const noexcept
{
	return m_impl->m_maxLod;
}

Float DirectX12Sampler::getMinLOD() const noexcept
{
	return m_impl->m_minLod;
}