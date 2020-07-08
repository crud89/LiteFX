#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class GraphicsDevice::GraphicsDeviceImpl : public Implement<GraphicsDevice> {
public:
	friend class GraphicsDevice;

private:
	const IRenderBackend* m_backend;
	ICommandQueue* m_graphicsQueue;
	ICommandQueue* m_transferQueue;

public:
	GraphicsDeviceImpl(GraphicsDevice* parent, const IRenderBackend* backend) noexcept :
		base(parent), m_backend(backend), m_graphicsQueue(nullptr), m_transferQueue(nullptr) { }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

GraphicsDevice::GraphicsDevice(const IRenderBackend* backend) :
	m_impl(makePimpl<GraphicsDeviceImpl>(this, backend))
{
	if (backend == nullptr)
		throw std::invalid_argument("The argument `backend` must be initialized.");

	if (backend->getAdapter() == nullptr)
		throw std::invalid_argument("The backend must use an adapter to create a device.");

	if (backend->getSurface() == nullptr)
		throw std::invalid_argument("The backend muse use a surface to create a device.");
}

GraphicsDevice::~GraphicsDevice() noexcept = default;

const IRenderBackend* GraphicsDevice::getBackend() const noexcept
{
	return m_impl->m_backend;
}

const ICommandQueue* GraphicsDevice::getGraphicsQueue() const noexcept
{
	return m_impl->m_graphicsQueue;
}

ICommandQueue* GraphicsDevice::getGraphicsQueue() noexcept
{
	return m_impl->m_graphicsQueue;
}

const ICommandQueue* GraphicsDevice::getTransferQueue() const noexcept
{
	return m_impl->m_transferQueue;
}

ICommandQueue* GraphicsDevice::getTransferQueue() noexcept
{
	return m_impl->m_transferQueue;
}

void GraphicsDevice::setGraphicsQueue(ICommandQueue* queue) noexcept
{
	m_impl->m_graphicsQueue = queue;
}

void GraphicsDevice::setTransferQueue(ICommandQueue* queue) noexcept
{
	m_impl->m_transferQueue = queue;
}