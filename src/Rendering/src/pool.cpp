#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

CommandPool::CommandPool(const Handle handle) :
	m_handle(handle)
{
	if (handle == nullptr)
		throw std::invalid_argument("The parameter `handle` must be initialized.");
}

const Handle CommandPool::getHandle() const
{
	return m_handle;
}