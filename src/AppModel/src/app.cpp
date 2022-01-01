#include <litefx/app.hpp>

using namespace LiteFX;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class App::AppImpl : public Implement<App> {
public:
	friend class App;

private:
	Dictionary<StringView, UniquePtr<IBackend>> m_backends;

public:
	AppImpl(App* parent) : 
		base(parent) 
	{
	}

public:
	const IBackend* findBackend(StringView name)
	{
		return m_backends.contains(name) ? m_backends[name].get() : nullptr;
	}

	void useBackend(UniquePtr<IBackend>&& backend)
	{
		//auto name = hash(backend->name());

		//if (m_backends.contains(hash))


		m_backends[backend->name()] = std::move(backend);
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

const IBackend* App::operator[](StringView name) const
{
	return m_impl->findBackend(name);
}

void App::use(UniquePtr<IBackend>&& backend)
{
	Logger::get(this->name()).trace("Using backend {0} (current backend: {1})...", fmt::ptr(backend.get()), fmt::ptr(m_impl->findBackend(backend->name())));
	return m_impl->useBackend(std::move(backend));
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

	this->instance()->run();
	return builder_type::go();
}