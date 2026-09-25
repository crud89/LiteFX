#pragma once

#include <litefx/app_api.hpp>
#include <litefx/app_formatters.hpp>

namespace LiteFX {
	using namespace LiteFX::Logging;

	/// @brief The base class for an app backend.
	///
	/// An @ref App can have one or more backends configured per @ref BackendType. However, only one backend of a specific backend type can be active at a time. Backends can be configured with a startup and
	/// shutdown callback. Starting a backend will automatically shutdown the active backend of the same type, if any. If the app exits, all backends are shutdown accordingly, allowing to properly cleanup any
	/// resources.
	///
	/// @see App
	class LITEFX_APPMODEL_API IBackend {
		friend class App;

	private:
		BackendState m_state = BackendState::Inactive;

	protected:
		IBackend() noexcept = default;
		IBackend(const IBackend&) = default;
		IBackend(IBackend&&) noexcept = default;
		IBackend& operator=(const IBackend&) = default;
		IBackend& operator=(IBackend&&) noexcept = default;

	public:
		virtual ~IBackend() noexcept = default;

	public:
		/// @brief Gets the type of the backend.
		///
		/// @return The type of the backend.
		/// @see BackendType
		virtual BackendType type() const noexcept = 0;

		/// @brief Returns the state of the backend.
		///
		/// @return The state of the backend.
		/// @see BackendState
		const BackendState& state() const noexcept { return m_state; }

		/// @brief Gets the name of the backend.
		///
		/// @return The name of the backend.
		virtual StringView name() const noexcept = 0;

	protected:
		/// @brief Returns the state of the backend.
		///
		/// @return The state of the backend.
		BackendState& state() noexcept { return m_state; }

		/// @brief Called by the parent @ref App, if the backend is started.
		virtual void activate() = 0;

		/// @brief Called by the parent @ref App, if the backend is stopped.
		virtual void deactivate() = 0;

	private:
		std::type_index typeId() const noexcept { return typeid(*this); }
	};

	/// @brief Concept that can be used to refer to backend implementations.
	template <typename T>
	concept backend = meta::implements<T, IBackend>;

	/// @brief Base class for additional event arguments.
	///
	/// @see Event
	class LITEFX_APPMODEL_API EventArgs {
	public:
		EventArgs() = default;

		EventArgs(const EventArgs&) = default;
		EventArgs(EventArgs&&) noexcept = default;
		EventArgs& operator=(const EventArgs&) = default;
		EventArgs& operator=(EventArgs&&) noexcept = default;
		virtual ~EventArgs() noexcept = default;
	};

	/// @brief Represents a handler for an @ref Event, that is assigned a unique token when created, so that it can be identified later.
	///
	/// Since a `std::function` or callable is non-comparable, it is not possible to identify and remove event handlers from an event, after registering them. To support this functionality, the event handler
	/// will be encapsulated in a delegate, which gets assigned with a token. This token is unique for the event, the delegate has been registered to.
	///
	/// @tparam TResult The result of the delegate function.
	/// @tparam ...TArgs The arguments of the delegate function.
	template <typename TResult, typename... TArgs>
	class Delegate final {
	public:
		using function_type = std::function<TResult(TArgs...)>;
		using token_type = size_t;

	private:
		function_type m_target;
		token_type m_token;

	public:
		/// @brief Creates a new delegate.
		///
		/// @param fn The delegate function.
		/// @param t The unique token of the delegate within the parent event.
		inline Delegate(function_type fn, token_type t) noexcept : 
			m_target(std::move(fn)), m_token(t) { }

	public:
		/// @brief Invokes the delegate function.
		///
		/// @param ...args The arguments passed to the function.
		/// @return The result of the delegate function call.
		inline TResult invoke(TArgs... args) const {
			return m_target(std::move(args)...);
		}

		/// @brief Returns the unique token of the delegate.
		///
		/// @return The unique token of the delegate.
		inline token_type token() const {
			return m_token;
		}

	public:
		/// @brief Invokes the delegate function.
		///
		/// @param ...args The arguments passed to the function.
		/// @return The result of the delegate function call.
		inline TResult operator()(TArgs... args) const {
			return this->invoke(std::move(args)...);
		}
	};

	/// @brief A class that is used to declare an event, which a number of listeners can subscribe to.
	///
	/// A listener that subscribes to the event is called *event handler*. The event handler needs to be invokable and identifiable. Because of this, a @ref Delegate is created for the event handler. A
	/// delegate stores the event handler, as well as a token to identify the event handler. Event handlers must expose the a common signature: they do not return anything and accept two parameters. The first
	/// parameter is an unformatted pointer to the event sender (i.e., the object that invoked the event handlers). The second parameter contains additional arguments (@p TEventArgs), that are passed to all
	/// handlers. Note that the sender can also be `nullptr`.
	///
	/// @tparam TEventArgs The type of the additional event arguments.
	/// @see EventArgs
	template <typename TEventArgs>
	class Event final {
	public:
		using event_args_type = TEventArgs;
		using delegate_type = Delegate<void, const void*, TEventArgs>;
		using function_type = typename delegate_type::function_type;
		using event_token_type = typename delegate_type::token_type;

	private:
		Array<delegate_type> m_subscribers{};

	public:
		/// @brief Initializes a new event.
		constexpr Event() = default;

		/// @brief Creates a copy of a event.
		///
		/// This constructor is implemented in order to support copying of types that contain events. Subscribers are not copied!
		///
		/// @param _other The event instance to copy.
		constexpr Event([[maybe_unused]] const Event& _other) { }

		/// @brief Takes over another instance of a event.
		///
		/// @param _other The event instance to take over.
		constexpr Event(Event&& _other) noexcept = default;

		/// @brief Assigns a event by copying it.
		///
		/// This operator is implemented in order to support copying of types that contain events. Subscribers are not copied and previous subscriptions are cleared!
		///
		/// @param _other The event instance to copy.
		/// @return A reference to the current event instance.
		constexpr Event& operator=([[maybe_unused]] const Event& _other) {
			m_subscribers.clear();
			return *this;
		}

		/// @brief Assigns a event by taking it over.
		///
		/// @param _other The event to take over.
		/// @return A reference to the current event instance.
		constexpr Event& operator=(Event&& _other) noexcept = default;

		/// @brief Releases the event instance.
		constexpr ~Event() noexcept = default;

	public:
		/// @brief Subscribes an event handler to the event.
		///
		/// @param subscriber A delegate for the event handler.
		/// @return A unique token of the event handler.
		event_token_type add(const function_type& subscriber) {
			const auto match = std::max_element(m_subscribers.begin(), m_subscribers.end(), [](const auto& lhs, const auto& rhs) { return lhs.token() < rhs.token(); });
			event_token_type token = match == m_subscribers.end() ? 0 : match->token() + 1;
			m_subscribers.emplace_back(subscriber, token);
			return token;
		}

		/// @brief Unsubscribes an event handler from the event.
		///
		/// @param subscriber A delegate for the event handler.
		/// @return `true`, if the event handler has been removed, `false` otherwise.
		bool remove(const delegate_type& subscriber) noexcept {
			return this->remove(subscriber.token());
		}

		/// @brief Unsubscribes an event handler from the event.
		///
		/// @param toke The unique token of the event handler.
		/// @return `true`, if the event handler has been removed, `false` otherwise.
		bool remove(event_token_type token) noexcept {
			const auto last = std::remove_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& s) { return s.token() == token; });

			if (last == m_subscribers.end())
				return false;

			m_subscribers.erase(last, m_subscribers.end());
			return true;
		}

		/// @brief Clears the event handlers.
		void clear() noexcept {
			m_subscribers.clear();
		}

		/// @brief Invokes all event handlers of the event.
		///
		/// @param sender The source of the event.
		/// @param args The additional event arguments.
		void invoke(const void* sender, const TEventArgs& args) const {
			for (const auto& handler : m_subscribers)
				handler(sender, args);
		}

		/// @brief Returns `true`, if the event contains a subscriber with the provided @p token.
		///
		/// @param token The token of an event.
		/// @return `true`, if the event contains a subscriber with the provided @p token, `false` otherwise.
		bool contains(event_token_type token) const noexcept {
			return std::find_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& d) { return d.token() == token; }) != m_subscribers.end();
		}

		/// @brief Returns the delegate associated with @p token.
		///
		/// @param token The token to query for.
		/// @return A reference of the delegate associated with @p token.
		/// @throws InvalidArgumentException Thrown, if the event does not have a subscriber with the provided token.
		const delegate_type& handler(event_token_type token) const {
			if (auto match = std::find_if(m_subscribers.begin(), m_subscribers.end(), [&token](const auto& d) { return d.token() == token; }); match != m_subscribers.end()) [[likely]]
				return *match;

			throw InvalidArgumentException("token", "The event does not contain the provided token.");
		}

	public:
		/// @brief Returns `true`, if any event handler is attached to the event, `false` otherwise.
		///
		/// @return `true`, if any event handler is attached to the event, `false` otherwise.
		explicit operator bool() const noexcept {
			return !m_subscribers.empty();
		}

		/// @brief Subscribes an event handler to the event.
		///
		/// @param subscriber A delegate for the event handler.
		/// @return A unique token of the event handler.
		event_token_type operator +=(const function_type& subscriber) {
			return this->add(subscriber);
		}

		/// @brief Unsubscribes an event handler from the event.
		///
		/// @param subscriber A delegate for the event handler.
		/// @return `true`, if the event handler has been removed, `false` otherwise.
		bool operator -=(const delegate_type& subscriber) noexcept {
			return this->remove(subscriber);
		}

		/// @brief Unsubscribes an event handler from the event.
		///
		/// @param toke The unique token of the event handler.
		/// @return `true`, if the event handler has been removed, `false` otherwise.
		bool operator -=(event_token_type token) noexcept {
			return this->remove(token);
		}

		/// @brief Invokes all event handlers of the event.
		///
		/// @param sender The source of the event.
		/// @param args The additional event arguments.
		void operator ()(const void* sender, const TEventArgs& args) const {
			this->invoke(sender, args);
		}

		/// @brief Returns the delegate associated with @p token.
		///
		/// @param token The token to query for.
		/// @return A reference of the delegate associated with @p token.
		/// @throws InvalidArgumentException Thrown, if the event does not have a subscriber with the provided token.
		const delegate_type& operator [](event_token_type token) const {
			return this->handler(token);
		}
	};

	/// @brief Stores event arguments of a window resize event.
	///
	/// @see App::resize
	struct LITEFX_APPMODEL_API ResizeEventArgs : public EventArgs {
	private:
		int m_width, m_height;

	public:
		/// @brief Creates a new set of window resize event arguments.
		///
		/// @param width The old window width.
		/// @param height The old window height.
		ResizeEventArgs(int width, int height) noexcept : m_width(width), m_height(height) { }
		ResizeEventArgs(const ResizeEventArgs&) = default;
		ResizeEventArgs(ResizeEventArgs&&) noexcept = default;
		~ResizeEventArgs() noexcept override = default;

	public:
		ResizeEventArgs& operator=(const ResizeEventArgs&) = default;
		ResizeEventArgs& operator=(ResizeEventArgs&&) noexcept = default;

	public:
		/// @brief Returns the new window width.
		///
		/// @return The new window width.
		inline int width() const noexcept {
			return m_width;
		}

		/// @brief Returns the new window height.
		///
		/// @return The new window height.
		inline int height() const noexcept {
			return m_height;
		}
	};

	/// @brief The base class for an application.
	///
	/// @see AppBuilder
	/// @see AppVersion
	class LITEFX_APPMODEL_API App {
		LITEFX_IMPLEMENTATION(AppImpl);
		LITEFX_BUILDER(AppBuilder);

	public:
		/// @brief Initializes a new app instance.
		App();
		App(const App&) = delete;
		App(App&&) noexcept = delete;
		auto operator=(const App&) = delete;
		auto operator=(App&&) noexcept = delete;

		virtual ~App() noexcept;

	public:
		/// @brief Returns the name of the app.
		///
		/// @return The name of the app.
		virtual StringView name() const noexcept = 0;

		/// @brief Returns the version of the app.
		///
		/// @return The version of the app.
		virtual AppVersion version() const noexcept = 0;

		/// @brief Returns the platform, the app is running on.
		///
		/// @return The platform, the app is running on.
		Platform platform() const noexcept;

		/// @brief Returns the registered backend instance for a type index.
		///
		/// @param type The type index of the requested backend.
		/// @return The registered backend instance for a type index, or `nullptr`, if the app has no backend of the provided type.
		const IBackend* operator[](std::type_index type) const;

		/// @brief Returns the registered backend instance for a type index.
		///
		/// @param type The type index of the requested backend.
		/// @return The registered backend instance for a type index, or `nullptr`, if the app has no backend of the provided type.
		const IBackend* getBackend(std::type_index type) const;

		/// @brief Returns all registered backend instances of a backend type.
		///
		/// @param type The backend type of the requested backends.
		/// @return All registered instances of @p type.
		Enumerable<const IBackend&> getBackends(const BackendType type) const;

	protected:
		/// @brief Returns the registered backend instance for a type index.
		///
		/// @param type The type index of the requested backend.
		/// @return The registered backend instance for a type index, or `nullptr`, if the app has no backend of the provided type.
		IBackend* getBackend(std::type_index type);

		/// @brief Returns the registered backend instance for a type index.
		///
		/// @tparam TBackend The type, the type index is derived from.
		/// @return The registered backend instance for a type index, or `nullptr`, if the app has no backend of the provided type.
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		TBackend* findBackend() {
			return dynamic_cast<TBackend*>(this->getBackend(typeid(TBackend)));
		}

		/// @brief Starts a backend.
		///
		/// Starting a backend will first stop the active backend of the same @ref BackendType. It will then call all start callbacks for the backend. If the backend has been started successfully, it calls @ref
		/// IBackend::activate before returning.
		///
		/// @param type The type index of the backend to start.
		/// @see Backend
		/// @see onBackendStart
		void startBackend(std::type_index type) const;

		/// @brief Stops a backend.
		///
		/// This method calls all stop callbacks for the backend, before returing. If the backend is not running, calling this method has no effect.
		///
		/// @param type The type index of the backend to start.
		/// @see Backend
		/// @see onBackendStop
		void stopBackend(std::type_index type) const;

		/// @brief Stops the active backend of @p type.
		///
		/// @param type The backend type for which the active backend should be stopped.
		/// @see stopBackend
		void stopActiveBackends(BackendType type) const;

		/// @brief Returns the active backend of the provided backend @p type.
		///
		/// @param type The type of the backend.
		/// @return The active backend of the provided backend type, or `std::nullptr`, if no backend is active.
		IBackend* activeBackend(BackendType type) const;

		/// @brief Returns the type index of the active backend of the provided backend @p type.
		///
		/// @param type The type of the backend.
		/// @return Type index of the active backend of the provided backend type, or the type index of `std::nullptr_t`, if no backend is active.
		std::type_index activeBackendType(BackendType type) const;

	private:
		/// @brief Registers a new callback, that gets called, if the backend of @p type is started.
		///
		/// @param type The type of the backend for which the callback is registered.
		/// @param callback The callback to register.
		void registerStartCallback(std::type_index type, const std::function<bool()>& callback);

		/// @brief Registers a new callback, that gets called, if the backend of @p type is stopped.
		///
		/// @param type The type of the backend for which the callback is registered.
		/// @param callback The callback to register.
		void registerStopCallback(std::type_index type, const std::function<void()>& callback);

	public:
		/// @brief Invoked, if a backend has been started.
		mutable Event<const IBackend*> backendStarted; // NOLINT

		/// @brief Invoked, if a backend has been stopped.
		mutable Event<const IBackend*> backendStopped; // NOLINT

		/// @brief Sets a callback that is called, if a backend is started.
		///
		/// A backend can have multiple start callbacks, that are executed if a backend is started. Typically such a callback is used to initialize a device and surface for an application. An application might
		/// use multiple callbacks, if it creates multiple devices, for example to create a plugin architecture.
		///
		/// Different to the @ref backendStarted event, this event can be strongly typed at compile time, thus different callbacks can be provided per backend type without requiring to differentiate them at
		/// run-time.
		///
		/// @tparam TBackend The type of the backend.
		/// @param callback The function to call during backend startup.
		/// @see onBackendStop
		/// @see backendStarted
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

		/// @brief Sets a callback that is called, if a backend is stopped.
		///
		/// Different to the @ref backendStopped event, this event can be strongly typed at compile time, thus different callbacks can be provided per backend type without requiring to differentiate them at
		/// run-time.
		///
		/// @tparam TBackend The type of the backend.
		/// @param callback The function to call during backend shutdown.
		/// @see onBackendStart
		/// @see backendStopped
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
		/// @brief Returns the registered backend instance for a type index.
		///
		/// @tparam TBackend The type, the type index is derived from.
		/// @return The registered backend instance for a type index, or `nullptr`, if the app has no backend of the provided type.
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		const TBackend* findBackend() const {
			return dynamic_cast<const TBackend*>(this->getBackend(typeid(TBackend)));
		}

		/// @brief Attempts to start a backend of type @p TBackend and stops the active backend of the same @ref BackendType, if any.
		///
		/// @tparam TBackend The type of the backend to start.
		/// @throws InvalidArgumentException Thrown, if no backend of type @p TBackend is registered.
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void startBackend() {
			this->startBackend(typeid(TBackend));
		}

		/// @brief Stops a backend, if it is currently running.
		///
		/// @tparam TBackend The type of the backend to stop.
		/// @throws InvalidArgumentException Thrown, if no backend of type @p TBackend is registered.
		template <typename TBackend> requires
			meta::implements<TBackend, IBackend>
		void stopBackend() {
			this->stopBackend(typeid(TBackend));
		}

	public:
		/// @brief Invoked, if the application has been started.
		mutable Event<EventArgs> startup; // NOLINT

		/// @brief Invoked during initialization.
		mutable Event<EventArgs> initializing; // NOLINT

		/// @brief Invoked, if the application has is shutting down.
		mutable Event<EventArgs> shutdown; // NOLINT

		/// @brief Adds a backend to the app.
		///
		/// @param backend The backend to add.
		/// @throws InvalidArgumentException Thrown, if the backend is not initialized.
		virtual void use(UniquePtr<IBackend>&& backend);

		/// @brief Starts the application.
		void run();

	public:
		/// @brief Invoked, if the app window or context gets resized.
		mutable Event<ResizeEventArgs> resized; // NOLINT

		/// @brief Called, if the application window resizes.
		///
		/// @param width The new width of the application window.
		/// @param height The new height of the application window.
		void resize(int width, int height);

	public:
		/// @brief Creates a new application builder.
		template <typename TApp, typename ...TArgs>
		[[nodiscard]] static AppBuilder build(TArgs&&... _args);
	};

	/// @brief Creates a new builder for an @ref App.
	class LITEFX_APPMODEL_API [[nodiscard]] AppBuilder : public Builder<App> {
	public:
		using Builder<App>::Builder;

	public:
		/// @copydoc Builder::use
		void use(UniquePtr<IBackend>&& backend);

		/// @brief Registers a sink for logging.
		template <typename TSink, typename ...TArgs> requires
			std::convertible_to<TSink*, ISink*>
		AppBuilder& logTo(TArgs&&... args) {
			auto sink = makeUnique<TSink>(std::forward<TArgs>(args)...);
			Logger::sinkTo(sink.get());
			return *this;
		}

		/// @brief Registers a new backend.
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