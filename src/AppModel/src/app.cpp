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
	Dictionary<std::type_index, std::function<bool()>> m_initializers;

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

void App::use(UniquePtr<IBackend>&& backend)
{
	auto type = backend->typeId();

	if (m_impl->m_backends.contains(type))
		throw InvalidArgumentException("Another backend of type {0} already has been registered. An application may only contain one backend of a certain type.", type.name());

	m_impl->m_backends[type] = std::move(backend);

	Logger::get(this->name()).debug("Registered backend type {0}.", type.name());
}

const std::function<bool()>& App::getInitializer(std::type_index type) const
{
	if (!m_impl->m_initializers.contains(type))
		throw InvalidArgumentException("No initializer has been registered for type {0}.", type.name());

	return m_impl->m_initializers[type];
}

void App::setInitializer(std::type_index type, const std::function<bool()>& initializer)
{
	m_impl->m_initializers[type] = initializer;
	Logger::get(this->name()).debug("Registered backend initializer for type {0}.", type.name());
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