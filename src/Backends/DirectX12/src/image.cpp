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
	Size2d m_extent;
	UInt32 m_elements{ 1 }; 
	D3D12_RESOURCE_STATES m_state;
	ImageDimensions m_dimensions;

public:
	DirectX12ImageImpl(DirectX12Image* parent, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_allocator(allocator), m_allocation(std::move(allocation)), m_extent(extent), m_format(format), m_state(initialState), m_dimensions(dimension)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Image::DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12ImageImpl>(this, extent, format, dimension, initialState, allocator, std::move(allocation))), DirectX12RuntimeObject<DirectX12Device>(device, &device), ComResource<ID3D12Resource>(nullptr)
{
	this->handle() = std::move(image);
}

DirectX12Image::~DirectX12Image() noexcept = default;

const UInt32& DirectX12Image::elements() const noexcept
{
	return m_impl->m_elements;
}

size_t DirectX12Image::size() const noexcept
{
	return this->elementSize() * m_impl->m_elements;
}

size_t DirectX12Image::elementSize() const noexcept
{
	// Rough estimation, that does not include alignment.
	return ::getSize(m_impl->m_format) * m_impl->m_extent.width() * m_impl->m_extent.height();
}

size_t DirectX12Image::elementAlignment() const noexcept
{
	return 0;
}

size_t DirectX12Image::alignedElementSize() const noexcept
{
	return this->elementSize();
}

const Size2d& DirectX12Image::extent() const noexcept
{
	return m_impl->m_extent;
}

const Format& DirectX12Image::format() const noexcept
{
	return m_impl->m_format;
}

const ImageDimensions& DirectX12Image::dimensions() const noexcept 
{
	return m_impl->m_dimensions;
}

const D3D12_RESOURCE_STATES& DirectX12Image::state() const noexcept
{
	return m_impl->m_state;
}

D3D12_RESOURCE_STATES& DirectX12Image::state() noexcept
{
	return m_impl->m_state;
}

D3D12_RESOURCE_BARRIER DirectX12Image::transitionTo(const D3D12_RESOURCE_STATES& state, const UInt32& element, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(this->handle().Get(), m_impl->m_state, state, element, flags);
	m_impl->m_state = state;
	return barrier;
}

void DirectX12Image::transitionTo(const DirectX12CommandBuffer& commandBuffer, const D3D12_RESOURCE_STATES& state, const UInt32& element, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(this->handle().Get(), m_impl->m_state, state, element, flags);
	m_impl->m_state = state;
	commandBuffer.handle()->ResourceBarrier(1, &barrier);
}

AllocatorPtr DirectX12Image::allocator() const noexcept
{
	return m_impl->m_allocator;
}

const D3D12MA::Allocation* DirectX12Image::allocationInfo() const noexcept
{
	return m_impl->m_allocation.get();
}

UniquePtr<DirectX12Image> DirectX12Image::allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create image resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4} }}", fmt::ptr(resource.Get()), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format);
	
	return makeUnique<DirectX12Image>(device, std::move(resource), extent, format, dimension, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Texture shared implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Texture::DirectX12TextureImpl : public Implement<DirectX12Texture> {
public:
	friend class DirectX12Texture;

private:
	const DirectX12DescriptorLayout& m_descriptorLayout;
	MultiSamplingLevel m_samples;
	UInt32 m_levels;

public:
	DirectX12TextureImpl(DirectX12Texture* parent, const DirectX12DescriptorLayout& descriptorLayout, const UInt32& levels, const MultiSamplingLevel& samples) :
		base(parent), m_descriptorLayout(descriptorLayout), m_samples(samples), m_levels(levels)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Texture shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Texture::DirectX12Texture(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, ComPtr<ID3D12Resource>&& image, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	DirectX12Image(device, std::move(image), extent, format, dimension, initialState, allocator, std::move(allocation)), m_impl(makePimpl<DirectX12TextureImpl>(this, layout, levels, samples))
{
}

DirectX12Texture::~DirectX12Texture() noexcept = default;

const UInt32& DirectX12Texture::binding() const noexcept
{
	return m_impl->m_descriptorLayout.binding();
}

const DirectX12DescriptorLayout& DirectX12Texture::layout() const noexcept
{
	return m_impl->m_descriptorLayout;
}

const MultiSamplingLevel& DirectX12Texture::samples() const noexcept
{
	return m_impl->m_samples;
}

const UInt32& DirectX12Texture::levels() const noexcept
{
	return m_impl->m_levels;
}

void DirectX12Texture::transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (elements != 1 || targetElement != 0) [[unlikely]]
		throw ArgumentOutOfRangeException("Textures currently do not support transforms for more than one element. The target element is required to be 0 (is {0}) and the number of elements is required to be 1 (is {1}).", targetElement, elements);

	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (this->state() != D3D12_RESOURCE_STATE_COPY_DEST)
		this->transitionTo(commandBuffer, D3D12_RESOURCE_STATE_COPY_DEST, targetElement);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& bufferDesc = this->handle()->GetDesc();
	this->getDevice()->handle()->GetCopyableFootprints(&bufferDesc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

	CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source.handle().Get(), footprint);
	CD3DX12_TEXTURE_COPY_LOCATION targetLocation(this->handle().Get(), 0);
	commandBuffer.handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);

	this->transitionTo(commandBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, targetElement);
}

void DirectX12Texture::transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (elements != 1 || sourceElement != 0) [[unlikely]]
		throw ArgumentOutOfRangeException("Textures currently do not support transforms for more than one element. The source element is required to be 0 (is {0}) and the number of elements is required to be 1 (is {1}).", targetElement, elements);

	if (target.elements() <= targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	if (this->state() != D3D12_RESOURCE_STATE_COPY_SOURCE)
		this->transitionTo(commandBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, sourceElement);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& bufferDesc = this->handle()->GetDesc();
	this->getDevice()->handle()->GetCopyableFootprints(&bufferDesc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

	CD3DX12_TEXTURE_COPY_LOCATION targetLocation(target.handle().Get(), footprint);
	CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(this->handle().Get(), 0);
	commandBuffer.handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);

	this->transitionTo(commandBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, sourceElement);
}

UniquePtr<DirectX12Texture> DirectX12Texture::allocate(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, AllocatorPtr allocator, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create texture resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Samples: {5}, Levels: {6} }}", fmt::ptr(resource.Get()), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, samples, levels);

	return makeUnique<DirectX12Texture>(device, layout, std::move(resource), extent, format, dimension, levels, samples, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Sampler implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Sampler::DirectX12SamplerImpl : public Implement<DirectX12Sampler> {
public:
	friend class DirectX12Sampler;

private:
	const DirectX12DescriptorLayout& m_layout;
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipMapBias;
	Float m_minLod, m_maxLod;
	Float m_anisotropy;

public:
	DirectX12SamplerImpl(DirectX12Sampler* parent, const DirectX12DescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_layout(layout), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Sampler::DirectX12Sampler(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	DirectX12RuntimeObject<DirectX12Device>(device, &device), m_impl(makePimpl<DirectX12SamplerImpl>(this, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
{
}

DirectX12Sampler::~DirectX12Sampler() noexcept = default;

const FilterMode& DirectX12Sampler::getMinifyingFilter() const noexcept
{
	return m_impl->m_minFilter;
}

const FilterMode& DirectX12Sampler::getMagnifyingFilter() const noexcept
{
	return m_impl->m_magFilter;
}

const BorderMode& DirectX12Sampler::getBorderModeU() const noexcept
{
	return m_impl->m_borderU;
}

const BorderMode& DirectX12Sampler::getBorderModeV() const noexcept
{
	return m_impl->m_borderV;
}

const BorderMode& DirectX12Sampler::getBorderModeW() const noexcept
{
	return m_impl->m_borderW;
}

const Float& DirectX12Sampler::getAnisotropy() const noexcept
{
	return m_impl->m_anisotropy;
}

const MipMapMode& DirectX12Sampler::getMipMapMode() const noexcept
{
	return m_impl->m_mipMapMode;
}

const Float& DirectX12Sampler::getMipMapBias() const noexcept
{
	return m_impl->m_mipMapBias;
}

const Float& DirectX12Sampler::getMaxLOD() const noexcept
{
	return m_impl->m_maxLod;
}

const Float& DirectX12Sampler::getMinLOD() const noexcept
{
	return m_impl->m_minLod;
}

const UInt32& DirectX12Sampler::binding() const noexcept
{
	return m_impl->m_layout.binding();
}

const DirectX12DescriptorLayout& DirectX12Sampler::layout() const noexcept
{
	return m_impl->m_layout;
}