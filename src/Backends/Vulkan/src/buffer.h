#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class _VMABufferBase : public virtual IBuffer, public IResource<VkBuffer> {
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocationInfo;

	public:
		_VMABufferBase(VkBuffer buffer, VmaAllocator& allocator, VmaAllocation allocation);
		_VMABufferBase(_VMABufferBase&&) = delete;
		_VMABufferBase(const _VMABufferBase&) = delete;
		virtual ~_VMABufferBase() noexcept;

	public:
		/// <inheritdoc />
		virtual void map(const void* const data, const size_t& size) override;
		
		/// <inheritdoc />
		virtual void transferFrom(const ICommandBuffer* commandBuffer, IBuffer* source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) override;
		
		/// <inheritdoc />
		virtual void transferTo(const ICommandBuffer* commandBuffer, IBuffer* target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const override;
	};

	class _VMABuffer : public _VMABufferBase, public Buffer {
	public:
		_VMABuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, VmaAllocation allocation);
		_VMABuffer(_VMABuffer&&) = delete;
		_VMABuffer(const _VMABuffer&) = delete;
		virtual ~_VMABuffer() noexcept;

	public:
		static UniquePtr<IBuffer> allocate(const BufferType& type, const UInt32& elements, const size_t& size, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAVertexBuffer : public _VMABufferBase, public virtual VulkanVertexBuffer {
	public:
		_VMAVertexBuffer(VkBuffer buffer, const VulkanVertexBufferLayout& layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAVertexBuffer(_VMAVertexBuffer&&) = delete;
		_VMAVertexBuffer(const _VMAVertexBuffer&) = delete;
		virtual ~_VMAVertexBuffer() noexcept;

	public:
		static UniquePtr<VulkanVertexBuffer> allocate(const VulkanVertexBufferLayout& layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAIndexBuffer : public _VMABufferBase, public virtual VulkanIndexBuffer {
	public:
		_VMAIndexBuffer(VkBuffer buffer, const VulkanIndexBufferLayout& layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAIndexBuffer(_VMAIndexBuffer&&) = delete;
		_VMAIndexBuffer(const _VMAIndexBuffer&) = delete;
		virtual ~_VMAIndexBuffer() noexcept;

	public:
		static UniquePtr<VulkanIndexBuffer> allocate(const VulkanIndexBufferLayout& layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMAConstantBuffer : public _VMABufferBase, public VulkanConstantBuffer {
	public:
		_VMAConstantBuffer(VkBuffer buffer, const IDescriptorLayout* layout, const UInt32& elements, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAConstantBuffer(_VMAConstantBuffer&&) = delete;
		_VMAConstantBuffer(const _VMAConstantBuffer&) = delete;
		virtual ~_VMAConstantBuffer() noexcept;

	public:
		static UniquePtr<IConstantBuffer> allocate(const IDescriptorLayout* layout, const UInt32& elements, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};
}