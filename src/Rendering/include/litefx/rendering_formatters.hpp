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
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::QueueType::Compute: name = "Compute"; break;
		case LiteFX::Rendering::QueueType::Graphics: name = "Graphics"; break;
		case LiteFX::Rendering::QueueType::Transfer: name = "Transfer"; break;
		case LiteFX::Rendering::QueueType::Other: name = "Other"; break;
		case LiteFX::Rendering::QueueType::None: name = "None"; break;
		}
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
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Rendering::ShaderType::Vertex: name = "Vertex"; break;
		case LiteFX::Rendering::ShaderType::TessellationControl: name = "Tessellation Control"; break;
		case LiteFX::Rendering::ShaderType::TessellationEvaluation: name = "Tessellation Evaluation"; break;
		case LiteFX::Rendering::ShaderType::Geometry: name = "Geometry"; break;
		case LiteFX::Rendering::ShaderType::Fragment: name = "Fragment"; break;
		case LiteFX::Rendering::ShaderType::Compute: name = "Compute"; break;
		case LiteFX::Rendering::ShaderType::Other: name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};