#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "buffer.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a DirectX12 <see cref="IImage" />.
	/// </summary>
	class DirectX12Image : public DirectX12RuntimeObject<DirectX12Device>, public IDirectX12Image, public ComResource<ID3D12Resource> {
		LITEFX_IMPLEMENTATION(DirectX12ImageImpl);

	public:
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr);
		DirectX12Image(DirectX12Image&&) = delete;
		DirectX12Image(const DirectX12Image&) = delete;
		virtual ~DirectX12Image() noexcept;

		// IDeviceMemory interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& elements() const noexcept override;

		/// <inheritdoc />
		virtual size_t size() const noexcept override;

		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual size_t elementAlignment() const noexcept override;

		/// <inheritdoc />
		virtual size_t alignedElementSize() const noexcept override;

		/// <inheritdoc />
		virtual const bool& writable() const noexcept override;

		/// <inheritdoc />
		virtual const ResourceState& state(const UInt32& subresource = 0) const override;

		/// <inheritdoc />
		virtual ResourceState& state(const UInt32& subresource = 0) override;

		// IImage interface.
	public:
		/// <inheritdoc />
		virtual size_t size(const UInt32& level) const noexcept override;

		/// <inheritdoc />
		virtual Size3d extent(const UInt32& level = 0) const noexcept override;

		/// <inheritdoc />
		virtual const Format& format() const noexcept override;

		/// <inheritdoc />
		virtual const ImageDimensions& dimensions() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& levels() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& layers() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& planes() const noexcept override;

		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

		// DirectX 12 image.
	public:
		virtual AllocatorPtr allocator() const noexcept;
		virtual const D3D12MA::Allocation* allocationInfo() const noexcept;
		virtual void generateMipMaps(const DirectX12CommandBuffer& commandBuffer) noexcept;

	public:
		static UniquePtr<DirectX12Image> allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="ISampler" />.
	/// </summary>
	class DirectX12Sampler : public DirectX12RuntimeObject<DirectX12Device>, public IDirectX12Sampler {
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
		explicit DirectX12Sampler(const DirectX12Device& device, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f);
		DirectX12Sampler(DirectX12Sampler&&) = delete;
		DirectX12Sampler(const DirectX12Sampler&) = delete;
		virtual ~DirectX12Sampler() noexcept;

		// ISampler interface.
	public:
		/// <inheritdoc />
		virtual const FilterMode& getMinifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual const FilterMode& getMagnifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeU() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeV() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeW() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getAnisotropy() const noexcept override;

		/// <inheritdoc />
		virtual const MipMapMode& getMipMapMode() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMipMapBias() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMaxLOD() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMinLOD() const noexcept override;
	};
}