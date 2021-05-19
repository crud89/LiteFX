#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class _VMAImageBase : public VulkanRuntimeObject<VulkanDevice>, public virtual IVulkanImage {
		LITEFX_IMPLEMENTATION(_VMAImageBaseImpl);

	public:
		_VMAImageBase(const VulkanDevice& device, VkImage image, VmaAllocator allocator, VmaAllocation allocation);
		_VMAImageBase(_VMAImageBase&&) = delete;
		_VMAImageBase(const _VMAImageBase&) = delete;
		virtual ~_VMAImageBase() noexcept;

	protected:
		virtual VmaAllocator& allocator() const noexcept;
		virtual VmaAllocation& allocationInfo() const noexcept;
		virtual VkImageView& view() const noexcept;
	};

	class _VMAImage : public _VMAImageBase, public Image {
	public:
		_VMAImage(const VulkanDevice& device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format);
		_VMAImage(const VulkanDevice& device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator allocator, VmaAllocation allocation);
		_VMAImage(_VMAImage&&) = delete;
		_VMAImage(const _VMAImage&) = delete;
		virtual ~_VMAImage() noexcept;

	public:
		virtual const VkImageView& getImageView() const noexcept override;

	public:
		static UniquePtr<IVulkanImage> allocate(const VulkanDevice& device, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	class _VMATexture : public _VMAImageBase, public Texture {
		LITEFX_IMPLEMENTATION(_VMATextureImpl);

	public:
		_VMATexture(const VulkanDevice& device, const VulkanDescriptorLayout* layout, VkImage image, const VkImageLayout& imageLayout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator allocator, VmaAllocation allocation);
		_VMATexture(_VMATexture&&) = delete;
		_VMATexture(const _VMATexture&) = delete;
		virtual ~_VMATexture() noexcept;

	public:
		virtual const VkImageView& getImageView() const noexcept override;

	public:
		/// <inheritdoc />
		/// <remarks>
		/// Note that images are always transferred as a whole. Transferring only regions is currently unsupported. Hence the <paramref name="size" /> and <paramref name="targetOffset" />
		/// parameters are ignored and can be simply set to <c>0</c>.
		/// </remarks>
		virtual void transferFrom(const ICommandBuffer* commandBuffer, IBuffer* source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) override;

		/// <inheritdoc />
		/// <remarks>
		/// Note that images are always transferred as a whole. Transferring only regions is currently unsupported. Hence the <paramref name="size" /> and <paramref name="sourceOffset" />
		/// parameters are ignored and can be simply set to <c>0</c>.
		/// </remarks>
		virtual void transferTo(const ICommandBuffer* commandBuffer, IBuffer* target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const override;

	public:
		static UniquePtr<ITexture> allocate(const VulkanDevice& device, const VulkanDescriptorLayout* layout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

}