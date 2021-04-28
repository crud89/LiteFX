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

public:
	GraphicsDeviceImpl(GraphicsDevice* parent, const IRenderBackend* backend) noexcept :
		base(parent), m_backend(backend) { }
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