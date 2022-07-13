#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IImage" />.
	/// </summary>
	class VulkanImage : public IVulkanImage, public Resource<VkImage> {
		LITEFX_IMPLEMENTATION(VulkanImageImpl);

	public:
		explicit VulkanImage(const VulkanDevice& device, VkImage image, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, VmaAllocator allocator = nullptr, VmaAllocation allocation = nullptr);
		VulkanImage(VulkanImage&&) = delete;
		VulkanImage(const VulkanImage&) = delete;
		virtual ~VulkanImage() noexcept;

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

		/// <inheritdoc />
		virtual const bool& writable() const noexcept override;

		/// <inheritdoc />
		virtual const ResourceState& state(const UInt32& subresource = 0) const override;

		/// <inheritdoc />
		virtual ResourceState& state(const UInt32& subresource = 0) override;

		// IImage interface.
	public:
		/// <inheritdoc />
		virtual size_t size(const UInt32& level) const noexcept override;

		/// <inheritdoc />
		virtual Size3d extent(const UInt32& level = 0) const noexcept override;

		/// <inheritdoc />
		virtual const Format& format() const noexcept override;

		/// <inheritdoc />
		virtual const ImageDimensions& dimensions() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& levels() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& layers() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& planes() const noexcept override;

		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

		// IVulkanImage interface.
	public:
		virtual VkImageAspectFlags aspectMask() const noexcept override;
		virtual VkImageAspectFlags aspectMask(const UInt32& plane) const override;
		virtual void resolveSubresource(const UInt32& subresource, UInt32& plane, UInt32& layer, UInt32& level) const override;
		virtual const VkImageView& imageView(const UInt32& plane = 0) const override;

	protected:
		virtual VmaAllocator& allocator() const noexcept;
		virtual VmaAllocation& allocationInfo() const noexcept;
		virtual VkImageView& imageView(const UInt32& plane = 0);

	public:
		static UniquePtr<VulkanImage> allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ISampler" />.
	/// </summary>
	class VulkanSampler : public IVulkanSampler, public Resource<VkSampler> {
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
		explicit VulkanSampler(const VulkanDevice& device, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f);
		VulkanSampler(VulkanSampler&&) = delete;
		VulkanSampler(const VulkanSampler&) = delete;
		virtual ~VulkanSampler() noexcept;

		// ISampler interface.
	public:
		/// <inheritdoc />
		virtual const FilterMode& getMinifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual const FilterMode& getMagnifyingFilter() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeU() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeV() const noexcept override;

		/// <inheritdoc />
		virtual const BorderMode& getBorderModeW() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getAnisotropy() const noexcept override;

		/// <inheritdoc />
		virtual const MipMapMode& getMipMapMode() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMipMapBias() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMaxLOD() const noexcept override;

		/// <inheritdoc />
		virtual const Float& getMinLOD() const noexcept override;
	};
}