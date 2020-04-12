#include <litefx/rendering_backends.hpp>

using namespace LiteFX::Rendering;

class RenderBackend::RenderBackendImpl {
private:
	const App& m_app;

public:
	RenderBackendImpl(const App& app) noexcept : m_app(app) { }

public:
	const App& getApp() const noexcept 
	{
		return m_app;
	}
};

RenderBackend::RenderBackend(const App& app) noexcept :
	m_impl(makePimpl<RenderBackendImpl>(app))
{
}

RenderBackend::~RenderBackend() noexcept = default;

BackendType RenderBackend::getType() const noexcept
{
	return BackendType::Rendering;
}

const App& RenderBackend::getApp() const noexcept
{
	return m_impl->getApp();
}