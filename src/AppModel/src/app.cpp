#include <litefx/app.hpp>

using namespace LiteFX;

class App::AppImpl {
private:
	Dictionary<BackendType, UniquePtr<IBackend>> m_backends;

public:
	AppImpl() = default;

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

App::App() : m_impl(makePimpl<AppImpl>()) { }

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
	return m_impl->useBackend(std::move(backend));
}