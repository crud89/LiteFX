#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

constexpr Format LiteFX::Rendering::Backends::Vk::getFormat(const VkFormat& format)
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

constexpr VkFormat LiteFX::Rendering::Backends::Vk::getFormat(Format format)
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

constexpr VkFormat LiteFX::Rendering::Backends::Vk::getFormat(BufferFormat format)
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

constexpr PolygonMode LiteFX::Rendering::Backends::Vk::getPolygonMode(const VkPolygonMode& mode)
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

constexpr VkPolygonMode LiteFX::Rendering::Backends::Vk::getPolygonMode(PolygonMode mode)
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

constexpr CullMode LiteFX::Rendering::Backends::Vk::getCullMode(const VkCullModeFlags& mode)
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

constexpr VkCullModeFlags LiteFX::Rendering::Backends::Vk::getCullMode(CullMode mode)
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

constexpr PrimitiveTopology LiteFX::Rendering::Backends::Vk::getPrimitiveTopology(const VkPrimitiveTopology& topology)
{
	switch (topology)
	{
	case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return PrimitiveTopology::LineList;
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

constexpr VkPrimitiveTopology LiteFX::Rendering::Backends::Vk::getPrimitiveTopology(const PrimitiveTopology& topology)
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

constexpr ShaderStage LiteFX::Rendering::Backends::Vk::getShaderStage(const VkShaderStageFlagBits& shaderType)
{
	switch (shaderType)
	{
	case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:
		return ShaderStage::Vertex;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return ShaderStage::TessellationControl;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return ShaderStage::TessellationEvaluation;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT:
		return ShaderStage::Geometry;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:
		return ShaderStage::Fragment;
	case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:
		return ShaderStage::Compute;
	default:
		return ShaderStage::Other;
	}
}

constexpr VkShaderStageFlagBits LiteFX::Rendering::Backends::Vk::getShaderStage(ShaderStage shaderType)
{
	switch (shaderType)
	{
	case ShaderStage::Vertex:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::TessellationControl:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderStage::TessellationEvaluation:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderStage::Geometry:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderStage::Fragment:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::Compute:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
	case ShaderStage::Other:
	default:
		throw std::invalid_argument("Unsupported shader type.");
	}
}

constexpr MultiSamplingLevel LiteFX::Rendering::Backends::Vk::getSamples(const VkSampleCountFlagBits& samples)
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

constexpr VkImageType LiteFX::Rendering::Backends::Vk::getImageType(ImageDimensions dimension)
{
	switch (dimension)
	{
	case ImageDimensions::DIM_1:
		return VkImageType::VK_IMAGE_TYPE_1D;
	case ImageDimensions::DIM_2:
	case ImageDimensions::CUBE:
		return VkImageType::VK_IMAGE_TYPE_2D;
	case ImageDimensions::DIM_3:
		return VkImageType::VK_IMAGE_TYPE_3D;
	default:
		throw std::invalid_argument("Unsupported image dimension.");
	}
}

constexpr VkImageViewType LiteFX::Rendering::Backends::Vk::getImageViewType(ImageDimensions dimension, UInt32 layers)
{
	switch (dimension)
	{
	case ImageDimensions::DIM_1:
		return layers == 1 ? VkImageViewType::VK_IMAGE_VIEW_TYPE_1D : VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case ImageDimensions::DIM_2:
		return layers == 1 ? VkImageViewType::VK_IMAGE_VIEW_TYPE_2D : VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case ImageDimensions::DIM_3:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
	case ImageDimensions::CUBE:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	default:
		throw std::invalid_argument("Unsupported image dimension.");
	}
}

constexpr VkSampleCountFlagBits LiteFX::Rendering::Backends::Vk::getSamples(MultiSamplingLevel samples)
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

constexpr VkCompareOp LiteFX::Rendering::Backends::Vk::getCompareOp(CompareOperation compareOp)
{
	switch (compareOp) {
	case CompareOperation::Never: return VkCompareOp::VK_COMPARE_OP_NEVER;
	case CompareOperation::Less: return VkCompareOp::VK_COMPARE_OP_LESS;
	case CompareOperation::Greater: return VkCompareOp::VK_COMPARE_OP_GREATER;
	case CompareOperation::Equal: return VkCompareOp::VK_COMPARE_OP_EQUAL;
	case CompareOperation::LessEqual: return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
	case CompareOperation::GreaterEqual: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
	case CompareOperation::NotEqual: return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
	case CompareOperation::Always: return VkCompareOp::VK_COMPARE_OP_ALWAYS;
	default: throw InvalidArgumentException("Unsupported compare operation.");
	}
}

constexpr VkStencilOp LiteFX::Rendering::Backends::Vk::getStencilOp(StencilOperation stencilOp)
{
	switch (stencilOp) {
	case StencilOperation::Keep: return VkStencilOp::VK_STENCIL_OP_KEEP;
	case StencilOperation::Zero: return VkStencilOp::VK_STENCIL_OP_ZERO;
	case StencilOperation::Invert: return VkStencilOp::VK_STENCIL_OP_INVERT;
	case StencilOperation::Replace: return VkStencilOp::VK_STENCIL_OP_REPLACE;
	case StencilOperation::IncrementClamp: return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case StencilOperation::IncrementWrap: return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case StencilOperation::DecrementClamp: return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case StencilOperation::DecrementWrap: return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;
	default: throw InvalidArgumentException("Unsupported stencil operation.");
	}
}

constexpr VkBlendFactor LITEFX_VULKAN_API LiteFX::Rendering::Backends::Vk::getBlendFactor(BlendFactor blendFactor)
{
	switch (blendFactor) {
	case BlendFactor::Zero: return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	case BlendFactor::One: return VkBlendFactor::VK_BLEND_FACTOR_ONE;
	case BlendFactor::SourceColor: return VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;
	case BlendFactor::OneMinusSourceColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BlendFactor::DestinationColor: return VK_BLEND_FACTOR_DST_COLOR;
	case BlendFactor::OneMinusDestinationColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BlendFactor::SourceAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
	case BlendFactor::OneMinusSourceAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::DestinationAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
	case BlendFactor::OneMinusDestinationAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
	case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::SourceAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case BlendFactor::Source1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::OneMinusSource1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::Source1Alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
	case BlendFactor::OneMinusSource1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	default: throw InvalidArgumentException("Unsupported blend factor.");
	}
}

constexpr VkBlendOp LITEFX_VULKAN_API LiteFX::Rendering::Backends::Vk::getBlendOperation(BlendOperation blendOperation)
{
	switch (blendOperation) {
	case BlendOperation::Add: return VkBlendOp::VK_BLEND_OP_ADD;
	case BlendOperation::Subtract: return VkBlendOp::VK_BLEND_OP_SUBTRACT;
	case BlendOperation::ReverseSubtract: return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;
	case BlendOperation::Minimum: return VkBlendOp::VK_BLEND_OP_MIN;
	case BlendOperation::Maximum: return VkBlendOp::VK_BLEND_OP_MAX;
	default: throw InvalidArgumentException("Unsupported blend operation.");
	}
}

constexpr VkPipelineStageFlags LITEFX_VULKAN_API LiteFX::Rendering::Backends::Vk::getPipelineStage(PipelineStage pipelineStage)
{
	if (pipelineStage == PipelineStage::None)
		return VK_PIPELINE_STAGE_NONE;
	else if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::All))
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	else if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Draw))
		return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	else if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Compute))
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	VkPipelineStageFlags sync{ };

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::InputAssembly))
		sync |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Vertex))
		sync |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::TessellationControl))
		sync |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::TessellationEvaluation))
		sync |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Geometry))
		sync |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Fragment))
		sync |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::DepthStencil))
		sync |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Indirect))
		sync |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::RenderTarget))
		sync |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Transfer) ||
		LITEFX_FLAG_IS_SET(pipelineStage, PipelineStage::Resolve))
		sync |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	return sync;
}

constexpr VkAccessFlags LITEFX_VULKAN_API LiteFX::Rendering::Backends::Vk::getResourceAccess(ResourceAccess resourceAccess)
{
	if (resourceAccess == ResourceAccess::None)
		return VK_ACCESS_NONE;

	VkAccessFlags access = { };

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::Common))
		access |= (VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT);

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::VertexBuffer))
		access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::IndexBuffer))
		access |= VK_ACCESS_INDEX_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::UniformBuffer))
		access |= VK_ACCESS_UNIFORM_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::RenderTarget))
		access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::DepthStencilRead))
		access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::DepthStencilWrite))
		access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::ShaderRead))
		access |= VK_ACCESS_SHADER_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::ShaderReadWrite))
		access |= (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::Indirect))
		access |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::TransferRead))
		access |= VK_ACCESS_TRANSFER_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::TransferWrite))
		access |= VK_ACCESS_TRANSFER_WRITE_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::ResolveRead))
		access |= VK_ACCESS_MEMORY_READ_BIT;

	if (LITEFX_FLAG_IS_SET(resourceAccess, ResourceAccess::ResolveWrite))
		access |= VK_ACCESS_MEMORY_WRITE_BIT;

	return access;
}

constexpr VkImageLayout LITEFX_VULKAN_API LiteFX::Rendering::Backends::Vk::getImageLayout(ImageLayout imageLayout)
{
	switch (imageLayout) {
	case ImageLayout::Common: return VK_IMAGE_LAYOUT_GENERAL;
	case ImageLayout::ShaderResource: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageLayout::ReadWrite: return VK_IMAGE_LAYOUT_GENERAL;
	case ImageLayout::CopySource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ImageLayout::CopyDestination: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ImageLayout::RenderTarget: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ImageLayout::DepthRead: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ImageLayout::DepthWrite: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case ImageLayout::ResolveSource: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ImageLayout::ResolveDestination: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
	default: throw InvalidArgumentException("Unsupported image layout.");
	}
}