#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

Format LiteFX::Rendering::Backends::getFormat(const VkFormat& format)
{
	switch (format)
	{
	case VK_FORMAT_R4G4_UNORM_PACK8:
		return Format::R4G4_UNORM;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		return Format::R4G4B4A4_UNORM;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		return Format::B4G4R4A4_UNORM;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		return Format::R5G6B5_UNORM;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		return Format::B5G6R5_UNORM;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		return Format::R5G5B5A1_UNORM;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		return Format::B5G5R5A1_UNORM;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		return Format::A1R5G5B5_UNORM;
	case VK_FORMAT_R8_UNORM:
		return Format::R8_UNORM;
	case VK_FORMAT_R8_SNORM:
		return Format::R8_SNORM;
	case VK_FORMAT_R8_USCALED:
		return Format::R8_USCALED;
	case VK_FORMAT_R8_SSCALED:
		return Format::R8_SSCALED;
	case VK_FORMAT_R8_UINT:
		return Format::R8_UINT;
	case VK_FORMAT_R8_SINT:
		return Format::R8_SINT;
	case VK_FORMAT_R8_SRGB:
		return Format::R8_SRGB;
	case VK_FORMAT_R8G8_UNORM:
		return Format::R8G8_UNORM;
	case VK_FORMAT_R8G8_SNORM:
		return Format::R8G8_SNORM;
	case VK_FORMAT_R8G8_USCALED:
		return Format::R8G8_USCALED;
	case VK_FORMAT_R8G8_SSCALED:
		return Format::R8G8_SSCALED;
	case VK_FORMAT_R8G8_UINT:
		return Format::R8G8_UINT;
	case VK_FORMAT_R8G8_SINT:
		return Format::R8G8_SINT;
	case VK_FORMAT_R8G8_SRGB:
		return Format::R8G8_SRGB;
	case VK_FORMAT_R8G8B8_UNORM:
		return Format::R8G8B8_UNORM;
	case VK_FORMAT_R8G8B8_SNORM:
		return Format::R8G8B8_SNORM;
	case VK_FORMAT_R8G8B8_USCALED:
		return Format::R8G8B8_USCALED;
	case VK_FORMAT_R8G8B8_SSCALED:
		return Format::R8G8B8_SSCALED;
	case VK_FORMAT_R8G8B8_UINT:
		return Format::R8G8B8_UINT;
	case VK_FORMAT_R8G8B8_SINT:
		return Format::R8G8B8_SINT;
	case VK_FORMAT_R8G8B8_SRGB:
		return Format::R8G8B8_SRGB;
	case VK_FORMAT_B8G8R8_UNORM:
		return Format::B8G8R8_UNORM;
	case VK_FORMAT_B8G8R8_SNORM:
		return Format::B8G8R8_SNORM;
	case VK_FORMAT_B8G8R8_USCALED:
		return Format::B8G8R8_USCALED;
	case VK_FORMAT_B8G8R8_SSCALED:
		return Format::B8G8R8_SSCALED;
	case VK_FORMAT_B8G8R8_UINT:
		return Format::B8G8R8_UINT;
	case VK_FORMAT_B8G8R8_SINT:
		return Format::B8G8R8_SINT;
	case VK_FORMAT_B8G8R8_SRGB:
		return Format::B8G8R8_SRGB;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return Format::R8G8B8A8_UNORM;
	case VK_FORMAT_R8G8B8A8_SNORM:
		return Format::R8G8B8A8_SNORM;
	case VK_FORMAT_R8G8B8A8_USCALED:
		return Format::R8G8B8A8_USCALED;
	case VK_FORMAT_R8G8B8A8_SSCALED:
		return Format::R8G8B8A8_SSCALED;
	case VK_FORMAT_R8G8B8A8_UINT:
		return Format::R8G8B8A8_UINT;
	case VK_FORMAT_R8G8B8A8_SINT:
		return Format::R8G8B8A8_SINT;
	case VK_FORMAT_R8G8B8A8_SRGB:
		return Format::R8G8B8A8_SRGB;
	case VK_FORMAT_B8G8R8A8_UNORM:
		return Format::B8G8R8A8_UNORM;
	case VK_FORMAT_B8G8R8A8_SNORM:
		return Format::B8G8R8A8_SNORM;
	case VK_FORMAT_B8G8R8A8_USCALED:
		return Format::B8G8R8A8_USCALED;
	case VK_FORMAT_B8G8R8A8_SSCALED:
		return Format::B8G8R8A8_SSCALED;
	case VK_FORMAT_B8G8R8A8_UINT:
		return Format::B8G8R8A8_UINT;
	case VK_FORMAT_B8G8R8A8_SINT:
		return Format::B8G8R8A8_SINT;
	case VK_FORMAT_B8G8R8A8_SRGB:
		return Format::B8G8R8A8_SRGB;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		return Format::A8B8G8R8_UNORM;
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		return Format::A8B8G8R8_SNORM;
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		return Format::A8B8G8R8_USCALED;
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		return Format::A8B8G8R8_SSCALED;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		return Format::A8B8G8R8_UINT;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		return Format::A8B8G8R8_SINT;
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		return Format::A8B8G8R8_SRGB;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		return Format::A2R10G10B10_UNORM;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		return Format::A2R10G10B10_SNORM;
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		return Format::A2R10G10B10_USCALED;
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		return Format::A2R10G10B10_SSCALED;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		return Format::A2R10G10B10_UINT;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		return Format::A2R10G10B10_SINT;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return Format::A2B10G10R10_UNORM;
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		return Format::A2B10G10R10_SNORM;
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		return Format::A2B10G10R10_USCALED;
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		return Format::A2B10G10R10_SSCALED;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		return Format::A2B10G10R10_UINT;
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		return Format::A2B10G10R10_SINT;
	case VK_FORMAT_R16_UNORM:
		return Format::R16_UNORM;
	case VK_FORMAT_R16_SNORM:
		return Format::R16_SNORM;
	case VK_FORMAT_R16_USCALED:
		return Format::R16_USCALED;
	case VK_FORMAT_R16_SSCALED:
		return Format::R16_SSCALED;
	case VK_FORMAT_R16_UINT:
		return Format::R16_UINT;
	case VK_FORMAT_R16_SINT:
		return Format::R16_SINT;
	case VK_FORMAT_R16_SFLOAT:
		return Format::R16_SFLOAT;
	case VK_FORMAT_R16G16_UNORM:
		return Format::R16G16_UNORM;
	case VK_FORMAT_R16G16_SNORM:
		return Format::R16G16_SNORM;
	case VK_FORMAT_R16G16_USCALED:
		return Format::R16G16_USCALED;
	case VK_FORMAT_R16G16_SSCALED:
		return Format::R16G16_SSCALED;
	case VK_FORMAT_R16G16_UINT:
		return Format::R16G16_UINT;
	case VK_FORMAT_R16G16_SINT:
		return Format::R16G16_SINT;
	case VK_FORMAT_R16G16_SFLOAT:
		return Format::R16G16_SFLOAT;
	case VK_FORMAT_R16G16B16_UNORM:
		return Format::R16G16B16_UNORM;
	case VK_FORMAT_R16G16B16_SNORM:
		return Format::R16G16B16_SNORM;
	case VK_FORMAT_R16G16B16_USCALED:
		return Format::R16G16B16_USCALED;
	case VK_FORMAT_R16G16B16_SSCALED:
		return Format::R16G16B16_SSCALED;
	case VK_FORMAT_R16G16B16_UINT:
		return Format::R16G16B16_UINT;
	case VK_FORMAT_R16G16B16_SINT:
		return Format::R16G16B16_SINT;
	case VK_FORMAT_R16G16B16_SFLOAT:
		return Format::R16G16B16_SFLOAT;
	case VK_FORMAT_R16G16B16A16_UNORM:
		return Format::R16G16B16A16_UNORM;
	case VK_FORMAT_R16G16B16A16_SNORM:
		return Format::R16G16B16A16_SNORM;
	case VK_FORMAT_R16G16B16A16_USCALED:
		return Format::R16G16B16A16_USCALED;
	case VK_FORMAT_R16G16B16A16_SSCALED:
		return Format::R16G16B16A16_SSCALED;
	case VK_FORMAT_R16G16B16A16_UINT:
		return Format::R16G16B16A16_UINT;
	case VK_FORMAT_R16G16B16A16_SINT:
		return Format::R16G16B16A16_SINT;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return Format::R16G16B16A16_SFLOAT;
	case VK_FORMAT_R32_UINT:
		return Format::R32_UINT;
	case VK_FORMAT_R32_SINT:
		return Format::R32_SINT;
	case VK_FORMAT_R32_SFLOAT:
		return Format::R32_SFLOAT;
	case VK_FORMAT_R32G32_UINT:
		return Format::R32G32_UINT;
	case VK_FORMAT_R32G32_SINT:
		return Format::R32G32_SINT;
	case VK_FORMAT_R32G32_SFLOAT:
		return Format::R32G32_SFLOAT;
	case VK_FORMAT_R32G32B32_UINT:
		return Format::R32G32B32_UINT;
	case VK_FORMAT_R32G32B32_SINT:
		return Format::R32G32B32_SINT;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return Format::R32G32B32_SFLOAT;
	case VK_FORMAT_R32G32B32A32_UINT:
		return Format::R32G32B32A32_UINT;
	case VK_FORMAT_R32G32B32A32_SINT:
		return Format::R32G32B32A32_SINT;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return Format::R32G32B32A32_SFLOAT;
	case VK_FORMAT_R64_UINT:
		return Format::R64_UINT;
	case VK_FORMAT_R64_SINT:
		return Format::R64_SINT;
	case VK_FORMAT_R64_SFLOAT:
		return Format::R64_SFLOAT;
	case VK_FORMAT_R64G64_UINT:
		return Format::R64G64_UINT;
	case VK_FORMAT_R64G64_SINT:
		return Format::R64G64_SINT;
	case VK_FORMAT_R64G64_SFLOAT:
		return Format::R64G64_SFLOAT;
	case VK_FORMAT_R64G64B64_UINT:
		return Format::R64G64B64_UINT;
	case VK_FORMAT_R64G64B64_SINT:
		return Format::R64G64B64_SINT;
	case VK_FORMAT_R64G64B64_SFLOAT:
		return Format::R64G64B64_SFLOAT;
	case VK_FORMAT_R64G64B64A64_UINT:
		return Format::R64G64B64A64_UINT;
	case VK_FORMAT_R64G64B64A64_SINT:
		return Format::R64G64B64A64_SINT;
	case VK_FORMAT_R64G64B64A64_SFLOAT:
		return Format::R64G64B64A64_SFLOAT;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		return Format::B10G11R11_UFLOAT;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		return Format::E5B9G9R9_UFLOAT;
	case VK_FORMAT_D16_UNORM:
		return Format::D16_UNORM;
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		return Format::X8_D24_UNORM;
	case VK_FORMAT_D32_SFLOAT:
		return Format::D32_SFLOAT;
	case VK_FORMAT_S8_UINT:
		return Format::S8_UINT;
	case VK_FORMAT_D16_UNORM_S8_UINT:
		return Format::D16_UNORM_S8_UINT;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return Format::D24_UNORM_S8_UINT;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return Format::D32_SFLOAT_S8_UINT;
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		return Format::BC1_RGB_UNORM;
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		return Format::BC1_RGB_SRGB;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		return Format::BC1_RGBA_UNORM;
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		return Format::BC1_RGBA_SRGB;
	case VK_FORMAT_BC2_UNORM_BLOCK:
		return Format::BC2_UNORM;
	case VK_FORMAT_BC2_SRGB_BLOCK:
		return Format::BC2_SRGB;
	case VK_FORMAT_BC3_UNORM_BLOCK:
		return Format::BC3_UNORM;
	case VK_FORMAT_BC3_SRGB_BLOCK:
		return Format::BC3_SRGB;
	case VK_FORMAT_BC4_UNORM_BLOCK:
		return Format::BC4_UNORM;
	case VK_FORMAT_BC4_SNORM_BLOCK:
		return Format::BC4_SNORM;
	case VK_FORMAT_BC5_UNORM_BLOCK:
		return Format::BC5_UNORM;
	case VK_FORMAT_BC5_SNORM_BLOCK:
		return Format::BC5_SNORM;
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		return Format::BC6H_UFLOAT;
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		return Format::BC6H_SFLOAT;
	case VK_FORMAT_BC7_UNORM_BLOCK:
		return Format::BC7_UNORM;
	case VK_FORMAT_BC7_SRGB_BLOCK:
		return Format::BC7_SRGB;
	default:
		return Format::Other;
	}
}

VkFormat LiteFX::Rendering::Backends::getFormat(const Format& format)
{
	switch (format)
	{
	case Format::R4G4_UNORM:
		return VK_FORMAT_R4G4_UNORM_PACK8;
	case Format::R4G4B4A4_UNORM:
		return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
	case Format::B4G4R4A4_UNORM:
		return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
	case Format::R5G6B5_UNORM:
		return VK_FORMAT_R5G6B5_UNORM_PACK16;
	case Format::B5G6R5_UNORM:
		return VK_FORMAT_B5G6R5_UNORM_PACK16;
	case Format::R5G5B5A1_UNORM:
		return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
	case Format::B5G5R5A1_UNORM:
		return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
	case Format::A1R5G5B5_UNORM:
		return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
	case Format::R8_UNORM:
		return VK_FORMAT_R8_UNORM;
	case Format::R8_SNORM:
		return VK_FORMAT_R8_SNORM;
	case Format::R8_USCALED:
		return VK_FORMAT_R8_USCALED;
	case Format::R8_SSCALED:
		return VK_FORMAT_R8_SSCALED;
	case Format::R8_UINT:
		return VK_FORMAT_R8_UINT;
	case Format::R8_SINT:
		return VK_FORMAT_R8_SINT;
	case Format::R8_SRGB:
		return VK_FORMAT_R8_SRGB;
	case Format::R8G8_UNORM:
		return VK_FORMAT_R8G8_UNORM;
	case Format::R8G8_SNORM:
		return VK_FORMAT_R8G8_SNORM;
	case Format::R8G8_USCALED:
		return VK_FORMAT_R8G8_USCALED;
	case Format::R8G8_SSCALED:
		return VK_FORMAT_R8G8_SSCALED;
	case Format::R8G8_UINT:
		return VK_FORMAT_R8G8_UINT;
	case Format::R8G8_SINT:
		return VK_FORMAT_R8G8_SINT;
	case Format::R8G8_SRGB:
		return VK_FORMAT_R8G8_SRGB;
	case Format::R8G8B8_UNORM:
		return VK_FORMAT_R8G8B8_UNORM;
	case Format::R8G8B8_SNORM:
		return VK_FORMAT_R8G8B8_SNORM;
	case Format::R8G8B8_USCALED:
		return VK_FORMAT_R8G8B8_USCALED;
	case Format::R8G8B8_SSCALED:
		return VK_FORMAT_R8G8B8_SSCALED;
	case Format::R8G8B8_UINT:
		return VK_FORMAT_R8G8B8_UINT;
	case Format::R8G8B8_SINT:
		return VK_FORMAT_R8G8B8_SINT;
	case Format::R8G8B8_SRGB:
		return VK_FORMAT_R8G8B8_SRGB;
	case Format::B8G8R8_UNORM:
		return VK_FORMAT_B8G8R8_UNORM;
	case Format::B8G8R8_SNORM:
		return VK_FORMAT_B8G8R8_SNORM;
	case Format::B8G8R8_USCALED:
		return VK_FORMAT_B8G8R8_USCALED;
	case Format::B8G8R8_SSCALED:
		return VK_FORMAT_B8G8R8_SSCALED;
	case Format::B8G8R8_UINT:
		return VK_FORMAT_B8G8R8_UINT;
	case Format::B8G8R8_SINT:
		return VK_FORMAT_B8G8R8_SINT;
	case Format::B8G8R8_SRGB:
		return VK_FORMAT_B8G8R8_SRGB;
	case Format::R8G8B8A8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case Format::R8G8B8A8_SNORM:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case Format::R8G8B8A8_USCALED:
		return VK_FORMAT_R8G8B8A8_USCALED;
	case Format::R8G8B8A8_SSCALED:
		return VK_FORMAT_R8G8B8A8_SSCALED;
	case Format::R8G8B8A8_UINT:
		return VK_FORMAT_R8G8B8A8_UINT;
	case Format::R8G8B8A8_SINT:
		return VK_FORMAT_R8G8B8A8_SINT;
	case Format::R8G8B8A8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case Format::B8G8R8A8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Format::B8G8R8A8_SNORM:
		return VK_FORMAT_B8G8R8A8_SNORM;
	case Format::B8G8R8A8_USCALED:
		return VK_FORMAT_B8G8R8A8_USCALED;
	case Format::B8G8R8A8_SSCALED:
		return VK_FORMAT_B8G8R8A8_SSCALED;
	case Format::B8G8R8A8_UINT:
		return VK_FORMAT_B8G8R8A8_UINT;
	case Format::B8G8R8A8_SINT:
		return VK_FORMAT_B8G8R8A8_SINT;
	case Format::B8G8R8A8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case Format::A8B8G8R8_UNORM:
		return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
	case Format::A8B8G8R8_SNORM:
		return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
	case Format::A8B8G8R8_USCALED:
		return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
	case Format::A8B8G8R8_SSCALED:
		return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
	case Format::A8B8G8R8_UINT:
		return VK_FORMAT_A8B8G8R8_UINT_PACK32;
	case Format::A8B8G8R8_SINT:
		return VK_FORMAT_A8B8G8R8_SINT_PACK32;
	case Format::A8B8G8R8_SRGB:
		return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
	case Format::A2R10G10B10_UNORM:
		return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	case Format::A2R10G10B10_SNORM:
		return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
	case Format::A2R10G10B10_USCALED:
		return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
	case Format::A2R10G10B10_SSCALED:
		return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
	case Format::A2R10G10B10_UINT:
		return VK_FORMAT_A2R10G10B10_UINT_PACK32;
	case Format::A2R10G10B10_SINT:
		return VK_FORMAT_A2R10G10B10_SINT_PACK32;
	case Format::A2B10G10R10_UNORM:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case Format::A2B10G10R10_SNORM:
		return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
	case Format::A2B10G10R10_USCALED:
		return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
	case Format::A2B10G10R10_SSCALED:
		return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
	case Format::A2B10G10R10_UINT:
		return VK_FORMAT_A2B10G10R10_UINT_PACK32;
	case Format::A2B10G10R10_SINT:
		return VK_FORMAT_A2B10G10R10_SINT_PACK32;
	case Format::R16_UNORM:
		return VK_FORMAT_R16_UNORM;
	case Format::R16_SNORM:
		return VK_FORMAT_R16_SNORM;
	case Format::R16_USCALED:
		return VK_FORMAT_R16_USCALED;
	case Format::R16_SSCALED:
		return VK_FORMAT_R16_SSCALED;
	case Format::R16_UINT:
		return VK_FORMAT_R16_UINT;
	case Format::R16_SINT:
		return VK_FORMAT_R16_SINT;
	case Format::R16_SFLOAT:
		return VK_FORMAT_R16_SFLOAT;
	case Format::R16G16_UNORM:
		return VK_FORMAT_R16G16_UNORM;
	case Format::R16G16_SNORM:
		return VK_FORMAT_R16G16_SNORM;
	case Format::R16G16_USCALED:
		return VK_FORMAT_R16G16_USCALED;
	case Format::R16G16_SSCALED:
		return VK_FORMAT_R16G16_SSCALED;
	case Format::R16G16_UINT:
		return VK_FORMAT_R16G16_UINT;
	case Format::R16G16_SINT:
		return VK_FORMAT_R16G16_SINT;
	case Format::R16G16_SFLOAT:
		return VK_FORMAT_R16G16_SFLOAT;
	case Format::R16G16B16_UNORM:
		return VK_FORMAT_R16G16B16_UNORM;
	case Format::R16G16B16_SNORM:
		return VK_FORMAT_R16G16B16_SNORM;
	case Format::R16G16B16_USCALED:
		return VK_FORMAT_R16G16B16_USCALED;
	case Format::R16G16B16_SSCALED:
		return VK_FORMAT_R16G16B16_SSCALED;
	case Format::R16G16B16_UINT:
		return VK_FORMAT_R16G16B16_UINT;
	case Format::R16G16B16_SINT:
		return VK_FORMAT_R16G16B16_SINT;
	case Format::R16G16B16_SFLOAT:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case Format::R16G16B16A16_UNORM:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case Format::R16G16B16A16_SNORM:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case Format::R16G16B16A16_USCALED:
		return VK_FORMAT_R16G16B16A16_USCALED;
	case Format::R16G16B16A16_SSCALED:
		return VK_FORMAT_R16G16B16A16_SSCALED;
	case Format::R16G16B16A16_UINT:
		return VK_FORMAT_R16G16B16A16_UINT;
	case Format::R16G16B16A16_SINT:
		return VK_FORMAT_R16G16B16A16_SINT;
	case Format::R16G16B16A16_SFLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case Format::R32_UINT:
		return VK_FORMAT_R32_UINT;
	case Format::R32_SINT:
		return VK_FORMAT_R32_SINT;
	case Format::R32_SFLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case Format::R32G32_UINT:
		return VK_FORMAT_R32G32_UINT;
	case Format::R32G32_SINT:
		return VK_FORMAT_R32G32_SINT;
	case Format::R32G32_SFLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case Format::R32G32B32_UINT:
		return VK_FORMAT_R32G32B32_UINT;
	case Format::R32G32B32_SINT:
		return VK_FORMAT_R32G32B32_SINT;
	case Format::R32G32B32_SFLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case Format::R32G32B32A32_UINT:
		return VK_FORMAT_R32G32B32A32_UINT;
	case Format::R32G32B32A32_SINT:
		return VK_FORMAT_R32G32B32A32_SINT;
	case Format::R32G32B32A32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Format::R64_UINT:
		return VK_FORMAT_R64_UINT;
	case Format::R64_SINT:
		return VK_FORMAT_R64_SINT;
	case Format::R64_SFLOAT:
		return VK_FORMAT_R64_SFLOAT;
	case Format::R64G64_UINT:
		return VK_FORMAT_R64G64_UINT;
	case Format::R64G64_SINT:
		return VK_FORMAT_R64G64_SINT;
	case Format::R64G64_SFLOAT:
		return VK_FORMAT_R64G64_SFLOAT;
	case Format::R64G64B64_UINT:
		return VK_FORMAT_R64G64B64_UINT;
	case Format::R64G64B64_SINT:
		return VK_FORMAT_R64G64B64_SINT;
	case Format::R64G64B64_SFLOAT:
		return VK_FORMAT_R64G64B64_SFLOAT;
	case Format::R64G64B64A64_UINT:
		return VK_FORMAT_R64G64B64A64_UINT;
	case Format::R64G64B64A64_SINT:
		return VK_FORMAT_R64G64B64A64_SINT;
	case Format::R64G64B64A64_SFLOAT:
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	case Format::B10G11R11_UFLOAT:
		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case Format::E5B9G9R9_UFLOAT:
		return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
	case Format::D16_UNORM:
		return VK_FORMAT_D16_UNORM;
	case Format::X8_D24_UNORM:
		return VK_FORMAT_X8_D24_UNORM_PACK32;
	case Format::D32_SFLOAT:
		return VK_FORMAT_D32_SFLOAT;
	case Format::S8_UINT:
		return VK_FORMAT_S8_UINT;
	case Format::D16_UNORM_S8_UINT:
		return VK_FORMAT_D16_UNORM_S8_UINT;
	case Format::D24_UNORM_S8_UINT:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case Format::D32_SFLOAT_S8_UINT:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case Format::BC1_RGB_UNORM:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case Format::BC1_RGB_SRGB:
		return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
	case Format::BC1_RGBA_UNORM:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case Format::BC1_RGBA_SRGB:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case Format::BC2_UNORM:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case Format::BC2_SRGB:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case Format::BC3_UNORM:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case Format::BC3_SRGB:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case Format::BC4_UNORM:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case Format::BC4_SNORM:
		return VK_FORMAT_BC4_SNORM_BLOCK;
	case Format::BC5_UNORM:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case Format::BC5_SNORM:
		return VK_FORMAT_BC5_SNORM_BLOCK;
	case Format::BC6H_UFLOAT:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case Format::BC6H_SFLOAT:
		return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case Format::BC7_UNORM:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case Format::BC7_SRGB:
		return VK_FORMAT_BC7_SRGB_BLOCK;
	default:
		throw std::invalid_argument("Unsupported format.");
	}
}

size_t LITEFX_VULKAN_API LiteFX::Rendering::Backends::getSize(const Format& format)
{
	switch (format)
	{
	case Format::None:
		return 0;
	case Format::R4G4_UNORM:
	case Format::R8_UNORM:
	case Format::R8_SNORM:
	case Format::R8_USCALED:
	case Format::R8_SSCALED:
	case Format::R8_UINT:
	case Format::R8_SINT:
	case Format::R8_SRGB:
	case Format::S8_UINT:
		return 1;
	case Format::R4G4B4A4_UNORM:
	case Format::B4G4R4A4_UNORM:
	case Format::R5G6B5_UNORM:
	case Format::B5G6R5_UNORM:
	case Format::R5G5B5A1_UNORM:
	case Format::B5G5R5A1_UNORM:
	case Format::A1R5G5B5_UNORM:
	case Format::R8G8_UNORM:
	case Format::R8G8_SNORM:
	case Format::R8G8_USCALED:
	case Format::R8G8_SSCALED:
	case Format::R8G8_UINT:
	case Format::R8G8_SINT:
	case Format::R8G8_SRGB:
	case Format::R16_UNORM:
	case Format::R16_SNORM:
	case Format::R16_USCALED:
	case Format::R16_SSCALED:
	case Format::R16_UINT:
	case Format::R16_SINT:
	case Format::R16_SFLOAT:
	case Format::D16_UNORM:
		return 2;
	case Format::R8G8B8_UNORM:
	case Format::R8G8B8_SNORM:
	case Format::R8G8B8_USCALED:
	case Format::R8G8B8_SSCALED:
	case Format::R8G8B8_UINT:
	case Format::R8G8B8_SINT:
	case Format::R8G8B8_SRGB:
	case Format::B8G8R8_UNORM:
	case Format::B8G8R8_SNORM:
	case Format::B8G8R8_USCALED:
	case Format::B8G8R8_SSCALED:
	case Format::B8G8R8_UINT:
	case Format::B8G8R8_SINT:
	case Format::B8G8R8_SRGB:
	case Format::D16_UNORM_S8_UINT:
		return 3;
	case Format::R8G8B8A8_UNORM:
	case Format::R8G8B8A8_SNORM:
	case Format::R8G8B8A8_USCALED:
	case Format::R8G8B8A8_SSCALED:
	case Format::R8G8B8A8_UINT:
	case Format::R8G8B8A8_SINT:
	case Format::R8G8B8A8_SRGB:
	case Format::B8G8R8A8_UNORM:
	case Format::B8G8R8A8_SNORM:
	case Format::B8G8R8A8_USCALED:
	case Format::B8G8R8A8_SSCALED:
	case Format::B8G8R8A8_UINT:
	case Format::B8G8R8A8_SINT:
	case Format::B8G8R8A8_SRGB:
	case Format::A8B8G8R8_UNORM:
	case Format::A8B8G8R8_SNORM:
	case Format::A8B8G8R8_USCALED:
	case Format::A8B8G8R8_SSCALED:
	case Format::A8B8G8R8_UINT:
	case Format::A8B8G8R8_SINT:
	case Format::A8B8G8R8_SRGB:
	case Format::A2R10G10B10_UNORM:
	case Format::A2R10G10B10_SNORM:
	case Format::A2R10G10B10_USCALED:
	case Format::A2R10G10B10_SSCALED:
	case Format::A2R10G10B10_UINT:
	case Format::A2R10G10B10_SINT:
	case Format::A2B10G10R10_UNORM:
	case Format::A2B10G10R10_SNORM:
	case Format::A2B10G10R10_USCALED:
	case Format::A2B10G10R10_SSCALED:
	case Format::A2B10G10R10_UINT:
	case Format::A2B10G10R10_SINT:
	case Format::R16G16_UNORM:
	case Format::R16G16_SNORM:
	case Format::R16G16_USCALED:
	case Format::R16G16_SSCALED:
	case Format::R16G16_UINT:
	case Format::R16G16_SINT:
	case Format::R16G16_SFLOAT:
	case Format::R32_UINT:
	case Format::R32_SINT:
	case Format::R32_SFLOAT:
	case Format::B10G11R11_UFLOAT:
	case Format::E5B9G9R9_UFLOAT:
	case Format::X8_D24_UNORM:
	case Format::D32_SFLOAT:
	case Format::D24_UNORM_S8_UINT:
		return 4;
	case Format::R16G16B16_UNORM:
	case Format::R16G16B16_SNORM:
	case Format::R16G16B16_USCALED:
	case Format::R16G16B16_SSCALED:
	case Format::R16G16B16_UINT:
	case Format::R16G16B16_SINT:
	case Format::R16G16B16_SFLOAT:
		return 6;
	case Format::R16G16B16A16_UNORM:
	case Format::R16G16B16A16_SNORM:
	case Format::R16G16B16A16_USCALED:
	case Format::R16G16B16A16_SSCALED:
	case Format::R16G16B16A16_UINT:
	case Format::R16G16B16A16_SINT:
	case Format::R16G16B16A16_SFLOAT:
	case Format::R32G32_UINT:
	case Format::R32G32_SINT:
	case Format::R32G32_SFLOAT:
	case Format::R64_UINT:
	case Format::R64_SINT:
	case Format::R64_SFLOAT:
	case Format::D32_SFLOAT_S8_UINT:	// NOTE: This may be wrong here - spec states, however, that 24 bits are unused.
	case Format::BC1_RGB_UNORM:
	case Format::BC1_RGB_SRGB:
	case Format::BC1_RGBA_UNORM:
	case Format::BC1_RGBA_SRGB:
	case Format::BC4_UNORM:
	case Format::BC4_SNORM:
		return 8;
	case Format::R32G32B32_UINT:
	case Format::R32G32B32_SINT:
	case Format::R32G32B32_SFLOAT:
		return 12;
	case Format::R32G32B32A32_UINT:
	case Format::R32G32B32A32_SINT:
	case Format::R32G32B32A32_SFLOAT:
	case Format::R64G64_UINT:
	case Format::R64G64_SINT:
	case Format::R64G64_SFLOAT:
	case Format::BC2_UNORM:
	case Format::BC2_SRGB:
	case Format::BC3_UNORM:
	case Format::BC3_SRGB:
	case Format::BC5_UNORM:
	case Format::BC5_SNORM:
	case Format::BC6H_UFLOAT:
	case Format::BC6H_SFLOAT:
	case Format::BC7_UNORM:
	case Format::BC7_SRGB:
		return 16;
	case Format::R64G64B64_UINT:
	case Format::R64G64B64_SINT:
	case Format::R64G64B64_SFLOAT:
		return 24;
	case Format::R64G64B64A64_UINT:
	case Format::R64G64B64A64_SINT:
	case Format::R64G64B64A64_SFLOAT:
		return 32;
	default:
		throw std::invalid_argument("Unsupported format.");
	}
}

VkFormat LITEFX_VULKAN_API LiteFX::Rendering::Backends::getFormat(const BufferFormat& format)
{
	switch (format)
	{
	case BufferFormat::X16F:
		return VK_FORMAT_R16_SFLOAT;
	case BufferFormat::X16I:
		return VK_FORMAT_R16_SINT;
	case BufferFormat::X16U:
		return VK_FORMAT_R16_UINT;
	case BufferFormat::XY16F:
		return VK_FORMAT_R16G16_SFLOAT;
	case BufferFormat::XY16I:
		return VK_FORMAT_R16G16_SINT;
	case BufferFormat::XY16U:
		return VK_FORMAT_R16G16_UINT;
	case BufferFormat::XYZ16F:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case BufferFormat::XYZ16I:
		return VK_FORMAT_R16G16B16_SINT;
	case BufferFormat::XYZ16U:
		return VK_FORMAT_R16G16B16_UINT;
	case BufferFormat::XYZW16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case BufferFormat::XYZW16I:
		return VK_FORMAT_R16G16B16A16_SINT;
	case BufferFormat::XYZW16U:
		return VK_FORMAT_R16G16B16A16_UINT;
	case BufferFormat::X32F:
		return VK_FORMAT_R32_SFLOAT;
	case BufferFormat::X32I:
		return VK_FORMAT_R32_SINT;
	case BufferFormat::X32U:
		return VK_FORMAT_R32_UINT;
	case BufferFormat::XY32F:
		return VK_FORMAT_R32G32_SFLOAT;
	case BufferFormat::XY32I:
		return VK_FORMAT_R32G32_SINT;
	case BufferFormat::XY32U:
		return VK_FORMAT_R32G32_UINT;
	case BufferFormat::XYZ32F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case BufferFormat::XYZ32I:
		return VK_FORMAT_R32G32B32_SINT;
	case BufferFormat::XYZ32U:
		return VK_FORMAT_R32G32B32_UINT;
	case BufferFormat::XYZW32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case BufferFormat::XYZW32I:
		return VK_FORMAT_R32G32B32A32_SINT;
	case BufferFormat::XYZW32U:
		return VK_FORMAT_R32G32B32A32_UINT;
	default:
		throw std::invalid_argument("Unsupported format.");
	}
}

PolygonMode LiteFX::Rendering::Backends::getPolygonMode(const VkPolygonMode& mode)
{
	switch (mode)
	{
	case VkPolygonMode::VK_POLYGON_MODE_LINE:
		return PolygonMode::Wireframe;
	case VkPolygonMode::VK_POLYGON_MODE_POINT:
		return PolygonMode::Point;
	case VkPolygonMode::VK_POLYGON_MODE_FILL:
		return PolygonMode::Solid;
	default:
		throw std::invalid_argument("Unsupported polygon mode.");
	}
}

VkPolygonMode LiteFX::Rendering::Backends::getPolygonMode(const PolygonMode& mode)
{
	switch (mode)
	{
	case PolygonMode::Solid:
		return VkPolygonMode::VK_POLYGON_MODE_FILL;
	case PolygonMode::Wireframe:
		return VkPolygonMode::VK_POLYGON_MODE_LINE;
	case PolygonMode::Point:
		return VkPolygonMode::VK_POLYGON_MODE_POINT;
	default:
		throw std::invalid_argument("Unsupported polygon mode.");
	}
}

CullMode LiteFX::Rendering::Backends::getCullMode(const VkCullModeFlags& mode)
{
	switch (mode)
	{
	case VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT:
		return CullMode::BackFaces;
	case VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT:
		return CullMode::FrontFaces;
	case VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK:
		return CullMode::Both;
	case VkCullModeFlagBits::VK_CULL_MODE_NONE:
		return CullMode::Disabled;
	default:
		throw std::invalid_argument("Unsupported cull mode.");
	}
}

VkCullModeFlags LiteFX::Rendering::Backends::getCullMode(const CullMode& mode)
{
	switch (mode)
	{
	case CullMode::BackFaces:
		return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
	case CullMode::FrontFaces:
		return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
	case CullMode::Both:
		return VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK;
	case CullMode::Disabled:
		return VkCullModeFlagBits::VK_CULL_MODE_NONE;
	default:
		throw std::invalid_argument("Unsupported cull mode.");
	}
}

PrimitiveTopology LiteFX::Rendering::Backends::getPrimitiveTopology(const VkPrimitiveTopology& topology)
{
	switch (topology)
	{
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return PrimitiveTopology::TriangleList;
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return PrimitiveTopology::LineStrip;
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return PrimitiveTopology::TriangleList;
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return PrimitiveTopology::TriangleStrip;
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return PrimitiveTopology::PointList;
	default:
		throw std::invalid_argument("Unsupported primitive topology.");
	}
}

VkPrimitiveTopology LiteFX::Rendering::Backends::getPrimitiveTopology(const PrimitiveTopology& topology)
{
	switch (topology)
	{
	case PrimitiveTopology::LineList:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveTopology::LineStrip:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitiveTopology::PointList:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PrimitiveTopology::TriangleList:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitiveTopology::TriangleStrip:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	default:
		throw std::invalid_argument("Unsupported primitive topology.");
	}
}

ShaderType LiteFX::Rendering::Backends::getShaderStage(const VkShaderStageFlagBits& shaderType)
{
	switch (shaderType)
	{
	case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:
		return ShaderType::Vertex;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return ShaderType::TessellationControl;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return ShaderType::TessellationEvaluation;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT:
		return ShaderType::Geometry;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:
		return ShaderType::Fragment;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:
		return ShaderType::Compute;
	default:
		return ShaderType::Other;
	}
}

VkShaderStageFlagBits LiteFX::Rendering::Backends::getShaderStage(const ShaderType& shaderType)
{
	switch (shaderType)
	{
	case ShaderType::Vertex:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::TessellationControl:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderType::TessellationEvaluation:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderType::Geometry:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderType::Fragment:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderType::Compute:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
	case ShaderType::Other:
	default:
		throw std::invalid_argument("Unsupported shader type.");
	}
}

MultiSamplingLevel LiteFX::Rendering::Backends::getSamples(const VkSampleCountFlagBits& samples)
{
	switch (samples)
	{
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT:
		return MultiSamplingLevel::x1;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT:
		return MultiSamplingLevel::x2;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT:
		return MultiSamplingLevel::x4;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT:
		return MultiSamplingLevel::x8;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT:
		return MultiSamplingLevel::x16;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT:
		return MultiSamplingLevel::x32;
	case VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT:
		return MultiSamplingLevel::x64;
	default:
		throw std::invalid_argument("Unsupported number of samples.");
	}
}

VkSampleCountFlagBits LiteFX::Rendering::Backends::getSamples(const MultiSamplingLevel& samples)
{
	switch (samples)
	{
	case MultiSamplingLevel::x1:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	case MultiSamplingLevel::x2:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT;
	case MultiSamplingLevel::x4:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;
	case MultiSamplingLevel::x8:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT;
	case MultiSamplingLevel::x16:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT;
	case MultiSamplingLevel::x32:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT;
	case MultiSamplingLevel::x64:
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT;
	default:
		throw std::invalid_argument("Unsupported number of samples.");
	}
}