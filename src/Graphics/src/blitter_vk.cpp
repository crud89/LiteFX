#include <litefx/gfx/blitter.hpp>

using namespace LiteFX::Graphics;
using namespace LiteFX::Rendering::Backends;

#ifdef LITEFX_BUILD_VULKAN_BACKEND

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

template<>
class Blitter<VulkanBackend>::BlitImpl {
    friend class Blitter<VulkanBackend>;

public:

private:
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

template <>
Blitter<VulkanBackend>::Blitter(const VulkanDevice& /*device*/) :
	m_impl()
{
}

template <>
void Blitter<VulkanBackend>::generateMipMaps(IVulkanImage& image, VulkanCommandBuffer& commandBuffer)
{
	if (!LITEFX_FLAG_IS_SET(image.usage(), ResourceUsage::MipMapTarget)) [[unlikely]]
		throw InvalidArgumentException("image", "The image must be declared with the `MipMapTarget` usage.");

	// Check if the image has enough levels to build a mip chain.
	if (image.levels() < 2u)
		return;

	VulkanBarrier startBarrier(PipelineStage::All, PipelineStage::Transfer);
	startBarrier.transition(image, 0, 1, 0, image.layers(), 0, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::Common, ImageLayout::Common);
	startBarrier.transition(image, 1, image.levels() - 1, 0, image.layers(), 0, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::Common);
	commandBuffer.barrier(startBarrier);

	for (UInt32 layer(0); layer < image.layers(); ++layer)
	{
		Int32 mipWidth = static_cast<Int32>(image.extent().width());
		Int32 mipHeight = static_cast<Int32>(image.extent().height());
		Int32 mipDepth = static_cast<Int32>(image.extent().depth());

		for (UInt32 level(1); level < image.levels(); ++level)
		{
			VulkanBarrier subBarrier(PipelineStage::Transfer, PipelineStage::Transfer);
			subBarrier.transition(image, level - 1, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::Common);
			commandBuffer.barrier(subBarrier);

			// Blit the image of the previous level into the current level.
			VkImageBlit blit{
				.srcSubresource = VkImageSubresourceLayers {
					.aspectMask = image.aspectMask(),
					.mipLevel = level - 1,
					.baseArrayLayer = layer,
					.layerCount = 1
				},
				.dstSubresource = VkImageSubresourceLayers {
					.aspectMask = image.aspectMask(),
					.mipLevel = level,
					.baseArrayLayer = layer,
					.layerCount = 1
				}
			};

			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 };

			::vkCmdBlitImage(std::as_const(commandBuffer).handle(), std::as_const(image).handle(), VK_IMAGE_LAYOUT_GENERAL, std::as_const(image).handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &blit, VK_FILTER_LINEAR);

			// Compute the new size.
			mipWidth = std::max(mipWidth / 2, 1);
			mipHeight = std::max(mipHeight / 2, 1);
			mipDepth = std::max(mipDepth / 2, 1);
		}
	}

	VulkanBarrier endBarrier(PipelineStage::Transfer, PipelineStage::All);
	endBarrier.transition(image, ResourceAccess::TransferRead | ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::Common);
	commandBuffer.barrier(endBarrier);
}

// ------------------------------------------------------------------------------------------------
// Export definition.
// ------------------------------------------------------------------------------------------------

template class LITEFX_GRAPHICS_API LiteFX::Graphics::Blitter<Backends::VulkanBackend>;

#endif // LITEFX_BUILD_VULKAN_BACKEND