#pragma once

#include "graphics_api.hpp"
#include <fmt/format.h>

using namespace LiteFX::Graphics;

template <>
struct LITEFX_GRAPHICS_API fmt::formatter<LiteFX::Graphics::PrimitiveTopology> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Graphics::PrimitiveTopology t, FormatContext& ctx) {
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