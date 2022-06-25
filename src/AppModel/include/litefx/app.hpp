#pragma once

#include <litefx/app_api.hpp>
#include <litefx/app_formatters.hpp>

namespace LiteFX {
	using namespace LiteFX::Logging;

	/// <summary>
	/// The base interface for an app backend.
	/// </summary>
	class LITEFX_APPMODEL_API IBackend {
		friend class App;

	private:
		BackendState m_state = BackendState::Inactive;

	public:
		virtual ~IBackend() noexcept = default;

	public:
		virtual BackendType type() const noexcept = 0;
		const BackendState& state() const noexcept { return m_state; }

	protected:
		BackendState& state() noexcept { return m_state; }
		virtual void activate() = 0;
		virtual void deactivate() = 0;

	private:
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
		virtual const IBackend* getBackend(std::type_index type) const;
		virtual Array<const IBackend*> getBackends(const BackendType type) const noexcept;

	protected:
		virtual IBackend* getBackend(std::type_index type);

		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		TBackend* findBackend() {
			return dynamic_cast<TBackend*>(this->getBackend(typeid(TBackend)));
		}

		virtual void startBackend(std::type_index type, IBackend* backend) const;
		virtual void stopBackend(std::type_index type, IBackend* backend) const;
		virtual void stopActiveBackends(const BackendType& type) const;

	private:
		void registerStartCallback(std::type_index type, const std::function<bool()>& callback);
		void registerStopCallback(std::type_index type, const std::function<void()>& callback);

	public:
		/// <summary>
		/// Sets a callback that is called, if a backend is started.
		/// </summary>
		/// <remarks>
		/// A backend can have multiple start callbacks, that are executed if a backend is started. Typically such a callback is used to initialize
		/// a device and surface for an application. An application might use multiple callbacks, if it creates multiple devices, for example to
		/// create a plugin architecture.
		/// </remarks>
		/// <typeparam name="TBackend">The type of the backend.</typeparam>
		/// <param name="callback">The function to call during backend startup.</param>
		/// <seealso cref="onBackendStop" />
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void onBackendStart(const std::function<bool(TBackend*)>& callback) {
			this->registerStartCallback(typeid(TBackend), [this, callback]() {
				auto backend = this->findBackend<TBackend>();

				if (backend == nullptr)
					throw InvalidArgumentException("No backend of type {0} has been registered.", typeid(TBackend).name());

				if (backend->state() == BackendState::Active)
					return true;
				else
					return callback(backend);
			});
		}

		/// <summary>
		/// Sets a callback that is called, if a backend is stopped.
		/// </summary>
		/// <typeparam name="TBackend">The type of the backend.</typeparam>
		/// <param name="callback">The function to call during backend shutdown.</param>
		/// <seealso cref="onBackendStart" />
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void onBackendStop(const std::function<void(TBackend*)>& callback) {
			this->registerStopCallback(typeid(TBackend), [this, callback]() {
				auto backend = this->findBackend<TBackend>();

				if (backend == nullptr)
					throw InvalidArgumentException("No backend of type {0} has been registered.", typeid(TBackend).name());

				if (backend->state() != BackendState::Inactive)
					callback(backend);
			});
		}

	public:
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		const TBackend* findBackend() const {
			return dynamic_cast<const TBackend*>(this->getBackend(typeid(TBackend)));
		}

		/// <summary>
		/// Attempts to start a backend of type <typeparamref name="TBackend" /> and stops the active backend of the same <see cref="BackendType" />, if any.
		/// </summary>
		/// <typeparam name="TBackend">The type of the backend to start.</typeparam>
		/// <exception cref="InvalidArgumentException">Thrown, if no backend of type <typeparamref name="TBackend" /> is registered.</exception>
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void startBackend() {
			this->startBackend(typeid(TBackend), static_cast<IBackend*>(this->findBackend<TBackend>()));
		}

		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void stopBackend() {
			this->stopBackend(typeid(TBackend), static_cast<IBackend*>(this->findBackend<TBackend>()));
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
			fmt::format_to(ctx.out(), "{} Version {}", app.engineIdentifier(), app.engineVersion()) :
			fmt::format_to(ctx.out(), "{}.{}.{}.{}", app.major(), app.minor(), app.patch(), app.revision());
	}
};