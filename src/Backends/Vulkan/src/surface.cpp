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
#ifdef VK_USE_PLATFORM_WIN32_KHR
	HWND m_hwnd;
#endif

public:
	VulkanSurfaceImpl(VulkanSurface* parent, const VkInstance& instance) :
		base(parent), m_instance(instance) 
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

#ifdef VK_USE_PLATFORM_WIN32_KHR

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent, const HWND hwnd) :
	Resource<VkSurfaceKHR>(surface), m_impl(makePimpl<VulkanSurfaceImpl>(this, parent))
{
	m_impl->m_hwnd = hwnd;
}

#else

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent) :
	Resource<VkSurfaceKHR>(surface), m_impl(makePimpl<VulkanSurfaceImpl>(this, parent))
{
}

#endif // VK_USE_PLATFORM_WIN32_KHR

VulkanSurface::~VulkanSurface() noexcept
{
	::vkDestroySurfaceKHR(m_impl->m_instance, this->handle(), nullptr);
}

const VkInstance& VulkanSurface::instance() const noexcept
{
	return m_impl->m_instance;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

const HWND VulkanSurface::windowHandle() const noexcept
{
	return m_impl->m_hwnd;
}

#endif // VK_USE_PLATFORM_WIN32_KHR