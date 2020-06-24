#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class RenderBackend::RenderBackendImpl : public Implement<RenderBackend> {
public:
	friend class RenderBackend;

private:
	const App& m_app;

public:
	RenderBackendImpl(RenderBackend* parent, const App& app) : base(parent), m_app(app) { }
};

RenderBackend::RenderBackend(const App& app) :
	m_impl(makePimpl<RenderBackendImpl>(this, app))
{
}

RenderBackend::~RenderBackend() noexcept = default;

BackendType RenderBackend::getType() const noexcept
{
	return BackendType::Rendering;
}

const App& RenderBackend::getApp() const noexcept
{
	return m_impl->m_app;
}