#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

Format LiteFX::Rendering::Backends::DX12::getFormat(const DXGI_FORMAT& format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return Format::R32G32B32A32_SFLOAT;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return Format::R32G32B32A32_UINT;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return Format::R32G32B32A32_SINT;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return Format::R32G32B32_SFLOAT;
	case DXGI_FORMAT_R32G32B32_UINT:
		return Format::R32G32B32_UINT;
	case DXGI_FORMAT_R32G32B32_SINT:
		return Format::R32G32B32_SINT;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return Format::R16G16B16A16_SFLOAT;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return Format::R16G16B16A16_UNORM;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return Format::R16G16B16A16_UINT;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return Format::R16G16B16A16_SNORM;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return Format::R16G16B16A16_SINT;
	case DXGI_FORMAT_R32G32_FLOAT:
		return Format::R32G32_SFLOAT;
	case DXGI_FORMAT_R32G32_UINT:
		return Format::R32G32_UINT;
	case DXGI_FORMAT_R32G32_SINT:
		return Format::R32G32_SINT;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return Format::D32_SFLOAT_S8_UINT;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return Format::A2R10G10B10_UNORM;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return Format::A2R10G10B10_UINT;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return Format::B10G11R11_UFLOAT;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return Format::R8G8B8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return Format::R8G8B8A8_SRGB;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return Format::R8G8B8A8_UINT;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return Format::R8G8B8A8_SNORM;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return Format::R8G8B8A8_SINT;
	case DXGI_FORMAT_R16G16_FLOAT:
		return Format::R16G16_SFLOAT;
	case DXGI_FORMAT_R16G16_UNORM:
		return Format::R16G16_UNORM;
	case DXGI_FORMAT_R16G16_UINT:
		return Format::R16G16_UINT;
	case DXGI_FORMAT_R16G16_SNORM:
		return Format::R16G16_SNORM;
	case DXGI_FORMAT_R16G16_SINT:
		return Format::R16G16_SINT;
	case DXGI_FORMAT_D32_FLOAT:
		return Format::D32_SFLOAT;
	case DXGI_FORMAT_R32_FLOAT:
		return Format::R32_SFLOAT;
	case DXGI_FORMAT_R32_UINT:
		return Format::R32_UINT;
	case DXGI_FORMAT_R32_SINT:
		return Format::R32_SINT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return Format::D24_UNORM_S8_UINT;
	case DXGI_FORMAT_R8G8_UNORM:
		return Format::R8G8_UNORM;
	case DXGI_FORMAT_R8G8_UINT:
		return Format::R8G8_UINT;
	case DXGI_FORMAT_R8G8_SNORM:
		return Format::R8G8_SNORM;
	case DXGI_FORMAT_R8G8_SINT:
		return Format::R8G8_SINT;
	case DXGI_FORMAT_R16_FLOAT:
		return Format::R16_SFLOAT;
	case DXGI_FORMAT_D16_UNORM:
		return Format::D16_UNORM;
	case DXGI_FORMAT_R16_UNORM:
		return Format::R16_UNORM;
	case DXGI_FORMAT_R16_UINT:
		return Format::R16_UINT;
	case DXGI_FORMAT_R16_SNORM:
		return Format::R16_SNORM;
	case DXGI_FORMAT_R16_SINT:
		return Format::R16_SINT;
	case DXGI_FORMAT_R8_UNORM:
		return Format::R8_UNORM;
	case DXGI_FORMAT_R8_UINT:
		return Format::R8_UINT;
	case DXGI_FORMAT_R8_SNORM:
		return Format::R8_SNORM;
	case DXGI_FORMAT_R8_SINT:
		return Format::R8_SINT;
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return Format::E5B9G9R9_UFLOAT;
	case DXGI_FORMAT_BC1_UNORM:
		return Format::BC1_RGB_UNORM;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return Format::BC1_RGB_SRGB;
	case DXGI_FORMAT_BC2_UNORM:
		return Format::BC2_UNORM;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return Format::BC2_SRGB;
	case DXGI_FORMAT_BC3_UNORM:
		return Format::BC3_UNORM;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return Format::BC3_SRGB;
	case DXGI_FORMAT_BC4_UNORM:
		return Format::BC4_UNORM;
	case DXGI_FORMAT_BC4_SNORM:
		return Format::BC4_SNORM;
	case DXGI_FORMAT_BC5_UNORM:
		return Format::BC5_UNORM;
	case DXGI_FORMAT_BC5_SNORM:
		return Format::BC5_SNORM;
	case DXGI_FORMAT_B5G6R5_UNORM:
		return Format::B5G6R5_UNORM;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return Format::B5G5R5A1_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return Format::B8G8R8A8_UNORM;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		return Format::A2B10G10R10_USCALED;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return Format::B8G8R8A8_SRGB;
	case DXGI_FORMAT_BC6H_UF16:
		return Format::BC6H_UFLOAT;
	case DXGI_FORMAT_BC6H_SF16:
		return Format::BC6H_SFLOAT;
	case DXGI_FORMAT_BC7_UNORM:
		return Format::BC7_UNORM;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return Format::BC7_SRGB;
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return Format::B4G4R4A4_UNORM;
	case DXGI_FORMAT_UNKNOWN:
		return Format::None;
	default:
		return Format::Other;
	}
}

DXGI_FORMAT LiteFX::Rendering::Backends::DX12::getFormat(const Format& format)
{
	switch (format)
	{
	case Format::R32G32B32A32_SFLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case Format::R32G32B32A32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case Format::R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case Format::R32G32B32_SFLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case Format::R32G32B32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case Format::R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_SINT;
	case Format::R16G16B16A16_SFLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case Format::R16G16B16A16_UNORM:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case Format::R16G16B16A16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case Format::R16G16B16A16_SNORM:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case Format::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case Format::R32G32_SFLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case Format::R32G32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case Format::R32G32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case Format::D32_SFLOAT_S8_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case Format::A2R10G10B10_UNORM:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case Format::A2R10G10B10_UINT:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case Format::B10G11R11_UFLOAT:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case Format::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case Format::R8G8B8A8_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case Format::R8G8B8A8_UINT:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case Format::R8G8B8A8_SNORM:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case Format::R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case Format::R16G16_SFLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case Format::R16G16_UNORM:
		return DXGI_FORMAT_R16G16_UNORM;
	case Format::R16G16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case Format::R16G16_SNORM:
		return DXGI_FORMAT_R16G16_SNORM;
	case Format::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case Format::D32_SFLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case Format::R32_SFLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case Format::R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case Format::R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case Format::D24_UNORM_S8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case Format::R8G8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case Format::R8G8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case Format::R8G8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case Format::R8G8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case Format::R16_SFLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case Format::D16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case Format::R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case Format::R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case Format::R16_SNORM:
		return DXGI_FORMAT_R16_SNORM;
	case Format::R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case Format::R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case Format::R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case Format::R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case Format::R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case Format::E5B9G9R9_UFLOAT:
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case Format::BC1_RGB_UNORM:
		return DXGI_FORMAT_BC1_UNORM;
	case Format::BC1_RGB_SRGB:
		return DXGI_FORMAT_BC1_UNORM_SRGB;
	case Format::BC2_UNORM:
		return DXGI_FORMAT_BC2_UNORM;
	case Format::BC2_SRGB:
		return DXGI_FORMAT_BC2_UNORM_SRGB;
	case Format::BC3_UNORM:
		return DXGI_FORMAT_BC3_UNORM;
	case Format::BC3_SRGB:
		return DXGI_FORMAT_BC3_UNORM_SRGB;
	case Format::BC4_UNORM:
		return DXGI_FORMAT_BC4_UNORM;
	case Format::BC4_SNORM:
		return DXGI_FORMAT_BC4_SNORM;
	case Format::BC5_UNORM:
		return DXGI_FORMAT_BC5_UNORM;
	case Format::BC5_SNORM:
		return DXGI_FORMAT_BC5_SNORM;
	case Format::B5G6R5_UNORM:
		return DXGI_FORMAT_B5G6R5_UNORM;
	case Format::B5G5R5A1_UNORM:
		return DXGI_FORMAT_B5G5R5A1_UNORM;
	case Format::B8G8R8A8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case Format::A2B10G10R10_USCALED:
		return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
	case Format::B8G8R8A8_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case Format::BC6H_UFLOAT:
		return DXGI_FORMAT_BC6H_UF16;
	case Format::BC6H_SFLOAT:
		return DXGI_FORMAT_BC6H_SF16;
	case Format::BC7_UNORM:
		return DXGI_FORMAT_BC7_UNORM;
	case Format::BC7_SRGB:
		return DXGI_FORMAT_BC7_UNORM_SRGB;
	case Format::B4G4R4A4_UNORM:
		return DXGI_FORMAT_B4G4R4A4_UNORM;
	default:
		throw InvalidArgumentException("Unsupported format: {0}", format);
	}
}

DXGI_FORMAT LiteFX::Rendering::Backends::DX12::getFormat(const BufferFormat& format)
{
	switch (format)
	{
	case BufferFormat::X16F:
		return DXGI_FORMAT_R16_FLOAT;
	case BufferFormat::X16I:
		return DXGI_FORMAT_R16_SINT;
	case BufferFormat::X16U:
		return DXGI_FORMAT_R16_UINT;
	case BufferFormat::XY16F:
		return DXGI_FORMAT_R16G16_FLOAT;
	case BufferFormat::XY16I:
		return DXGI_FORMAT_R16G16_SINT;
	case BufferFormat::XY16U:
		return DXGI_FORMAT_R16G16_UINT;
	case BufferFormat::X32F:
		return DXGI_FORMAT_R32_FLOAT;
	case BufferFormat::X32I:
		return DXGI_FORMAT_R32_SINT;
	case BufferFormat::X32U:
		return DXGI_FORMAT_R32_UINT;
	case BufferFormat::XY32F:
		return DXGI_FORMAT_R32G32_FLOAT;
	case BufferFormat::XY32I:
		return DXGI_FORMAT_R32G32_SINT;
	case BufferFormat::XY32U:
		return DXGI_FORMAT_R32G32_UINT;
	case BufferFormat::XYZ32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case BufferFormat::XYZ32I:
		return DXGI_FORMAT_R32G32B32_SINT;
	case BufferFormat::XYZ32U:
		return DXGI_FORMAT_R32G32B32_UINT;
	case BufferFormat::XYZW32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case BufferFormat::XYZW32I:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case BufferFormat::XYZW32U:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case BufferFormat::XYZW16F:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case BufferFormat::XYZW16I:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case BufferFormat::XYZW16U:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	default:
		throw InvalidArgumentException("Unsupported format: {0}.", format);
	}
}

bool LiteFX::Rendering::Backends::DX12::isSRGB(const Format& format)
{
	return 
		format == Format::A8B8G8R8_SRGB || 
		format == Format::B8G8R8A8_SRGB || 
		format == Format::B8G8R8_SRGB ||
		format == Format::BC1_RGBA_SRGB ||
		format == Format::BC1_RGB_SRGB ||
		format == Format::BC2_SRGB ||
		format == Format::BC3_SRGB ||
		format == Format::BC7_SRGB ||
		format == Format::R8G8B8A8_SRGB || 
		format == Format::R8G8B8_SRGB || 
		format == Format::R8G8_SRGB || 
		format == Format::R8_SRGB;
}

D3D12_RESOURCE_DIMENSION LiteFX::Rendering::Backends::DX12::getImageType(const ImageDimensions& dimensions)
{
	switch (dimensions)
	{
	case ImageDimensions::DIM_1:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case ImageDimensions::DIM_2:
	case ImageDimensions::CUBE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case ImageDimensions::DIM_3:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	default:
		throw std::invalid_argument("Unsupported image dimensionality.");
	}
}

PolygonMode LiteFX::Rendering::Backends::DX12::getPolygonMode(const D3D12_FILL_MODE& mode)
{
	switch (mode)
	{
	case D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME:
		return PolygonMode::Wireframe;
	case D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID:
		return PolygonMode::Solid;
	default:
		throw std::invalid_argument("Unsupported polygon mode.");
	}
}

D3D12_FILL_MODE LiteFX::Rendering::Backends::DX12::getPolygonMode(const PolygonMode& mode)
{
	switch (mode)
	{
	case PolygonMode::Solid:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	case PolygonMode::Wireframe:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
	default:
		throw std::invalid_argument("Unsupported polygon mode.");
	}
}

CullMode LiteFX::Rendering::Backends::DX12::getCullMode(const D3D12_CULL_MODE& mode)
{
	switch (mode)
	{
	case D3D12_CULL_MODE::D3D12_CULL_MODE_BACK:
		return CullMode::BackFaces;
	case D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT:
		return CullMode::FrontFaces;
	case D3D12_CULL_MODE::D3D12_CULL_MODE_NONE:
		return CullMode::Disabled;
	default:
		throw std::invalid_argument("Unsupported cull mode.");
	}
}

D3D12_CULL_MODE LiteFX::Rendering::Backends::DX12::getCullMode(const CullMode& mode)
{
	switch (mode)
	{
	case CullMode::BackFaces:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	case CullMode::FrontFaces:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
	case CullMode::Disabled:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	default:
		throw std::invalid_argument("Unsupported cull mode.");
	}
}

PrimitiveTopology LiteFX::Rendering::Backends::DX12::getPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY& topology)
{
	switch (topology)
	{
	case D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		return PrimitiveTopology::LineStrip;
	case D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		return PrimitiveTopology::LineStrip;
	case D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		return PrimitiveTopology::TriangleList;
	case D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		return PrimitiveTopology::TriangleStrip;
	case D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
		return PrimitiveTopology::PointList;
	default:
		throw std::invalid_argument("Unsupported primitive topology.");
	}
}

D3D12_PRIMITIVE_TOPOLOGY LiteFX::Rendering::Backends::DX12::getPrimitiveTopology(const PrimitiveTopology& topology)
{
	switch (topology)
	{
	case PrimitiveTopology::LineList:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case PrimitiveTopology::LineStrip:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case PrimitiveTopology::PointList:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case PrimitiveTopology::TriangleList:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case PrimitiveTopology::TriangleStrip:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default:
		throw std::invalid_argument("Unsupported primitive topology.");
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE LiteFX::Rendering::Backends::DX12::getPrimitiveTopologyType(const PrimitiveTopology& topology)
{
	switch (topology)
	{
	case PrimitiveTopology::PointList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case PrimitiveTopology::LineList:
	case PrimitiveTopology::LineStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case PrimitiveTopology::TriangleList:
	case PrimitiveTopology::TriangleStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default:
		throw std::invalid_argument("Unsupported primitive topology.");
	}
}

LPCTSTR LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getSemanticName(const AttributeSemantic& semantic)
{
	switch (semantic)
	{
	case LiteFX::Rendering::AttributeSemantic::Binormal: 
		return "BINORMAL";
	case LiteFX::Rendering::AttributeSemantic::BlendIndices: 
		return "BLENDINDICES";
	case LiteFX::Rendering::AttributeSemantic::BlendWeight: 
		return "BLENDWEIGHT";
	case LiteFX::Rendering::AttributeSemantic::Color: 
		return "COLOR";
	case LiteFX::Rendering::AttributeSemantic::Normal: 
		return "NORMAL";
	case LiteFX::Rendering::AttributeSemantic::Position: 
		return "POSITION";
	case LiteFX::Rendering::AttributeSemantic::TransformedPosition: 
		return "POSITIONT";
	case LiteFX::Rendering::AttributeSemantic::PointSize: 
		return "PSIZE";
	case LiteFX::Rendering::AttributeSemantic::Tangent: 
		return "TANGENT";
	case LiteFX::Rendering::AttributeSemantic::TextureCoordinate: 
		return "TEXCOORD";
	default:
		throw InvalidArgumentException("Unsupported semantic {0}.", static_cast<UInt32>(semantic));
	}
}

String LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getVendorName(const UInt32& vendorId)
{
	switch (vendorId)
	{
	case 0x10001: return "VIV";
	case 0x10002: return "VSI";
	case 0x10003: return "KAZAN";
	case 0x10004: return "CODEPLAY";
	case 0x10005: return "MESA";
	case 0x10006: return "POCL";
	case 0x1002: return "AMD";
	case 0x10DE: return "NVidia";
	case 0x8086: return "Intel";
	case 0x1010: return "ImgTec";
	case 0x13B5: return "ARM";
	case 0x5143: return "Qualcomm";
	default: return "Unknown";
	}
}

D3D12_COMPARISON_FUNC LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getCompareOp(const CompareOperation& compareOp)
{
	switch (compareOp) {
	case CompareOperation::Never: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
	case CompareOperation::Less: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	case CompareOperation::Greater: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
	case CompareOperation::Equal: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
	case CompareOperation::LessEqual: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CompareOperation::GreaterEqual: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CompareOperation::NotEqual: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CompareOperation::Always: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
	default: throw InvalidArgumentException("Unsupported compare operation.");
	}
}

D3D12_STENCIL_OP LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getStencilOp(const StencilOperation& stencilOp)
{
	switch (stencilOp) {
	case StencilOperation::Keep: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	case StencilOperation::Zero: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_ZERO;
	case StencilOperation::Invert: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INVERT;
	case StencilOperation::Replace: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
	case StencilOperation::IncrementClamp: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR_SAT;
	case StencilOperation::IncrementWrap: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR;
	case StencilOperation::DecrementClamp: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR_SAT;
	case StencilOperation::DecrementWrap: return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR;
	default: throw InvalidArgumentException("Unsupported stencil operation.");
	}
}

D3D12_BLEND LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getBlendFactor(const BlendFactor& blendFactor)
{
	switch (blendFactor) {
	case BlendFactor::Zero: return D3D12_BLEND_ZERO;
	case BlendFactor::One: return D3D12_BLEND_ONE;
	case BlendFactor::SourceColor: return D3D12_BLEND_SRC_COLOR;
	case BlendFactor::OneMinusSourceColor: return D3D12_BLEND_INV_SRC_COLOR;
	case BlendFactor::DestinationColor: return D3D12_BLEND_DEST_COLOR;
	case BlendFactor::OneMinusDestinationColor: return D3D12_BLEND_INV_DEST_COLOR;
	case BlendFactor::SourceAlpha: return D3D12_BLEND_SRC_ALPHA;
	case BlendFactor::OneMinusSourceAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
	case BlendFactor::DestinationAlpha: return D3D12_BLEND_DEST_ALPHA;
	case BlendFactor::OneMinusDestinationAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
	case BlendFactor::ConstantColor: return D3D12_BLEND_BLEND_FACTOR;
	case BlendFactor::OneMinusConstantColor: return D3D12_BLEND_INV_BLEND_FACTOR;
	case BlendFactor::ConstantAlpha: return D3D12_BLEND_BLEND_FACTOR;
	case BlendFactor::OneMinusConstantAlpha: return D3D12_BLEND_INV_BLEND_FACTOR;
	case BlendFactor::SourceAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
	case BlendFactor::Source1Color: return D3D12_BLEND_SRC1_COLOR;
	case BlendFactor::OneMinusSource1Color: return D3D12_BLEND_INV_SRC1_COLOR;
	case BlendFactor::Source1Alpha: return D3D12_BLEND_SRC1_ALPHA;
	case BlendFactor::OneMinusSource1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
	default: throw InvalidArgumentException("Unsupported blend factor.");
	}
}

D3D12_BLEND_OP LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getBlendOperation(const BlendOperation& blendOperation)
{
	switch (blendOperation) {
	case BlendOperation::Add: return D3D12_BLEND_OP_ADD;
	case BlendOperation::Subtract: return D3D12_BLEND_OP_SUBTRACT;
	case BlendOperation::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
	case BlendOperation::Minimum: return D3D12_BLEND_OP_MIN;
	case BlendOperation::Maximum: return D3D12_BLEND_OP_MAX;
	default: throw InvalidArgumentException("Unsupported blend operation.");
	}
}

D3D12_RESOURCE_STATES LITEFX_DIRECTX12_API LiteFX::Rendering::Backends::DX12::getResourceState(const ResourceState& resourceState)
{
	switch (resourceState) {
	case ResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
	case ResourceState::UniformBuffer:
	case ResourceState::VertexBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceState::IndexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case ResourceState::ReadOnly: return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
	case ResourceState::GenericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
	case ResourceState::ReadWrite: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case ResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceState::CopyDestination: return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
	case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
	case ResourceState::ResolveSource: return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	case ResourceState::ResolveDestination: return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	default: throw InvalidArgumentException("Unsupported resource state.");
	}
}