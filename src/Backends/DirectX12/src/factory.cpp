#include <litefx/backends/dx12.hpp>
#include "buffer.h"
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl {
public:
	friend class DirectX12GraphicsFactory;

private:
	WeakPtr<const DirectX12Device> m_device;
	AllocatorPtr m_allocator;

public:
	DirectX12GraphicsFactoryImpl(const DirectX12Device& device) :
		m_device(device.weak_from_this())
	{
		// Initialize memory allocator.
		D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
		//allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED;
		allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
		allocatorDesc.pAdapter = device.adapter().handle().Get();
		allocatorDesc.pDevice = device.handle().Get();
		allocatorDesc.PreferredBlockSize = 0;	// TODO: Make configurable.

		D3D12MA::Allocator* allocator{};
		raiseIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &allocator), "Unable to create D3D12 memory allocator.");
		m_allocator.reset(allocator, D3D12MADeleter{});

		// Listen to swap chain buffer swap events, in order to call `SetCurrentFrameIndex`.
		device.swapChain().swapped += std::bind(&DirectX12GraphicsFactory::DirectX12GraphicsFactoryImpl::onBackBufferSwap, this, std::placeholders::_1, std::placeholders::_2);
	}

private:
	void onBackBufferSwap([[maybe_unused]] const void* sender, const ISwapChain::BackBufferSwapEventArgs& e) {
		m_allocator->SetCurrentFrameIndex(e.backBuffer());
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsFactory::DirectX12GraphicsFactory(const DirectX12Device& device) :
	m_impl(device)
{
}

DirectX12GraphicsFactory::~DirectX12GraphicsFactory() noexcept = default;

bool DirectX12GraphicsFactory::supportsResizableBaseAddressRegister() const noexcept
{
	return m_impl->m_allocator->IsGPUUploadHeapSupported();
}

SharedPtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const
{
	return this->createBuffer("", type, heap, elementSize, elements, usage);
}

SharedPtr<IDirectX12Buffer> DirectX12GraphicsFactory::createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const
{
	// Validate inputs.
	if ((type == BufferType::Vertex || type == BufferType::Index || type == BufferType::Uniform) && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex, index and uniform/constant buffers cannot be written to.");

	if (type == BufferType::AccelerationStructure && LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: acceleration structures cannot be used as build inputs for other acceleration structures.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;

	// Constant buffers are aligned to 256 byte chunks. All other buffers can be aligned to a multiple of 16 bytes (D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT). The actual amount of memory allocated 
	// is then defined as the smallest multiple of 64kb, that's greater or equal to `resourceDesc.Width` below. For more info, see:
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getresourceallocationinfo#remarks.
	size_t elementAlignment = 0;

	switch (type)
	{
	case BufferType::Uniform: elementAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT; break;
	case BufferType::Vertex:
	case BufferType::Index:   elementAlignment = 0; break;
	default:                  elementAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT; break;
	}

	D3D12_RESOURCE_DESC1 resourceDesc { };
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = elements * (elementAlignment > 0 ? Math::align(elementSize, elementAlignment) : elementSize);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	if (type == BufferType::AccelerationStructure)
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

	D3D12MA::ALLOCATION_DESC allocationDesc { };

	switch (heap)
	{
	case ResourceHeap::Dynamic:
	case ResourceHeap::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case ResourceHeap::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case ResourceHeap::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("heap", "The buffer heap {0} is not supported.", heap);
	}

	return DirectX12Buffer::allocate(name, m_impl->m_allocator, type, elements, elementSize, elementAlignment, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	return this->createVertexBuffer("", layout, heap, elements, usage);
}

SharedPtr<IDirectX12VertexBuffer> DirectX12GraphicsFactory::createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	// Validate usage.
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: vertex buffers cannot be written to.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;

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

	switch (heap)
	{
	case ResourceHeap::Dynamic:
	case ResourceHeap::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case ResourceHeap::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case ResourceHeap::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("heap", "The buffer heap {0} is not supported.", heap);
	}

	return DirectX12VertexBuffer::allocate(name, layout, m_impl->m_allocator, elements, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	return this->createIndexBuffer("", layout, heap, elements, usage);
}

SharedPtr<IDirectX12IndexBuffer> DirectX12GraphicsFactory::createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const
{
	// Validate usage.
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: index buffers cannot be written to.");

	// Set heap-default usages.
	if (heap == ResourceHeap::Staging && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferSource))
		usage |= ResourceUsage::TransferSource;
	else if (heap == ResourceHeap::Readback && !LITEFX_FLAG_IS_SET(usage, ResourceUsage::TransferDestination))
		usage |= ResourceUsage::TransferDestination;

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

	switch (heap)
	{
	case ResourceHeap::Dynamic:
	case ResourceHeap::Staging:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		break;
	case ResourceHeap::Resource:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case ResourceHeap::Readback:
		allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		break;
	default:
		throw InvalidArgumentException("heap", "The buffer heap {0} is not supported.", heap);
	}

	return DirectX12IndexBuffer::allocate(name, layout, m_impl->m_allocator, elements, usage, resourceDesc, allocationDesc);
}

SharedPtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	return this->createTexture("", format, size, dimension, levels, layers, samples, usage);
}

SharedPtr<IDirectX12Image> DirectX12GraphicsFactory::createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot allocate texture from a released device instance.");

	// Validate usage flags
	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::AccelerationStructureBuildInput)) [[unlikely]]
		throw InvalidArgumentException("usage", "Invalid resource usage has been specified: image resources cannot be used as build inputs for other acceleration structures.");

	// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
	if (dimension == ImageDimensions::CUBE && layers != 6) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", std::make_pair(6u, 6u), layers, "A cube map must be defined with 6 layers, but {0} are provided.", layers);

	if (dimension == ImageDimensions::DIM_3 && layers != 1) [[unlikely]]
		throw ArgumentOutOfRangeException("layers", std::make_pair(1u, 1u), layers, "A 3D texture can only have one layer, but {0} are provided.", layers);
	// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

	auto width = std::max<UInt32>(1, static_cast<UInt32>(size.width()));
	auto height = std::max<UInt32>(1, static_cast<UInt32>(size.height()));
	auto depth = std::max<UInt32>(1, static_cast<UInt32>(size.depth()));

	D3D12_RESOURCE_FLAGS flags = LITEFX_FLAG_IS_SET(usage, ResourceUsage::AllowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	if (LITEFX_FLAG_IS_SET(usage, ResourceUsage::RenderTarget))
	{
		if (::hasDepth(format) || ::hasStencil(format))
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		else
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	D3D12_RESOURCE_DESC1 resourceDesc = {
		.Dimension = DX12::getImageType(dimension),
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = static_cast<UInt16>(dimension == ImageDimensions::DIM_3 ? depth : layers),
		.MipLevels = static_cast<UInt16>(levels),
		.Format = DX12::getFormat(format),
		.SampleDesc = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ static_cast<UInt32>(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN },
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags,
	};

	D3D12MA::ALLOCATION_DESC allocationDesc { .HeapType = D3D12_HEAP_TYPE_DEFAULT };
	
	return DirectX12Image::allocate(name, *device.get(), m_impl->m_allocator, {width, height, depth}, format, dimension, levels, layers, samples, usage, resourceDesc, allocationDesc);
}

Generator<SharedPtr<IDirectX12Image>> DirectX12GraphicsFactory::createTextures(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const
{
	return [](SharedPtr<const DirectX12GraphicsFactory> factory, Format format, Size3d size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) -> Generator<SharedPtr<IDirectX12Image>> {
		for (;;)
			co_yield factory->createTexture(format, size, dimension, levels, layers, samples, usage);
	}(this->shared_from_this(), format, size, dimension, levels, layers, samples, usage);
}

SharedPtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return DirectX12Sampler::allocate(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

SharedPtr<IDirectX12Sampler> DirectX12GraphicsFactory::createSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return DirectX12Sampler::allocate(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
}

Generator<SharedPtr<IDirectX12Sampler>> DirectX12GraphicsFactory::createSamplers(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const
{
	return [](SharedPtr<const DirectX12GraphicsFactory> factory, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) -> Generator<SharedPtr<IDirectX12Sampler>> {
		for (;;)
			co_yield factory->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
	}(this->shared_from_this(), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

UniquePtr<DirectX12BottomLevelAccelerationStructure> DirectX12GraphicsFactory::createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<DirectX12BottomLevelAccelerationStructure>(flags, name);
}

UniquePtr<DirectX12TopLevelAccelerationStructure> DirectX12GraphicsFactory::createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const
{
	return makeUnique<DirectX12TopLevelAccelerationStructure>(flags, name);
}