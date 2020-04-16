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