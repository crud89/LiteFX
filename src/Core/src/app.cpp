#include <litefx/core_types.hpp>

using namespace LiteFX;

class App::AppImpl {
private:
	Platform m_platform;

public:
	AppImpl(const Platform& platform) noexcept : m_platform(platform) { }

public:
	Platform getPlatform() const noexcept
	{
		return m_platform;
	}
};

App::App(const Platform& platform) noexcept :
	m_impl(makePimpl<AppImpl>(platform))
{
}

App::~App() noexcept = default;

int App::start(const int argc, const char** argv)
{
	return this->start(Array<String>(argv, argv + argc));
}

Platform App::getPlatform() const noexcept
{
	return m_impl->getPlatform();
}