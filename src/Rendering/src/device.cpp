#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

GraphicsDevice::GraphicsDevice(const Handle handle) :
	m_handle(handle)
{
	if (handle == nullptr)
		throw std::invalid_argument("The parameter `handle` must be initialized.");
}

const Handle GraphicsDevice::getHandle() const
{
	return m_handle;
}