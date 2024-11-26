#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "buffer.h"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a DirectX12 <see cref="IImage" />.
	/// </summary>
	class DirectX12Image : public virtual IDirectX12Image, public ComResource<ID3D12Resource>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12ImageImpl);

	public:
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr, const String& name = "");
		
		DirectX12Image(DirectX12Image&&) noexcept = delete;
		DirectX12Image(const DirectX12Image&) noexcept = delete;
		DirectX12Image& operator=(DirectX12Image&&) noexcept = delete;
		DirectX12Image& operator=(const DirectX12Image&) noexcept = delete;
		~DirectX12Image() noexcept override;

		// IDeviceMemory interface.
	public:
		/// <inheritdoc />
		UInt32 elements() const noexcept override;

		/// <inheritdoc />
		size_t size() const noexcept override;

		/// <inheritdoc />
		size_t elementSize() const noexcept override;

		/// <inheritdoc />
		size_t elementAlignment() const noexcept override;

		/// <inheritdoc />
		size_t alignedElementSize() const noexcept override;

		/// <inheritdoc />
		ResourceUsage usage() const noexcept override;

		/// <inheritdoc />
		UInt64 virtualAddress() const noexcept override;

		// IImage interface.
	public:
		/// <inheritdoc />
		size_t size(UInt32 level) const noexcept override;

		/// <inheritdoc />
		Size3d extent(UInt32 level = 0) const noexcept override;

		/// <inheritdoc />
		Format format() const noexcept override;

		/// <inheritdoc />
		ImageDimensions dimensions() const noexcept override;

		/// <inheritdoc />
		UInt32 levels() const noexcept override;

		/// <inheritdoc />
		UInt32 layers() const noexcept override;

		/// <inheritdoc />
		UInt32 planes() const noexcept override;

		/// <inheritdoc />
		MultiSamplingLevel samples() const noexcept override;

		// DirectX 12 image.
	public:
		virtual AllocatorPtr allocator() const noexcept;
		virtual const D3D12MA::Allocation* allocationInfo() const noexcept;

	public:
		static UniquePtr<DirectX12Image> allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static UniquePtr<DirectX12Image> allocate(const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="ISampler" />.
	/// </summary>
	class DirectX12Sampler : public virtual IDirectX12Sampler, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12SamplerImpl);

	public:
		/// <summary>
		/// Initializes a new sampler instance.
		/// </summary>
		/// <param name="magFilter"></param>
		/// <param name="minFilter"></param>
		/// <param name="borderU"></param>
		/// <param name="borderV"></param>
		/// <param name="borderW"></param>
		/// <param name="mipMapMode"></param>
		/// <param name="mipMapBias"></param>
		/// <param name="maxLod"></param>
		/// <param name="minLod"></param>
		/// <param name="anisotropy"></param>
		explicit DirectX12Sampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "");
		
		DirectX12Sampler(DirectX12Sampler&&) noexcept = delete;
		DirectX12Sampler(const DirectX12Sampler&) noexcept = delete;
		auto operator=(DirectX12Sampler&&) noexcept = delete;
		auto operator=(const DirectX12Sampler&) noexcept = delete;
		~DirectX12Sampler() noexcept override;

		// ISampler interface.
	public:
		/// <inheritdoc />
		FilterMode getMinifyingFilter() const noexcept override;

		/// <inheritdoc />
		FilterMode getMagnifyingFilter() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeU() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeV() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeW() const noexcept override;

		/// <inheritdoc />
		Float getAnisotropy() const noexcept override;

		/// <inheritdoc />
		MipMapMode getMipMapMode() const noexcept override;

		/// <inheritdoc />
		Float getMipMapBias() const noexcept override;

		/// <inheritdoc />
		Float getMaxLOD() const noexcept override;

		/// <inheritdoc />
		Float getMinLOD() const noexcept override;
	};
}

#pragma warning(pop)