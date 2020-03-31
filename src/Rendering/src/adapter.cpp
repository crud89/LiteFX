#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

GraphicsAdapter::GraphicsAdapter(const Handle handle) :
	m_handle(handle)
{
	if (handle == nullptr)
		throw std::invalid_argument("The `handle` parameter must be initialized.");
}

const GraphicsAdapter::Handle GraphicsAdapter::getHandle() const
{
	return m_handle;
}