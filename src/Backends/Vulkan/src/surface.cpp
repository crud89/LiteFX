#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSurface::VulkanSurfaceImpl : public Implement<VulkanSurface> {
public:
    friend class VulkanSurface;

private:
	VkInstance m_instance;

public:
	VulkanSurfaceImpl(VulkanSurface* parent, const VkInstance& instance) :
		base(parent), m_instance(instance) 
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent) :
	Resource<VkSurfaceKHR>(surface), m_impl(makePimpl<VulkanSurfaceImpl>(this, parent))
{
}

VulkanSurface::~VulkanSurface() noexcept
{
	::vkDestroySurfaceKHR(m_impl->m_instance, this->handle(), nullptr);
}

const VkInstance& VulkanSurface::instance() const noexcept
{
	return m_impl->m_instance;
}