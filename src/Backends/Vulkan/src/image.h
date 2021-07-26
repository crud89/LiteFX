#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IImage" />.
	/// </summary>
	class VulkanImage : public VulkanRuntimeObject<VulkanDevice>, public IVulkanImage, public Resource<VkImage> {
		LITEFX_IMPLEMENTATION(VulkanImageImpl);

	public:
		explicit VulkanImage(const VulkanDevice& device, VkImage image, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const bool& writable, const ResourceState& initialState, VmaAllocator allocator = nullptr, VmaAllocation allocation = nullptr);
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
		virtual const ResourceState& state() const noexcept override;

		/// <inheritdoc />
		virtual ResourceState& state() noexcept override;

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

		// IVulkanImage interface.
	public:
		virtual const VkImageView& imageView(const UInt32& plane = 0) const override;

	protected:
		virtual VmaAllocator& allocator() const noexcept;
		virtual VmaAllocation& allocationInfo() const noexcept;
		virtual VkImageView& imageView(const UInt32& plane = 0);

	public:
		static UniquePtr<VulkanImage> allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const bool& writable, const ResourceState& initialState, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ITexture" />.
	/// </summary>
	class VulkanTexture : public VulkanImage, public IVulkanTexture {
		LITEFX_IMPLEMENTATION(VulkanTextureImpl);

	public:
		explicit VulkanTexture(const VulkanDevice& device, VkImage image, const VkImageLayout& imageLayout, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, VmaAllocator allocator, VmaAllocation allocation);
		VulkanTexture(VulkanTexture&&) = delete;
		VulkanTexture(const VulkanTexture&) = delete;
		virtual ~VulkanTexture() noexcept;

		// ITexture interface.
	public:
		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void generateMipMaps(const VulkanCommandBuffer& commandBuffer) const noexcept override;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		virtual void receiveData(const VulkanCommandBuffer& commandBuffer, const bool& receive) const noexcept override;

		/// <inheritdoc />
		virtual void sendData(const VulkanCommandBuffer& commandBuffer, const bool& emit) const noexcept override;

		/// <inheritdoc />
		virtual void transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const UInt32& sourceElement = 0, const UInt32& targetMipMapLevel = 0, const UInt32& mipMapLevels = 1, const bool& leaveSourceState = false, const bool& leaveTargetState = false, const UInt32& layer = 0, const UInt32& plane = 0) const override;

		/// <inheritdoc />
		virtual void transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const UInt32& sourceMipMapLevel = 0, const UInt32& targetElement = 0, const UInt32& mipMapLevels = 1, const bool& leaveSourceState = false, const bool& leaveTargetState = false, const UInt32& layer = 0, const UInt32& plane = 0) const override;

		// IVulkanImage interface.
	public:
		virtual const VkImageView& imageView(const UInt32& plane = 0) const override {
			return VulkanImage::imageView(plane);
		}

	public:
		static UniquePtr<VulkanTexture> allocate(const VulkanDevice& device, const Size3d& extent, const Format& format, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& writable, const ResourceState& initialState, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ISampler" />.
	/// </summary>
	class VulkanSampler : public VulkanRuntimeObject<VulkanDevice>, public IVulkanSampler, public Resource<VkSampler> {
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