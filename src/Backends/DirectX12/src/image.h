#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "buffer.h"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// @brief Implements a DirectX12 @ref IImage.
	class DirectX12Image : public virtual IDirectX12Image, public ComResource<ID3D12Resource>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12ImageImpl);
		friend class DirectX12SwapChain::DirectX12SwapChainImpl; // Allows swap chain to wrap back buffer images.
		friend class DirectX12GraphicsFactory; // Allows aliasing resource to be directly initialized.
		friend struct SharedObject::Allocator<DirectX12Image>;

	private:
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, AllocatorPtr allocator = nullptr, AllocationPtr allocation = nullptr, const String& name = "");
		
	public:
		DirectX12Image(DirectX12Image&&) noexcept = delete;
		DirectX12Image(const DirectX12Image&) = delete;
		DirectX12Image& operator=(DirectX12Image&&) noexcept = delete;
		DirectX12Image& operator=(const DirectX12Image&) = delete;
		~DirectX12Image() override;

		// IDeviceMemory interface.
	public:
		/// @copydoc IDeviceMemory::elements()
		UInt32 elements() const noexcept override;

		/// @copydoc IDeviceMemory::size()
		size_t size() const noexcept override;

		/// @copydoc IDeviceMemory::elementSize()
		size_t elementSize() const noexcept override;

		/// @copydoc IDeviceMemory::elementAlignment()
		size_t elementAlignment() const noexcept override;

		/// @copydoc IDeviceMemory::alignedElementSize()
		size_t alignedElementSize() const noexcept override;

		/// @copydoc IDeviceMemory::usage()
		ResourceUsage usage() const noexcept override;

		/// @copydoc IDeviceMemory::virtualAddress()
		UInt64 virtualAddress() const noexcept override;

		// IImage interface.
	public:
		/// @copydoc IImage::size()
		size_t size(UInt32 level) const override;

		/// @copydoc IImage::extent()
		Size3d extent(UInt32 level = 0) const noexcept override;

		/// @copydoc IImage::format()
		Format format() const noexcept override;

		/// @copydoc IImage::dimensions()
		ImageDimensions dimensions() const noexcept override;

		/// @copydoc IImage::levels()
		UInt32 levels() const noexcept override;

		/// @copydoc IImage::layers()
		UInt32 layers() const noexcept override;

		/// @copydoc IImage::planes()
		UInt32 planes() const noexcept override;

		/// @copydoc IImage::samples()
		MultiSamplingLevel samples() const noexcept override;

		// DirectX 12 image.
	public:
		/// @brief Returns the pointer to the underlying allocator.
		/// 
		/// @return The pointer to the underlying allocator.
		AllocatorPtr allocator() const noexcept;

		/// @brief Returns the allocation info.
		/// 
		/// @return A pointer to the allocation info.
		const D3D12MA::Allocation* allocationInfo() const noexcept;

	private:
		static inline auto create(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const AllocatorPtr& allocator = nullptr, AllocationPtr allocation = nullptr, const String& name = "") {
			return SharedObject::create<DirectX12Image>(device, std::move(image), extent, format, dimension, levels, layers, samples, usage, resourceDesc, allocator, std::move(allocation), name);
		}

	public:
		static SharedPtr<IDirectX12Image> allocate(const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static bool tryAllocate(SharedPtr<IDirectX12Image>& image, const String& name, const DirectX12Device& device, AllocatorPtr allocator, const Size3d& extent, Format format, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);

		static bool move(SharedPtr<IDirectX12Image> image, D3D12MA::Allocation* to, const DirectX12CommandBuffer& commandBuffer);
	};

	/// @brief Implements a DirectX 12 @ref ISampler.
	class DirectX12Sampler : public virtual IDirectX12Sampler, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12SamplerImpl);
		friend struct SharedObject::Allocator<DirectX12Sampler>;

	private:
		/// @brief Initializes a new sampler instance.
		///
		/// @param magFilter
		/// @param minFilter
		/// @param borderU
		/// @param borderV
		/// @param borderW
		/// @param mipMapMode
		/// @param mipMapBias
		/// @param maxLod
		/// @param minLod
		/// @param anisotropy
		explicit DirectX12Sampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "");
		
		DirectX12Sampler(DirectX12Sampler&&) noexcept = delete;
		DirectX12Sampler(const DirectX12Sampler&) = delete;
		auto operator=(DirectX12Sampler&&) noexcept = delete;
		auto operator=(const DirectX12Sampler&) = delete;

	public:
		~DirectX12Sampler() noexcept override;

		// ISampler interface.
	public:
		/// @copydoc ISampler::getMinifyingFilter()
		FilterMode getMinifyingFilter() const noexcept override;

		/// @copydoc ISampler::getMagnifyingFilter()
		FilterMode getMagnifyingFilter() const noexcept override;

		/// @copydoc ISampler::getBorderModeU()
		BorderMode getBorderModeU() const noexcept override;

		/// @copydoc ISampler::getBorderModeV()
		BorderMode getBorderModeV() const noexcept override;

		/// @copydoc ISampler::getBorderModeW()
		BorderMode getBorderModeW() const noexcept override;

		/// @copydoc ISampler::getAnisotropy()
		Float getAnisotropy() const noexcept override;

		/// @copydoc ISampler::getMipMapMode()
		MipMapMode getMipMapMode() const noexcept override;

		/// @copydoc ISampler::getMipMapBias()
		Float getMipMapBias() const noexcept override;

		/// @copydoc ISampler::getMaxLOD()
		Float getMaxLOD() const noexcept override;

		/// @copydoc ISampler::getMinLOD()
		Float getMinLOD() const noexcept override;

	public:
		static inline SharedPtr<DirectX12Sampler> copy(const IDirectX12Sampler& sampler) {
			return SharedObject::create<DirectX12Sampler>(sampler.getMagnifyingFilter(), sampler.getMinifyingFilter(), sampler.getBorderModeU(), sampler.getBorderModeV(), sampler.getBorderModeW(), sampler.getMipMapMode(), sampler.getMipMapBias(), sampler.getMinLOD(), sampler.getMaxLOD(), sampler.getAnisotropy(), sampler.name());
		}

		static inline SharedPtr<DirectX12Sampler> allocate(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "") {
			return SharedObject::create<DirectX12Sampler>(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
		}
	};
}

#pragma warning(pop)