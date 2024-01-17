#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IImage" />.
	/// </summary>
	class VulkanImage : public virtual IVulkanImage, public Resource<VkImage>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanImageImpl);

	public:
		explicit VulkanImage(const VulkanDevice& device, VkImage image, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, ImageLayout initialLayout, VmaAllocator allocator = nullptr, VmaAllocation allocation = nullptr, const String& name = "");
		VulkanImage(VulkanImage&&) = delete;
		VulkanImage(const VulkanImage&) = delete;
		virtual ~VulkanImage() noexcept;

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

		/// <inheritdoc />
		ImageLayout layout(UInt32 subresource = 0) const override;

		/// <inheritdoc />
		ImageLayout& layout(UInt32 subresource = 0) override;

		// IImage interface.
	public:
		/// <inheritdoc />
		size_t size(UInt32 level) const noexcept override;

		/// <inheritdoc />
		Size3d extent(UInt32 level = 0) const noexcept override;

		/// <inheritdoc />
		Format format() const noexcept override;

		/// <inheritdoc />
		ImageDimensions dimensions() const noexcept override;

		/// <inheritdoc />
		UInt32 levels() const noexcept override;

		/// <inheritdoc />
		UInt32 layers() const noexcept override;

		/// <inheritdoc />
		UInt32 planes() const noexcept override;

		/// <inheritdoc />
		MultiSamplingLevel samples() const noexcept override;

		// IVulkanImage interface.
	public:
		VkImageAspectFlags aspectMask() const noexcept override;
		VkImageAspectFlags aspectMask(UInt32 plane) const override;
		const VkImageView& imageView(UInt32 plane = 0) const override;

	protected:
		virtual VmaAllocator& allocator() const noexcept;
		virtual VmaAllocation& allocationInfo() const noexcept;
		virtual VkImageView& imageView(UInt32 plane = 0);

	public:
		static UniquePtr<VulkanImage> allocate(const VulkanDevice& device, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, ImageLayout initialLayout, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static UniquePtr<VulkanImage> allocate(const String& name, const VulkanDevice& device, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, ImageLayout initialLayout, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ISampler" />.
	/// </summary>
	class VulkanSampler : public virtual IVulkanSampler, public Resource<VkSampler>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanSamplerImpl);

	public:
		/// <summary>
		/// Initializes a new sampler instance.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="magFilter"></param>
		/// <param name="minFilter"></param>
		/// <param name="borderU"></param>
		/// <param name="borderV"></param>
		/// <param name="borderW"></param>
		/// <param name="mipMapMode"></param>
		/// <param name="mipMapBias"></param>
		/// <param name="maxLod"></param>
		/// <param name="minLod"></param>
		/// <param name="anisotropy"></param>
		explicit VulkanSampler(const VulkanDevice& device, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "");
		VulkanSampler(VulkanSampler&&) = delete;
		VulkanSampler(const VulkanSampler&) = delete;
		virtual ~VulkanSampler() noexcept;

		// ISampler interface.
	public:
		/// <inheritdoc />
		FilterMode getMinifyingFilter() const noexcept override;

		/// <inheritdoc />
		FilterMode getMagnifyingFilter() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeU() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeV() const noexcept override;

		/// <inheritdoc />
		BorderMode getBorderModeW() const noexcept override;

		/// <inheritdoc />
		Float getAnisotropy() const noexcept override;

		/// <inheritdoc />
		MipMapMode getMipMapMode() const noexcept override;

		/// <inheritdoc />
		Float getMipMapBias() const noexcept override;

		/// <inheritdoc />
		Float getMaxLOD() const noexcept override;

		/// <inheritdoc />
		Float getMinLOD() const noexcept override;
	};
}