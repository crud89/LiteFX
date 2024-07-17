#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>

using namespace LiteFX::Rendering;

size_t LiteFX::Rendering::getSize(Format format)
{
	switch (format) {
	using enum Format;
	case None:
		return 0;
	case R4G4_UNORM:
	case R8_UNORM:
	case R8_SNORM:
	case R8_USCALED:
	case R8_SSCALED:
	case R8_UINT:
	case R8_SINT:
	case R8_SRGB:
	case S8_UINT:
		return 1;
	case R4G4B4A4_UNORM:
	case B4G4R4A4_UNORM:
	case R5G6B5_UNORM:
	case B5G6R5_UNORM:
	case R5G5B5A1_UNORM:
	case B5G5R5A1_UNORM:
	case A1R5G5B5_UNORM:
	case R8G8_UNORM:
	case R8G8_SNORM:
	case R8G8_USCALED:
	case R8G8_SSCALED:
	case R8G8_UINT:
	case R8G8_SINT:
	case R8G8_SRGB:
	case R16_UNORM:
	case R16_SNORM:
	case R16_USCALED:
	case R16_SSCALED:
	case R16_UINT:
	case R16_SINT:
	case R16_SFLOAT:
	case D16_UNORM:
		return 2;
	case R8G8B8_UNORM:
	case R8G8B8_SNORM:
	case R8G8B8_USCALED:
	case R8G8B8_SSCALED:
	case R8G8B8_UINT:
	case R8G8B8_SINT:
	case R8G8B8_SRGB:
	case B8G8R8_UNORM:
	case B8G8R8_SNORM:
	case B8G8R8_USCALED:
	case B8G8R8_SSCALED:
	case B8G8R8_UINT:
	case B8G8R8_SINT:
	case B8G8R8_SRGB:
	case D16_UNORM_S8_UINT:
		return 3;
	case R8G8B8A8_UNORM:
	case R8G8B8A8_SNORM:
	case R8G8B8A8_USCALED:
	case R8G8B8A8_SSCALED:
	case R8G8B8A8_UINT:
	case R8G8B8A8_SINT:
	case R8G8B8A8_SRGB:
	case B8G8R8A8_UNORM:
	case B8G8R8A8_SNORM:
	case B8G8R8A8_USCALED:
	case B8G8R8A8_SSCALED:
	case B8G8R8A8_UINT:
	case B8G8R8A8_SINT:
	case B8G8R8A8_SRGB:
	case A8B8G8R8_UNORM:
	case A8B8G8R8_SNORM:
	case A8B8G8R8_USCALED:
	case A8B8G8R8_SSCALED:
	case A8B8G8R8_UINT:
	case A8B8G8R8_SINT:
	case A8B8G8R8_SRGB:
	case A2R10G10B10_UNORM:
	case A2R10G10B10_SNORM:
	case A2R10G10B10_USCALED:
	case A2R10G10B10_SSCALED:
	case A2R10G10B10_UINT:
	case A2R10G10B10_SINT:
	case A2B10G10R10_UNORM:
	case A2B10G10R10_SNORM:
	case A2B10G10R10_USCALED:
	case A2B10G10R10_SSCALED:
	case A2B10G10R10_UINT:
	case A2B10G10R10_SINT:
	case R16G16_UNORM:
	case R16G16_SNORM:
	case R16G16_USCALED:
	case R16G16_SSCALED:
	case R16G16_UINT:
	case R16G16_SINT:
	case R16G16_SFLOAT:
	case R32_UINT:
	case R32_SINT:
	case R32_SFLOAT:
	case B10G11R11_UFLOAT:
	case E5B9G9R9_UFLOAT:
	case X8_D24_UNORM:
	case D32_SFLOAT:
	case D24_UNORM_S8_UINT:
		return 4;
	case R16G16B16_UNORM:
	case R16G16B16_SNORM:
	case R16G16B16_USCALED:
	case R16G16B16_SSCALED:
	case R16G16B16_UINT:
	case R16G16B16_SINT:
	case R16G16B16_SFLOAT:
		return 6;
	case R16G16B16A16_UNORM:
	case R16G16B16A16_SNORM:
	case R16G16B16A16_USCALED:
	case R16G16B16A16_SSCALED:
	case R16G16B16A16_UINT:
	case R16G16B16A16_SINT:
	case R16G16B16A16_SFLOAT:
	case R32G32_UINT:
	case R32G32_SINT:
	case R32G32_SFLOAT:
	case R64_UINT:
	case R64_SINT:
	case R64_SFLOAT:
	case D32_SFLOAT_S8_UINT:	// NOTE: This may be wrong here - spec states, however, that 24 bits are unused.
	case BC1_RGB_UNORM:
	case BC1_RGB_SRGB:
	case BC1_RGBA_UNORM:
	case BC1_RGBA_SRGB:
	case BC4_UNORM:
	case BC4_SNORM:
		return 8;
	case R32G32B32_UINT:
	case R32G32B32_SINT:
	case R32G32B32_SFLOAT:
		return 12;
	case R32G32B32A32_UINT:
	case R32G32B32A32_SINT:
	case R32G32B32A32_SFLOAT:
	case R64G64_UINT:
	case R64G64_SINT:
	case R64G64_SFLOAT:
	case BC2_UNORM:
	case BC2_SRGB:
	case BC3_UNORM:
	case BC3_SRGB:
	case BC5_UNORM:
	case BC5_SNORM:
	case BC6H_UFLOAT:
	case BC6H_SFLOAT:
	case BC7_UNORM:
	case BC7_SRGB:
		return 16;
	case R64G64B64_UINT:
	case R64G64B64_SINT:
	case R64G64B64_SFLOAT:
		return 24;
	case R64G64B64A64_UINT:
	case R64G64B64A64_SINT:
	case R64G64B64A64_SFLOAT:
		return 32;
	default:
		throw InvalidArgumentException("format", "Unsupported format: {0}.", format);
	}
}

bool LiteFX::Rendering::hasDepth(Format format)
{
	const Array<Format> depthFormats = {
		Format::D16_UNORM,
		Format::D32_SFLOAT,
		Format::X8_D24_UNORM,
		Format::D16_UNORM_S8_UINT,
		Format::D24_UNORM_S8_UINT,
		Format::D32_SFLOAT_S8_UINT
	};

	return std::any_of(std::begin(depthFormats), std::end(depthFormats), [&](Format f) { return f == format; });
}

bool LiteFX::Rendering::hasStencil(Format format)
{
	const Array<Format> stencilFormats = {
		Format::D16_UNORM_S8_UINT,
		Format::D24_UNORM_S8_UINT,
		Format::D32_SFLOAT_S8_UINT,
		Format::S8_UINT
	};

	return std::any_of(std::begin(stencilFormats), std::end(stencilFormats), [&](Format f) { return f == format; });
}