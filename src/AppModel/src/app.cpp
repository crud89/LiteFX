#include <litefx/app.hpp>

using namespace LiteFX;

class App::AppImpl {
private:
	Dictionary<BackendType, UniquePtr<IBackend>> m_backends;
	Platform m_platform;

public:
	AppImpl(const Platform& platform) noexcept : m_platform(platform) { }

public:
	const Platform& getPlatform() const noexcept
	{
		return m_platform;
	}

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

App::App(const Platform& platform) noexcept :
	m_impl(makePimpl<AppImpl>(platform))
{
}

App::~App() noexcept = default;

const Platform& App::getPlatform() const noexcept
{
	return m_impl->getPlatform();
}

const IBackend* App::findBackend(const BackendType& type) const
{
	return m_impl->findBackend(type);
}

const IBackend* App::operator[](const BackendType& type) const
{
	return this->findBackend(type);
}

void App::useBackend(UniquePtr<IBackend>&& backend)
{
	return m_impl->useBackend(std::move(backend));
}