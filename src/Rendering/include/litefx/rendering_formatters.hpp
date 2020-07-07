#pragma once

#include "rendering_api.hpp"

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::GraphicsAdapterType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::GraphicsAdapterType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::GraphicsAdapterType::CPU: name = "CPU"; break;
		case LiteFX::Rendering::GraphicsAdapterType::GPU: name = "GPU"; break;
		case LiteFX::Rendering::GraphicsAdapterType::Other: name = "Other"; break;
		case LiteFX::Rendering::GraphicsAdapterType::None: name = "None"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::QueueType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::QueueType t, FormatContext& ctx) {
		Array<String> names;

		if (t == LiteFX::Rendering::QueueType::None)
			names.push_back("None");
		else if(t == LiteFX::Rendering::QueueType::Other)
			names.push_back("Other");
		else 
		{
			if ((t & LiteFX::Rendering::QueueType::Compute) == LiteFX::Rendering::QueueType::Compute)
				names.push_back("Compute");
			if ((t & LiteFX::Rendering::QueueType::Graphics) == LiteFX::Rendering::QueueType::Graphics)
				names.push_back("Graphics");
			if ((t & LiteFX::Rendering::QueueType::Transfer) == LiteFX::Rendering::QueueType::Transfer)
				names.push_back("Transfer");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::Format> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::Format t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::Format::B8G8R8A8_UNORM: name = "B8G8R8A8_UNORM"; break;
		case LiteFX::Rendering::Format::B8G8R8A8_UNORM_SRGB: name = "B8G8R8A8_UNORM_SRGB"; break;
		case LiteFX::Rendering::Format::None: name = "None"; break;
		case LiteFX::Rendering::Format::Other: name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferType t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::BufferType::Uniform: name = "Uniform"; break;
		case LiteFX::Rendering::BufferType::Storage: name = "Storage"; break;
		case LiteFX::Rendering::BufferType::Index:   name = "Index";   break;
		case LiteFX::Rendering::BufferType::Vertex:  name = "Vertex";  break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferUsage> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferUsage t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t)
		{
		case LiteFX::Rendering::BufferUsage::Staging:  name = "Staging";  break;
		case LiteFX::Rendering::BufferUsage::Resource: name = "Resource"; break;
		case LiteFX::Rendering::BufferUsage::Dynamic:  name = "Dynamic";  break;
		case LiteFX::Rendering::BufferUsage::Readback: name = "Readback"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::ShaderType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::ShaderType t, FormatContext& ctx) {
		Array<String> names;

		if (t == LiteFX::Rendering::ShaderType::Other)
			names.push_back("Other");
		else
		{
			if ((t & LiteFX::Rendering::ShaderType::Vertex) == LiteFX::Rendering::ShaderType::Vertex)
				names.push_back("Vertex");
			if ((t & LiteFX::Rendering::ShaderType::TessellationControl) == LiteFX::Rendering::ShaderType::TessellationControl)
				names.push_back("Tessellation Control");
			if ((t & LiteFX::Rendering::ShaderType::TessellationEvaluation) == LiteFX::Rendering::ShaderType::TessellationEvaluation)
				names.push_back("Tessellation Evaluation");
			if ((t & LiteFX::Rendering::ShaderType::Geometry) == LiteFX::Rendering::ShaderType::Geometry)
				names.push_back("Geometry");
			if ((t & LiteFX::Rendering::ShaderType::Fragment) == LiteFX::Rendering::ShaderType::Fragment)
				names.push_back("Fragment");
			if ((t & LiteFX::Rendering::ShaderType::Compute) == LiteFX::Rendering::ShaderType::Compute)
				names.push_back("Compute");
		}

		String name = Join(names, " | ");
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::BufferFormat> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::BufferFormat t, FormatContext& ctx) {
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
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::PolygonMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::PolygonMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::PolygonMode::Solid: name = "Solid"; break;
		case LiteFX::Rendering::PolygonMode::Wireframe: name = "Wireframe"; break;
		case LiteFX::Rendering::PolygonMode::Point: name = "Point"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::CullMode> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::CullMode t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::CullMode::FrontFaces: name = "FrontFaces"; break;
		case LiteFX::Rendering::CullMode::BackFaces: name = "BackFaces"; break;
		case LiteFX::Rendering::CullMode::Both: name = "Both"; break;
		case LiteFX::Rendering::CullMode::Disabled: name = "Disabled"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::CullOrder> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::CullOrder t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::CullOrder::ClockWise: name = "ClockWise"; break;
		case LiteFX::Rendering::CullOrder::CounterClockWise: name = "CounterClockWise"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::RenderTargetType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::RenderTargetType t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::RenderTargetType::Color: name = "Color"; break;
		case LiteFX::Rendering::RenderTargetType::Depth: name = "Depth"; break;
		case LiteFX::Rendering::RenderTargetType::Present: name = "Present"; break;
		case LiteFX::Rendering::RenderTargetType::Transfer: name = "Transfer"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_RENDERING_API fmt::formatter<LiteFX::Rendering::MultiSamplingLevel> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Rendering::MultiSamplingLevel t, FormatContext& ctx) {
		string_view name;

		switch (t)
		{
		case LiteFX::Rendering::MultiSamplingLevel::x1: name = "1"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x2: name = "2"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x4: name = "4"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x8: name = "8"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x16: name = "16"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x32: name = "32"; break;
		case LiteFX::Rendering::MultiSamplingLevel::x64: name = "64"; break;
		default: name = "Invalid"; break;
		}

		return formatter<string_view>::format(name, ctx);
	}
};