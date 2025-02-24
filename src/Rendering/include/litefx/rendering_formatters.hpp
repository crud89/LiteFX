#pragma once

#include "rendering_api.hpp"

using namespace LiteFX::Rendering;

template <>
struct LITEFX_RENDERING_API std::formatter<GraphicsAdapterType> : std::formatter<std::string_view> {
	auto format(GraphicsAdapterType t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum GraphicsAdapterType;
		case CPU: name = "CPU"; break;
		case GPU: name = "GPU"; break;
		case Other: name = "Other"; break;
		case None: name = "None"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<QueueType> : std::formatter<std::string_view> {
	auto format(QueueType t, std::format_context& ctx) const {
		Array<String> names;

		if (t == QueueType::None)
			names.emplace_back("None");
		else if(LITEFX_FLAG_IS_SET(t, QueueType::Other))
			names.emplace_back("Other");
		else 
		{
			if ((t & QueueType::Compute) == QueueType::Compute)
				names.emplace_back("Compute");
			if ((t & QueueType::Graphics) == QueueType::Graphics)
				names.emplace_back("Graphics");
			if ((t & QueueType::Transfer) == QueueType::Transfer)
				names.emplace_back("Transfer");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<QueuePriority> : std::formatter<std::string_view> {
	auto format(QueuePriority t, std::format_context& ctx) const {
		String name;

		switch (t) {
		using enum QueuePriority;
		default:
		case Normal: name = "Normal"; break;
		case High: name = "High"; break;
		case Realtime: name = "Realtime"; break;
		}
		
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<Format> : std::formatter<std::string_view> {
	auto format(Format t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum Format;
		case R4G4_UNORM: name = "R4G4_UNORM"; break;
		case R4G4B4A4_UNORM: name = "R4G4B4A4_UNORM"; break;
		case B4G4R4A4_UNORM: name = "B4G4R4A4_UNORM"; break;
		case R5G6B5_UNORM: name = "R5G6B5_UNORM"; break;
		case B5G6R5_UNORM: name = "B5G6R5_UNORM"; break;
		case R5G5B5A1_UNORM: name = "R5G5B5A1_UNORM"; break;
		case B5G5R5A1_UNORM: name = "B5G5R5A1_UNORM"; break;
		case A1R5G5B5_UNORM: name = "A1R5G5B5_UNORM"; break;
		case R8_UNORM: name = "R8_UNORM"; break;
		case R8_SNORM: name = "R8_SNORM"; break;
		case R8_USCALED: name = "R8_USCALED"; break;
		case R8_SSCALED: name = "R8_SSCALED"; break;
		case R8_UINT: name = "R8_UINT"; break;
		case R8_SINT: name = "R8_SINT"; break;
		case R8_SRGB: name = "R8_SRGB"; break;
		case R8G8_UNORM: name = "R8G8_UNORM"; break;
		case R8G8_SNORM: name = "R8G8_SNORM"; break;
		case R8G8_USCALED: name = "R8G8_USCALED"; break;
		case R8G8_SSCALED: name = "R8G8_SSCALED"; break;
		case R8G8_UINT: name = "R8G8_UINT"; break;
		case R8G8_SINT: name = "R8G8_SINT"; break;
		case R8G8_SRGB: name = "R8G8_SRGB"; break;
		case R8G8B8_UNORM: name = "R8G8B8_UNORM"; break;
		case R8G8B8_SNORM: name = "R8G8B8_SNORM"; break;
		case R8G8B8_USCALED: name = "R8G8B8_USCALED"; break;
		case R8G8B8_SSCALED: name = "R8G8B8_SSCALED"; break;
		case R8G8B8_UINT: name = "R8G8B8_UINT"; break;
		case R8G8B8_SINT: name = "R8G8B8_SINT"; break;
		case R8G8B8_SRGB: name = "R8G8B8_SRGB"; break;
		case B8G8R8_UNORM: name = "B8G8R8_UNORM"; break;
		case B8G8R8_SNORM: name = "B8G8R8_SNORM"; break;
		case B8G8R8_USCALED: name = "B8G8R8_USCALED"; break;
		case B8G8R8_SSCALED: name = "B8G8R8_SSCALED"; break;
		case B8G8R8_UINT: name = "B8G8R8_UINT"; break;
		case B8G8R8_SINT: name = "B8G8R8_SINT"; break;
		case B8G8R8_SRGB: name = "B8G8R8_SRGB"; break;
		case R8G8B8A8_UNORM: name = "R8G8B8A8_UNORM"; break;
		case R8G8B8A8_SNORM: name = "R8G8B8A8_SNORM"; break;
		case R8G8B8A8_USCALED: name = "R8G8B8A8_USCALED"; break;
		case R8G8B8A8_SSCALED: name = "R8G8B8A8_SSCALED"; break;
		case R8G8B8A8_UINT: name = "R8G8B8A8_UINT"; break;
		case R8G8B8A8_SINT: name = "R8G8B8A8_SINT"; break;
		case R8G8B8A8_SRGB: name = "R8G8B8A8_SRGB"; break;
		case B8G8R8A8_UNORM: name = "B8G8R8A8_UNORM"; break;
		case B8G8R8A8_SNORM: name = "B8G8R8A8_SNORM"; break;
		case B8G8R8A8_USCALED: name = "B8G8R8A8_USCALED"; break;
		case B8G8R8A8_SSCALED: name = "B8G8R8A8_SSCALED"; break;
		case B8G8R8A8_UINT: name = "B8G8R8A8_UINT"; break;
		case B8G8R8A8_SINT: name = "B8G8R8A8_SINT"; break;
		case B8G8R8A8_SRGB: name = "B8G8R8A8_SRGB"; break;
		case A8B8G8R8_UNORM: name = "A8B8G8R8_UNORM"; break;
		case A8B8G8R8_SNORM: name = "A8B8G8R8_SNORM"; break;
		case A8B8G8R8_USCALED: name = "A8B8G8R8_USCALED"; break;
		case A8B8G8R8_SSCALED: name = "A8B8G8R8_SSCALED"; break;
		case A8B8G8R8_UINT: name = "A8B8G8R8_UINT"; break;
		case A8B8G8R8_SINT: name = "A8B8G8R8_SINT"; break;
		case A8B8G8R8_SRGB: name = "A8B8G8R8_SRGB"; break;
		case A2R10G10B10_UNORM: name = "A2R10G10B10_UNORM"; break;
		case A2R10G10B10_SNORM: name = "A2R10G10B10_SNORM"; break;
		case A2R10G10B10_USCALED: name = "A2R10G10B10_USCALED"; break;
		case A2R10G10B10_SSCALED: name = "A2R10G10B10_SSCALED"; break;
		case A2R10G10B10_UINT: name = "A2R10G10B10_UINT"; break;
		case A2R10G10B10_SINT: name = "A2R10G10B10_SINT"; break;
		case A2B10G10R10_UNORM: name = "A2B10G10R10_UNORM"; break;
		case A2B10G10R10_SNORM: name = "A2B10G10R10_SNORM"; break;
		case A2B10G10R10_USCALED: name = "A2B10G10R10_USCALED"; break;
		case A2B10G10R10_SSCALED: name = "A2B10G10R10_SSCALED"; break;
		case A2B10G10R10_UINT: name = "A2B10G10R10_UINT"; break;
		case A2B10G10R10_SINT: name = "A2B10G10R10_SINT"; break;
		case R16_UNORM: name = "R16_UNORM"; break;
		case R16_SNORM: name = "R16_SNORM"; break;
		case R16_USCALED: name = "R16_USCALED"; break;
		case R16_SSCALED: name = "R16_SSCALED"; break;
		case R16_UINT: name = "R16_UINT"; break;
		case R16_SINT: name = "R16_SINT"; break;
		case R16_SFLOAT: name = "R16_SFLOAT"; break;
		case R16G16_UNORM: name = "R16G16_UNORM"; break;
		case R16G16_SNORM: name = "R16G16_SNORM"; break;
		case R16G16_USCALED: name = "R16G16_USCALED"; break;
		case R16G16_SSCALED: name = "R16G16_SSCALED"; break;
		case R16G16_UINT: name = "R16G16_UINT"; break;
		case R16G16_SINT: name = "R16G16_SINT"; break;
		case R16G16_SFLOAT: name = "R16G16_SFLOAT"; break;
		case R16G16B16_UNORM: name = "R16G16B16_UNORM"; break;
		case R16G16B16_SNORM: name = "R16G16B16_SNORM"; break;
		case R16G16B16_USCALED: name = "R16G16B16_USCALED"; break;
		case R16G16B16_SSCALED: name = "R16G16B16_SSCALED"; break;
		case R16G16B16_UINT: name = "R16G16B16_UINT"; break;
		case R16G16B16_SINT: name = "R16G16B16_SINT"; break;
		case R16G16B16_SFLOAT: name = "R16G16B16_SFLOAT"; break;
		case R16G16B16A16_UNORM: name = "R16G16B16A16_UNORM"; break;
		case R16G16B16A16_SNORM: name = "R16G16B16A16_SNORM"; break;
		case R16G16B16A16_USCALED: name = "R16G16B16A16_USCALED"; break;
		case R16G16B16A16_SSCALED: name = "R16G16B16A16_SSCALED"; break;
		case R16G16B16A16_UINT: name = "R16G16B16A16_UINT"; break;
		case R16G16B16A16_SINT: name = "R16G16B16A16_SINT"; break;
		case R16G16B16A16_SFLOAT: name = "R16G16B16A16_SFLOAT"; break;
		case R32_UINT: name = "R32_UINT"; break;
		case R32_SINT: name = "R32_SINT"; break;
		case R32_SFLOAT: name = "R32_SFLOAT"; break;
		case R32G32_UINT: name = "R32G32_UINT"; break;
		case R32G32_SINT: name = "R32G32_SINT"; break;
		case R32G32_SFLOAT: name = "R32G32_SFLOAT"; break;
		case R32G32B32_UINT: name = "R32G32B32_UINT"; break;
		case R32G32B32_SINT: name = "R32G32B32_SINT"; break;
		case R32G32B32_SFLOAT: name = "R32G32B32_SFLOAT"; break;
		case R32G32B32A32_UINT: name = "R32G32B32A32_UINT"; break;
		case R32G32B32A32_SINT: name = "R32G32B32A32_SINT"; break;
		case R32G32B32A32_SFLOAT: name = "R32G32B32A32_SFLOAT"; break;
		case R64_UINT: name = "R64_UINT"; break;
		case R64_SINT: name = "R64_SINT"; break;
		case R64_SFLOAT: name = "R64_SFLOAT"; break;
		case R64G64_UINT: name = "R64G64_UINT"; break;
		case R64G64_SINT: name = "R64G64_SINT"; break;
		case R64G64_SFLOAT: name = "R64G64_SFLOAT"; break;
		case R64G64B64_UINT: name = "R64G64B64_UINT"; break;
		case R64G64B64_SINT: name = "R64G64B64_SINT"; break;
		case R64G64B64_SFLOAT: name = "R64G64B64_SFLOAT"; break;
		case R64G64B64A64_UINT: name = "R64G64B64A64_UINT"; break;
		case R64G64B64A64_SINT: name = "R64G64B64A64_SINT"; break;
		case R64G64B64A64_SFLOAT: name = "R64G64B64A64_SFLOAT"; break;
		case B10G11R11_UFLOAT: name = "B10G11R11_UFLOAT"; break;
		case E5B9G9R9_UFLOAT: name = "E5B9G9R9_UFLOAT"; break;
		case D16_UNORM: name = "D16_UNORM"; break;
		case X8_D24_UNORM: name = "X8_D24_UNORM"; break;
		case D32_SFLOAT: name = "D32_SFLOAT"; break;
		case S8_UINT: name = "S8_UINT"; break;
		case D16_UNORM_S8_UINT: name = "D16_UNORM_S8_UINT"; break;
		case D24_UNORM_S8_UINT: name = "D24_UNORM_S8_UINT"; break;
		case D32_SFLOAT_S8_UINT: name = "D32_SFLOAT_S8_UINT"; break;
		case BC1_RGB_UNORM: name = "BC1_RGB_UNORM"; break;
		case BC1_RGB_SRGB: name = "BC1_RGB_SRGB"; break;
		case BC1_RGBA_UNORM: name = "BC1_RGBA_UNORM"; break;
		case BC1_RGBA_SRGB: name = "BC1_RGBA_SRGB"; break;
		case BC2_UNORM: name = "BC2_UNORM"; break;
		case BC2_SRGB: name = "BC2_SRGB"; break;
		case BC3_UNORM: name = "BC3_UNORM"; break;
		case BC3_SRGB: name = "BC3_SRGB"; break;
		case BC4_UNORM: name = "BC4_UNORM"; break;
		case BC4_SNORM: name = "BC4_SNORM"; break;
		case BC5_UNORM: name = "BC5_UNORM"; break;
		case BC5_SNORM: name = "BC5_SNORM"; break;
		case BC6H_UFLOAT: name = "BC6H_UFLOAT"; break;
		case BC6H_SFLOAT: name = "BC6H_SFLOAT"; break;
		case BC7_UNORM: name = "BC7_UNORM"; break;
		case BC7_SRGB: name = "BC7_SRGB"; break;
		case None: name = "None"; break;
		case Other: name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<DescriptorType> : std::formatter<std::string_view> {
	auto format(DescriptorType t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum DescriptorType;
		case Sampler: name = "Sampler"; break;
		case ConstantBuffer: name = "ContantBuffer"; break;
		case StructuredBuffer: name = "StructuredBuffer"; break;
		case RWStructuredBuffer: name = "RWStructuredBuffer"; break;
		case Texture: name = "Texture"; break;
		case RWTexture: name = "RWTexture"; break;
		case Buffer: name = "Buffer"; break;
		case RWBuffer: name = "RWBuffer"; break;
		case ByteAddressBuffer: name = "ByteAddressBuffer"; break;
		case RWByteAddressBuffer: name = "RWByteAddressBuffer"; break;
		case InputAttachment: name = "Input Attachment"; break;
		case AccelerationStructure: name = "Acceleration Structure"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<BufferType> : std::formatter<std::string_view> {
	auto format(BufferType t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum BufferType;
		case Index:                 name = "Index"; break;
		case Vertex:                name = "Vertex"; break;
		case Uniform:               name = "Uniform"; break;
		case Storage:               name = "Storage"; break;
		case Texel:                 name = "Texel"; break;
		case AccelerationStructure: name = "Acceleration Structure"; break;
		case ShaderBindingTable:    name = "Shader Binding Table"; break;
        case Indirect:              name = "Indirect"; break;
		case Other:                 name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<ResourceUsage> : std::formatter<std::string_view> {
	auto format(ResourceUsage t, std::format_context& ctx) const {
		Array<String> names;

		if (t == ResourceUsage::None)
			names.emplace_back("None");
		else if (t == ResourceUsage::Default)
			names.emplace_back("Default");
		else
		{
			if ((t & ResourceUsage::AllowWrite) == ResourceUsage::AllowWrite)
				names.emplace_back("AllowWrite");
			if ((t & ResourceUsage::TransferSource) == ResourceUsage::TransferSource)
				names.emplace_back("TransferSource");
			if ((t & ResourceUsage::TransferDestination) == ResourceUsage::TransferDestination)
				names.emplace_back("TransferDestination");
			if ((t & ResourceUsage::AccelerationStructureBuildInput) == ResourceUsage::AccelerationStructureBuildInput)
				names.emplace_back("AccelerationStructureBuildInput");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<ResourceHeap> : std::formatter<std::string_view> {
	auto format(ResourceHeap t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum ResourceHeap;
		case Staging:  name = "Staging";  break;
		case Resource: name = "Resource"; break;
		case Dynamic:  name = "Dynamic";  break;
		case Readback: name = "Readback"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<IndexType> : std::formatter<std::string_view> {
	auto format(IndexType t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum IndexType;
		case UInt16: name = "UInt16"; break;
		case UInt32: name = "UInt32"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<LiteFX::Rendering::PrimitiveTopology> : std::formatter<std::string_view> {
	auto format(LiteFX::Rendering::PrimitiveTopology t, std::format_context& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		using enum PrimitiveTopology;
		case PointList: name = "PointList"; break;
		case LineList: name = "LineList"; break;
		case TriangleList: name = "TriangleList"; break;
		case LineStrip: name = "LineStrip"; break;
		case TriangleStrip: name = "TriangleStrip"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<ShaderStage> : std::formatter<std::string_view> {
	auto format(ShaderStage t, std::format_context& ctx) const {
		Array<String> names;

		if (t == ShaderStage::Other)
			names.emplace_back("Other");
		else if (t == ShaderStage::MeshPipeline)
			names.emplace_back("Mesh Shading");
		if (t == ShaderStage::RayTracingPipeline)
			names.emplace_back("Ray Tracing");
		else
		{
			if ((t & ShaderStage::Vertex) == ShaderStage::Vertex)
				names.emplace_back("Vertex");
			if ((t & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
				names.emplace_back("Tessellation Control");
			if ((t & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
				names.emplace_back("Tessellation Evaluation");
			if ((t & ShaderStage::Geometry) == ShaderStage::Geometry)
				names.emplace_back("Geometry");
			if ((t & ShaderStage::Fragment) == ShaderStage::Fragment)
				names.emplace_back("Fragment");
			if ((t & ShaderStage::Compute) == ShaderStage::Compute)
				names.emplace_back("Compute");
			if ((t & ShaderStage::Mesh) == ShaderStage::Mesh)
				names.emplace_back("Mesh");
			if ((t & ShaderStage::Task) == ShaderStage::Task)
				names.emplace_back("Task");
			if ((t & ShaderStage::RayGeneration) == ShaderStage::RayGeneration)
				names.emplace_back("Ray Generation");
			if ((t & ShaderStage::AnyHit) == ShaderStage::AnyHit)
				names.emplace_back("Any Hit");
			if ((t & ShaderStage::ClosestHit) == ShaderStage::ClosestHit)
				names.emplace_back("Closest Hit");
			if ((t & ShaderStage::Intersection) == ShaderStage::Intersection)
				names.emplace_back("Intersection");
			if ((t & ShaderStage::Miss) == ShaderStage::Miss)
				names.emplace_back("Miss");
			if ((t & ShaderStage::Callable) == ShaderStage::Callable)
				names.emplace_back("Callable");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<BufferFormat> : std::formatter<std::string_view> {
	auto format(BufferFormat t, std::format_context& ctx) const {
		Array<String> names;

		// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
		switch (::getBufferFormatChannels(t))
		{
		case 1:
			names.emplace_back("X");
			break;
		case 2:
			names.emplace_back("XY");
			break;
		case 3:
			names.emplace_back("XYZ");
			break;
		case 4:
			names.emplace_back("XYZW");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatChannelSize(t))
		{
		case 8:
			names.emplace_back("8");
			break;
		case 16:
			names.emplace_back("16");
			break;
		case 32:
			names.emplace_back("32");
			break;
		case 64:
			names.emplace_back("64");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatType(t))
		{
		case 0x01:
			names.emplace_back("F");
			break;
		case 0x02:
			names.emplace_back("I");
			break;
		case 0x04:
			names.emplace_back("S");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}
		// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

		String name = Join(names);
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<PolygonMode> : std::formatter<std::string_view> {
	auto format(PolygonMode t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum PolygonMode;
		case Solid: name = "Solid"; break;
		case Wireframe: name = "Wireframe"; break;
		case Point: name = "Point"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<CullMode> : std::formatter<std::string_view> {
	auto format(CullMode t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum CullMode;
		case FrontFaces: name = "FrontFaces"; break;
		case BackFaces: name = "BackFaces"; break;
		case Both: name = "Both"; break;
		case Disabled: name = "Disabled"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<CullOrder> : std::formatter<std::string_view> {
	auto format(CullOrder t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum CullOrder;
		case ClockWise: name = "ClockWise"; break;
		case CounterClockWise: name = "CounterClockWise"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<RenderTargetType> : std::formatter<std::string_view> {
	auto format(RenderTargetType t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum RenderTargetType;
		case Color: name = "Color"; break;
		case DepthStencil: name = "DepthStencil"; break;
		case Present: name = "Present"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<MultiSamplingLevel> : std::formatter<std::string_view> {
	auto format(MultiSamplingLevel t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum MultiSamplingLevel;
		case x1: name = "1"; break;
		case x2: name = "2"; break;
		case x4: name = "4"; break;
		case x8: name = "8"; break;
		case x16: name = "16"; break;
		case x32: name = "32"; break;
		case x64: name = "64"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<FilterMode> : std::formatter<std::string_view> {
	auto format(FilterMode t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum FilterMode;
		case Nearest: name = "Nearest"; break;
		case Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<MipMapMode> : std::formatter<std::string_view> {
	auto format(MipMapMode t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum MipMapMode;
		case Nearest: name = "Nearest"; break;
		case Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<BorderMode> : std::formatter<std::string_view> {
	auto format(BorderMode t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum BorderMode;
		case Repeat: name = "Repeat"; break;
		case ClampToEdge: name = "ClampToEdge"; break;
		case ClampToBorder: name = "ClampToBorder"; break;
		case RepeatMirrored: name = "RepeatMirrored"; break;
		case ClampToEdgeMirrored: name = "ClampToEdgeMirrored"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<AttributeSemantic> : std::formatter<std::string_view> {
	auto format(AttributeSemantic t, std::format_context& ctx) const {
		string_view name;

		switch (t) {
		using enum AttributeSemantic;
		case Binormal: name = "Binormal"; break;
		case BlendIndices: name = "BlendIndices"; break;
		case BlendWeight: name = "BlendWeight"; break;
		case Color: name = "Color"; break;
		case Normal: name = "Normal"; break;
		case Position: name = "Position"; break;
		case TransformedPosition: name = "TransformedPosition"; break;
		case PointSize: name = "PointSize"; break;
		case Tangent: name = "Tangent"; break;
		case TextureCoordinate: name = "TextureCoordinate"; break;
		default: name = "Unknown"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<GeometryFlags> : std::formatter<std::string_view> {
	auto format(GeometryFlags t, std::format_context& ctx) const {
		Array<String> names;

		if (t == GeometryFlags::None)
			names.emplace_back("None");
		else
		{
			if ((t & GeometryFlags::Opaque) == GeometryFlags::Opaque)
				names.emplace_back("Opaque");
			if ((t & GeometryFlags::OneShotAnyHit) == GeometryFlags::OneShotAnyHit)
				names.emplace_back("OneShotAnyHit");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<AccelerationStructureFlags> : std::formatter<std::string_view> {
	auto format(AccelerationStructureFlags t, std::format_context& ctx) const {
		Array<String> names;

		if (t == AccelerationStructureFlags::None)
			names.emplace_back("None");
		else
		{
			if ((t & AccelerationStructureFlags::AllowUpdate) == AccelerationStructureFlags::AllowUpdate)
				names.emplace_back("AllowUpdate");
			if ((t & AccelerationStructureFlags::AllowCompaction) == AccelerationStructureFlags::AllowCompaction)
				names.emplace_back("AllowCompaction");
			if ((t & AccelerationStructureFlags::PreferFastTrace) == AccelerationStructureFlags::AllowCompaction)
				names.emplace_back("PreferFastTrace");
			if ((t & AccelerationStructureFlags::PreferFastBuild) == AccelerationStructureFlags::PreferFastBuild)
				names.emplace_back("PreferFastBuild");
			if ((t & AccelerationStructureFlags::MinimizeMemory) == AccelerationStructureFlags::MinimizeMemory)
				names.emplace_back("MinimizeMemory");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API std::formatter<InstanceFlags> : std::formatter<std::string_view> {
	auto format(InstanceFlags t, std::format_context& ctx) const {
		Array<String> names;

		if (t == InstanceFlags::None)
			names.emplace_back("None");
		else
		{
			if ((t & InstanceFlags::DisableCull) == InstanceFlags::DisableCull)
				names.emplace_back("DisableCull");
			if ((t & InstanceFlags::FlipWinding) == InstanceFlags::FlipWinding)
				names.emplace_back("FlipWinding");
			if ((t & InstanceFlags::ForceOpaque) == InstanceFlags::ForceOpaque)
				names.emplace_back("ForceOpaque");
			if ((t & InstanceFlags::ForceNonOpaque) == InstanceFlags::ForceNonOpaque)
				names.emplace_back("ForceNonOpaque");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};
