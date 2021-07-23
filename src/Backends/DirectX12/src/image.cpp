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
	UInt32 m_elements{ 1 }, m_levels, m_layers; 
	D3D12_RESOURCE_STATES m_state;
	ImageDimensions m_dimensions;

public:
	DirectX12ImageImpl(DirectX12Image* parent, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
		base(parent), m_allocator(allocator), m_allocation(std::move(allocation)), m_extent(extent), m_format(format), m_state(initialState), m_dimensions(dimension), m_levels(levels), m_layers(layers)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Image Base shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Image::DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	m_impl(makePimpl<DirectX12ImageImpl>(this, extent, format, dimension, levels, layers, initialState, allocator, std::move(allocation))), DirectX12RuntimeObject<DirectX12Device>(device, &device), ComResource<ID3D12Resource>(nullptr)
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

		return totalSize * m_impl->m_elements;
	}
}

size_t DirectX12Image::elementSize() const noexcept
{
	return this->size() / m_impl->m_elements;
}

size_t DirectX12Image::elementAlignment() const noexcept
{
	// TODO: Support for 64 byte packed "small" resources.
	return 256;
}

size_t DirectX12Image::alignedElementSize() const noexcept
{
	return this->elementSize();
}

size_t DirectX12Image::size(const UInt32& level) const noexcept
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

Size3d DirectX12Image::extent(const UInt32& level) const noexcept
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


const Format& DirectX12Image::format() const noexcept
{
	return m_impl->m_format;
}

const ImageDimensions& DirectX12Image::dimensions() const noexcept 
{
	return m_impl->m_dimensions;
}

const UInt32& DirectX12Image::levels() const noexcept
{
	return m_impl->m_levels;
}

const UInt32& DirectX12Image::layers() const noexcept
{
	return m_impl->m_layers;
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

UniquePtr<DirectX12Image> DirectX12Image::allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create image resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated image {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Levels: {5}, Layers: {6} }}", fmt::ptr(resource.Get()), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, levels, layers);
	
	return makeUnique<DirectX12Image>(device, std::move(resource), extent, format, dimension, levels, layers, initialState, allocator, AllocationPtr(allocation));
}

// ------------------------------------------------------------------------------------------------
// Texture shared implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Texture::DirectX12TextureImpl : public Implement<DirectX12Texture> {
public:
	friend class DirectX12Texture;

private:
	MultiSamplingLevel m_samples;

public:
	DirectX12TextureImpl(DirectX12Texture* parent, const MultiSamplingLevel& samples) :
		base(parent), m_samples(samples)
	{
	}

private:
	UInt32 getSubresourceId(const UInt32& level, const UInt32& layer, const UInt32& plane)
	{
		return level + (layer * m_parent->levels()) + (plane * m_parent->levels() * m_parent->layers());
	}
};

// ------------------------------------------------------------------------------------------------
// Texture shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Texture::DirectX12Texture(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator, AllocationPtr&& allocation) :
	DirectX12Image(device, std::move(image), extent, format, dimension, levels, layers, initialState, allocator, std::move(allocation)), m_impl(makePimpl<DirectX12TextureImpl>(this, samples))
{
}

DirectX12Texture::~DirectX12Texture() noexcept = default;

const MultiSamplingLevel& DirectX12Texture::samples() const noexcept
{
	return m_impl->m_samples;
}

void DirectX12Texture::generateMipMaps(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	struct Parameters {
		Float sizeX;
		Float sizeY;
		Float sRGB;
		Float padding { 0.f };
	} parametersData;

	parametersData.sRGB = ::isSRGB(this->format()) ? 1.f : 0.f;

	// Get the pipeline.
	auto& pipeline = this->getDevice()->blitPipeline();

	// Create and bind the parameters.
	const auto& resourceBindingsLayout = pipeline.layout().descriptorSet(0);
	auto resourceBindings = resourceBindingsLayout.allocate();
	const auto& parametersLayout = resourceBindingsLayout.descriptor(0);
	auto parameters = this->getDevice()->factory().createBuffer(parametersLayout.type(), BufferUsage::Dynamic, parametersLayout.elementSize(), 1);

	// Create and bind the sampler.
	const auto& samplerBindingsLayout = pipeline.layout().descriptorSet(1);
	auto samplerBindings = samplerBindingsLayout.allocate();
	auto sampler = this->getDevice()->factory().createSampler(FilterMode::Linear, FilterMode::Linear, BorderMode::ClampToEdge, BorderMode::ClampToEdge, BorderMode::ClampToEdge);
	samplerBindings->update(0, *sampler);

	// Begin the pipeline usage.
	pipeline.use();
	auto size = this->extent();

	for (int l(0); l < this->layers(); ++l)
	{
		for (UInt32 i(1); i < this->levels(); ++i, size /= 2)
		{
			// Update the invocation parameters.
			// TODO: Use push constants here.
			parametersData.sizeX = 1.f / static_cast<Float>(std::max<size_t>(size.width(), 1));
			parametersData.sizeY = 1.f / static_cast<Float>(std::max<size_t>(size.height(), 1));
			resourceBindings->update(parametersLayout.binding(), *parameters);

			// Bind the previous mip map level to the SRV at binding point 1.
			resourceBindings->update(1, *this, 0, i - 1, 1);

			// Bind the current level to the UAV at binding point 2.
			resourceBindings->update(2, *this, 0, i, 1);

			// Dispatch the pipeline.
			pipeline.dispatch({ 8, 8, 1 });
		}
	}

	pipeline.submit(true);
}

void DirectX12Texture::receiveData(const DirectX12CommandBuffer& commandBuffer, const bool& receive) const noexcept
{
	if ((receive && this->state() != D3D12_RESOURCE_STATE_COPY_DEST) || (!receive && this->state() == D3D12_RESOURCE_STATE_COPY_DEST))
		this->transitionTo(commandBuffer, receive ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void DirectX12Texture::sendData(const DirectX12CommandBuffer& commandBuffer, const bool& emit) const noexcept
{
	if ((emit && this->state() != D3D12_RESOURCE_STATE_COPY_SOURCE) || (!emit && this->state() == D3D12_RESOURCE_STATE_COPY_SOURCE))
		this->transitionTo(commandBuffer, emit ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void DirectX12Texture::transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (this->levels() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The image has only {0} mip-map levels, but a transfer for {1} levels starting from level {2} has been requested. For transfers of multiple layers or planes, use multiple transfer commands instead.", this->levels(), elements, targetElement);

	source.sendData(commandBuffer, true);	
	this->receiveData(commandBuffer, true);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& bufferDesc = this->handle()->GetDesc();

	for (int resource(0); resource < elements; ++resource)
	{
		this->getDevice()->handle()->GetCopyableFootprints(&bufferDesc, sourceElement + resource, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source.handle().Get(), footprint), targetLocation(this->handle().Get(), m_impl->getSubresourceId(targetElement + resource, layer, plane));
		commandBuffer.handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}

	if (!leaveSourceState)
		source.sendData(commandBuffer, false);

	if (!leaveTargetState)
		this->receiveData(commandBuffer, false);
}

void DirectX12Texture::transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements, const bool& leaveSourceState, const bool& leaveTargetState, const UInt32& layer, const UInt32& plane) const
{
	if (target.elements() <= targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	if (this->levels() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The image has only {0} mip-map levels, but a transfer for {1} levels starting from level {2} has been requested. For transfers of multiple layers or planes, use multiple transfer commands instead.", this->levels(), elements, sourceElement);

	this->sendData(commandBuffer, true);
	target.receiveData(commandBuffer, true);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& bufferDesc = this->handle()->GetDesc();

	for (int resource(0); resource < elements; ++resource)
	{
		this->getDevice()->handle()->GetCopyableFootprints(&bufferDesc, m_impl->getSubresourceId(sourceElement + resource, layer, plane), 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(this->handle().Get(), footprint), targetLocation(target.handle().Get(), targetElement + resource);
		commandBuffer.handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}

	if (!leaveSourceState)
		this->sendData(commandBuffer, false);

	if (!leaveTargetState)
		target.receiveData(commandBuffer, false);
}

UniquePtr<DirectX12Texture> DirectX12Texture::allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc)
{
	if (allocator == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("The allocator must be initialized.");

	ComPtr<ID3D12Resource> resource;
	D3D12MA::Allocation* allocation;
	raiseIfFailed<RuntimeException>(allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)), "Unable to create texture resource.");
	LITEFX_DEBUG(DIRECTX12_LOG, "Allocated texture {0} with {1} bytes {{ Extent: {2}x{3} Px, Format: {4}, Samples: {5}, Levels: {6}, Layers: {7} }}", fmt::ptr(resource.Get()), ::getSize(format) * extent.width() * extent.height(), extent.width(), extent.height(), format, samples, levels, layers);

	return makeUnique<DirectX12Texture>(device, std::move(resource), extent, format, dimension, levels, layers, samples, initialState, allocator, AllocationPtr(allocation));
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

public:
	DirectX12SamplerImpl(DirectX12Sampler* parent, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipMapBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sampler shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Sampler::DirectX12Sampler(const DirectX12Device& device, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	DirectX12RuntimeObject<DirectX12Device>(device, &device), m_impl(makePimpl<DirectX12SamplerImpl>(this, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
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