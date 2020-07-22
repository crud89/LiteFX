#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class _VMABuffer : public virtual Buffer, public IResource<VkBuffer> {
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocationInfo;

	public:
		static UniquePtr<IBuffer> makeBuffer(const BufferType& type, const UInt32& elements, const UInt32& size, VmaAllocator& allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);

	public:
		_VMABuffer(VkBuffer buffer, const BufferType& type, const UInt32& elements, const UInt32& size, VmaAllocator& allocator, VmaAllocation allocation);
		_VMABuffer(_VMABuffer&&) = delete;
		_VMABuffer(const _VMABuffer&) = delete;
		virtual ~_VMABuffer() noexcept;

	public:
		virtual void map(const void* const data, const size_t& size) override;
		virtual void transfer(const ICommandQueue* commandQueue, IBuffer* target, const size_t& size, const size_t& offset = 0, const size_t& targetOffset = 0) const override;
	};

}