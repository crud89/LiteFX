#pragma once

#include <format>
#include "graphics_api.hpp"

using namespace LiteFX::Graphics;

template <>
struct LITEFX_GRAPHICS_API std::formatter<LiteFX::Graphics::PrimitiveTopology> : std::formatter<std::string_view> {
	auto format(LiteFX::Graphics::PrimitiveTopology t, std::format_context& ctx) const {
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