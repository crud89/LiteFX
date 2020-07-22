#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	template <typename TBase>
	class _VMABufferBase : public virtual TBase, public IResource<VkBuffer> {
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocationInfo;

	public:
		_VMABufferBase(VkBuffer buffer, VmaAllocator& allocator, VmaAllocation allocation) :
			IResource(buffer), m_allocator(allocator), m_allocationInfo(allocation) { }
		_VMABufferBase(_VMABufferBase&&) = delete;
		_VMABufferBase(const _VMABufferBase&) = delete;

		virtual ~_VMABufferBase() noexcept {
			::vmaDestroyBuffer(m_allocator, this->handle(), m_allocationInfo);
			LITEFX_TRACE(VULKAN_LOG, "Destroyed buffer {0} {{ Type: {1} }}", fmt::ptr(this->handle()), this->getType());
		}

	public:
		virtual void map(const void* const data, const size_t& size) override {
			void* buffer;

			if (::vmaMapMemory(m_allocator, m_allocationInfo, &buffer) != VK_SUCCESS)
				throw std::runtime_error("Unable to map buffer memory.");

			auto result = ::memcpy_s(buffer, this->getSize(), data, size);

			if (result != 0) {
				LITEFX_ERROR(VULKAN_LOG, "Error mapping buffer to device memory: {#X}.", result);
				throw std::runtime_error("Error mapping buffer to device memory.");
			}

			::vmaUnmapMemory(m_allocator, m_allocationInfo);
		}

		virtual void transfer(const ICommandQueue* commandQueue, IBuffer* source, const size_t& size, const size_t& offset = 0, const size_t& targetOffset = 0) const override {
			auto transferQueue = dynamic_cast<const VulkanQueue*>(commandQueue);
			auto sourceBuffer = dynamic_cast<const IResource<VkBuffer>*>(source);

			if (sourceBuffer == nullptr)
				throw std::invalid_argument("The transfer source buffer must be initialized and a valid Vulkan buffer.");

			if (transferQueue == nullptr)
				throw std::invalid_argument("The transfer queue must be initialized and a valid Vulkan command queue.");

			auto device = dynamic_cast<const VulkanDevice*>(transferQueue->getDevice());

			if (device == nullptr)
				throw std::runtime_error("The transfer queue must be bound to a valid Vulkan device.");

			auto commandBuffer = makeUnique<const VulkanCommandBuffer>(transferQueue);

			// Begin the transfer recording.
			commandBuffer->begin();

			// Create a copy command and add it to the command buffer.
			VkBufferCopy copyInfo{};
			copyInfo.size = size;
			copyInfo.srcOffset = offset;
			copyInfo.dstOffset = targetOffset;
			::vkCmdCopyBuffer(commandBuffer->handle(), sourceBuffer->handle(), this->handle(), 1, &copyInfo);

			// End the transfer recording and submit the buffer.
			commandBuffer->end();
			commandBuffer->submit(true);
		}
	};

	class _VMABuffer : public _VMABufferBase<Buffer> {
	public:
		_VMABuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, VmaAllocation allocation);
		_VMABuffer(_VMABuffer&&) = delete;
		_VMABuffer(const _VMABuffer&) = delete;
		virtual ~_VMABuffer() noexcept;

	public:
		static UniquePtr<IBuffer> allocate(const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAVertexBuffer : public _VMABufferBase<VertexBuffer> {
	public:
		_VMAVertexBuffer(VkBuffer buffer, const IVertexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAVertexBuffer(_VMAVertexBuffer&&) = delete;
		_VMAVertexBuffer(const _VMAVertexBuffer&) = delete;
		virtual ~_VMAVertexBuffer() noexcept;

	public:
		static UniquePtr<IVertexBuffer> allocate(const IVertexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAIndexBuffer : public _VMABufferBase<IndexBuffer> {
	public:
		_VMAIndexBuffer(VkBuffer buffer, const IIndexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAIndexBuffer(_VMAIndexBuffer&&) = delete;
		_VMAIndexBuffer(const _VMAIndexBuffer&) = delete;
		virtual ~_VMAIndexBuffer() noexcept;

	public:
		static UniquePtr<IIndexBuffer> allocate(const IIndexBufferLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAConstantBuffer : public _VMABufferBase<ConstantBuffer> {
	public:
		_VMAConstantBuffer(VkBuffer buffer, const IDescriptorLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAConstantBuffer(_VMAConstantBuffer&&) = delete;
		_VMAConstantBuffer(const _VMAConstantBuffer&) = delete;
		virtual ~_VMAConstantBuffer() noexcept;

	public:
		static UniquePtr<IConstantBuffer> allocate(const IDescriptorLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

}