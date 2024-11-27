#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "vk_mem_alloc.h"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IBuffer" />.
	/// </summary>
	class VulkanBuffer : public virtual IVulkanBuffer, public Resource<VkBuffer>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanBufferImpl);

	public:
		explicit VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name);

		VulkanBuffer(VulkanBuffer&&) noexcept = delete;
		VulkanBuffer(const VulkanBuffer&) noexcept = delete;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) noexcept = delete;
		~VulkanBuffer() noexcept override;

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

		// VulkanBuffer.
	public:
		static UniquePtr<IVulkanBuffer> allocate(BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanBuffer> allocate(const String& name, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IVertexBuffer" />.
	/// </summary>
	class VulkanVertexBuffer : public VulkanBuffer, public virtual IVulkanVertexBuffer {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferImpl);

	public:
		explicit VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name = "");
		
		VulkanVertexBuffer(VulkanVertexBuffer&&) noexcept = delete;
		VulkanVertexBuffer(const VulkanVertexBuffer&) noexcept = delete;
		VulkanVertexBuffer& operator=(VulkanVertexBuffer&&) noexcept = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) noexcept = delete;
		~VulkanVertexBuffer() noexcept override;

		// VertexBuffer interface.
	public:
		/// <inheritdoc />
		const VulkanVertexBufferLayout& layout() const noexcept override;

		// VulkanVertexBuffer.
	public:
		static UniquePtr<IVulkanVertexBuffer> allocate(const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanVertexBuffer> allocate(const String& name, const VulkanVertexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="IIndexBuffer" />.
	/// </summary>
	class VulkanIndexBuffer : public VulkanBuffer, public virtual IVulkanIndexBuffer {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferImpl);

	public:
		explicit VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VmaAllocation& allocation, const String& name = "");
		
		VulkanIndexBuffer(VulkanIndexBuffer&&) noexcept = delete;
		VulkanIndexBuffer(const VulkanIndexBuffer&) noexcept = delete;
		VulkanIndexBuffer& operator=(VulkanIndexBuffer&&) noexcept = delete;
		VulkanIndexBuffer& operator=(const VulkanIndexBuffer&) noexcept = delete;
		~VulkanIndexBuffer() noexcept override;

		// IndexBuffer interface.
	public:
		/// <inheritdoc />
		const VulkanIndexBufferLayout& layout() const noexcept override;

		// VulkanIndexBuffer.
	public:
		static UniquePtr<IVulkanIndexBuffer> allocate(const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<IVulkanIndexBuffer> allocate(const String& name, const VulkanIndexBufferLayout& layout, UInt32 elements, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};
}

#pragma warning(pop)