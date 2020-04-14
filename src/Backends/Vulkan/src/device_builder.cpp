#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanDeviceBuilder::VulkanDeviceBuilderImpl {
private:
	QueueType m_queueType;
	Format m_format;

public:
	VulkanDeviceBuilderImpl() : m_queueType(QueueType::Graphics), m_format(Format::B8G8R8A8_UNORM_SRGB) { }

public:
	void setQueue(const QueueType& queueType)
	{
		m_queueType = queueType;
	}

	const QueueType& getQueue()
	{
		return m_queueType;
	}

	void setFormat(const Format& format)
	{
		m_format = format;
	}

	const Format& getFormat()
	{
		return m_format;
	}
};

VulkanDeviceBuilder::VulkanDeviceBuilder(UniquePtr<VulkanDevice>&& instance) noexcept :
	GraphicsDeviceBuilder(std::move(instance)), m_impl(makePimpl<VulkanDeviceBuilderImpl>())
{
}

VulkanDeviceBuilder::~VulkanDeviceBuilder() noexcept = default;

UniquePtr<VulkanDevice> VulkanDeviceBuilder::go()
{
	auto surface = this->instance()->getSurface();
	auto queue = this->instance()->getAdapter()->findQueue(m_impl->getQueue(), surface);

	if (queue == nullptr)
	    throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");

	this->instance()->create(m_impl->getFormat(), queue);
	
	return GraphicsDeviceBuilder::go();
}

VulkanDeviceBuilder& VulkanDeviceBuilder::withFormat(const Format& format)
{
	m_impl->setFormat(format);
	return *this;
}

VulkanDeviceBuilder& VulkanDeviceBuilder::withQueue(const QueueType& queueType)
{
	m_impl->setQueue(queueType);
	return *this;
}