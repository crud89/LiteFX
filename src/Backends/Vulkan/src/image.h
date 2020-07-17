#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class _VMAImage : public VulkanTexture {
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocationInfo;

	public:
		static UniquePtr<ITexture> makeImage(const VulkanDevice* device, const Format& format, const Size2d& size, const UInt32& binding, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);

	public:
		_VMAImage(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size, const UInt32& binding, VmaAllocator& allocator, VmaAllocation allocation);
		_VMAImage(_VMAImage&&) = delete;
		_VMAImage(const _VMAImage&) = delete;
		virtual ~_VMAImage() noexcept;

	public:
		virtual void map(const void* const data, const size_t& size) override;
	};

}