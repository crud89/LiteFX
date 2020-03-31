#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

RenderDevice::RenderDevice(const Handle handle) :
	m_handle(handle)
{
	if (handle == nullptr)
		throw std::invalid_argument("The `handle` parameter must be initialized.");
}

const RenderDevice::Handle RenderDevice::getHandle() const
{
	return m_handle;
}