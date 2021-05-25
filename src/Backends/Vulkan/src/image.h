#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>

#include "vk_mem_alloc.h"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a Vulkan <see cref="IImage" />.
	/// </summary>
	class VulkanImage : public VulkanRuntimeObject<VulkanDevice>, public IVulkanImage {
		LITEFX_IMPLEMENTATION(VulkanImageImpl);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="image"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit VulkanImage(const VulkanDevice& device, VkImage image, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator allocator = nullptr, VmaAllocation allocation = nullptr);
		VulkanImage(VulkanImage&&) = delete;
		VulkanImage(const VulkanImage&) = delete;
		virtual ~VulkanImage() noexcept;

		// IDeviceMemory interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& elements() const noexcept override;

		/// <inheritdoc />
		virtual size_t size() const noexcept override;

		// IImage interface.
	public:
		/// <inheritdoc />
		virtual const Size2d& extent() const noexcept override;

		/// <inheritdoc />
		virtual const Format& format() const noexcept override;

		// IVulkanImage interface.
	public:
		/// <inheritdoc />
		virtual const VkImageView& imageView() const noexcept;

	protected:
		virtual VmaAllocator& allocator() const noexcept;
		virtual VmaAllocation& allocationInfo() const noexcept;
		virtual VkImageView& imageView() noexcept;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="elements"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="allocator"></param>
		/// <param name="createInfo"></param>
		/// <param name="allocationInfo"></param>
		/// <param name="allocationResult"></param>
		/// <returns></returns>
		static UniquePtr<VulkanImage> allocate(const VulkanDevice& device, const UInt32& elements, const Size2d& extent, const Format& format, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ITexture" />.
	/// </summary>
	class VulkanTexture : public VulkanImage, public IVulkanTexture {
		LITEFX_IMPLEMENTATION(VulkanTextureImpl);

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="layout"></param>
		/// <param name="image"></param>
		/// <param name="imageLayout"></param>
		/// <param name="elements"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="levels"></param>
		/// <param name="samples"></param>
		/// <param name="allocator"></param>
		/// <param name="allocation"></param>
		explicit VulkanTexture(const VulkanDevice& device, const VulkanDescriptorLayout& layout, VkImage image, const VkImageLayout& imageLayout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator allocator, VmaAllocation allocation);
		VulkanTexture(VulkanTexture&&) = delete;
		VulkanTexture(const VulkanTexture&) = delete;
		virtual ~VulkanTexture() noexcept;

		// IBindable interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		// IDescriptor interface.
	public:
		/// <inheritdoc />
		virtual const VulkanDescriptorLayout& layout() const noexcept override;

		// ITexture interface.
	public:
		/// <inheritdoc />
		virtual const MultiSamplingLevel& samples() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& levels() const noexcept override;

		// ITransferable interface.
	public:
		/// <inheritdoc />
		/// <remarks>
		/// Note that images are always transferred as a whole. Transferring only regions is currently unsupported. Hence the <paramref name="size" /> and <paramref name="targetOffset" />
		/// parameters are ignored and can be simply set to <c>0</c>.
		/// </remarks>
		virtual void transferFrom(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) override;

		/// <inheritdoc />
		/// <remarks>
		/// Note that images are always transferred as a whole. Transferring only regions is currently unsupported. Hence the <paramref name="size" /> and <paramref name="sourceOffset" />
		/// parameters are ignored and can be simply set to <c>0</c>.
		/// </remarks>
		/// <inheritdoc />
		virtual void transferTo(const VulkanCommandBuffer& commandBuffer, const IVulkanBuffer& target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const override;

		// IVulkanTexture interface.
	public:
		virtual const VkImageLayout& imageLayout() const noexcept override;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="device"></param>
		/// <param name="layout"></param>
		/// <param name="elements"></param>
		/// <param name="extent"></param>
		/// <param name="format"></param>
		/// <param name="levels"></param>
		/// <param name="samples"></param>
		/// <param name="allocator"></param>
		/// <param name="createInfo"></param>
		/// <param name="allocationInfo"></param>
		/// <param name="allocationResult"></param>
		/// <returns></returns>
		static UniquePtr<VulkanTexture> allocate(const VulkanDevice& device, const VulkanDescriptorLayout& layout, const UInt32& elements, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
	};

	/// <summary>
	/// Implements a Vulkan <see cref="ISampler" />.
	/// </summary>
	class VulkanSampler : public VulkanRuntimeObject<VulkanDevice>, public IVulkanSampler {
		LITEFX_IMPLEMENTATION(VulkanSamplerImpl);

	public:
		/// <summary>
		/// Initializes a new sampler instance.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="layout"></param>
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
		explicit VulkanSampler(const VulkanDevice& device, const VulkanDescriptorLayout& layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f);
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