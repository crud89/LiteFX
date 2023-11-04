#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "buffer.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a DirectX12 <see cref="IImage" />.
	/// </summary>
	class DirectX12Image : public virtual IDirectX12Image, public ComResource<ID3D12Resource>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12ImageImpl);

	public:
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr, const String& name = "");
		DirectX12Image(DirectX12Image&&) = delete;
		DirectX12Image(const DirectX12Image&) = delete;
		virtual ~DirectX12Image() noexcept;

		// IDeviceMemory interface.
	public:
		/// <inheritdoc />
		virtual UInt32 elements() const noexcept override;

		/// <inheritdoc />
		virtual size_t size() const noexcept override;

		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual size_t elementAlignment() const noexcept override;

		/// <inheritdoc />
		virtual size_t alignedElementSize() const noexcept override;

		/// <inheritdoc />
		virtual bool writable() const noexcept override;

		/// <inheritdoc />
		virtual ImageLayout layout(UInt32 subresource = 0) const override;

		/// <inheritdoc />
		virtual ImageLayout& layout(UInt32 subresource = 0) override;

		// IImage interface.
	public:
		/// <inheritdoc />
		virtual size_t size(UInt32 level) const noexcept override;

		/// <inheritdoc />
		virtual Size3d extent(UInt32 level = 0) const noexcept override;

		/// <inheritdoc />
		virtual Format format() const noexcept override;

		/// <inheritdoc />
		virtual ImageDimensions dimensions() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 levels() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 layers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 planes() const noexcept override;

		/// <inheritdoc />
		virtual MultiSamplingLevel samples() const noexcept override;

		// DirectX 12 image.
	public:
		virtual AllocatorPtr allocator() const noexcept;
		virtual const D3D12MA::Allocation* allocationInfo() const noexcept;

	public:
		static UniquePtr<DirectX12Image> allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static UniquePtr<DirectX12Image> allocate(const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, bool writable, ImageLayout initialLayout, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
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
		/// <param name="device"></param>
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
		explicit DirectX12Sampler(const DirectX12Device& device, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "");
		DirectX12Sampler(DirectX12Sampler&&) = delete;
		DirectX12Sampler(const DirectX12Sampler&) = delete;
		virtual ~DirectX12Sampler() noexcept;

		// ISampler interface.
	public:
		/// <inheritdoc />
		virtual FilterMode getMinifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual FilterMode getMagnifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual BorderMode getBorderModeU() const noexcept override;

		/// <inheritdoc />
		virtual BorderMode getBorderModeV() const noexcept override;

		/// <inheritdoc />
		virtual BorderMode getBorderModeW() const noexcept override;

		/// <inheritdoc />
		virtual Float getAnisotropy() const noexcept override;

		/// <inheritdoc />
		virtual MipMapMode getMipMapMode() const noexcept override;

		/// <inheritdoc />
		virtual Float getMipMapBias() const noexcept override;

		/// <inheritdoc />
		virtual Float getMaxLOD() const noexcept override;

		/// <inheritdoc />
		virtual Float getMinLOD() const noexcept override;
	};
}