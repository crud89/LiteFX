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

public:
	AppImpl(App* parent) : 
		base(parent) 
	{
	}

public:
	const IBackend* findBackend(std::type_index type)
	{
		return m_backends.contains(type) ? m_backends[type].get() : nullptr;
	}

	void useBackend(UniquePtr<IBackend>&& backend, std::type_index type)
	{
		if (m_backends.contains(type))
			throw InvalidArgumentException("Another backend of type {0} already has been registered. An application may only contain one backend of a certain type.", type.name());

		m_backends[type] = std::move(backend);
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
	return m_impl->findBackend(type);
}

void App::use(UniquePtr<IBackend>&& backend)
{
	Logger::get(this->name()).debug("Registered backend type {0}.", backend->typeId().name());
	m_impl->useBackend(std::move(backend), backend->typeId());
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