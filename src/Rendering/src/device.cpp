#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class GraphicsDevice::GraphicsDeviceImpl {
private:
	const IGraphicsAdapter* m_adapter;
	const ISurface* m_surface;

public:
	GraphicsDeviceImpl(const IGraphicsAdapter* adapter, const ISurface* surface) noexcept : 
		m_adapter(adapter), m_surface(surface) { }

public:
	const IGraphicsAdapter* getAdapter() const 
	{
		return m_adapter;
	}

	const ISurface* getSurface() const
	{
		return m_surface;
	}
};

GraphicsDevice::GraphicsDevice(const IGraphicsAdapter* adapter, const ISurface* surface) :
	m_impl(makePimpl<GraphicsDeviceImpl>(adapter, surface))
{
	if (adapter == nullptr)
		throw std::invalid_argument("The argument `adapter` must be initialized.");

	if (surface == nullptr)
		throw std::invalid_argument("The argument `surface` must be initialized.");
}

GraphicsDevice::~GraphicsDevice() noexcept = default;

const IGraphicsAdapter* GraphicsDevice::getAdapter() const
{
	return m_impl->getAdapter();
}

const ISurface* GraphicsDevice::getSurface() const
{
	return m_impl->getSurface();
}