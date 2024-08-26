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
	/// Base class for additional event arguments.
	/// </summary>
	/// <seealso cref="Event" />
	class LITEFX_APPMODEL_API EventArgs {
	public:
		EventArgs() = default;
		EventArgs(const EventArgs&) = default;
		EventArgs(EventArgs&&) = default;
		virtual ~EventArgs() noexcept = default;

	public:
		EventArgs& operator=(const EventArgs&) = default;
		EventArgs& operator=(EventArgs&&) = default;
	};

	/// <summary>
	/// Represents a handler for an <see cref="Event" />, that is assigned a unique token when created, so that it can be identified later.
	/// </summary>
	/// <remarks>
	/// Since a `std::function` or callable us not comparable, it is not possible to identify and remove event handlers from an event, after registering 
	/// them. To support this functionality, the event handler will be encapsulated in a delegate, which gets assigned with a token. This token is unique
	/// for the event, the delegate has been registered to.
	/// </remarks>
	/// <typeparam name="TResult">The result of the delegate function.</typeparam>
	/// <typeparam name="...TArgs">The arguments of the delegate function.</typeparam>
	template <typename TResult, typename... TArgs>
	class Delegate {
	public:
		using function_type = std::function<TResult(TArgs...)>;
		using token_type = size_t;

	private:
		function_type m_target;
		token_type m_token;

	public:
		/// <summary>
		/// Creates a new delegate.
		/// </summary>
		/// <param name="fn">The delegate function.</param>
		/// <param name="t">The unique token of the delegate within the parent event.</param>
		inline Delegate(function_type fn, token_type t) noexcept : m_target(fn), m_token(t) { }

	public:
		/// <summary>
		/// Invokes the delegate function.
		/// </summary>
		/// <param name="...args">The arguments passed to the function.</param>
		/// <returns>The result of the delegate function call.</returns>
		inline TResult invoke(TArgs... args) const {
			return m_target(args...);
		}

		/// <summary>
		/// Returns the unique token of the delegate.
		/// </summary>
		/// <returns>The unique token of the delegate.</returns>
		inline token_type token() const {
			return m_token;
		}

	public:
		/// <summary>
		/// Invokes the delegate function.
		/// </summary>
		/// <param name="...args">The arguments passed to the function.</param>
		/// <returns>The result of the delegate function call.</returns>
		inline TResult operator()(TArgs... args) const {
			return this->invoke(args...);
		}
	};

	/// <summary>
	/// A class that is used to declare an event, which a number of listeners can subscribe to.
	/// </summary>
	/// <remarks>
	/// A listener that subscribes to the event is called *event handler*. The event handler needs to be invokable and identifiable. Because of this, a
	/// <see cref="Delegate" /> is created for the event handler. A delegate stores the event handler, as well as a token to identify the event handler.
	/// Event handlers must expose the a common signature: they do not return anything and accept two parameters. The first parameter is an unformatted 
	/// pointer to the event sender (i.e., the object that invoked the event handlers). The second parameter contains additional arguments 
	/// (<typeparamref name="TEventArgs" />), that are passed to all handlers. Note that the sender can also be `nullptr`.
	/// </remarks>
	/// <typeparam name="TEventArgs">The type of the additional event arguments.</typeparam>
	/// <seealso cref="EventArgs" />
	template <typename TEventArgs>
	class Event {
	public:
		using event_args_type = TEventArgs;
		using delegate_type = Delegate<void, const void*, TEventArgs>;
		using function_type = typename delegate_type::function_type;
		using event_token_type = typename delegate_type::token_type;

	private:
		Array<delegate_type> m_subscribers;

	public:
		/// <summary>
		/// Initializes a new event.
		/// </summary>
		Event() = default;
		Event(const Event&) = delete;
		Event(Event&&) = delete;

	public:
		/// <summary>
		/// Subscribes an event handler to the event.
		/// </summary>
		/// <param name="subscriber">A delegate for the event handler.</param>
		/// <returns>A unique token of the event handler.</returns>
		event_token_type add(function_type subscriber) noexcept {
			const auto match = std::max_element(m_subscribers.begin(), m_subscribers.end(), [](const auto& lhs, const auto& rhs) { return lhs.token() < rhs.token(); });
			event_token_type token = match == m_subscribers.end() ? 0 : match->token() + 1;
			m_subscribers.emplace_back(subscriber, token);
			return token;
		}

		/// <summary>
		/// Unsubscribes an event handler from the event.
		/// </summary>
		/// <param name="subscriber">A delegate for the event handler.</param>
		/// <returns>`true`, if the event handler has been removed, `false` otherwise.</returns>
		bool remove(delegate_type subscriber) noexcept {
			return this->remove(subscriber.token());
		}

		/// <summary>
		/// Unsubscribes an event handler from the event.
		/// </summary>
		/// <param name="toke">The unique token of the event handler.</param>
		/// <returns>`true`, if the event handler has been removed, `false` otherwise.</returns>
		bool remove(event_token_type token) noexcept {
			const auto last = std::remove_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& s) { return s.token() == token; });

			if (last == m_subscribers.end())
				return false;

			m_subscribers.erase(last, m_subscribers.end());
			return true;
		}

		/// <summary>
		/// Clears the event handlers.
		/// </summary>
		void clear() noexcept {
			m_subscribers.clear();
		}

		/// <summary>
		/// Invokes all event handlers of the event.
		/// </summary>
		/// <param name="sender">The source of the event.</param>
		/// <param name="args">The additional event arguments.</param>
		void invoke(const void* sender, TEventArgs args) const {
			for (const auto& handler : m_subscribers)
				handler(sender, args);
		}

		/// <summary>
		/// Returns `true`, if the event contains a subscriber with the provided <paramref name="token" />.
		/// </summary>
		/// <param name="token">The token of an event.</param>
		/// <returns>`true`, if the event contains a subscriber with the provided <paramref name="token" />, `false` otherwise.</returns>
		bool contains(event_token_type token) const noexcept {
			return std::find_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& d) { return d.token() == token; }) != m_subscribers.end();
		}

		/// <summary>
		/// Returns the delegate associated with <paramref name="token" />.
		/// </summary>
		/// <param name="token">The token to query for.</param>
		/// <returns>A reference of the delegate associated with <paramref name="token" />.</returns>
		/// <exception cref="InvalidArgumentException">Thrown, if the event does not have a subscriber with the provided token.</exception>
		const delegate_type& handler(event_token_type token) const {
			if (auto match = std::find_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& d) { return d.token() == token; }); match != m_subscribers.end()) [[likely]]
				return *match;

			throw InvalidArgumentException("token", "The event does not contain the provided token.");
		}

	public:
		/// <summary>
		/// Returns `true`, if any event handler is attached to the event, `false` otherwise. 
		/// </summary>
		/// <returns>`true`, if any event handler is attached to the event, `false` otherwise.</returns>
		explicit operator bool() const noexcept {
			return !m_subscribers.empty();
		}

		/// <summary>
		/// Subscribes an event handler to the event.
		/// </summary>
		/// <param name="subscriber">A delegate for the event handler.</param>
		/// <returns>A unique token of the event handler.</returns>
		event_token_type operator +=(function_type subscriber) {
			return this->add(subscriber);
		}

		/// <summary>
		/// Unsubscribes an event handler from the event.
		/// </summary>
		/// <param name="subscriber">A delegate for the event handler.</param>
		/// <returns>`true`, if the event handler has been removed, `false` otherwise.</returns>
		bool operator -=(delegate_type subscriber) noexcept {
			return this->remove(subscriber);
		}

		/// <summary>
		/// Unsubscribes an event handler from the event.
		/// </summary>
		/// <param name="toke">The unique token of the event handler.</param>
		/// <returns>`true`, if the event handler has been removed, `false` otherwise.</returns>
		bool operator -=(event_token_type token) noexcept {
			return this->remove(token);
		}

		/// <summary>
		/// Invokes all event handlers of the event.
		/// </summary>
		/// <param name="sender">The source of the event.</param>
		/// <param name="args">The additional event arguments.</param>
		void operator ()(const void* sender, TEventArgs args) const {
			this->invoke(sender, args);
		}

		/// <summary>
		/// Returns the delegate associated with <paramref name="token" />.
		/// </summary>
		/// <param name="token">The token to query for.</param>
		/// <returns>A reference of the delegate associated with <paramref name="token" />.</returns>
		/// <exception cref="InvalidArgumentException">Thrown, if the event does not have a subscriber with the provided token.</exception>
		const delegate_type& operator [](event_token_type token) const {
			return this->handler(token);
		}
	};

	/// <summary>
	/// Stores event arguments of a window resize event.
	/// </summary>
	/// <seealso cref="App::resize" />
	struct LITEFX_APPMODEL_API ResizeEventArgs : public EventArgs {
	private:
		int m_width, m_height;

	public:
		/// <summary>
		/// Creates a new set of window resize event arguments.
		/// </summary>
		/// <param name="width">The old window width.</param>
		/// <param name="height">The old window height.</param>
		ResizeEventArgs(int width, int height) : m_width(width), m_height(height) { }
		ResizeEventArgs(const ResizeEventArgs&) = default;
		ResizeEventArgs(ResizeEventArgs&&) = default;
		virtual ~ResizeEventArgs() = default;

	public:
		ResizeEventArgs& operator=(const ResizeEventArgs&) = default;
		ResizeEventArgs& operator=(ResizeEventArgs&&) = default;

	public:
		/// <summary>
		/// Returns the new window width.
		/// </summary>
		/// <returns>The new window width.</returns>
		inline int width() const noexcept {
			return m_width;
		}

		/// <summary>
		/// Returns the new window height.
		/// </summary>
		/// <returns>The new window height.</returns>
		inline int height() const noexcept {
			return m_height;
		}
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
		virtual Enumerable<const IBackend*> getBackends(const BackendType type) const noexcept;

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
			meta::implements<TBackend, IBackend>
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
		virtual void stopActiveBackends(BackendType type) const;

		/// <summary>
		/// Returns the active backend of the provided backend <paramref name="type" />.
		/// </summary>
		/// <param name="type">The type of the backend.</param>
		/// <returns>The active backend of the provided backend type, or <c>std::nullptr</c>, if no backend is active.</returns>
		virtual IBackend* activeBackend(BackendType type) const;

		/// <summary>
		/// Returns the type index of the active backend of the provided backend <paramref name="type" />.
		/// </summary>
		/// <param name="type">The type of the backend.</param>
		/// <returns>Type index of the active backend of the provided backend type, or the type index of <c>std::nullptr_t</c>, if no backend is active.</returns>
		virtual std::type_index activeBackendType(BackendType type) const;

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
		/// Invoked, if a backend has been started.
		/// </summary>
		mutable Event<const IBackend*> backendStarted;

		/// <summary>
		/// Invoked, if a backend has been stopped.
		/// </summary>
		mutable Event<const IBackend*> backendStopped;

		/// <summary>
		/// Sets a callback that is called, if a backend is started.
		/// </summary>
		/// <remarks>
		/// A backend can have multiple start callbacks, that are executed if a backend is started. Typically such a callback is used to initialize
		/// a device and surface for an application. An application might use multiple callbacks, if it creates multiple devices, for example to
		/// create a plugin architecture.
		/// </remarks>
		/// <remarks>
		/// Different to the <see cref="backendStarted" /> event, this event can be strongly typed at compile time, thus different callbacks can be 
		/// provided per backend type without requiring to differentiate them at run-time.
		/// </remarks>
		/// <typeparam name="TBackend">The type of the backend.</typeparam>
		/// <param name="callback">The function to call during backend startup.</param>
		/// <seealso cref="onBackendStop" />
		/// <seealso cref="backendStarted" />
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void onBackendStart(const std::function<bool(TBackend*)>& callback) {
			this->registerStartCallback(typeid(TBackend), [this, callback]() {
				auto backend = this->findBackend<TBackend>();

				if (backend == nullptr)
					throw InvalidArgumentException("callback", "No backend of type {0} has been registered.", typeid(TBackend).name());

				if (backend->state() == BackendState::Active)
					return true;
				else
					return callback(backend);
			});
		}

		/// <summary>
		/// Sets a callback that is called, if a backend is stopped.
		/// </summary>
		/// <remarks>
		/// Different to the <see cref="backendStopped" /> event, this event can be strongly typed at compile time, thus different callbacks can be 
		/// provided per backend type without requiring to differentiate them at run-time.
		/// </remarks>
		/// <typeparam name="TBackend">The type of the backend.</typeparam>
		/// <param name="callback">The function to call during backend shutdown.</param>
		/// <seealso cref="onBackendStart" />
		/// <seealso cref="backendStopped" />
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void onBackendStop(const std::function<void(TBackend*)>& callback) {
			this->registerStopCallback(typeid(TBackend), [this, callback]() {
				auto backend = this->findBackend<TBackend>();

				if (backend == nullptr)
					throw InvalidArgumentException("callback", "No backend of type {0} has been registered.", typeid(TBackend).name());

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
			meta::implements<TBackend, IBackend>
		const TBackend* findBackend() const {
			return dynamic_cast<const TBackend*>(this->getBackend(typeid(TBackend)));
		}

		/// <summary>
		/// Attempts to start a backend of type <typeparamref name="TBackend" /> and stops the active backend of the same <see cref="BackendType" />, if any.
		/// </summary>
		/// <typeparam name="TBackend">The type of the backend to start.</typeparam>
		/// <exception cref="InvalidArgumentException">Thrown, if no backend of type <typeparamref name="TBackend" /> is registered.</exception>
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void startBackend() {
			this->startBackend(typeid(TBackend));
		}

		/// <summary>
		/// Stops a backend, if it is currently running.
		/// </summary>
		/// <typeparam name="TBackend">The type of the backend to stop.</typeparam>
		/// <exception cref="InvalidArgumentException">Thrown, if no backend of type <typeparamref name="TBackend" /> is registered.</exception>
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void stopBackend() {
			this->stopBackend(typeid(TBackend));
		}

	public:
		/// <summary>
		/// Invoked, if the application has been started.
		/// </summary>
		mutable Event<EventArgs> startup;

		/// <summary>
		/// Invoked during initialization.
		/// </summary>
		mutable Event<EventArgs> initializing;

		/// <summary>
		/// Invoked, if the application has is shutting down.
		/// </summary>
		mutable Event<EventArgs> shutdown;

		/// <summary>
		/// Adds a backend to the app.
		/// </summary>
		/// <param name="backend">The backend to add.</param>
		/// <exception cref="InvalidArgumentException">Thrown, if the backend is not initialized.</exception>
		virtual void use(UniquePtr<IBackend>&& backend);

		/// <summary>
		/// Starts the application.
		/// </summary>
		void run();

	public:
		/// <summary>
		/// Invoked, if the app window or context gets resized.
		/// </summary>
		mutable Event<ResizeEventArgs> resized;

		/// <summary>
		/// Called, if the application window resizes.
		/// </summary>
		/// <param name="width">The new width of the application window.</param>
		/// <param name="height">The new height of the application window.</param>
		void resize(int width, int height);

	public:
		/// <summary>
		/// Creates a new application builder.
		/// </summary>
		template <typename TApp, typename ...TArgs>
		[[nodiscard]] static AppBuilder build(TArgs&&... _args);
	};

	/// <summary>
	/// Creates a new builder for an <see cref="App" />.
	/// </summary>
	class LITEFX_APPMODEL_API [[nodiscard]] AppBuilder : public Builder<App> {
	public:
		using Builder<App>::Builder;

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
			meta::implements<TBackend, IBackend>
		AppBuilder& useBackend(TArgs&&... args) {
			this->use(makeUnique<TBackend>(*this->instance(), std::forward<TArgs>(args)...));
			return *this;
		}
	};

	template<typename TApp, typename ...TArgs>
	inline AppBuilder App::build(TArgs && ..._args)
	{
		return AppBuilder(makeUnique<TApp>(std::forward<TArgs>(_args)...));
	}
}