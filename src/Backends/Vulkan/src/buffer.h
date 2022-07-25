#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IBuffer" />.
	/// </summary>
	class VulkanBuffer : public IVulkanBuffer, public Resource<VkBuffer>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanBufferImpl);

	public:
		explicit VulkanBuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name);
		VulkanBuffer(VulkanBuffer&&) = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		virtual ~VulkanBuffer() noexcept;

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

		/// <inheritdoc />
		virtual const bool& writable() const noexcept override;

		/// <inheritdoc />
		virtual const ResourceState& state(const UInt32& subresource = 0) const override;

		/// <inheritdoc />
		virtual ResourceState& state(const UInt32& subresource = 0) override;

		// IMappable interface.
	public:
		/// <inheritdoc />
		virtual void map(const void* const data, const size_t& size, const UInt32& element = 0) override;

		/// <inheritdoc />
		virtual void map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement = 0) override;

		// VulkanBuffer.
	public:
		static UniquePtr<IVulkanBuffer> allocate(const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanBuffer> allocate(const String& name, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const bool& writable, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IVertexBuffer" />.
	/// </summary>
	class VulkanVertexBuffer : public VulkanBuffer, public IVulkanVertexBuffer {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferImpl);

	public:
		explicit VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name = "");
		VulkanVertexBuffer(VulkanVertexBuffer&&) = delete;
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		virtual ~VulkanVertexBuffer() noexcept;

		// VertexBuffer interface.
	public:
		/// <inheritdoc />
		const VulkanVertexBufferLayout& layout() const noexcept override;

		// VulkanVertexBuffer.
	public:
		static UniquePtr<IVulkanVertexBuffer> allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanVertexBuffer> allocate(const String& name, const VulkanVertexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IIndexBuffer" />.
	/// </summary>
	class VulkanIndexBuffer : public VulkanBuffer, public IVulkanIndexBuffer {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferImpl);

	public:
		explicit VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name = "");
		VulkanIndexBuffer(VulkanIndexBuffer&&) = delete;
		VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
		virtual ~VulkanIndexBuffer() noexcept;

		// IndexBuffer interface.
	public:
		/// <inheritdoc />
		virtual const VulkanIndexBufferLayout& layout() const noexcept override;

		// VulkanIndexBuffer.
	public:
		static UniquePtr<IVulkanIndexBuffer> allocate(const VulkanIndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanIndexBuffer> allocate(const String& name, const VulkanIndexBufferLayout& layout, const UInt32& elements, const ResourceState& initialState, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};
}