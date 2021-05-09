#pragma once

#include "rendering_api.hpp"

using namespace LiteFX;

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::GraphicsAdapterType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::GraphicsAdapterType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::GraphicsAdapterType;
		case CPU: name = "CPU"; break;
		case GPU: name = "GPU"; break;
		case Other: name = "Other"; break;
		case None: name = "None"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::QueueType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::QueueType t, FormatContext& ctx) {
		Array<String> names;

		if (t == Rendering::QueueType::None)
			names.push_back("None");
		else if(t == Rendering::QueueType::Other)
			names.push_back("Other");
		else 
		{
			if ((t & Rendering::QueueType::Compute) == Rendering::QueueType::Compute)
				names.push_back("Compute");
			if ((t & Rendering::QueueType::Graphics) == Rendering::QueueType::Graphics)
				names.push_back("Graphics");
			if ((t & Rendering::QueueType::Transfer) == Rendering::QueueType::Transfer)
				names.push_back("Transfer");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::QueuePriority> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::QueuePriority t, FormatContext& ctx) {
		String name;

		switch (t) {
		using enum Rendering::QueuePriority;
		default:
		case Normal: name = "Normal"; break;
		case High: name = "High"; break;
		case Realtime: name = "Realtime"; break;
		}
		
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::Format> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::Format t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::Format;
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
struct LITEFX_RENDERING_API fmt::formatter<Rendering::DescriptorType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::DescriptorType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::DescriptorType;
		case Sampler: name = "Sampler"; break;
		case Storage: name = "Uniform"; break;
		case Uniform: name = "Storage"; break;
		case Image:   name = "Image"; break;
		case InputAttachment: name = "Input Attachment"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::BufferType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::BufferType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::BufferType;
		case Index:      name = "Index";      break;
		case Vertex:     name = "Vertex";     break;
		case Uniform:    name = "Uniform"; break;
		case Storage:    name = "Storage"; break;
		case Other:      name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::BufferUsage> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::BufferUsage t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::BufferUsage;
		case Staging:  name = "Staging";  break;
		case Resource: name = "Resource"; break;
		case Dynamic:  name = "Dynamic";  break;
		case Readback: name = "Readback"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::IndexType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::IndexType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		using enum Rendering::IndexType;
		case UInt16: name = "UInt16"; break;
		case UInt32: name = "UInt32"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::ShaderStage> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::ShaderStage t, FormatContext& ctx) {
		Array<String> names;

		if (t == Rendering::ShaderStage::Other)
			names.push_back("Other");
		else
		{
			if ((t & Rendering::ShaderStage::Vertex) == Rendering::ShaderStage::Vertex)
				names.push_back("Vertex");
			if ((t & Rendering::ShaderStage::TessellationControl) == Rendering::ShaderStage::TessellationControl)
				names.push_back("Tessellation Control");
			if ((t & Rendering::ShaderStage::TessellationEvaluation) == Rendering::ShaderStage::TessellationEvaluation)
				names.push_back("Tessellation Evaluation");
			if ((t & Rendering::ShaderStage::Geometry) == Rendering::ShaderStage::Geometry)
				names.push_back("Geometry");
			if ((t & Rendering::ShaderStage::Fragment) == Rendering::ShaderStage::Fragment)
				names.push_back("Fragment");
			if ((t & Rendering::ShaderStage::Compute) == Rendering::ShaderStage::Compute)
				names.push_back("Compute");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::BufferFormat> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::BufferFormat t, FormatContext& ctx) {
		Array<String> names;

		switch (::getBufferFormatChannels(t))
		{
		case 1:
			names.push_back("X");
			break;
		case 2:
			names.push_back("XY");
			break;
		case 3:
			names.push_back("XYZ");
			break;
		case 4:
			names.push_back("XYZW");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatChannelSize(t))
		{
		case 8:
			names.push_back("8");
			break;
		case 16:
			names.push_back("16");
			break;
		case 32:
			names.push_back("32");
			break;
		case 64:
			names.push_back("64");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		switch (::getBufferFormatType(t))
		{
		case 0x01:
			names.push_back("F");
			break;
		case 0x02:
			names.push_back("I");
			break;
		case 0x04:
			names.push_back("S");
			break;
		default:
			return formatter<string_view>::format("Invalid", ctx);
		}

		String name = Join(names);
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::PolygonMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::PolygonMode t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::PolygonMode;
		case Solid: name = "Solid"; break;
		case Wireframe: name = "Wireframe"; break;
		case Point: name = "Point"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::CullMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::CullMode t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::CullMode;
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
struct LITEFX_RENDERING_API fmt::formatter<Rendering::CullOrder> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::CullOrder t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::CullOrder;
		case ClockWise: name = "ClockWise"; break;
		case CounterClockWise: name = "CounterClockWise"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::RenderTargetType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::RenderTargetType t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::RenderTargetType;
		case Color: name = "Color"; break;
		case Depth: name = "Depth"; break;
		case Present: name = "Present"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::MultiSamplingLevel> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::MultiSamplingLevel t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::MultiSamplingLevel;
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
struct LITEFX_RENDERING_API fmt::formatter<Rendering::FilterMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::FilterMode t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::FilterMode;
		case Nearest: name = "Nearest"; break;
		case Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::MipMapMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::MipMapMode t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::MipMapMode;
		case Nearest: name = "Nearest"; break;
		case Linear: name = "Linear"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<Rendering::BorderMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::BorderMode t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::BorderMode;
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
struct LITEFX_RENDERING_API fmt::formatter<Rendering::AttributeSemantic> : formatter<string_view> {
	template <typename FormatContext>
	auto format(Rendering::AttributeSemantic t, FormatContext& ctx) {
		string_view name;

		switch (t) {
		using enum Rendering::AttributeSemantic;
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