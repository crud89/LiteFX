#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanSwapChain::VulkanSwapChainImpl {
private:
	const VulkanDevice* m_device;

public:
	VulkanSwapChainImpl(const VulkanDevice* device) noexcept :
		m_device(device) { }

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
}

VulkanSwapChain::~VulkanSwapChain() noexcept = default;

const IGraphicsDevice* VulkanSwapChain::getDevice() const noexcept
{
	return m_impl->getDevice();
}