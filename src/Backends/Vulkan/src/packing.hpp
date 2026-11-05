#pragma once

#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

constexpr std::pair<UInt32, UInt32> getBlockExtent(Format format) noexcept
{
	switch (format)
	{
		using enum Format;
	case BC1_RGB_UNORM:
	case BC1_RGB_SRGB:
	case BC1_RGBA_UNORM:
	case BC1_RGBA_SRGB:
	case BC2_UNORM:
	case BC2_SRGB:
	case BC3_UNORM:
	case BC3_SRGB:
	case BC4_UNORM:
	case BC4_SNORM:
	case BC5_UNORM:
	case BC5_SNORM:
	case BC6H_UFLOAT:
	case BC6H_SFLOAT:
	case BC7_UNORM:
	case BC7_SRGB:
		return { 4u, 4u }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
	default:
		return { 1u, 1u }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
	}
}

inline size_t getPackedSize(const IVulkanImage& image, UInt32 level)
{
	// TODO: Handle the plane for depth/stencil formats.
	auto [blockWidth, blockHeight] = getBlockExtent(image.format());
	auto extent = image.extent(level);

	UInt64 blocksX = (extent.width() + blockWidth - 1) / blockWidth;
	UInt64 blocksY = (extent.height() + blockHeight - 1) / blockHeight;
	UInt64 depth = image.dimensions() == ImageDimensions::DIM_3 ? extent.depth() : 1;

	return static_cast<size_t>(blocksX * blocksY * depth) * ::getSize(image.format());
}

inline UInt64 getCopyAlignment(Format format)
{
	const UInt64 blockSize = ::getSize(format);
	return ::hasDepth(format) || ::hasStencil(format) ? std::max<UInt64>(blockSize, sizeof(UInt32)) : blockSize;
}

inline VkBufferImageCopy makeCopyRegion(const IVulkanImage& image, UInt32 subresource, UInt64 bufferOffset)
{
	UInt32 layer{ 0u }, level{ 0u }, plane{ 0u };
	image.resolveSubresource(subresource, plane, layer, level);
	auto extent = image.extent(level);

	return {
		.bufferOffset = bufferOffset,
		.imageSubresource = VkImageSubresourceLayers{
			.aspectMask = image.aspectMask(plane),
			.mipLevel = level,
			.baseArrayLayer = layer,
			.layerCount = 1
		},
		.imageExtent = {
			static_cast<UInt32>(extent.width()),
			static_cast<UInt32>(extent.height()),
			static_cast<UInt32>(extent.depth())
		}
	};
}