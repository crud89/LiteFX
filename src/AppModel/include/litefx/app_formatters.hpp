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

template <>
struct fmt::formatter<LiteFX::AppVersion> {
	bool engineVersion = false;

	constexpr auto parse(format_parse_context& ctx) {
		auto it = ctx.begin(), end = ctx.end();

		if (it != end && (*it == 'e'))
		{
			engineVersion = true;
			it++;
		}

		if (it != end && *it != '}')
			throw format_error("Invalid version format: expected: `}`.");

		return it;
	}

	template <typename FormatContext>
	auto format(const LiteFX::AppVersion& app, FormatContext& ctx) {
		return engineVersion ?
			fmt::format_to(ctx.out(), "{} Version {}", app.engineIdentifier(), app.engineVersion()) :
			fmt::format_to(ctx.out(), "{}.{}.{}.{}", app.major(), app.minor(), app.patch(), app.revision());
	}
};