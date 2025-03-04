#include <litefx/app.hpp>

using namespace LiteFX;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class App::AppImpl {
public:
	friend class App;

private:
	Dictionary<std::type_index, UniquePtr<IBackend>> m_backends{};
	std::multimap<std::type_index, const std::function<bool()>> m_startCallbacks{};
	std::multimap<std::type_index, const std::function<void()>> m_stopCallbacks{};

private:
	IBackend* findBackend(std::type_index type) const
	{
		return m_backends.contains(type) ? m_backends.at(type).get() : nullptr;
	};
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

App::App() :
	m_impl() 
{
}

App::~App() noexcept
{
	for (auto& backend : m_impl->m_backends | std::views::filter([](const auto& b) { return b.second->state() == BackendState::Active; }))
	{
		try
		{
			this->stopBackend(backend.first);
		}
		catch (...)
		{
			// Drop the exception, as there's nothing we can do anymore at this point (not even log, as the application name is no longer provided by the parent class).
			continue;
		}
	}
}

Platform App::platform() const noexcept
{
#if defined(_WIN32) || defined(WINCE)
	return Platform::Win32;
#else
	return Platform::Other;
#endif
}

const IBackend* App::operator[](std::type_index type) const
{
	return this->getBackend(type);
}

const IBackend* App::getBackend(std::type_index type) const
{
	return m_impl->findBackend(type);
}

IBackend* App::getBackend(std::type_index type)
{
	return m_impl->findBackend(type);
}

void App::startBackend(std::type_index type) const
{
	auto backend = m_impl->findBackend(type);

	if (backend == nullptr)
		throw InvalidArgumentException("type", "No backend of type {0} has been registered.", type.name());

	if (backend->state() == BackendState::Active)
		return;

	// Stop all active backends.
	this->stopActiveBackends(backend->type());

	// Call the start callbacks for the backend.
	const auto callbacks = m_impl->m_startCallbacks.equal_range(type);

	for (auto it = callbacks.first; it != callbacks.second; ++it)
		if (!it->second())
			throw RuntimeException("Unable to start backend {0}.", type.name());

	// Set the backend to active.
	backend->activate();

	// Publish event.
	this->backendStarted(this, backend);
}

void App::stopBackend(std::type_index type) const
{
	auto backend = m_impl->findBackend(type);

	if (backend == nullptr)
		throw InvalidArgumentException("type", "No backend of type {0} has been registered.", type.name());

	if (backend->state() != BackendState::Inactive)
	{
		// Call the stop callbacks for the backend.
		const auto callbacks = m_impl->m_stopCallbacks.equal_range(type);

		for (auto it = callbacks.first; it != callbacks.second; ++it)
			it->second();

		// Set the backend state to inactive.
		static_cast<IBackend*>(backend)->deactivate();

		// Publish event.
		this->backendStopped(this, backend);
	}
}

void App::stopActiveBackends(BackendType type) const
{
	for (auto& backend : m_impl->m_backends | std::views::filter([type](const auto& b) { return b.second->type() == type && b.second->state() == BackendState::Active; }))
		this->stopBackend(backend.first);
}

// The following methods return at the first match (if any), so the subsequent matches are unreachable. However there is no place for `std::unreachable` in this pattern, so we simply drop the warning.
#pragma warning(push)
#pragma warning(disable:4702)
IBackend* App::activeBackend(BackendType type) const
{
	for (auto& backend : m_impl->m_backends | std::views::filter([type](const auto& b) { return b.second->type() == type && b.second->state() == BackendState::Active; }))
		return backend.second.get();

	return nullptr;
}

std::type_index App::activeBackendType(BackendType type) const
{
	for (auto& backend : m_impl->m_backends | std::views::filter([type](const auto& b) { return b.second->type() == type && b.second->state() == BackendState::Active; }))
		return backend.first;
	
	return typeid(std::nullptr_t);
}
#pragma warning(pop)

void App::registerStartCallback(std::type_index type, const std::function<bool()>& callback)
{
	m_impl->m_startCallbacks.insert(std::make_pair(type, callback));
}

void App::registerStopCallback(std::type_index type, const std::function<void()>& callback)
{
	m_impl->m_stopCallbacks.insert(std::make_pair(type, callback));
}

Enumerable<const IBackend&> App::getBackends(const BackendType type) const
{
	return m_impl->m_backends |
		std::views::transform([](const auto& backend) -> const IBackend& { return *backend.second.get(); }) |
		std::views::filter([type](const auto& backend) { return backend.type() == type; });
}

void App::use(UniquePtr<IBackend>&& backend)
{
	auto type = backend->typeId();

	if (m_impl->m_backends.contains(type))
		throw InvalidArgumentException("backend", "Another backend of type {0} already has been registered. An application may only contain one backend of a certain type.", type.name());

	m_impl->m_backends[type] = std::move(backend);

	Logger::get(this->name()).debug("Registered backend type {0}.", type.name());
}

void App::run()
{
	// Initialize the app.
	Logger::get(this->name()).debug("Initializing app...");
	this->initializing(this, { });

	// Start the app.
	Logger::get(this->name()).info("Starting app (Version {1}) on platform {0}...", this->platform(), this->version());
	Logger::get(this->name()).debug("Using engine: {0:e}.", this->version());

	// Start the first registered rendering backend for each backend type.
	for (BackendType type : VALID_BACKEND_TYPES)
	{
		auto backends = this->getBackends(type);

		if (!backends.empty())
			this->startBackend(this->getBackends(type).begin()->typeId());
	}

	// Fire startup event.
	this->startup(this, { });

	// Shutdown the app.
	Logger::get(this->name()).debug("Shutting down app...", this->version());

	for (auto& backend : m_impl->m_backends | std::views::filter([](const auto& b) { return b.second->state() == BackendState::Active; }))
		this->stopBackend(backend.first);

	this->shutdown(this, { });
}

void App::resize(int width, int height)
{
	// Ensure the area is at least 1 pixel into each direction.
	width = std::max(width, 1);
	height = std::max(height, 1);

	// Publish event.
	Logger::get(this->name()).trace("OnResize (width = {0}, height = {1}).", width, height);
	this->resized(this, { width, height });
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void AppBuilder::use(UniquePtr<IBackend>&& backend)
{
	this->instance()->use(std::move(backend));
}