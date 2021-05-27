#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class VulkanBuffer : public VulkanRuntimeObject<VulkanDevice>, public IVulkanBuffer {
		LITEFX_IMPLEMENTATION(VulkanBufferImpl);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="buffer"></param>
		/// <param name="type"></param>
		/// <param name="elements"></param>
		/// <param name="elementSize"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit VulkanBuffer(const VulkanDevice& device, VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const VmaAllocator& allocator, const VmaAllocation& allocation);
		VulkanBuffer(VulkanBuffer&&) = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		virtual ~VulkanBuffer() noexcept;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		virtual void transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) override;

		/// <inheritdoc />
		virtual void transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const override;

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

		// IMappable interface.
	public:
		/// <inheritdoc />
		virtual void map(const void* const data, const size_t& size) override;

		// VulkanBuffer.
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="type"></param>
		/// <param name="elements"></param>
		/// <param name="elementSize"></param>
		/// <param name="allocator"></param>
		/// <param name="createInfo"></param>
		/// <param name="allocationInfo"></param>
		/// <param name="allocationResult"></param>
		/// <returns></returns>
		static UniquePtr<IVulkanBuffer> allocate(const VulkanDevice& device, const BufferType& type, const UInt32& elements, const size_t& elementSize, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class VulkanVertexBuffer : public VulkanBuffer, public virtual IVulkanVertexBuffer {
		LITEFX_IMPLEMENTATION(VulkanVertexBufferImpl);

	public:
		explicit VulkanVertexBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanVertexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation);
		VulkanVertexBuffer(VulkanVertexBuffer&&) = delete;
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		virtual ~VulkanVertexBuffer() noexcept;

		// IVertexBuffer interface.
	public:
		/// <inheritdoc />
		const VulkanVertexBufferLayout& layout() const noexcept override;

		// IBindable interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		// VulkanVertexBuffer.
	public:
		static UniquePtr<IVulkanVertexBuffer> allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class VulkanIndexBuffer : public VulkanBuffer, public virtual IVulkanIndexBuffer {
		LITEFX_IMPLEMENTATION(VulkanIndexBufferImpl);

	public:
		explicit VulkanIndexBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanIndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation);
		VulkanIndexBuffer(VulkanIndexBuffer&&) = delete;
		VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
		virtual ~VulkanIndexBuffer() noexcept;

		// IIndexBuffer interface.
	public:
		/// <inheritdoc />
		virtual const VulkanIndexBufferLayout& layout() const noexcept override;

		// VulkanIndexBuffer.
	public:
		static UniquePtr<IVulkanIndexBuffer> allocate(const VulkanIndexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class VulkanConstantBuffer : public VulkanBuffer, public virtual IVulkanConstantBuffer {
		LITEFX_IMPLEMENTATION(VulkanConstantBufferImpl);

	public:
		explicit VulkanConstantBuffer(const VulkanDevice& device, VkBuffer buffer, const VulkanDescriptorLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VmaAllocation& allocation);
		VulkanConstantBuffer(VulkanConstantBuffer&&) = delete;
		VulkanConstantBuffer(const VulkanConstantBuffer&) = delete;
		virtual ~VulkanConstantBuffer() noexcept;

		// IDescriptor interface.
	public:
		/// <inheritdoc />
		const VulkanDescriptorLayout& layout() const noexcept override;

		// IBindable interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		// VulkanConstantBuffer.
	public:
		static UniquePtr<IVulkanConstantBuffer> allocate(const VulkanDescriptorLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};
}