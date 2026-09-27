#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	struct VmaAllocationDeleter {
		VmaAllocator allocator{};

		void operator()(auto* ptr) noexcept {
			::vmaFreeMemory(allocator, ptr);
		}
	};

	typedef SharedPtr<VmaAllocation_T> AllocationPtr;

	/// @brief Implements a Vulkan @ref IBuffer.
	class VulkanBuffer : public virtual IVulkanBuffer, public Resource<VkBuffer>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanBufferImpl);
		friend struct SharedObject::Allocator<VulkanBuffer>;
		friend class VulkanGraphicsFactory;

	protected:
		explicit VulkanBuffer(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const AllocationPtr& allocation = nullptr, const String& name = "");

		VulkanBuffer(VulkanBuffer&&) noexcept = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

	public:
		~VulkanBuffer() noexcept override;

		// IBuffer interface.
	public:
		/// @copydoc IBuffer::type()
		BufferType type() const noexcept override;

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

		// IMappable interface.
	public:
		/// @copydoc IMappable::map(const void* const, size_t, UInt32)
		void map(const void* const data, size_t size, UInt32 element = 0) override;

		/// @copydoc IMappable::map(Span<const void* const>, size_t, UInt32)
		void map(Span<const void* const> data, size_t elementSize, UInt32 firstElement = 0) override;

		/// @copydoc IMappable::map(void*, size_t, UInt32, bool)
		void map(void* data, size_t size, UInt32 element = 0, bool write = true) override;

		/// @copydoc IMappable::map(Span<void*>, size_t, UInt32, bool)
		void map(Span<void*> data, size_t elementSize, UInt32 firstElement = 0, bool write = true) override;

		/// @copydoc IMappable::write
		void write(const void* const data, size_t size, size_t offset = 0) override;

		/// @copydoc IMappable::read
		void read(void* data, size_t size, size_t offset = 0) override;

	protected:
		/// @brief Returns the underlying allocator handle.
		/// 
		/// @return The handle of the underlying allocator.
		VmaAllocator allocator() const noexcept;

		/// @brief Returns the allocation info.
		/// 
		/// @return The allocation info.
		VmaAllocation allocationInfo() const noexcept;

	private:
		static inline auto create(VkBuffer buffer, BufferType type, UInt32 elements, size_t elementSize, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator = nullptr, const AllocationPtr& allocation = nullptr, const String& name = "") {
			return SharedObject::create<VulkanBuffer>(buffer, type, elements, elementSize, alignment, usage, createInfo, device, allocator, allocation, name);
		}

		// VulkanBuffer.
	public:
		static SharedPtr<IVulkanBuffer> allocate(const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static bool tryAllocate(SharedPtr<IVulkanBuffer>& buffer, const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);

		static bool move(SharedPtr<IVulkanBuffer> buffer, VmaAllocation to, const VulkanCommandBuffer& commandBuffer);
	};

	/// @brief Implements a Vulkan @ref IVertexBuffer.
	class VulkanVertexBuffer : public VulkanBuffer, public virtual IVulkanVertexBuffer {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferImpl);
		friend struct SharedObject::Allocator<VulkanVertexBuffer>;
		friend class VulkanGraphicsFactory;

	private:
		explicit VulkanVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const AllocationPtr& allocation = nullptr, const String& name = "");
		
		VulkanVertexBuffer(VulkanVertexBuffer&&) noexcept = delete;
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		VulkanVertexBuffer& operator=(VulkanVertexBuffer&&) noexcept = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;

	public:
		~VulkanVertexBuffer() noexcept override;

		// VertexBuffer interface.
	public:
		/// @copydoc VertexBuffer::layout
		const VulkanVertexBufferLayout& layout() const noexcept override;

	private:
		static inline auto create(VkBuffer buffer, const VulkanVertexBufferLayout& layout, UInt32 elements, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator = nullptr, const AllocationPtr& allocation = nullptr, const String& name = "") {
			return SharedObject::create<VulkanVertexBuffer>(buffer, layout, elements, alignment, usage, createInfo, device, allocator, allocation, name);
		}

		// VulkanVertexBuffer.
	public:
		static SharedPtr<IVulkanVertexBuffer> allocate(const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static bool tryAllocate(SharedPtr<IVulkanVertexBuffer>& buffer, const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// @brief Implements a Vulkan @ref IIndexBuffer.
	class VulkanIndexBuffer : public VulkanBuffer, public virtual IVulkanIndexBuffer {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferImpl);
		friend struct SharedObject::Allocator<VulkanIndexBuffer>;
		friend class VulkanGraphicsFactory;

	private:
		explicit VulkanIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator, const AllocationPtr& allocation = nullptr, const String& name = "");
		
		VulkanIndexBuffer(VulkanIndexBuffer&&) noexcept = delete;
		VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
		VulkanIndexBuffer& operator=(VulkanIndexBuffer&&) noexcept = delete;
		VulkanIndexBuffer& operator=(const VulkanIndexBuffer&) = delete;

	public:
		~VulkanIndexBuffer() noexcept override;

		// IndexBuffer interface.
	public:
		/// @copydoc IndexBuffer::layout
		const VulkanIndexBufferLayout& layout() const noexcept override;

	private:
		static inline auto create(VkBuffer buffer, const VulkanIndexBufferLayout& layout, UInt32 elements, size_t alignment, ResourceUsage usage, const VkBufferCreateInfo& createInfo, const VulkanDevice& device, const VmaAllocator& allocator = nullptr, const AllocationPtr& allocation = nullptr, const String& name = "") {
			return SharedObject::create<VulkanIndexBuffer>(buffer, layout, elements, alignment, usage, createInfo, device, allocator, allocation, name);
		}

		// VulkanIndexBuffer.
	public:
		static SharedPtr<IVulkanIndexBuffer> allocate(const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static bool tryAllocate(SharedPtr<IVulkanIndexBuffer>& buffer, const String& name, const ResourceAllocationInfo::BufferInfo& bufferInfo, size_t alignment, ResourceUsage usage, const VulkanDevice& device, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};
}

#pragma warning(pop)