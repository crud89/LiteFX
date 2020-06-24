#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl : public Implement<VulkanCommandBuffer> {
public:
	friend class VulkanCommandBuffer;

private:
	const VulkanDevice* m_device;

public:
	VulkanCommandBufferImpl(VulkanCommandBuffer* parent, const VulkanDevice* device) :
		base(parent), m_device(device) { }

public:
	VkCommandBuffer initialize(const VulkanCommandBuffer& parent)
	{
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandPool = m_device->getCommandPool();
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;

		if (::vkAllocateCommandBuffers(m_device->handle(), &bufferInfo, &buffer) != VK_SUCCESS)
		    throw std::runtime_error("Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanDevice* device) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this, device)), IResource(nullptr)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");

	this->handle() = m_impl->initialize(*this);
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept = default;