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