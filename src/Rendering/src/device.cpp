#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class GraphicsDevice::GraphicsDeviceImpl {
public:
	friend class GraphicsDevice;

private:
	const IGraphicsAdapter* m_adapter;
	const ISurface* m_surface;
	const ICommandQueue* m_queue;

public:
	GraphicsDeviceImpl(const IGraphicsAdapter* adapter, const ISurface* surface) noexcept :
		m_adapter(adapter), m_surface(surface), m_queue(nullptr) { }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

GraphicsDevice::GraphicsDevice(const IGraphicsAdapter* adapter, const ISurface* surface) :
	m_impl(makePimpl<GraphicsDeviceImpl>(adapter, surface))
{
	if (adapter == nullptr)
		throw std::invalid_argument("The argument `adapter` must be initialized.");

	if (surface == nullptr)
		throw std::invalid_argument("The argument `surface` must be initialized.");
}

GraphicsDevice::~GraphicsDevice() noexcept = default;

const IGraphicsAdapter* GraphicsDevice::getAdapter() const noexcept
{
	return m_impl->m_adapter;
}

const ISurface* GraphicsDevice::getSurface() const noexcept
{
	return m_impl->m_surface;
}

const ICommandQueue* GraphicsDevice::getQueue() const noexcept
{
	return m_impl->m_queue;
}

void GraphicsDevice::setQueue(ICommandQueue* queue) noexcept
{
	m_impl->m_queue = queue;
}