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
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit DirectX12Image(const DirectX12Device& device, ComPtr<ID3D12Resource>&& image, const Size2d& extent, const Format& format, const D3D12_RESOURCE_STATES& initialState, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr);
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
		/// <param name="elements"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="allocator"></param>
		/// <param name="createInfo"></param>
		/// <param name="allocationInfo"></param>
		/// <param name="allocationResult"></param>
		/// <returns></returns>
		static UniquePtr<DirectX12Image> allocate(const DirectX12Device& device, AllocatorPtr allocator, const Size2d& extent, const Format& format, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};
}