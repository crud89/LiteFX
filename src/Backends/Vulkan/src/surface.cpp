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
		base(parent), m_instance(instance) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSurface::VulkanSurface(const VkSurfaceKHR& surface, const VkInstance& parent) :
	IResource(surface), m_impl(makePimpl<VulkanSurfaceImpl>(this, parent))
{
}

VulkanSurface::~VulkanSurface() noexcept
{
	::vkDestroySurfaceKHR(m_impl->m_instance, this->handle(), nullptr);
}

UniquePtr<ISurface> VulkanSurface::createSurface(const VulkanBackend& backend, surface_callback predicate)
{
    auto instance = backend.handle();

    if (instance == nullptr)
        throw std::runtime_error("The backend is not initialized.");

    auto surface = predicate(instance);
    return std::move(UniquePtr<ISurface>(new VulkanSurface(surface, instance)));
}

// ------------------------------------------------------------------------------------------------
// Platform-specific implementation.
// ------------------------------------------------------------------------------------------------

#ifdef VK_USE_PLATFORM_WIN32_KHR

UniquePtr<ISurface> VulkanSurface::createSurface(const VkInstance& instance, const HWND& hwnd)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = ::GetModuleHandle(nullptr);

    VkSurfaceKHR surface;

    if (::vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Unable to create vulkan surface for provided window.");

    return UniquePtr<ISurface>(new VulkanSurface(surface, instance));
}

#endif