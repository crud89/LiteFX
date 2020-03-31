#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

RenderBackend::RenderBackend(const App& app) :
	m_app(app)
{
}

const App& RenderBackend::getApp() const
{
	return m_app;
}