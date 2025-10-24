#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Image Base implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Image::DirectX12ImageImpl {
public:
	friend class DirectX12Image;

private:
	AllocatorPtr m_allocator;
	AllocationPtr m_allocation;
	Format m_format;
	Size3d m_extent;
	UInt32 m_elements, m_levels, m_layers, m_planes; 
	ImageDimensions m_dimensions;
	ResourceUsage m_usage;
	MultiSamplingLevel m_samples;
	D3D12_RESOURCE_DESC1 m_resourceDesc;

public:
	DirectX12ImageImpl(const DirectX12Device& device, Size3d extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const D3D12_RESOURCE_DESC1& resourceDesc) :
		m_allocator(std::move(allocator)), m_allocation(std::move(allocation)), m_format(format), m_extent(std::move(extent)), m_levels(levels), m_layers(layers), m_planes{ ::D3D12GetFormatPlaneCount(device.handle().Get(), DX12::getFormat(format)) }, m_dimensions(dimension), m_usage(usage), m_samples(samples), m_resourceDesc(resourceDesc)
	{
		m_elements = m_planes * m_layers * m_levels;
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Image::DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name) :
	ComResource<ID3D12Resource>(nullptr), m_impl(device, extent, format, dimension, levels, layers, samples, usage, std::move(allocator), std::move(allocation), resourceDesc)
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
	// Attempt to get the pixel size. This ensures the nothrow guarantee.
	size_t pixelSize{ };

	try
	{
		pixelSize = ::getSize(m_impl->m_format);
	}
	catch (const InvalidArgumentException&)
	{
		LITEFX_ERROR(DIRECTX12_LOG, "Unsupported pixel format detected: {}.", std::to_underlying(m_impl->m_format));
		return 0;
	}
	catch (...)
	{
		return 0;
	}

	if (m_impl->m_allocation) [[likely]]
		return static_cast<size_t>(m_impl->m_allocation->GetSize());
	else
	{
		auto elementSize = pixelSize * m_impl->m_extent.width() * m_impl->m_extent.height() * m_impl->m_extent.depth() * m_impl->m_layers;
		auto totalSize = elementSize;

		for (UInt32 l(1); l < m_impl->m_levels; ++l)
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
	return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
}

size_t DirectX12Image::alignedElementSize() const noexcept
{
	// TODO: Align this by `elementAlignment`.
	return this->elementSize();
}

ResourceUsage DirectX12Image::usage() const noexcept
{
	return m_impl->m_usage;
}

UInt64 DirectX12Image::virtualAddress() const noexcept
{
	return static_cast<UInt64>(this->handle()->GetGPUVirtualAddress());
}

size_t DirectX12Image::size(UInt32 level) const noexcept
{
	if (level >= m_impl->m_levels)
		return 0;

	// Attempt to get the pixel size. This ensures the nothrow guarantee.
	size_t pixelSize{ };

	try
	{
		pixelSize = ::getSize(m_impl->m_format);
	}
	catch (const InvalidArgumentException&)
	{
		LITEFX_ERROR(DIRECTX12_LOG, "Unsupported pixel format detected: {}.", std::to_underlying(m_impl->m_format));
		return 0;
	}
	catch (...)
	{
		return 0;
	}

	auto size = this->extent(level);

	switch (this->dimensions())
	{
	case ImageDimensions::DIM_1: return pixelSize * size.width();
	case ImageDimensions::CUBE:
	case ImageDimensions::DIM_2: return pixelSize * size.width() * size.height();
	default:
	case ImageDimensions::DIM_3: return pixelSize * size.width() * size.height() * size.depth();
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

void DirectX12Image::reset(ComPtr<ID3D12Resource>&& image, AllocationPtr&& allocation)
{
	this->handle() = std::move(image);
	m_impl->m_allocation = std::move(allocation);
}

SharedPtr<IDirectX12Image> DirectX12Image::allocate(const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	bool isDepthStencil = ::hasDepth(format) || ::hasStencil(format);

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	raiseIfFailed(allocator->CreateResource3(&allocationDesc, &resourceDesc, isDepthStencil ? D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ : D3D12_BARRIER_LAYOUT_COMMON, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create image resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Usage: {7} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, usage, samples);
	
	return SharedObject::create<DirectX12Image>(device, std::move(resource), extent, format, dimension, levels, layers, samples, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
}

bool DirectX12Image::tryAllocate(SharedPtr<IDirectX12Image>& image, const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	image = nullptr;

	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("allocator", "The allocator must be initialized.");

	bool isDepthStencil = ::hasDepth(format) || ::hasStencil(format);

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation{};
	auto result = allocator->CreateResource3(&allocationDesc, &resourceDesc, isDepthStencil ? D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ : D3D12_BARRIER_LAYOUT_COMMON, nullptr, 0, nullptr, &allocation, IID_PPV_ARGS(&resource));

	if (FAILED(result))
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocation for image {0} with {1} bytes failed: 0x{9:08X} {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Usage: {7} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, usage, samples, result);
		return false;
	}
	else
	{
		LITEFX_DEBUG(DIRECTX12_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6}, Samples: {8}, Usage: {7} }}", name.empty() ? std::format("{0}", static_cast<void*>(resource.Get())) : name, ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers, usage, samples);

		image = SharedObject::create<DirectX12Image>(device, std::move(resource), extent, format, dimension, levels, layers, samples, usage, resourceDesc, std::move(allocator), AllocationPtr(allocation), name);
		return true;
	}
}

bool DirectX12Image::move(SharedPtr<IDirectX12Image> image, D3D12MA::Allocation* to, const DirectX12CommandBuffer& commandBuffer)
{
	// NOTES: If this method returns true, the command buffer must be executed and all bindings to the image must be updated afterwards, otherwise the result of this operation is undefined behavior.

	if (image == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("from");
	
	if (to == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("to");

	auto& source = dynamic_cast<DirectX12Image&>(*image);
	const auto device = commandBuffer.queue()->device();
	const auto& resourceDesc = source.m_impl->m_resourceDesc;
	auto allocator = source.m_impl->m_allocator;

	ComPtr<ID3D12Resource> resource;
	auto result = (*device).handle()->CreatePlacedResource2(to->GetHeap(), to->GetOffset(), std::addressof(resourceDesc), D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr, 0, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));

	if (FAILED(result)) [[unlikely]]
		return false;

	to->SetResource(resource.Get());
	commandBuffer.handle()->CopyResource(resource.Get(), source.handle().Get());

	// Reset the resource and return.
	// NOTE: At this point, the previous resource does still exist, but is inaccessible through the current instance. The only remaining reference should be stored by the source allocation during 
	//       defragmentation. After it gets released, the resource should also be removed. If a reference is stored somewhere else this leaks, but you should never store the reference obtained by
	//       calling `handle` manually.
	//       The new resource handle is valid beyond this point, but may contain uninitialized data. Any attempt of using the resource must be properly synchronized to execute after the submission
	//       of `commandBuffer`.
	source.reset(std::move(resource), AllocationPtr(to));
	return true;
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Sampler::DirectX12SamplerImpl {
public:
	friend class DirectX12Sampler;

private:
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;

public:
	DirectX12SamplerImpl(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) :
		m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Sampler::DirectX12Sampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy, const String& name) :
	m_impl(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy)
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