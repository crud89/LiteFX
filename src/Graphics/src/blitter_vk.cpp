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

Blitter<VulkanBackend>::Blitter(const VulkanDevice& /*device*/) :
	m_impl()
{
}

void Blitter<VulkanBackend>::generateMipMaps(IVulkanImage& image, VulkanCommandBuffer& commandBuffer)
{
	VulkanBarrier startBarrier(PipelineStage::None, PipelineStage::Transfer);
	startBarrier.transition(image, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
	commandBuffer.barrier(startBarrier);

	for (UInt32 layer(0); layer < image.layers(); ++layer)
	{
		Int32 mipWidth = static_cast<Int32>(image.extent().width());
		Int32 mipHeight = static_cast<Int32>(image.extent().height());
		Int32 mipDepth = static_cast<Int32>(image.extent().depth());

		for (UInt32 level(1); level < image.levels(); ++level)
		{
			VulkanBarrier subBarrier(PipelineStage::Transfer, PipelineStage::Transfer);
			subBarrier.transition(image, level - 1, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
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

			::vkCmdBlitImage(std::as_const(commandBuffer).handle(), std::as_const(image).handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, std::as_const(image).handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			// Compute the new size.
			mipWidth = std::max(mipWidth / 2, 1);
			mipHeight = std::max(mipHeight / 2, 1);
			mipDepth = std::max(mipDepth / 2, 1);
		}

		VulkanBarrier subBarrier(PipelineStage::Transfer, PipelineStage::Transfer);
		subBarrier.transition(image, image.levels() - 1, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
		subBarrier.transition(image, 0, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
		commandBuffer.barrier(subBarrier);
	}

	VulkanBarrier endBarrier(PipelineStage::Transfer, PipelineStage::All);
	endBarrier.transition(image, ResourceAccess::TransferRead | ResourceAccess::TransferWrite, ResourceAccess::ShaderRead, ImageLayout::ShaderResource);
	commandBuffer.barrier(endBarrier);
}

// ------------------------------------------------------------------------------------------------
// Export definition.
// ------------------------------------------------------------------------------------------------

template class LITEFX_GRAPHICS_API Blitter<Backends::VulkanBackend>;

#endif // LITEFX_BUILD_VULKAN_BACKEND