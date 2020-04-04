#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanSurface::VulkanSurfaceImpl {
private:
	VkSurfaceKHR m_surface;
	VkInstance m_instance;

public:
	VulkanSurfaceImpl(const VkSurfaceKHR& surface, const VkInstance& parent) noexcept :
		m_surface(surface), m_instance(parent) { 
	}
	~VulkanSurfaceImpl() noexcept { ::vkDestroySurfaceKHR(m_instance, m_surface, nullptr); }
};

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent) noexcept :
	IResource(surface), m_impl(makePimpl<VulkanSurfaceImpl>(surface, parent))
{
}

VulkanSurface::~VulkanSurface() noexcept = default;