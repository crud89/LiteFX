#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "D3D12MemAlloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	struct D3D12MADeleter {
		void operator()(auto* ptr) noexcept {
			ptr->Release();
		}
	};

	typedef SharedPtr<D3D12MA::Allocator> AllocatorPtr;
	typedef UniquePtr<D3D12MA::Allocation, D3D12MADeleter> AllocationPtr;

	class DirectX12Buffer : public DirectX12RuntimeObject<DirectX12Device>, public IDirectX12Buffer, public ComResource<ID3D12Resource> {
		LITEFX_IMPLEMENTATION(DirectX12BufferImpl);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="buffer"></param>
		/// <param name="type"></param>
		/// <param name="elements"></param>
		/// <param name="elementSize"></param>
		/// <param name="alignment"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit DirectX12Buffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr);
		DirectX12Buffer(DirectX12Buffer&&) = delete;
		DirectX12Buffer(const DirectX12Buffer&) = delete;
		virtual ~DirectX12Buffer() noexcept;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		virtual void transferFrom(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& source, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transferTo(const DirectX12CommandBuffer& commandBuffer, const IDirectX12Buffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		// IBuffer interface.
	public:
		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;

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

		// IMappable interface.
	public:
		/// <inheritdoc />
		virtual void map(const void* const data, const size_t& size, const UInt32& element = 0) override;

		/// <inheritdoc />
		virtual void map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement = 0) override;

		// DirectX12Buffer.
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allocator"></param>
		/// <param name="type"></param>
		/// <param name="elements"></param>
		/// <param name="elementSize"></param>
		/// <param name="alignment"></param>
		/// <param name="initialState"></param>
		/// <param name="resourceDesc"></param>
		/// <param name="allocationDesc"></param>
		/// <returns></returns>
		static UniquePtr<IDirectX12Buffer> allocate(const DirectX12Device& device, AllocatorPtr allocator, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	class DirectX12VertexBuffer : public DirectX12Buffer, public IDirectX12VertexBuffer {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferImpl);

	public:
		explicit DirectX12VertexBuffer(const DirectX12Device& device, ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, const UInt32& elements, AllocatorPtr allocator, AllocationPtr&& allocation);
		DirectX12VertexBuffer(DirectX12VertexBuffer&&) = delete;
		DirectX12VertexBuffer(const DirectX12VertexBuffer&) = delete;
		virtual ~DirectX12VertexBuffer() noexcept;

		// IVertexBuffer interface.
	public:
		/// <inheritdoc />
		const DirectX12VertexBufferLayout& layout() const noexcept override;

		// IBindable interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		// DirectX12VertexBuffer.
	public:
		static UniquePtr<IDirectX12VertexBuffer> allocate(const DirectX12Device& device, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, const UInt32& elements, const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	//class DirectX12IndexBuffer : public DirectX12Buffer, public IDirectX12IndexBuffer {
	//	LITEFX_IMPLEMENTATION(DirectX12IndexBufferImpl);

	//public:
	//	explicit DirectX12IndexBuffer(const DirectX12Device& device, VkBuffer buffer, const DirectX12IndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation);
	//	DirectX12IndexBuffer(DirectX12IndexBuffer&&) = delete;
	//	DirectX12IndexBuffer(const DirectX12IndexBuffer&) = delete;
	//	virtual ~DirectX12IndexBuffer() noexcept;

	//	// IIndexBuffer interface.
	//public:
	//	/// <inheritdoc />
	//	virtual const DirectX12IndexBufferLayout& layout() const noexcept override;

	//	// DirectX12IndexBuffer.
	//public:
	//	static UniquePtr<IDirectX12IndexBuffer> allocate(const DirectX12IndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	//};

	//class DirectX12ConstantBuffer : public DirectX12Buffer, public IDirectX12ConstantBuffer {
	//	LITEFX_IMPLEMENTATION(DirectX12ConstantBufferImpl);

	//public:
	//	explicit DirectX12ConstantBuffer(const DirectX12Device& device, VkBuffer buffer, const DirectX12DescriptorLayout& layout, const UInt32& elements, const size_t& alignment, const VmaAllocator& allocator, const VmaAllocation& allocation);
	//	DirectX12ConstantBuffer(DirectX12ConstantBuffer&&) = delete;
	//	DirectX12ConstantBuffer(const DirectX12ConstantBuffer&) = delete;
	//	virtual ~DirectX12ConstantBuffer() noexcept;

	//	// IDescriptor interface.
	//public:
	//	/// <inheritdoc />
	//	const DirectX12DescriptorLayout& layout() const noexcept override;

	//	// IBindable interface.
	//public:
	//	/// <inheritdoc />
	//	virtual const UInt32& binding() const noexcept override;

	//	// DirectX12ConstantBuffer.
	//public:
	//	static UniquePtr<IDirectX12ConstantBuffer> allocate(const DirectX12DescriptorLayout& layout, const UInt32& elements, const size_t& alignment, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	//};
}