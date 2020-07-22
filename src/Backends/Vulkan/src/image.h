#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	class IVulkanImage : public virtual IImage, public IResource<VkImage> {
	public:
		IVulkanImage(VkImage image) : IResource(image) { }
		IVulkanImage(const IVulkanImage&) = delete;
		IVulkanImage(IVulkanImage&&) = delete;
		virtual ~IVulkanImage() noexcept = default;

	public:
		virtual const VkImageView& getImageView() const noexcept = 0;
	};

	template <typename TBase>
	class _VMAImageBase : public virtual TBase, public IVulkanImage {
	protected:
		VmaAllocator m_allocator;
		VmaAllocation m_allocationInfo;

	public:
		_VMAImageBase(_VMAImageBase&&) = delete;
		_VMAImageBase(const _VMAImageBase&) = delete;

		_VMAImageBase(VkImage image, VmaAllocator allocator, VmaAllocation allocation) :
			IVulkanImage(image), m_allocator(allocator), m_allocationInfo(allocation) { }

		virtual ~_VMAImageBase() noexcept {
			if (m_allocator != nullptr && m_allocationInfo != nullptr) {
				::vmaDestroyImage(m_allocator, this->handle(), m_allocationInfo);
				LITEFX_TRACE(VULKAN_LOG, "Destroyed image {0}", fmt::ptr(this->handle()));
			}
		}

	public:
		virtual void transfer(const ICommandQueue* commandQueue, IBuffer* source, const size_t& size, const size_t& offset = 0, const size_t& targetOffset = 0) const override {
			//auto transferQueue = dynamic_cast<const VulkanQueue*>(commandQueue);
			//auto sourceBuffer = dynamic_cast<const IResource<VkBuffer>*>(source);

			//if (sourceBuffer == nullptr)
			//	throw std::invalid_argument("The transfer source buffer must be initialized and a valid Vulkan buffer.");

			//if (transferQueue == nullptr)
			//	throw std::invalid_argument("The transfer queue must be initialized and a valid Vulkan command queue.");

			//auto device = dynamic_cast<const VulkanDevice*>(transferQueue->getDevice());

			//if (device == nullptr)
			//	throw std::runtime_error("The transfer queue must be bound to a valid Vulkan device.");

			//auto commandBuffer = makeUnique<const VulkanCommandBuffer>(transferQueue);

			//// Begin the transfer recording.
			//commandBuffer->begin();

			//// Create a copy command and add it to the command buffer.
			//VkBufferCopy copyInfo{};
			//copyInfo.size = size;
			//copyInfo.srcOffset = offset;
			//copyInfo.dstOffset = targetOffset;
			//::vkCmdCopyBuffer(commandBuffer->handle(), sourceBuffer->handle(), this->handle(), 1, &copyInfo);

			//// End the transfer recording and submit the buffer.
			//commandBuffer->end();
			//commandBuffer->submit(true);
			throw;
		}
	};

	class _VMAImage : public _VMAImageBase<Image>, public VulkanRuntimeObject {
	private:
		VkImageView m_view;

	public:
		_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format);
		_VMAImage(const VulkanDevice* device, VkImage image, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format, VmaAllocator allocator, VmaAllocation allocation);
		_VMAImage(_VMAImage&&) = delete;
		_VMAImage(const _VMAImage&) = delete;
		virtual ~_VMAImage() noexcept;

	public:
		virtual const VkImageView& getImageView() const noexcept override;

	public:
		static UniquePtr<IImage> allocate(const VulkanDevice* device, const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

}