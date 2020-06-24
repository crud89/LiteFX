#include <litefx/app.hpp>

using namespace LiteFX;

class App::AppImpl : public Implement<App> {
public:
	friend class App;

private:
	Dictionary<BackendType, UniquePtr<IBackend>> m_backends;

public:
	AppImpl(App* parent) : base(parent) { }

public:
	const IBackend* findBackend(const BackendType& type)
	{
		return m_backends.find(type) == m_backends.end() ? nullptr : m_backends[type].get();
	}

public:
	void useBackend(UniquePtr<IBackend>&& backend)
	{
		auto type = backend->getType();
		m_backends[type] = std::move(backend);
	}
};

App::App() : m_impl(makePimpl<AppImpl>(this)) { }

App::~App() noexcept = default;

Platform App::getPlatform() const noexcept
{
#if defined(_WIN32) || defined(WINCE)
	return Platform::Win32;
#else
	return Platform::Other;
#endif
}

const IBackend* App::findBackend(const BackendType& type) const
{
	return m_impl->findBackend(type);
}

const IBackend* App::operator[](const BackendType& type) const
{
	return this->findBackend(type);
}

void App::use(UniquePtr<IBackend>&& backend)
{
	Logger::get(this->getName()).trace("Using backend {0} (current backend: {1})...", fmt::ptr(backend.get()), fmt::ptr(m_impl->findBackend(backend->getType())));
	return m_impl->useBackend(std::move(backend));
}

void App::resize(int width, int height)
{
	Logger::get(this->getName()).trace("OnResize (width = {0}, height = {1}).", width, height);
}