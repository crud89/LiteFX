#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanQueue::VulkanQueueImpl {
private:
	QueueType m_type;

public:
	VulkanQueueImpl(const QueueType& type) noexcept : 
		m_type(type) { }
	~VulkanQueueImpl() noexcept = default;

public:
	const QueueType& getType() const noexcept
	{
		return m_type;
	}
};

VulkanQueue::VulkanQueue(const QueueType& type) noexcept :
	IResource(nullptr), m_impl(makePimpl<VulkanQueueImpl>(type))
{
}

VulkanQueue::~VulkanQueue() noexcept = default;

const QueueType& VulkanQueue::getType() const noexcept
{
	return m_impl->getType();
}