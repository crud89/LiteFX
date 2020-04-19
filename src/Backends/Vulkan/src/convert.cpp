#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

Format LiteFX::Rendering::Backends::getFormat(const VkFormat& format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:
		return Format::B8G8R8A8_UNORM;
	case VK_FORMAT_B8G8R8A8_SRGB:
		return Format::B8G8R8A8_UNORM_SRGB;
	default:
		return Format::Other;
	}
}

VkFormat LiteFX::Rendering::Backends::getFormat(const Format& format)
{
	switch (format)
	{
	case Format::B8G8R8A8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Format::B8G8R8A8_UNORM_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	default:
		return VK_FORMAT_MAX_ENUM;
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
	default:
		return PolygonMode::Solid;
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
		return VkPolygonMode::VK_POLYGON_MODE_POINT;
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
	default:
		return CullMode::Disabled;
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
	default:
		return VkCullModeFlagBits::VK_CULL_MODE_NONE;
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
	default:
		return PrimitiveTopology::PointList;
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
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	}
}