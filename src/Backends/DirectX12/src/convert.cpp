#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

Format LiteFX::Rendering::Backends::getFormat(const DXGI_FORMAT& format)
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

DXGI_FORMAT LiteFX::Rendering::Backends::getFormat(const Format& format)
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

DXGI_FORMAT LiteFX::Rendering::Backends::getFormat(const BufferFormat& format)
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

PolygonMode LiteFX::Rendering::Backends::getPolygonMode(const D3D12_FILL_MODE& mode)
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

D3D12_FILL_MODE LiteFX::Rendering::Backends::getPolygonMode(const PolygonMode& mode)
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

CullMode LiteFX::Rendering::Backends::getCullMode(const D3D12_CULL_MODE& mode)
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

D3D12_CULL_MODE LiteFX::Rendering::Backends::getCullMode(const CullMode& mode)
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

PrimitiveTopology LiteFX::Rendering::Backends::getPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY& topology)
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

D3D12_PRIMITIVE_TOPOLOGY LiteFX::Rendering::Backends::getPrimitiveTopology(const PrimitiveTopology& topology)
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

D3D12_PRIMITIVE_TOPOLOGY_TYPE LiteFX::Rendering::Backends::getPrimitiveTopologyType(const PrimitiveTopology& topology)
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