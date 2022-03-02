#pragma once

#include <litefx/app_api.hpp>

template <>
struct LITEFX_APPMODEL_API fmt::formatter<LiteFX::Platform> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::Platform t, FormatContext& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::Platform::Win32: name = "Win32"; break;
		case LiteFX::Platform::Other: name = "Other"; break;
		case LiteFX::Platform::None: name = "None"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <>
struct LITEFX_APPMODEL_API fmt::formatter<LiteFX::BackendType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(LiteFX::BackendType t, FormatContext& ctx) const {
		string_view name = "Invalid";
		switch (t) {
		case LiteFX::BackendType::Rendering: name = "Rendering"; break;
		//case LiteFX::BackendType::Physics: name = "Physics"; break;
		case LiteFX::BackendType::Other: name = "Other"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};