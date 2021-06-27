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
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="image"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="dimension"></param>
		/// <param name="initialState"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr);
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

		// IImage interface.
	public:
		/// <inheritdoc />
		virtual const Size2d& extent() const noexcept override;

		/// <inheritdoc />
		virtual const Format& format() const noexcept override;

		/// <inheritdoc />
		virtual const ImageDimensions& dimensions() const noexcept override;

		// IDirectX12Resource interface.
	public:
		virtual const D3D12_RESOURCE_STATES& state() const noexcept override;
		virtual D3D12_RESOURCE_STATES& state() noexcept override;
		virtual D3D12_RESOURCE_BARRIER transitionTo(const D3D12_RESOURCE_STATES& state, const UInt32& element = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, const D3D12_RESOURCE_BARRIER_FLAGS& flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) const override;
		virtual void transitionTo(const DirectX12CommandBuffer& commandBuffer, const D3D12_RESOURCE_STATES& state, const UInt32& element = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, const D3D12_RESOURCE_BARRIER_FLAGS& flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) const override;

		// DirectX 12 image.
	protected:
		virtual AllocatorPtr allocator() const noexcept;
		virtual const D3D12MA::Allocation* allocationInfo() const noexcept;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allocator"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="dimension"></param>
		/// <param name="initialState"></param>
		/// <param name="resourceDesc"></param>
		/// <param name="allocationDesc"></param>
		/// <returns></returns>
		static UniquePtr<DirectX12Image> allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="ITexture" />.
	/// </summary>
	class DirectX12Texture : public DirectX12Image, public IDirectX12Texture {
		LITEFX_IMPLEMENTATION(DirectX12TextureImpl);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="layout"></param>
		/// <param name="image"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="levels"></param>
		/// <param name="samples"></param>
		/// <param name="initialState"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit DirectX12Texture(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, ComPtr<ID3D12Resource>&& image, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr);
		DirectX12Texture(DirectX12Texture&&) = delete;
		DirectX12Texture(const DirectX12Texture&) = delete;
		virtual ~DirectX12Texture() noexcept;

		// IBindable interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		// IDescriptor interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12DescriptorLayout& layout() const noexcept override;

		// ITexture interface.
	public:
		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& levels() const noexcept override;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		virtual void transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		/// <remarks>
		/// Note that images are always transferred as a whole. Transferring only regions is currently unsupported. Hence the <paramref name="size" /> and <paramref name="sourceOffset" />
		/// parameters are ignored and can be simply set to <c>0</c>.
		/// </remarks>
		virtual void transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="layout"></param>
		/// <param name="allocator"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="levels"></param>
		/// <param name="samples"></param>
		/// <param name="initialState"></param>
		/// <param name="resourceDesc"></param>
		/// <param name="allocationDesc"></param>
		/// <returns></returns>
		static UniquePtr<DirectX12Texture> allocate(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, AllocatorPtr allocator, const Size2d& extent, const Format& format, const ImageDimensions& dimension, const UInt32& levels, const MultiSamplingLevel& samples, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
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
		/// <param name="layout"></param>
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
		explicit DirectX12Sampler(const DirectX12Device& device, const DirectX12DescriptorLayout& layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f);
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

		// IBindable interface.
	public:
		virtual const UInt32& binding() const noexcept override;

		// IDescriptor interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12DescriptorLayout& layout() const noexcept override;
	};
}