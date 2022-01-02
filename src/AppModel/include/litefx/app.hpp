#pragma once

#include <litefx/app_api.hpp>
#include <litefx/app_formatters.hpp>

namespace LiteFX {
	using namespace LiteFX::Logging;

	class LITEFX_APPMODEL_API IBackend {
		friend class App;

	private:
		BackendState m_state = BackendState::Uninitialized;

	public:
		virtual ~IBackend() noexcept = default;

	public:
		virtual BackendType type() const noexcept = 0;
		const BackendState& state() const noexcept { return m_state; }

	private:
		BackendState& state() noexcept { return m_state; }
		std::type_index typeId() const noexcept { return typeid(*this); }
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
		virtual String name() const noexcept = 0;
		virtual AppVersion version() const noexcept = 0;
		Platform platform() const noexcept;
		virtual const IBackend* operator[](std::type_index type) const;

		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		const TBackend* findBackend() const {
			return dynamic_cast<const TBackend*>(this->operator[](typeid(TBackend)));
		}

		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void startBackend(const std::function<bool(const TBackend*)>& initializer) {
			const auto backend = this->findBackend<TBackend>();

			if (backend == nullptr)
				throw InvalidArgumentException("No backend of type {0} has been registered.", typeid(TBackend).name());

			if (!initializer(backend))
				throw RuntimeException("The backend of type {0} could not be initialized.", typeid(TBackend).name());
		}

	public:
		virtual void use(UniquePtr<IBackend>&& backend);
		virtual void run() = 0;
		virtual void initialize() = 0;

	public:
		virtual void resize(int width, int height);

	public:
		template <typename TApp, typename ...TArgs>
		static AppBuilder build(TArgs&&... _args) { 
			return AppBuilder(makeUnique<TApp>(std::forward<TArgs>(_args)...)); 
		}
	};

	class LITEFX_APPMODEL_API AppBuilder : public Builder<AppBuilder, App> {
	public:
		using builder_type::Builder;

	public:
		void use(UniquePtr<IBackend>&& backend);
		virtual UniquePtr<App> go() override;

		template <typename TSink, typename ...TArgs> requires
			std::convertible_to<TSink*, ISink*>
		AppBuilder& logTo(TArgs&&... args) {
			auto sink = makeUnique<TSink>(std::forward<TArgs>(args)...);
			Logger::sinkTo(sink.get());
			return *this;
		}

		template <typename TBackend, typename ...TArgs> requires
			rtti::implements<TBackend, IBackend>
		AppBuilder& useBackend(TArgs&&... args) {
			this->use(makeUnique<TBackend>(*this->instance(), std::forward<TArgs>(args)...));
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
		int major() const noexcept;
		int minor() const noexcept;
		int patch() const noexcept;
		int revision() const noexcept;
		int engineMajor() const noexcept;
		int engineMinor() const noexcept;
		int engineRevision() const noexcept;
		int engineStatus() const noexcept;
		String engineIdentifier() const noexcept;
		String engineVersion() const noexcept;
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
			format_to(ctx.out(), "{} Version {}", app.engineIdentifier(), app.engineVersion()) :
			format_to(ctx.out(), "{}.{}.{}.{}", app.major(), app.minor(), app.patch(), app.revision());
	}
};