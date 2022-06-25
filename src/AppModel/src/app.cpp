#include <litefx/app.hpp>

using namespace LiteFX;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class App::AppImpl : public Implement<App> {
public:
	friend class App;

private:
	Dictionary<std::type_index, UniquePtr<IBackend>> m_backends;
	std::multimap<std::type_index, const std::function<bool()>&> m_startCallbacks;
	std::multimap<std::type_index, const std::function<void()>&> m_stopCallbacks;

public:
	AppImpl(App* parent) : 
		base(parent) 
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

App::App() : 
	m_impl(makePimpl<AppImpl>(this)) 
{
}

App::~App() noexcept = default;

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
	return m_impl->m_backends.contains(type) ? m_impl->m_backends[type].get() : nullptr;
}

IBackend* App::getBackend(std::type_index type)
{
	return m_impl->m_backends.contains(type) ? m_impl->m_backends[type].get() : nullptr;
}

void App::startBackend(std::type_index type, IBackend* backend) const
{
	if (backend == nullptr)
		throw InvalidArgumentException("No backend of type {0} has been registered.", type.name());

	if (backend->state() == BackendState::Active)
		return;

	// Stop all active backends.
	this->stopActiveBackends(backend->type());

	// Call the start callbacks for the backend.
	const auto callbacks = m_impl->m_startCallbacks.equal_range(type);

	for (auto it = callbacks.first; it != callbacks.second; ++it)
		if (!it->second())
			throw new RuntimeException("Unable to start backend {0}.", type.name());

	// Set the backend to active.
	backend->activate();
}

void App::stopBackend(std::type_index type, IBackend* backend) const
{
	if (backend == nullptr)
		throw InvalidArgumentException("No backend of type {0} has been registered.", type.name());

	if (backend->state() != BackendState::Inactive)
	{
		// Call the stop callbacks for the backend.
		const auto callbacks = m_impl->m_stopCallbacks.equal_range(type);

		for (auto it = callbacks.first; it != callbacks.second; ++it)
			it->second();

		// Set the backend state to inactive.
		static_cast<IBackend*>(backend)->deactivate();
	}
}

void App::stopActiveBackends(const BackendType& type) const
{
	for (auto& backend : m_impl->m_backends | std::views::filter([type](const auto& b) { return b.second->type() == type && b.second->state() == BackendState::Active; }))
		this->stopBackend(backend.first, backend.second.get());
}

void App::registerStartCallback(std::type_index type, const std::function<bool()>& callback)
{
	m_impl->m_startCallbacks.insert(std::make_pair(type, callback));
}

void App::registerStopCallback(std::type_index type, const std::function<void()>& callback)
{
	m_impl->m_stopCallbacks.insert(std::make_pair(type, callback));
}

Array<const IBackend*> App::getBackends(const BackendType type) const noexcept
{
	return m_impl->m_backends |
		std::views::transform([](const auto& backend) { return backend.second.get(); }) |
		std::views::filter([type](const auto backend) { return backend->type() == type; }) |
		ranges::to<Array<const IBackend*>>();
}

void App::use(UniquePtr<IBackend>&& backend)
{
	auto type = backend->typeId();

	if (m_impl->m_backends.contains(type))
		throw InvalidArgumentException("Another backend of type {0} already has been registered. An application may only contain one backend of a certain type.", type.name());

	m_impl->m_backends[type] = std::move(backend);

	Logger::get(this->name()).debug("Registered backend type {0}.", type.name());
}

void App::resize(int width, int height)
{
	Logger::get(this->name()).trace("OnResize (width = {0}, height = {1}).", width, height);
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void AppBuilder::use(UniquePtr<IBackend>&& backend)
{
	this->instance()->use(std::move(backend));
}

UniquePtr<App> AppBuilder::go()
{
	Logger::get(this->instance()->name()).info("Starting app (Version {1}) on platform {0}...", this->instance()->platform(), this->instance()->version());
	Logger::get(this->instance()->name()).debug("Using engine: {0:e}.", this->instance()->version());

	this->instance()->initialize();
	return builder_type::go();
}