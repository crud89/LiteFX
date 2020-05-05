#pragma once

#include <litefx/app_api.hpp>
#include <litefx/app_formatters.hpp>

namespace LiteFX {
	using namespace LiteFX::Logging;

	class IBackend {
	public:
		virtual ~IBackend() noexcept = default;

	public:
		virtual BackendType getType() const noexcept = 0;
	};

	class LITEFX_APPMODEL_API App {
		LITEFX_IMPLEMENTATION(AppImpl);
		LITEFX_BUILDER(AppBuilder);

	public:
		App();
		App(const App&) = delete;
		App(App&&) = delete;
		virtual ~App() noexcept;

	public:
		virtual String getName() const noexcept = 0;
		virtual AppVersion getVersion() const noexcept = 0;
		Platform getPlatform() const noexcept;
		virtual const IBackend* findBackend(const BackendType& type) const;
		virtual const IBackend* operator[](const BackendType& type) const;

	public:
		virtual void use(UniquePtr<IBackend>&& backend);
		virtual void run() = 0;

	public:
		template <typename TApp, typename ...TArgs>
		static AppBuilder build(TArgs&&... _args) { 
			return AppBuilder(makeUnique<TApp>(std::forward<TArgs>(_args)...)); 
		}
	};

	class LITEFX_APPMODEL_API AppBuilder : public Builder<AppBuilder, App> {
	public:
		using builder_type::Builder;

	protected:
		virtual const IBackend* findBackend(const BackendType& type) const noexcept;

	public:
		void use(UniquePtr<IBackend>&& backend);
		virtual UniquePtr<App> go() override;

		template <typename TSink, std::enable_if_t<std::is_convertible_v<TSink*, ISink*>, int> = 0, typename ...TArgs>
		AppBuilder& logTo(TArgs&&... args) {
			auto sink = makeUnique<TSink>(std::forward<TArgs>(args)...);
			Logger::sinkTo(sink.get());
			return *this;
		}
	};

	class LITEFX_APPMODEL_API AppVersion {
		LITEFX_IMPLEMENTATION(AppVersionImpl);

	public:
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0) noexcept;
		AppVersion(const AppVersion&) = delete;
		AppVersion(AppVersion&&) = delete;
		virtual ~AppVersion() noexcept;

	public:
		int getMajor() const noexcept;
		int getMinor() const noexcept;
		int getPatch() const noexcept;
		int getRevision() const noexcept;
		int getEngineMajor() const noexcept;
		int getEngineMinor() const noexcept;
		int getEngineRevision() const noexcept;
		int getEngineStatus() const noexcept;
		String getEngineIdentifier() const noexcept;
		String getEngineVersion() const noexcept;
	};

}

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
			format_to(ctx.out(), "{} Version {}", app.getEngineIdentifier(), app.getEngineVersion()) :
			format_to(ctx.out(), "{}.{}.{}.{}", app.getMajor(), app.getMinor(), app.getPatch(), app.getRevision());
	}
};