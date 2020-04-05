#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanSurface::VulkanSurfaceImpl {
private:
	VkInstance m_instance;

public:
	VulkanSurfaceImpl(const VkInstance& instance) noexcept :
		m_instance(instance) { }

public:

public:
	const VkInstance& getInstance() const
	{
		return m_instance;
	}
};

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent) noexcept :
	IResource(surface), m_impl(makePimpl<VulkanSurfaceImpl>(parent))
{
}

VulkanSurface::~VulkanSurface() noexcept
{
	::vkDestroySurfaceKHR(m_impl->getInstance(), this->handle(), nullptr);
}