#pragma once

#include <litefx/app_api.hpp>
#include <litefx/app_formatters.hpp>

namespace LiteFX {
	using namespace LiteFX::Logging;

	/// <summary>
	/// The base class for an app backend.
	/// </summary>
	/// <remarks>
	/// An <see cref="App" /> can have one or more backends configured per <see cref="BackendType" />. However, only one backend of a specific backend
	/// type can be active at a time. Backends can be configured with a startup and shutdown callback. Starting a backend will automatically shutdown
	/// the active backend of the same type, if any. If the app exits, all backends are shutdown accordingly, allowing to properly cleanup any resources.
	/// </remarks>
	/// <seealso cref="App" />
	class LITEFX_APPMODEL_API IBackend {
		friend class App;

	private:
		BackendState m_state = BackendState::Inactive;

	public:
		virtual ~IBackend() noexcept = default;

	public:
		/// <summary>
		/// Gets the type of the backend.
		/// </summary>
		/// <returns>The type of the backend.</returns>
		/// <seealso cref="BackendType" />
		virtual BackendType type() const noexcept = 0;

		/// <summary>
		/// Returns the state of the backend.
		/// </summary>
		/// <returns>The state of the backend.</returns>
		/// <seealso cref="BackendState" />
		const BackendState& state() const noexcept { return m_state; }

		/// <summary>
		/// Gets the name of the backend.
		/// </summary>
		/// <returns>The name of the backend.</returns>
		virtual String name() const noexcept = 0;

	protected:
		/// <summary>
		/// Returns the state of the backend.
		/// </summary>
		/// <returns>The state of the backend.</returns>
		BackendState& state() noexcept { return m_state; }

		/// <summary>
		/// Called by the parent <see cref="App" />, if the backend is started.
		/// </summary>
		virtual void activate() = 0;

		/// <summary>
		/// Called by the parent <see cref="App" />, if the backend is stopped.
		/// </summary>
		virtual void deactivate() = 0;

	private:
		std::type_index typeId() const noexcept { return typeid(*this); }
	};

	/// <summary>
	/// The base class for an application.
	/// </summary>
	/// <seealso cref="AppBuilder" />
	/// <seealso cref="AppVersion" />
	class LITEFX_APPMODEL_API App {
		LITEFX_IMPLEMENTATION(AppImpl);
		LITEFX_BUILDER(AppBuilder);

	public:
		/// <summary>
		/// Initializes a new app instance.
		/// </summary>
		App();
		App(const App&) = delete;
		App(App&&) = delete;

		virtual ~App() noexcept;

	public:
		/// <summary>
		/// Returns the name of the app.
		/// </summary>
		/// <returns>The name of the app.</returns>
		virtual String name() const noexcept = 0;

		/// <summary>
		/// Returns the version of the app.
		/// </summary>
		/// <returns>The version of the app.</returns>
		virtual AppVersion version() const noexcept = 0;

		/// <summary>
		/// Returns the platform, the app is running on.
		/// </summary>
		/// <returns>The platform, the app is running on.</returns>
		Platform platform() const noexcept;

		/// <summary>
		/// Returns the registered backend instance for a type index.
		/// </summary>
		/// <param name="type">The type index of the requested backend.</param>
		/// <returns>The registered backend instance for a type index, or <c>nullptr</c>, if the app has no backend of the provided type.</returns>
		virtual const IBackend* operator[](std::type_index type) const;

		/// <summary>
		/// Returns the registered backend instance for a type index.
		/// </summary>
		/// <param name="type">The type index of the requested backend.</param>
		/// <returns>The registered backend instance for a type index, or <c>nullptr</c>, if the app has no backend of the provided type.</returns>
		virtual const IBackend* getBackend(std::type_index type) const;
		
		/// <summary>
		/// Returns all registered backend instances of a backend type.
		/// </summary>
		/// <param name="type">The backend type of the requested backends.</param>
		/// <returns>All registered instances of <paramref name="type" />.</returns>
		virtual Array<const IBackend*> getBackends(const BackendType type) const noexcept;

	protected:
		/// <summary>
		/// Returns the registered backend instance for a type index.
		/// </summary>
		/// <param name="type">The type index of the requested backend.</param>
		/// <returns>The registered backend instance for a type index, or <c>nullptr</c>, if the app has no backend of the provided type.</returns>
		virtual IBackend* getBackend(std::type_index type);

		/// <summary>
		/// Returns the registered backend instance for a type index.
		/// </summary>
		/// <typeparam name="TBackend">The type, the type index is derived from.</typeparam>
		/// <returns>The registered backend instance for a type index, or <c>nullptr</c>, if the app has no backend of the provided type.</returns>
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		TBackend* findBackend() {
			return dynamic_cast<TBackend*>(this->getBackend(typeid(TBackend)));
		}

		/// <summary>
		/// Starts a backend.
		/// </summary>
		/// <remarks>
		/// Starting a backend will first stop the active backend of the same <see cref="BackendType" />. It will then call all start callbacks for the 
		/// backend. If the backend has been started successfully, it calls <see cref="IBackend::activate" /> before returning.
		/// </remarks>
		/// <param name="type">The type index of the backend to start.</param>
		/// <seealso cref="Backend" />
		/// <seealso cref="onBackendStart" />
		virtual void startBackend(std::type_index type) const;

		/// <summary>
		/// Stops a backend.
		/// </summary>
		/// <remarks>
		/// This method calls all stop callbacks for the backend, before returing. If the backend is not running, calling this method has no effect.
		/// </remarks>
		/// <param name="type">The type index of the backend to start.</param>
		/// <seealso cref="Backend" />
		/// <seealso cref="onBackendStop" />
		virtual void stopBackend(std::type_index type) const;

		/// <summary>
		/// Stops the active backend of <paramref name="type" />.
		/// </summary>
		/// <param name="type">The backend type for which the active backend should be stopped.</param>
		/// <seealso cref="stopBackend" />
		virtual void stopActiveBackends(const BackendType& type) const;

		/// <summary>
		/// Returns the active backend of the provided backend <paramref name="type" />.
		/// </summary>
		/// <param name="type">The type of the backend.</param>
		/// <returns>The active backend of the provided backend type, or <c>std::nullptr</c>, if no backend is active.</returns>
		virtual IBackend* activeBackend(const BackendType& type) const;

		/// <summary>
		/// Returns the type index of the active backend of the provided backend <paramref name="type" />.
		/// </summary>
		/// <param name="type">The type of the backend.</param>
		/// <returns>Type index of the active backend of the provided backend type, or the type index of <c>std::nullptr_t</c>, if no backend is active.</returns>
		virtual std::type_index activeBackendType(const BackendType& type) const;

	private:
		/// <summary>
		/// Registers a new callback, that gets called, if the backend of <paramref name="type" /> is started.
		/// </summary>
		/// <param name="type">The type of the backend for which the callback is registered.</param>
		/// <param name="callback">The callback to register.</param>
		void registerStartCallback(std::type_index type, const std::function<bool()>& callback);

		/// <summary>
		/// Registers a new callback, that gets called, if the backend of <paramref name="type" /> is stopped.
		/// </summary>
		/// <param name="type">The type of the backend for which the callback is registered.</param>
		/// <param name="callback">The callback to register.</param>
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
		/// <summary>
		/// Returns the registered backend instance for a type index.
		/// </summary>
		/// <typeparam name="TBackend">The type, the type index is derived from.</typeparam>
		/// <returns>The registered backend instance for a type index, or <c>nullptr</c>, if the app has no backend of the provided type.</returns>
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
			this->startBackend(typeid(TBackend));
		}

		/// <summary>
		/// Stops a backend, if it is currently running.
		/// </summary>
		/// <typeparam name="TBackend">The type of the backend to stop.</typeparam>
		/// <exception cref="InvalidArgumentException">Thrown, if no backend of type <typeparamref name="TBackend" /> is registered.</exception>
		template <typename TBackend> requires
			rtti::implements<TBackend, IBackend>
		void stopBackend() {
			this->stopBackend(typeid(TBackend));
		}

	public:
		/// <summary>
		/// Adds a backend to the app.
		/// </summary>
		/// <param name="backend">The backend to add.</param>
		/// <exception cref="InvalidArgumentException">Thrown, if the backend is not initialized.</exception>
		virtual void use(UniquePtr<IBackend>&& backend);

		/// <summary>
		/// Starts the application.
		/// </summary>
		virtual void run() = 0;

		/// <summary>
		/// Called to initialize the application state.
		/// </summary>
		virtual void initialize() = 0;

	public:
		/// <summary>
		/// Called, if the application window resizes.
		/// </summary>
		/// <param name="width">The new width of the application window.</param>
		/// <param name="height">The new height of the application window.</param>
		virtual void resize(int width, int height);

	public:
		/// <summary>
		/// Creates a new application builder.
		/// </summary>
		template <typename TApp, typename ...TArgs>
		static AppBuilder build(TArgs&&... _args) { 
			return AppBuilder(makeUnique<TApp>(std::forward<TArgs>(_args)...)); 
		}
	};

	/// <summary>
	/// Creates a new builder for an <see cref="App" />.
	/// </summary>
	class LITEFX_APPMODEL_API AppBuilder : public Builder<AppBuilder, App> {
	public:
		using builder_type::Builder;

	protected:
		/// <inheritdoc />
		virtual void build() override;

	public:
		/// <inheritdoc />
		void use(UniquePtr<IBackend>&& backend);

		/// <summary>
		/// Registers a sink for logging.
		/// </summary>
		template <typename TSink, typename ...TArgs> requires
			std::convertible_to<TSink*, ISink*>
		AppBuilder& logTo(TArgs&&... args) {
			auto sink = makeUnique<TSink>(std::forward<TArgs>(args)...);
			Logger::sinkTo(sink.get());
			return *this;
		}

		/// <summary>
		/// Registers a new backend.
		/// </summary>
		template <typename TBackend, typename ...TArgs> requires
			rtti::implements<TBackend, IBackend>
		AppBuilder& useBackend(TArgs&&... args) {
			this->use(makeUnique<TBackend>(*this->instance(), std::forward<TArgs>(args)...));
			return *this;
		}
	};

	/// <summary>
	/// Contains the version of an <see cref="App" />.
	/// </summary>
	class LITEFX_APPMODEL_API AppVersion {
		LITEFX_IMPLEMENTATION(AppVersionImpl);

	public:
		/// <summary>
		/// Creates a new app version instance.
		/// </summary>
		/// <param name="major">The major version of the app.</param>
		/// <param name="minor">The minor version of the app.</param>
		/// <param name="patch">The patch number of the app.</param>
		/// <param name="revision">The revision of the app.</param>
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0) noexcept;
		AppVersion(const AppVersion&) = delete;
		AppVersion(AppVersion&&) = delete;

		virtual ~AppVersion() noexcept;

	public:
		/// <summary>
		/// Gets the major version of the app.
		/// </summary>
		/// <returns>The major version of the app.</returns>
		int major() const noexcept;

		/// <summary>
		/// Gets the minor version of the app.
		/// </summary>
		/// <returns>The minor version of the app.</returns>
		int minor() const noexcept;

		/// <summary>
		/// Gets the patch number of the app.
		/// </summary>
		/// <returns>The patch number of the app.</returns>
		int patch() const noexcept;

		/// <summary>
		/// Gets the revision of the app.
		/// </summary>
		/// <returns>The revision of the app.</returns>
		int revision() const noexcept;

		/// <summary>
		/// Gets the major version of the engine build.
		/// </summary>
		/// <returns>The major version of the engine build.</returns>
		int engineMajor() const noexcept;

		/// <summary>
		/// Gets the minor version of the engine build.
		/// </summary>
		/// <returns>The minor version of the engine build.</returns>
		int engineMinor() const noexcept;

		/// <summary>
		/// Gets the revision of the engine build.
		/// </summary>
		/// <returns>The revision of the engine build.</returns>
		int engineRevision() const noexcept;

		/// <summary>
		/// Gets the status of the engine build.
		/// </summary>
		/// <returns>The status of the engine build.</returns>
		int engineStatus() const noexcept;
		
		/// <summary>
		/// Gets the identifier of the engine build.
		/// </summary>
		/// <returns>The identifier of the engine build.</returns>
		String engineIdentifier() const noexcept;

		/// <summary>
		/// Gets the version string of the engine build.
		/// </summary>
		/// <returns>The version string of the engine build.</returns>
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