#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRuntimeObject::VulkanRuntimeObjectImpl : public Implement<VulkanRuntimeObject> {
public:
	friend class VulkanRuntimeObject;

private:
	const VulkanDevice* m_device;

public:
	VulkanRuntimeObjectImpl(VulkanRuntimeObject* parent, const VulkanDevice* device) : base(parent), m_device(device)
	{
		if (device == nullptr)
			throw std::invalid_argument("The device must be initialized.");
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRuntimeObject::VulkanRuntimeObject(const VulkanDevice* device) :
	m_impl(makePimpl<VulkanRuntimeObjectImpl>(this, device))
{
}

VulkanRuntimeObject::~VulkanRuntimeObject() noexcept = default;

const VulkanDevice* VulkanRuntimeObject::getDevice() const noexcept
{
	return m_impl->m_device;
}