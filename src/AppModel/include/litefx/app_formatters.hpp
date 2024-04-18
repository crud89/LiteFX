#pragma once

#include <litefx/app_api.hpp>

template <>
struct LITEFX_APPMODEL_API std::formatter<LiteFX::Platform> : std::formatter<std::string_view> {
	auto format(LiteFX::Platform t, std::format_context& ctx) const {
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
struct LITEFX_APPMODEL_API std::formatter<LiteFX::BackendType> : std::formatter<std::string_view> {
	auto format(LiteFX::BackendType t, std::format_context& ctx) const {
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
struct std::formatter<LiteFX::AppVersion> {
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

	auto format(const LiteFX::AppVersion& app, std::format_context& ctx) const {
		return engineVersion ?
			std::format_to(ctx.out(), "{} Version {}", app.engineIdentifier(), app.engineVersion()) :
			std::format_to(ctx.out(), "{}.{}.{}.{}", app.major(), app.minor(), app.patch(), app.revision());
	}
};