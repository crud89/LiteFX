#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class VulkanBuffer : public VulkanRuntimeObject<VulkanDevice>, public IVulkanBuffer, public Resource<VkBuffer> {
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
		/// <param name="alignment"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit VulkanBuffer(const VulkanDevice& device, VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const VmaAllocator& allocator, const VmaAllocation& allocation);
		VulkanBuffer(VulkanBuffer&&) = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		virtual ~VulkanBuffer() noexcept;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		virtual void receiveData(const VulkanCommandBuffer& commandBuffer, const bool& receive) const noexcept override;

		/// <inheritdoc />
		virtual void sendData(const VulkanCommandBuffer& commandBuffer, const bool& emit) const noexcept override;

		/// <inheritdoc />
		virtual void transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1, const bool& leaveSourceState = false, const bool& leaveTargetState = false, const UInt32& layer = 0, const UInt32& plane = 0) const override;

		/// <inheritdoc />
		virtual void transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1, const bool& leaveSourceState = false, const bool& leaveTargetState = false, const UInt32& layer = 0, const UInt32& plane = 0) const override;

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
		static UniquePtr<IVulkanBuffer> allocate(const VulkanDevice& device, const BufferType& type, const UInt32& elements, const size_t& elementSize, const size_t& alignment, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class VulkanVertexBuffer : public VulkanBuffer, public IVulkanVertexBuffer {
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

		// VulkanVertexBuffer.
	public:
		static UniquePtr<IVulkanVertexBuffer> allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, const VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class VulkanIndexBuffer : public VulkanBuffer, public IVulkanIndexBuffer {
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
}