#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanSwapChain::VulkanSwapChainImpl {
private:
	const VulkanDevice* m_device;

public:
	VulkanSwapChainImpl(const VulkanDevice* device) noexcept :
		m_device(device) { }

public:
	void initialize(const VulkanSwapChain& parent)
	{
		uint32_t images;
		::vkGetSwapchainImagesKHR(m_device->handle(), parent.handle(), &images, nullptr);

		Array<VkImage> imageChain(images);
		::vkGetSwapchainImagesKHR(m_device->handle(), parent.handle(), &images, imageChain.data());
		
		throw;
	}

public:
	const VulkanDevice* getDevice() const noexcept
	{
		return m_device;
	}
};

VulkanSwapChain::VulkanSwapChain(const VkSwapchainKHR& swapChain, const VulkanDevice* device) :
	IResource(swapChain), m_impl(makePimpl<VulkanSwapChainImpl>(device))
{
	if (swapChain == nullptr)
		throw std::invalid_argument("The argument `swapChain` must be initialized.");

	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");

	m_impl->initialize(*this);
}

VulkanSwapChain::~VulkanSwapChain() noexcept = default;

const IGraphicsDevice* VulkanSwapChain::getDevice() const noexcept
{
	return m_impl->getDevice();
}