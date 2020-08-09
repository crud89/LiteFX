#pragma once

#include "graphics_api.hpp"
#include <fmt/format.h>

template <>
struct LITEFX_GRAPHICS_API fmt::formatter<LiteFX::Graphics::PrimitiveTopology> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Graphics::PrimitiveTopology t, FormatContext& ctx) {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Graphics::PrimitiveTopology::PointList: name = "PointList"; break;
		case LiteFX::Graphics::PrimitiveTopology::LineList: name = "LineList"; break;
		case LiteFX::Graphics::PrimitiveTopology::TriangleList: name = "TriangleList"; break;
		case LiteFX::Graphics::PrimitiveTopology::LineStrip: name = "LineStrip"; break;
		case LiteFX::Graphics::PrimitiveTopology::TriangleStrip: name = "TriangleStrip"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};