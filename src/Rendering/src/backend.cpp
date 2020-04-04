#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class RenderBackend::RenderBackendImpl {
private:
	const App& m_app;

public:
	RenderBackendImpl(const App& app) noexcept :
		m_app(app) { }
	RenderBackendImpl::~RenderBackendImpl() noexcept { }

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

const App& RenderBackend::getApp() const noexcept
{
	return m_impl->getApp();
}