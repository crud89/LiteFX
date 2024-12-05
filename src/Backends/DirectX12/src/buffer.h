#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/dx12.hpp>
#include "D3D12MemAlloc.h"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	struct D3D12MADeleter {
		void operator()(auto* ptr) noexcept {
			ptr->Release();
		}
	};

	typedef SharedPtr<D3D12MA::Allocator> AllocatorPtr;
	typedef UniquePtr<D3D12MA::Allocation, D3D12MADeleter> AllocationPtr;

	/// <summary>
	/// Implements a DirectX 12 <see cref="IBuffer" />.
	/// </summary>
	class DirectX12Buffer : public virtual IDirectX12Buffer, public ComResource<ID3D12Resource>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(DirectX12BufferImpl);
		friend struct SharedAllocator<DirectX12Buffer>;

	protected:
		explicit DirectX12Buffer(ComPtr<ID3D12Resource>&& buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, AllocatorPtr allocator = nullptr, AllocationPtr&& allocation = nullptr, const String& name = "");
		
		DirectX12Buffer(DirectX12Buffer&&) noexcept = delete;
		DirectX12Buffer(const DirectX12Buffer&) = delete;
		DirectX12Buffer& operator=(DirectX12Buffer&&) noexcept = delete;
		DirectX12Buffer& operator=(const DirectX12Buffer&) = delete;

	public:
		~DirectX12Buffer() noexcept override;

		// IBuffer interface.
	public:
		/// <inheritdoc />
		BufferType type() const noexcept override;

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

		// IMappable interface.
	public:
		/// <inheritdoc />
		void map(const void* const data, size_t size, UInt32 element = 0) override;

		/// <inheritdoc />
		void map(Span<const void* const> data, size_t elementSize, UInt32 firstElement = 0) override;

		/// <inheritdoc />
		void map(void* data, size_t size, UInt32 element = 0, bool write = true) override;

		/// <inheritdoc />
		void map(Span<void*> data, size_t elementSize, UInt32 firstElement = 0, bool write = true) override;

		// DirectX 12 buffer.
	protected:
		virtual AllocatorPtr allocator() const noexcept;
		virtual const D3D12MA::Allocation* allocationInfo() const noexcept;

	public:
		static SharedPtr<IDirectX12Buffer> allocate(AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static SharedPtr<IDirectX12Buffer> allocate(const String& name, AllocatorPtr allocator, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="IVertexBuffer" />.
	/// </summary>
	class DirectX12VertexBuffer : public DirectX12Buffer, public virtual IDirectX12VertexBuffer {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferImpl);
		friend struct SharedAllocator<DirectX12VertexBuffer>;

	private:
		explicit DirectX12VertexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12VertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name = "");

		DirectX12VertexBuffer(DirectX12VertexBuffer&&) noexcept = delete;
		DirectX12VertexBuffer(const DirectX12VertexBuffer&) = delete;
		DirectX12VertexBuffer& operator=(DirectX12VertexBuffer&&) noexcept = delete;
		DirectX12VertexBuffer& operator=(const DirectX12VertexBuffer&) = delete;

	public:
		~DirectX12VertexBuffer() noexcept override;

		// VertexBuffer interface.
	public:
		/// <inheritdoc />
		const DirectX12VertexBufferLayout& layout() const noexcept override;

		// IDirectX12VertexBuffer interface.
	public:
		const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept override;

		// DirectX 12 Vertex Buffer.
	public:
		static SharedPtr<IDirectX12VertexBuffer> allocate(const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static SharedPtr<IDirectX12VertexBuffer> allocate(const String& name, const DirectX12VertexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="IIndexBuffer" />.
	/// </summary>
	class DirectX12IndexBuffer : public DirectX12Buffer, public virtual IDirectX12IndexBuffer {
		LITEFX_IMPLEMENTATION(DirectX12IndexBufferImpl);
		friend struct SharedAllocator<DirectX12IndexBuffer>;

	private:
		explicit DirectX12IndexBuffer(ComPtr<ID3D12Resource>&& buffer, const DirectX12IndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, AllocatorPtr allocator, AllocationPtr&& allocation, const String& name = "");

		DirectX12IndexBuffer(DirectX12IndexBuffer&&) noexcept = delete;
		DirectX12IndexBuffer(const DirectX12IndexBuffer&) = delete;
		DirectX12IndexBuffer& operator=(DirectX12IndexBuffer&&) noexcept = delete;
		DirectX12IndexBuffer& operator=(const DirectX12IndexBuffer&) = delete;
	
	public:
		~DirectX12IndexBuffer() noexcept override;

		// IndexBuffer interface.
	public:
		/// <inheritdoc />
		const DirectX12IndexBufferLayout& layout() const noexcept override;

		// IDirectX12IndexBuffer interface.
	public:
		const D3D12_INDEX_BUFFER_VIEW& view() const noexcept override;

		// DirectX 12 Index Buffer.
	public:
		static SharedPtr<IDirectX12IndexBuffer> allocate(const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
		static SharedPtr<IDirectX12IndexBuffer> allocate(const String& name, const DirectX12IndexBufferLayout& layout, AllocatorPtr allocator, UInt32 elements, ResourceUsage usage, const D3D12_RESOURCE_DESC1& resourceDesc, const D3D12MA::ALLOCATION_DESC& allocationDesc);
	};
}

#pragma warning(pop)