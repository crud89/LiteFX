#pragma once

#include <litefx/rendering.hpp>
#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Rendering;

	/// @brief Implements a Vulkan @ref IImage.
	class VulkanImage : public virtual IVulkanImage, public Resource<VkImage>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanImageImpl);
		friend class VulkanSwapChain::VulkanSwapChainImpl;
		friend struct SharedObject::Allocator<VulkanImage>;
		friend class VulkanGraphicsFactory;

	private:
		explicit VulkanImage(VkImage image, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const VkImageCreateInfo& createInfo, VmaAllocator allocator = nullptr, const AllocationPtr& allocation = nullptr, const String& name = "");
		
		VulkanImage(VulkanImage&&) noexcept = delete;
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(VulkanImage&&) noexcept = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

	public:
		~VulkanImage() noexcept override;

		// IDeviceMemory interface.
	public:
		/// @copydoc IDeviceMemory::elements()
		UInt32 elements() const noexcept override;

		/// @copydoc IDeviceMemory::size()
		size_t size() const noexcept override;

		/// @copydoc IDeviceMemory::elementSize()
		size_t elementSize() const noexcept override;

		/// @copydoc IDeviceMemory::elementAlignment()
		size_t elementAlignment() const noexcept override;

		/// @copydoc IDeviceMemory::alignedElementSize()
		size_t alignedElementSize() const noexcept override;

		/// @copydoc IDeviceMemory::usage()
		ResourceUsage usage() const noexcept override;

		/// @copydoc IDeviceMemory::virtualAddress()
		UInt64 virtualAddress() const noexcept override;

		// IImage interface.
	public:
		/// @copydoc IImage::size()
		size_t size(UInt32 level) const override;

		/// @copydoc IImage::extent()
		Size3d extent(UInt32 level = 0) const noexcept override;

		/// @copydoc IImage::format()
		Format format() const noexcept override;

		/// @copydoc IImage::dimensions()
		ImageDimensions dimensions() const noexcept override;

		/// @copydoc IImage::levels()
		UInt32 levels() const noexcept override;

		/// @copydoc IImage::layers()
		UInt32 layers() const noexcept override;

		/// @copydoc IImage::planes()
		UInt32 planes() const noexcept override;

		/// @copydoc IImage::samples()
		MultiSamplingLevel samples() const noexcept override;

		// IVulkanImage interface.
	public:
		/// @copydoc IVulkanImage::aspectMask()
		VkImageAspectFlags aspectMask() const noexcept override;
		
		/// @copydoc IVulkanImage::aspectMask(UInt32)
		VkImageAspectFlags aspectMask(UInt32 plane) const override;

	protected:
		/// @brief Returns the underlying allocator handle.
		/// 
		/// @return The handle of the underlying allocator.
		VmaAllocator allocator() const noexcept;

		/// @brief Returns the allocation info.
		/// 
		/// @return The allocation info.
		VmaAllocation allocationInfo() const noexcept;

	private:
		static inline auto create(VkImage image, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, const VkImageCreateInfo& createInfo, VmaAllocator allocator = nullptr, const AllocationPtr& allocation = nullptr, const String& name = "") {
			return SharedObject::create<VulkanImage>(image, extent, format, dimensions, levels, layers, samples, usage, createInfo, allocator, allocation, name);
		}

	public:
		static SharedPtr<IVulkanImage> allocate(const String& name, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);
		static bool tryAllocate(SharedPtr<IVulkanImage>& image, const String& name, const Size3d& extent, Format format, ImageDimensions dimensions, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, VmaAllocator& allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo, VmaAllocationInfo* allocationResult = nullptr);

		static bool move(SharedPtr<IVulkanImage> image, VmaAllocation to, const VulkanCommandBuffer& commandBuffer);
	};

	/// @brief Implements a Vulkan @ref ISampler.
	class VulkanSampler : public virtual IVulkanSampler, public Resource<VkSampler>, public virtual StateResource {
		LITEFX_IMPLEMENTATION(VulkanSamplerImpl);
		friend struct SharedObject::Allocator<VulkanSampler>;

	private:
		/// @brief Initializes a new sampler instance.
		///
		/// @param device
		/// @param magFilter
		/// @param minFilter
		/// @param borderU
		/// @param borderV
		/// @param borderW
		/// @param mipMapMode
		/// @param mipMapBias
		/// @param maxLod
		/// @param minLod
		/// @param anisotropy
		explicit VulkanSampler(const VulkanDevice& device, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "");
		
		VulkanSampler(VulkanSampler&&) noexcept = delete;
		VulkanSampler(const VulkanSampler&) = delete;
		VulkanSampler& operator=(VulkanSampler&&) noexcept = delete;
		VulkanSampler& operator=(const VulkanSampler&) = delete;

	public:
		~VulkanSampler() noexcept override;

	private:
		const VulkanDevice& device() const;

		// ISampler interface.
	public:
		/// @copydoc ISampler::getMinifyingFilter()
		FilterMode getMinifyingFilter() const noexcept override;

		/// @copydoc ISampler::getMagnifyingFilter()
		FilterMode getMagnifyingFilter() const noexcept override;

		/// @copydoc ISampler::getBorderModeU()
		BorderMode getBorderModeU() const noexcept override;

		/// @copydoc ISampler::getBorderModeV()
		BorderMode getBorderModeV() const noexcept override;

		/// @copydoc ISampler::getBorderModeW()
		BorderMode getBorderModeW() const noexcept override;

		/// @copydoc ISampler::getAnisotropy()
		Float getAnisotropy() const noexcept override;

		/// @copydoc ISampler::getMipMapMode()
		MipMapMode getMipMapMode() const noexcept override;

		/// @copydoc ISampler::getMipMapBias()
		Float getMipMapBias() const noexcept override;

		/// @copydoc ISampler::getMaxLOD()
		Float getMaxLOD() const noexcept override;

		/// @copydoc ISampler::getMinLOD()
		Float getMinLOD() const noexcept override;

	public:
		static inline SharedPtr<VulkanSampler> copy(const IVulkanSampler& sampler) {
			return allocate(dynamic_cast<const VulkanSampler&>(sampler).device(), sampler.getMagnifyingFilter(), sampler.getMinifyingFilter(), sampler.getBorderModeU(), sampler.getBorderModeV(), sampler.getBorderModeW(), sampler.getMipMapMode(), sampler.getMipMapBias(), sampler.getMinLOD(), sampler.getMaxLOD(), sampler.getAnisotropy(), sampler.name());
		}

		static inline SharedPtr<VulkanSampler> allocate(const VulkanDevice& device, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f, const String& name = "") {
			return SharedObject::create<VulkanSampler>(device, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy, name);
		}
	};
}

#pragma warning(pop)